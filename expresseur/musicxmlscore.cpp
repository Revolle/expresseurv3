/////////////////////////////////////////////////////////////////////////////
// Name:        muscixmlscore.cpp
// Purpose:     display a musicxml of the score 
// Author:      Franck REVOLLE
// Modified by:
// Created:     27/07/2015
// Copyright:   (c) Franck REVOLLE Expresseur
// Licence:    Expresseur licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>


#include "wx/dialog.h"
#include "wx/filename.h"
#include "wx/sizer.h"
#include "wx/textfile.h"
#include "wx/valgen.h"
#include "wx/tokenzr.h"
#include "wx/config.h"
#include "wx/dcclient.h"
#include "wx/msgdlg.h"
#include "wx/image.h"
#include "wx/dcbuffer.h"
#include "wx/xml/xml.h"
#include "wx/filefn.h"
#include "wx/wfstream.h"
#include "wx/zipstrm.h"
#include "wx/dynarray.h"
#include "wx/utils.h"
#include "wx/arrstr.h"
#include "wx/dir.h"
#include "wx/datetime.h"
#include "wx/time.h"
#include "wx/longlong.h"
#include "wx/datetime.h"

#include "global.h"
#include "basslua.h"
#include "mxconf.h"
#include "viewerscore.h"
#include "expression.h"
#include "luafile.h"

#include "musicxml.h"
#include "musicxmlcompile.h"
#include "musicxmlscore.h"

// Specify if the size of the MuseScore page is in the XML or in the muse-script, depending the platform
#ifdef RUN_WIN
#define MUSE_SCORE_DEF_XML true
#define DEF_INCH 25400
#endif
#ifdef RUN_MAC
#define MUSE_SCORE_DEF_XML false
#define DEF_INCH 25600
#endif
#ifdef RUN_LINUX
#define MUSE_SCORE_DEF_XML false
#define DEF_INCH 25578
#endif

#define PREFIX_CACHE "CACHE_EXPRESSEUR"
#define WIDTH_SEPARATOR_PAGE 10

// CRC tool to optimize calcualtion of MuseScore pages

// poly is: x^64 + x^62 + x^57 + x^55 + x^54 + x^53 + x^52 + x^47 + x^46 + x^45 + x^40 + x^39 + 
//          x^38 + x^37 + x^35 + x^33 + x^32 + x^31 + x^29 + x^27 + x^24 + x^23 + x^22 + x^21 + 
//          x^19 + x^17 + x^13 + x^12 + x^10 + x^9  + x^7  + x^4  + x^1  + 1
//
// represented here with lsb = highest degree term
//
// 1100100101101100010101111001010111010111100001110000111101000010_
// ||  |  | || ||   | | ||||  | | ||| | ||||    |||    |||| |    | |
// ||  |  | || ||   | | ||||  | | ||| | ||||    |||    |||| |    | +- x^64 (implied)
// ||  |  | || ||   | | ||||  | | ||| | ||||    |||    |||| |    |
// ||  |  | || ||   | | ||||  | | ||| | ||||    |||    |||| |    +--- x^62
// ||  |  | || ||   | | ||||  | | ||| | ||||    |||    |||| +-------- x^57
// .......................................................................
// ||
// |+---------------------------------------------------------------- x^1
// +----------------------------------------------------------------- x^0 (1)
wxULongLong crc_poly = 0xC96C5795D7870F42;

// input is dividend: as 0000000000000000000000000000000000000000000000000000000000000000<8-bit byte>
// where the lsb of the 8-bit byte is the coefficient of the highest degree term (x^71) of the dividend
// so division is really for input byte * x^64

// you may wonder how 72 bits will fit in 64-bit data type... well as the shift-right occurs, 0's are supplied
// on the left (most significant) side ... when the 8 shifts are done, the right side (where the input
// byte was placed) is discarded

// when done, table[XX] (where XX is a byte) is equal to the CRC of 00 00 00 00 00 00 00 00 XX
//
wxULongLong crc_table[256];

void crc_generate_table()
{
    for(unsigned int i=0; i<256; ++i)
    {
    	wxULongLong crc = i;

    	for(unsigned int j=0; j<8; ++j)
    	{
            // is current coefficient set?
			wxULongLong alreadySet = crc & 1;
    		if(alreadySet != 0)
            {
                // yes, then assume it gets zero'd (by implied x^64 coefficient of dividend)
                crc >>= 1;
    
                // and add rest of the divisor
    			crc ^= crc_poly;
            }
    		else
    		{
    			// no? then move to next coefficient
    			crc >>= 1;
            }
    	}
        crc_table[i] = crc;
    }
}

// will give an example CRC calculation for input array {0xDE, 0xAD}
//
// each byte represents a group of 8 coefficients for 8 dividend terms
//
// the actual polynomial dividend is:
//
// = DE       AD       00 00 00 00 00 00 00 00 (hex)
// = 11011110 10101101 0000000000000000000...0 (binary)
//   || ||||  | | || |
//   || ||||  | | || +------------------------ x^71
//   || ||||  | | |+-------------------------- x^69
//   || ||||  | | +--------------------------- x^68
//   || ||||  | +----------------------------- x^66
//   || ||||  +------------------------------- x^64
//   || ||||  
//   || |||+---------------------------------- x^78
//   || ||+----------------------------------- x^77
//   || |+------------------------------------ x^76
//   || +------------------------------------- x^75
//   |+--------------------------------------- x^73
//   +---------------------------------------- x^72
//

// the basic idea behind how the table lookup results can be used with one
// another is that:
//
// Mod(A * x^n, P(x)) = Mod(x^n * Mod(A, P(X)), P(X))
//
// in other words, an input data shifted towards the higher degree terms
// changes the pre-computed crc of the input data by shifting it also
// the same amount towards higher degree terms (mod the polynomial)

// here is an example:
//
// 1) input:
//
//    00 00 00 00 00 00 00 00 AD DE
//          
// 2) index crc table for byte DE (really for dividend 00 00 00 00 00 00 00 00 DE)
//
//    we get A8B4AFBDC5A6ACA4
//
// 3) apply that to the input stream:
//
//    00 00 00 00 00 00 00 00 AD DE 
//       A8 B4 AF BD C5 A6 AC A4
//    -----------------------------
//    00 A8 B4 AF BD C5 A6 AC 09
//
// 4) index crc table for byte 09 (really for dividend 00 00 00 00 00 00 00 00 09)
// 
//    we get 448FCBB7FCB9E309
//
// 5) apply that to the input stream
//
//    00 A8 B4 AF BD C5 A6 AC 09
//    44 8F CB B7 FC B9 E3 09
//    --------------------------
//    44 27 7F 18 41 7C 45 A5
//
//
wxULongLong crc_value = 0 ;
void crc_cumulate(char *stream, unsigned int n)
{
	// cumulate CRC of stream (lenght=n)
    for(unsigned int i=0; i<n; ++i)
    {
    	wxULongLong mc = wxULongLong(stream[i]);
        wxULongLong index = mc ^ crc_value;
		unsigned long ii = index.GetLo() % 256;
        wxULongLong lookup = crc_table[ii];

        crc_value >>= 8;
        crc_value ^= lookup;
    }
}

enum
{
	IDM_MUSICXML_PANEL = ID_MUSICXML 
};

wxBEGIN_EVENT_TABLE(musicxmlscore, wxPanel)
EVT_LEFT_DOWN(musicxmlscore::OnLeftDown)
EVT_PAINT(musicxmlscore::onPaint)
wxEND_EVENT_TABLE()

musicxmlscore::musicxmlscore(wxWindow *parent, wxWindowID id, mxconf* lconf )
: viewerscore(parent, id)
{
	crc_generate_table();


	mParent = parent;
	mConf = lconf;
	nrChord = -1;
	xmlName.Clear();
	
	basslua_call(moduleScore, functionScoreInitScore, "" );

	inch = (float)(DEF_INCH) * ((float)(mConf->get(CONFIG_CORRECTINCH, 1000))/1000.0) / 1000.0;
	musescore_def_xml = (bool)(MUSE_SCORE_DEF_XML) ;

	docOK = false;
	xmlCompile = NULL;
	currentDC = NULL ;

	// locate the musescore exe for the rendering of the musial score
	musescoreexe = mConf->get(CONFIG_MUSESCORE, "");
	if (musescoreexe.IsEmpty() == false)
	{
		wxFileName fm(musescoreexe);
		if (fm.IsFileExecutable() == false)
			musescoreexe.Empty();
	}
	if (musescoreexe.IsEmpty())
	{
#ifdef RUN_WIN
		wxString x86folder = wxGetenv("ProgramFiles(x86)");
		wxFileName fm(x86folder + "\\" );
		fm.AppendDir("MuseScore 2");
		fm.AppendDir("bin");
		fm.SetFullName("MuseScore.exe");
		if (fm.IsFileExecutable())
			musescoreexe = fm.GetFullPath();
#endif
#ifdef RUN_MAC
		wxFileName fm;
		fm.Assign(mxconf::getAppDir()) ;
		fm.AppendDir("MuseScore 2.app");
		fm.AppendDir("Contents");
		fm.AppendDir("MacOS");
		fm.SetName("mscore");
		//wxMessageBox(fm.GetFullPath(),"musescore ?");
		if (fm.IsFileExecutable())
			musescoreexe = fm.GetFullPath();
#endif
#ifdef RUN_LINUX
		wxString mpath = wxGetenv("PATH");
		wxArrayString mpaths = wxStringTokenize ( mpath, ":" );
		unsigned int nbpath = mpaths.GetCount();
		for(unsigned int nrpath = 0 ; nrpath < nbpath; nrpath ++ )
		{
			wxFileName fm(mpaths[nrpath] + "/" );
			fm.SetFullName("musescore");
			if (fm.IsFileExecutable())
			{
				musescoreexe = fm.GetFullPath();
				break ;
			}
		} 
#endif
	}
	if (musescoreexe.IsEmpty())
	{
		wxMessageBox("MuseScore cannot be found in your workstation.\nPlease select the path of MuseScore in your system.\nInfo: MuseScore is used for the graphical rendering of the score.");
		wxFileDialog
			openFileDialog(this, _("select MuseScore app"), "", "",
#ifdef RUN_WIN
				"exe files (*.exe)|*.exe", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
#endif
#ifdef RUN_MAC
		"app files (*.app)|*.app", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
#endif
#ifdef RUN_LINUX
		"app files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
#endif
		if (openFileDialog.ShowModal() == wxID_OK)
		{
			wxFileName fm(openFileDialog.GetPath());
#ifdef RUN_MAC
			fm.AppendDir(fm.GetFullName());
			fm.AppendDir("Contents");
			fm.AppendDir("MacOS");
			fm.SetName("mscore");
			fm.SetExt("");
#endif
			if (fm.IsFileExecutable())
			{
				wxMessageBox(fm.GetFullPath(),"MuseScore OK");
				musescoreexe = openFileDialog.GetPath();
			}
			else
				wxMessageBox(fm.GetFullPath(),"MuseScore : not recognized as an exe");
		}
	}
	mConf->set(CONFIG_MUSESCORE, musescoreexe);

	/*
	char buflog[512];
	strcpy(buflog, musescoreexe.c_str());
	mlog_in("musicxmlscore / creator / musescore.exe=%s", buflog);
	*/ 

	cleanTmp();
	zoom(mConf->get(CONFIG_ZOOM_MUSICXML, 0));
}
musicxmlscore::~musicxmlscore()
{
	basslua_call(moduleScore, functionScoreInitScore, "" );
	if (xmlCompile != NULL)
		delete xmlCompile;
	xmlCompile = NULL;
	if ( currentDC)
		delete currentDC;
	currentDC = NULL ;
	cleanTmp();
}
void musicxmlscore::cleanTmp()
{
	wxDir dir(mxconf::getTmpDir());
	if (dir.IsOpened())
	{
		wxString filename;
		wxFileName ft;
		ft.SetPath(mxconf::getTmpDir());
		bool cont = dir.GetFirst(&filename, "expresseur*.png", wxDIR_FILES);
		while (cont)
		{
			ft.SetFullName(filename);
			wxRemoveFile(ft.GetFullPath());
			cont = dir.GetNext(&filename);
		}
	}
}
void musicxmlscore::cleanCache(int nbDayCache)
{
		wxDir dir(mxconf::getTmpDir());
	if (dir.IsOpened())
	{
		wxDateTime mlimitdate = wxDateTime::Now();
		if ( nbDayCache > 0 )
			mlimitdate.Subtract(wxDateSpan(0, 0, 0, nbDayCache));
		wxString filename;
		wxFileName ft;
		ft.SetPath(mxconf::getTmpDir());
		wxString sn;
		sn.Printf("%s*.*", PREFIX_CACHE);
		bool cont = dir.GetFirst(&filename, sn , wxDIR_FILES);
		while (cont)
		{
			ft.SetFullName(filename);
			wxDateTime dateFile = ft.GetModificationTime();
			if ((nbDayCache == -1 ) || ( dateFile.IsEarlierThan(mlimitdate)))
			{
				wxRemoveFile(ft.GetFullPath());
			}
			cont = dir.GetNext(&filename);
		}
	}
}
bool musicxmlscore::isOk()
{
	if ((xmlCompile == NULL) || (musescoreexe.IsEmpty()))
		return false;
	return (xmlCompile->isOk());
}
bool musicxmlscore::setFile(const wxFileName &lfilename)
{
	wxBusyCursor wait;

	/*
	char buflog[512];
	strcpy(buflog, lfilename.GetFullPath().c_str());
	mlog_in("musicxmlscore / setFile / lfilename=%s", buflog );
	*/

	if (xmlCompile != NULL)
		delete xmlCompile;
	xmlCompile = new musicxmlcompile();

	wxFileName fm;
	fm.SetPath(mxconf::getTmpDir());
	fm.SetFullName("expresseur_in.xml");
	xmlCompile->music_xml_complete_file = fm.GetFullPath();

	if ((lfilename.GetExt() == SUFFIXE_MUSICXML) || (lfilename.GetExt() == SUFFIXE_MUSICMXL))
	{
		// extract the music_xml_complete_file from the xmlname file ( compressed with zipped or not )
		if (!xmlExtractXml(lfilename))
			return false;
		wxFileName txtfile(lfilename);
		txtfile.SetExt(SUFFIXE_TEXT);
		xmlCompile->setNameFile(txtfile, lfilename);
		// compile the musicxml-source-file, to create the music-xml-expresseur MUSICXML_FILE for display
		xmlCompile->loadXmlFile(xmlCompile->music_xml_complete_file, false);
	}
	if (lfilename.GetExt() == SUFFIXE_TEXT)
	{
		wxFileName xmlfile = xmlCompile->loadTxtFile(lfilename);
		if ((!xmlfile.IsOk()) || ((xmlfile.GetExt() != SUFFIXE_MUSICXML) && (xmlfile.GetExt() != SUFFIXE_MUSICMXL)))
			return false;
		xmlCompile->setNameFile(lfilename, xmlfile);
		// extract the music_xml_complete_file from the xmlname file ( compressed with zipped or not )
		if (!xmlExtractXml(xmlfile))
			return false;
		// compile the musicxml-source-file, to create the music-xml-expresseur MUSICXML_FILE for display
		xmlCompile->loadXmlFile(xmlCompile->music_xml_complete_file,  true);
	}
	return isOk();
}
bool musicxmlscore::xmlExtractXml(wxFileName f)
{
	// extract musicXML file, not conpressed , in the file MUSICXML_COMPLETE_FILE

	wxBusyCursor wait;
	if ((f.GetExt() == SUFFIXE_MUSICXML) && (f.IsFileReadable()))
	{
		// xml file not compressed. Copy it directky to the temporary full score MUSICXML_COMPLETE_FILE
		if (!wxCopyFile(f.GetFullPath(), xmlCompile->music_xml_complete_file))
		{
			wxString s;
			s.Printf("File %s cannot be copy in %s", f.GetFullPath(), xmlCompile->music_xml_complete_file);
			wxMessageBox(s);
			return false;
		}
		return true;
	}
	if ((f.GetExt() != SUFFIXE_MUSICMXL) || (!(f.IsFileReadable())))
		return false;
	// xml file compressed ( mxl ). unzip to the temporary full score MUSICXML_COMPLETE_FILE
	wxFFileInputStream in(f.GetFullPath());
	if (!in.IsOk())
	{
		wxString s;
		s.Printf("Error opening stream file %s", xmlName.GetFullName());
		wxMessageBox(s);
		return false;
	}

	wxZipInputStream zip(in);
	if (!zip.IsOk())
	{
		wxString s;
		s.Printf("Error reading zip structure of %s", xmlName.GetFullName());
		wxMessageBox(s);
		return false;
	}

	wxZipEntry *zipEntry;
	zipEntry = zip.GetNextEntry();
	while (zipEntry != NULL)
	{
		wxString name = zipEntry->GetName();
		delete zipEntry;
		wxFileName ffzip(name);
		if (ffzip.GetDirCount() == 0)
		{
			wxFileOutputStream  stream_out(xmlCompile->music_xml_complete_file);
			if (!stream_out.IsOk())
			{
				wxString s;
				s.Printf("Error reading zip entry %s of %s", name, xmlName.GetFullName());
				wxMessageBox(s);
				return false;
			}
			zip.Read(stream_out);
			if (zip.LastRead() < 10)
			{
				wxString s;
				s.Printf("Error content in zip entry %s of %s", name, xmlName.GetFullName());
				wxMessageBox(s);
				return false;
			}
			stream_out.Close();
			zip.CloseEntry();
			return true;
		}
		zipEntry = zip.GetNextEntry();
	}
	return false;
}
void musicxmlscore::zoom(int zoom)
{
	switch (zoom)
	{
	case -3: fzoom = 0.6;  break;
	case -2: fzoom = 0.75;  break;
	case -1: fzoom = 0.87;  break;
	case 0: fzoom = 1.0;  break;
	case 1: fzoom = 1.25;  break;
	case 2: fzoom = 1.5;  break;
	case 3: fzoom = 2.0;  break;
	default: fzoom = 1.0;  break;
	}
}
bool musicxmlscore::displayFile(wxSize sizeClient)
{
	// display the music xml file

	if (!isOk())
		return false;
	docOK = newLayout(sizeClient) ;
	return docOK;
}
int musicxmlscore::getTrackCount()
{
	if (!isOk())
		return 0;

	return (xmlCompile->getTracksCount());
}
wxString musicxmlscore::getTrackName(int trackNr)
{
	if (!isOk())
		return "";
	wxString name;
	name = xmlCompile->getTrackName(trackNr);

	return (name);
}
wxString musicxmlscore::getNamePage(wxFileName fp , int pageNr)
{
	// set the image file name according to the page-number
	wxString prefixe = fp.GetName();
	wxString fn;
	fn.Printf("%s-%d", prefixe, pageNr);
	fp.SetName(fn);

	if (!fp.IsFileReadable())
	{
		fn.Printf("%s-0%d", prefixe, pageNr);
		fp.SetName(fn);

		if (!fp.IsFileReadable())
		{
			fn.Printf("%s-00%d", prefixe, pageNr);
			fp.SetName(fn);

			if (!fp.IsFileReadable())
			{
				// page does not exist ????
				mlog_in("error isFileReadable drawpage %s\n",(const char*)(fp.GetFullPath().c_str()));
				return wxEmptyString;
			}
		}
	}
	fn = fp.GetFullPath();
	return fn ;
}
bool musicxmlscore::getScorePosition(int *absolute_measure_nr, int *measure_nr, int *repeat , int *beat, int *t)
{
	return xmlCompile->getScorePosition(currentPos , absolute_measure_nr, measure_nr, repeat , beat, t);
}
bool musicxmlscore::setPage(int pos , wxRect *rectPos )
{
	if (!isOk() || !docOK || (pos < 0))	return false;

	// get the pagenr adn misc info
	int pageNr;
	bool turnPage;
	if (!(xmlCompile->getPosEvent(pos, &pageNr, rectPos, &turnPage)))	return false;

	if ((pageNr < 1 )||(pageNr > totalPages))	return false;

	// already the right page(s)
	if ((currentPageNr == pageNr) && (currentTurnPage == turnPage))	return true;

	// recreate the current memory DC
	currentPageNr = pageNr ;
	currentTurnPage = turnPage ;

	wxBitmap emptyBitmap(sizePage);
	if ( currentDC )
		delete currentDC ;
	currentDC = new wxMemoryDC(emptyBitmap);
	if (!currentDC->IsOk())
	{
		delete currentDC;
		return false;
	}

	wxFileName fp(musescorepng);
	wxString fn = getNamePage(fp, pageNr);
	if (fn.IsEmpty()) return false;
	wxBitmap fnbitmap(fn, wxBITMAP_TYPE_PNG);


	currentPageNrPartial = -1 ;

	if (turnPage)
	{
		{
			// half page on right
			wxDCClipper clipTurnPage(*currentDC, wxRect(sizePage.GetWidth() / 2 + sizePage.GetWidth() / WIDTH_SEPARATOR_PAGE, 0, sizePage.GetWidth() / 2 - sizePage.GetWidth() / WIDTH_SEPARATOR_PAGE, sizePage.GetHeight()));
			currentDC->SetBackground(this->GetBackgroundColour());
			currentDC->Clear();
			currentDC->DrawBitmap(fnbitmap, 0, 0);
		}
		{
			// anticipate half of the next page
			wxDCClipper clipTurnPage(*currentDC, wxRect(0, 0, sizePage.GetWidth() / 2 + sizePage.GetWidth() / WIDTH_SEPARATOR_PAGE, sizePage.GetHeight()));
			wxString fnturn = getNamePage(fp, pageNr + 1);
			if (!fnturn.IsEmpty())
			{
				wxBitmap fnturnbitmap(fnturn, wxBITMAP_TYPE_PNG);
				currentDC->SetBackground(this->GetBackgroundColour());
				currentDC->Clear();
				currentDC->DrawBitmap(fnturnbitmap, 0, 0);
				currentDC->GradientFillLinear(wxRect(sizePage.GetWidth() / 2, 0, sizePage.GetWidth() / (WIDTH_SEPARATOR_PAGE * 2), sizePage.GetHeight()), this->GetBackgroundColour(), *wxWHITE, wxRIGHT);
				currentDC->GradientFillLinear(wxRect(sizePage.GetWidth() / 2 + sizePage.GetWidth() / (WIDTH_SEPARATOR_PAGE * 2), 0, sizePage.GetWidth() / (WIDTH_SEPARATOR_PAGE * 2), sizePage.GetHeight()), this->GetBackgroundColour(), *wxWHITE, wxLEFT);
				currentPageNrPartial = pageNr + 1;
			}
		}
	}
	else
	{
		// full page
		currentDC->SetBackground(this->GetBackgroundColour());
		currentDC->Clear();
		currentDC->DrawBitmap(fnbitmap, 0, 0);
	}

	if (totalPages > 0)
	{
		// write the page indexes on the bottom
		wxSize sizePageNr = currentDC->GetTextExtent("0");
		buttonPage.SetHeight(sizePageNr.GetHeight());
		buttonPage.SetY(sizePage.GetHeight() - sizePageNr.GetHeight());
		buttonPage.SetX(0);
		buttonPage.SetWidth(sizePage.GetWidth());
		int widthNrPage = sizePage.GetWidth() / totalPages;
		wxDCClipper clipPageNr(*currentDC, buttonPage);
		currentDC->SetBackground(this->GetBackgroundColour());
		currentDC->Clear();
		currentDC->SetTextForeground(*wxBLACK);
		//currentDC->SetTextBackground(*wxWHITE);
		for (int nrPage = 0; nrPage < totalPages; nrPage++)
		{
			wxString spage;
			if (nrPage == (pageNr -1))
				spage.Printf("[%d]", nrPage + 1);
			else
				spage.Printf("%d", nrPage + 1);
			wxSize sizeNrPage = currentDC->GetTextExtent(spage);
			int xsPage = widthNrPage * nrPage + widthNrPage / 2 - sizeNrPage.GetWidth() / 2;
			currentDC->DrawText(spage, xsPage, buttonPage.y);
		}
	}
	return true;
}
bool musicxmlscore::setCursor(wxDC& dc , int pos,bool red )
{
	wxRect rectPos ;

	if ( ! setPage(pos,&rectPos ) ) return false ;

	// redraw the page(s)
	dc.Clear() ;
	if (!dc.Blit(0, 0, sizePage.GetWidth(), sizePage.GetHeight(), currentDC, 0, 0))
		return false;

	// draw the cursor
	rectPos.y += rectPos.height + 1 ;
	rectPos.height /= 3 ;
	if (rectPos.height < 3)
		rectPos.height = 3;
	wxDCClipper cursorclip(dc, rectPos);
	if (red)
		dc.SetBackground(*wxRED_BRUSH);
	else
		dc.SetBackground(*wxGREEN_BRUSH);
	dc.Clear();
	return true;
}
void musicxmlscore::setPosition(int pos, bool playing)
{
	if (!isOk() || !docOK || (pos < 0))	return ;

	// onIdle : set the current pos
	newPaintPos = pos ;
	newPaintPlaying = playing ;
	if ((pos != prevPos) || (playing != prevPlaying))
	{
		wxClientDC dc(this);
		if (setCursor(dc, pos, playing))
		{
			prevPos = pos;
			prevPlaying = playing;
			currentPos = pos;
		}
		nbSetPosition ++ ;
	}
}
void musicxmlscore::onPaint(wxPaintEvent& WXUNUSED(event))
{
	// onPaint
	wxPaintDC dc(this);
	if (!isOk() || !docOK  || (newPaintPos < 0))	return;

	if (true) // ((newPaintPos != prevPaintPos) || (newPaintPlaying != prevPaintPlaying))
	{
		if (setCursor(dc, newPaintPos, newPaintPlaying))
		{
			prevPaintPos = newPaintPos;
			prevPaintPlaying = newPaintPlaying;
			currentPos = newPaintPos;
		}
		nbPaint ++ ;
	}
}
int musicxmlscore::getNbPaint()
{
	int i = nbPaint ;
	nbPaint = 0 ;
	return i;
}
int musicxmlscore::getNbSetPosition()
{
	int i =  nbSetPosition;
	nbSetPosition = 0 ;
	return i ;
}
void musicxmlscore::gotoNextPage(bool forward)
{
	int pageNr = currentPageNr;
	pageNr += (forward) ? 1 : -1;
	if ((pageNr < 1) || (pageNr > totalPages))
		return;
	int nrEvent = xmlCompile->pageToEventNr(pageNr);
	if (nrEvent != -1)
		basslua_call(moduleScore, functionScoreGotoNrEvent, "i", nrEvent + 1);
}
void musicxmlscore::OnLeftDown(wxMouseEvent& event)
{
	if (!isOk())
		return;
	wxClientDC dc(this);
	wxPoint mPoint = event.GetLogicalPosition(dc);
	if (buttonPage.Contains(mPoint) && ( totalPages > 0 ))
	{
		int nrPage = mPoint.x / (sizePage.GetWidth()  / totalPages ) + 1 ;
		int nrEvent;
		nrEvent = xmlCompile->pageToEventNr(nrPage);
		if (nrEvent == -1)
			return;
		prevPos = -1;
		prevPlaying = true;
		prevPaintPos = -1;
		prevPaintPlaying = true;
		basslua_call(moduleScore, functionScoreGotoNrEvent, "i", nrEvent + 1);
		return;
	}
	int mpage = currentPageNr;
	if ((currentPageNrPartial != -1) && (mPoint.x < sizePage.GetWidth()/2))
		mpage = currentPageNrPartial;
	int nrEvent;
	nrEvent = xmlCompile->pointToEventNr(mpage , mPoint);
	if (nrEvent == -1)
		return;
	prevPos = -1;
	prevPlaying = -1;
	basslua_call(moduleScore, functionScoreGotoNrEvent, "i", nrEvent + 1 );
}
void musicxmlscore::gotoPosition()
{
	if (xmlCompile == NULL)
		return;
	wxTextEntryDialog mdialog(NULL, "Expresseur measure (prefix !). eg !12\nScore measure (optional repetion with *). eg 6*2 ( measure 6 2nd time)\nA label as described in the text file. eg A*2 (label A 2nd time)", "Expresseur");
	if (mdialog.ShowModal() == wxID_OK)
	{
		wxString sgoto = mdialog.GetValue();
		int nrEvent = xmlCompile->stringToEventNr(sgoto);
		if (nrEvent != -1)
			basslua_call(moduleScore, functionScoreGotoNrEvent, "i", nrEvent + 1);
	}
}
void musicxmlscore::crc_init()
{
	crc_value = 0 ;
}
wxULongLong musicxmlscore::crc_cumulate_file(wxString fname)
{
	// cumulate the 64bits CRC of the file
    FILE *fp;
    size_t fsz;
    long   off_end;
    int    rc;
    char *buf ;
    fp = fopen ( fname.c_str() , "rb" );
    if( !fp ) return crc_value;
    rc = fseek(fp, 0L, SEEK_END);
    if( 0 != rc ) { fclose(fp); return crc_value; }
    if( 0 > (off_end = ftell(fp)) ) { fclose(fp); return crc_value; }
    fsz = (size_t)off_end;
    buf = (char*)(malloc( fsz+1));
    if( NULL == buf ) { fclose(fp); return crc_value; }
    rewind(fp);
    if( fsz != fread(buf, 1, fsz, fp) ) {fclose(fp); free(buf); return crc_value; }
	fclose(fp);
 	crc_cumulate(buf,fsz);
	free(buf);
	return crc_value ;
}
wxULongLong musicxmlscore::crc_cumulate_string(wxString s)
{
	// cumulate the 64bits CRC of the string
	crc_cumulate(s.char_str(),s.Len());
	return crc_value ;
}
bool musicxmlscore::newLayout(wxSize sizeClient)
{
	if (!isOk())
		return false;
	wxBusyCursor waitcursor;

	wxFileName fm;

	// window big enough ?
	if ((sizeClient.GetWidth() < 200) || (sizeClient.GetHeight() < 100))
		return false;

	// avoir uselees compilation if there is no change
	//if ((sizeClient.GetX() == previousSizeClient.GetX() ) && (sizeClient.GetY() == previousSizeClient.GetY() ) && ( ! xmlCompile->isModified ) && ( fzoom == previousZoom))
	//	return docOK ;

	// calculation for the page layout
	xmlCompile->isModified = false ;

	sizePage.SetWidth( sizeClient.GetX() );
	sizePage.SetHeight( sizeClient.GetY() );

	resolution_dpi = 100.0;
	tenths = 40.0;
	millimeters = 7.055 ;


	if ( ! musescore_def_xml )
	{
		// specifies the size in the MuseScore script 
		resolution_dpi *= fzoom ;
	}

	float width_inch = sizePage.GetWidth()  / resolution_dpi  ;
	float margin_inch = 10.0 / resolution_dpi  ;
	float pwidth_inch = width_inch - 2.0 * margin_inch ;
	float height_inch = sizePage.GetHeight() / resolution_dpi  ;

	if ( musescore_def_xml )
	{
		// specifies the size in the MusicXML
		millimeters *= fzoom;
		float sizeX_tenths = ((sizePage.GetWidth() * inch) / resolution_dpi) * (tenths / millimeters);
		float sizeY_tenths = ((sizePage.GetHeight() * inch) / resolution_dpi) * (tenths / millimeters);
		xmlCompile->compiled_score->defaults.scaling.tenths = tenths;
		xmlCompile->compiled_score->defaults.scaling.millimeters = millimeters;
		xmlCompile->compiled_score->defaults.page_layout.page_width = sizeX_tenths;
		xmlCompile->compiled_score->defaults.page_layout.page_height = sizeY_tenths;
		xmlCompile->compiled_score->defaults.page_layout.margin = 10.0 ;
	}

	// clena useless temp files
	cleanTmp();

	fm.SetPath(mxconf::getTmpDir());

	// prepare the musicXml file to display
	fm.SetFullName("expresseur_out.xml");
	xmlCompile->music_xml_displayed_file = fm.GetFullPath();
	xmlCompile->compiled_score->write(xmlCompile->music_xml_displayed_file, true);

	// the file to store position of notes
	fm.SetFullName("expresseur_pos.txt");
	musescorepos = fm.GetFullPath();

	// the file to store position of notes
	fm.SetFullName("expresseur_score.png");
	musescorepng = fm.GetFullPath();


	// prepare the script for MuseScore
	fm.SetFullName("expresseur_scan.qml");
	musescorescript = fm.GetFullPath();
	wxTextFile fin, fout;
	wxFileName fm2;
	fm2.Assign(mxconf::getCwdDir());
	fm2.SetFullName("scan_position.qml");
	fin.Open(fm2.GetFullPath());
	if (fm.FileExists())
		wxRemoveFile(musescorescript);
	fout.Create(musescorescript);
	fout.Open(musescorescript);
	if (fin.IsOpened() && fout.IsOpened())
	{
		wxString line;
		wxString fposition(musescorepos);
		wxString fpng(musescorepng);
		wxString fwidth;
		wxString fheight;
		wxString fpwidth;
		wxString fmargin ;
		fwidth.Printf("%f",width_inch);
		fheight.Printf("%f",height_inch);
		fpwidth.Printf("%f",pwidth_inch);
		fmargin.Printf("%f",margin_inch);
		fposition.Replace("\\", "\\\\", true);
		fpng.Replace("\\", "\\\\", true);
		int line_nb = fin.GetLineCount();
		int nbReplaceSizing;
		for (int line_nr = 0; line_nr < line_nb; line_nr++)
		{
			line = fin.GetLine(line_nr);
			line.Replace("__expresseur_pos.txt__", fposition);
			line.Replace("__expresseur_out.png__", fpng);
			nbReplaceSizing = 0;
			nbReplaceSizing += line.Replace("__width__", fwidth);
			nbReplaceSizing += line.Replace("__height__", fheight);
			nbReplaceSizing += line.Replace("__pwidth__", fpwidth);
			nbReplaceSizing += line.Replace("__margin__", fmargin);
			if ( nbReplaceSizing == 0 )
				fout.AddLine(line);
			else
			{
				if ( ! musescore_def_xml )
					fout.AddLine(line);
			}
		}
		fin.Close();
		fout.Write();
		fout.Close();
	}

	// prepare the command line to run MuseScore
	wxString command;
	//"C:\Program Files (x86)\MuseScore 2\bin/MuseScore.exe" - s -r <resolution> -m <user_temp>\musicxml.xml - p <user_temp>\scan_position.qml - o <user_temp>\expresseur_out.png
	command.Printf("\"%s\" -s -r %d -m %s -p %s", musescoreexe, (int)resolution_dpi, xmlCompile->music_xml_displayed_file, musescorescript);

	bool alreadyAvailable = false;
	// calculate the CRC of this display
	crc_init();
	crc_cumulate_file(xmlCompile->music_xml_displayed_file);
	crc_cumulate_file(musescorescript);
	wxString ssize ; ssize.Printf("%d %d %f", sizeClient.GetX() , sizeClient.GetY(), fzoom ); 
	crc_cumulate_string(ssize);
	wxULongLong crc_this = crc_cumulate_string(command);
	wxString prefix_cache ;
	prefix_cache.Printf("%s_%s_",PREFIX_CACHE , crc_this.ToString());

	wxFileName fnMusescorepos(musescorepos);
	fm.SetName( prefix_cache + fnMusescorepos.GetName() );
	fm.SetExt(fnMusescorepos.GetExt());
	if ( fm.FileExists())
	{
		// MuseScore result already available in cache. Let's reuse it
		alreadyAvailable = true ;
		if ( ! wxCopyFile(fm.GetFullPath(), fnMusescorepos.GetFullPath()) ) alreadyAvailable = false ;
		readPos();
		// copy pages
		wxFileName fmodele(musescorepng);
		wxFileName fsource(musescorepng);
		wxFileName fdest(musescorepng);
		wxString ffn;
		int pp = 1;
		while (true)
		{
			ffn.Printf("%s-%d", fmodele.GetName(), pp);
			fsource.SetName( prefix_cache + ffn );
			if (fsource.IsFileReadable())
			{
				fdest.SetName(ffn);
				if ( ! wxCopyFile(fsource.GetFullPath(),fdest.GetFullPath()) ) alreadyAvailable = false ;
			}
			else
			{
				ffn.Printf("%s-0%d", fmodele.GetName(), pp);
				fsource.SetName( prefix_cache + ffn );
				if (fsource.IsFileReadable())
				{
					fdest.SetName(ffn);
					if ( ! wxCopyFile(fsource.GetFullPath(),fdest.GetFullPath()) ) alreadyAvailable = false ;
				}
				else
				{
					ffn.Printf("%s-00%d", fmodele.GetName(), pp);
					fsource.SetName( prefix_cache + ffn );
					if (fsource.IsFileReadable())
					{
						fdest.SetName(ffn);
						if ( ! wxCopyFile(fsource.GetFullPath(),fdest.GetFullPath()) ) alreadyAvailable = false ;
					}
					else
					{
						break ;
					}
				}
			}
			pp++;
		}
	}
	if ( alreadyAvailable)
	{ 
		((wxFrame *)mParent)->SetStatusText("Score pages : read from cache",1);
	}
	else
	{
		((wxFrame *)mParent)->SetStatusText("Score pages : computation by MuseScore in progress", 1);
		// run the MuseScore batch to build he pages and the positions
		char bufMuseScoreBatch[1024];
		strcpy(bufMuseScoreBatch,command.c_str());
		mlog_in("musicxmlscore newLayout musescore-batch : %s",bufMuseScoreBatch);
		wxFileName::SetCwd(mxconf::getTmpDir());
		long lexec = wxExecute(command, wxEXEC_SYNC | wxEXEC_HIDE_CONSOLE);
		if (lexec < 0)
		{
			return false;
		}
		
		// wait for the position file, which should dclare the end of the MuseScore batch
		for(int nbTry=0; nbTry < 10 ; nbTry ++ )
		{
			if ( readPos() )
				break ;
			wxSleep(1);
		}

		((wxFrame *)mParent)->SetStatusText("Score pages : stored in cache", 1);

		// cache this result for potential reuse
		fm.SetName( prefix_cache + fnMusescorepos.GetName()) ;
		fm.SetExt(fnMusescorepos.GetExt());

		if ( ! wxCopyFile(musescorepos,fm.GetFullPath()) ) alreadyAvailable = false ;

		wxFileName fdest(musescorepng);
		wxFileName fsource(musescorepng);
		wxFileName fmodele(musescorepng);
		wxString ffn;
		int pp = 1;
		while (true)
		{
			ffn.Printf("%s-%d", fmodele.GetName(), pp);
			fsource.SetName(ffn);
			if (fsource.IsFileReadable())
			{
				fdest.SetName( prefix_cache + ffn );
				wxCopyFile(fsource.GetFullPath(),fdest.GetFullPath())  ;
			}
			else
			{
				ffn.Printf("%s-0%d", fmodele.GetName(), pp);
				fsource.SetName(ffn);
				if (fsource.IsFileReadable())
				{
					fdest.SetName( prefix_cache + ffn );
					wxCopyFile(fsource.GetFullPath(),fdest.GetFullPath()) ;
				}
				else
				{
					ffn.Printf("%s-00%d", fmodele.GetName(), pp);
					fsource.SetName(ffn);
					if (fsource.IsFileReadable())
					{
						fdest.SetName( prefix_cache + ffn );
						wxCopyFile(fsource.GetFullPath(),fdest.GetFullPath()) ;
					}
					else
					{
						break;
					}
				}
			}
			pp++;
		}
	}
		
	currentPageNr = -1;
	currentPageNrPartial = -1;
	prevPos = -1 ;
	prevPaintPos = -1 ;
	prevPlaying = true ;
	return true ;
}
bool musicxmlscore::readPos()
{
	int previous_page_nr = -1;
	bool returnCode = false ;
	wxFileName fp(musescorepos);
	if (! fp.IsFileReadable())
		return false;
	wxTextFile f;
	f.Open(fp.GetFullPath());
	if (f.IsOpened() == false)
		return false;
	wxArrayInt measureTurnPage;
	wxString line;
	int line_nb = f.GetLineCount();
	for (int line_nr = 0; line_nr < line_nb; line_nr++)
	{
		line = f.GetLine(line_nr);
		char bufline[10000];
		strcpy(bufline,line.c_str());
		if ( line == "eof" )
		{
			returnCode = true ;
			break ;
		}
		wxArrayString msplit = wxSplit(line, ' ');
		long l;
		if (msplit[0].ToLong(&l) == false)
		{
			char buferr[1024];
			strcpy(buferr,msplit[0].c_str());
			mlog_in("musicxmlscore / readpos / err [0] tolong(%s)",buferr);
			break;
		}
		int nr_measure = l;
		if (msplit[1].ToLong(&l) == false)
		{
			char buferr[1024];
			strcpy(buferr,msplit[1].c_str());
			mlog_in("musicxmlscore / readpos / err [1] tolong(%s)",buferr);
			break;
		}
		int t_480 = l;
		if (msplit[2].ToLong(&l) == false)
		{
			char buferr[1024];
			strcpy(buferr,msplit[2].c_str());
			mlog_in("musicxmlscore / readpos / err [2] tolong(%s)",buferr);
			break;
		}
		int page_nr = l;
		if (page_nr != previous_page_nr)
		{
			measureTurnPage.Add(nr_measure - 1);
		}
		previous_page_nr = page_nr;
		totalPages = page_nr;
		double d;
		if (msplit[3].ToDouble(&d) == false)
		{
			msplit[3].Replace(".",",");
			if (msplit[3].ToDouble(&d) == false)
			{
				char buferr[1024];
				strcpy(buferr,msplit[3].c_str());
				mlog_in("musicxmlscore / readpos / err [3] tolong(%s)",buferr);
				break;
			}
		}
		float x_tenth = d * 10.0;
		if (msplit[4].ToDouble(&d) == false)
		{
			msplit[4].Replace(".",",");
			if (msplit[4].ToDouble(&d) == false)
			{
				char buferr[1024];
				strcpy(buferr,msplit[4].c_str());
				mlog_in("musicxmlscore / readpos / err [4] tolong(%s)",buferr);
				break;
			}
		}
		float y_tenth = d * 10.0;
		if (msplit[5].ToDouble(&d) == false)
		{
			msplit[5].Replace(".",",");
			if (msplit[5].ToDouble(&d) == false)
			{
				char buferr[1024];
				strcpy(buferr,msplit[5].c_str());
				mlog_in("musicxmlscore / readpos / err [5] tolong(%s)",buferr);
				break;
			}
		}
		float width_tenth = d * 10.0;
		if (msplit[6].ToDouble(&d) == false)
		{
			msplit[6].Replace(".",",");
			if (msplit[6].ToDouble(&d) == false)
			{
				char buferr[1024];
				strcpy(buferr,msplit[6].c_str());
				mlog_in("musicxmlscore / readpos / err [6] tolong(%s)",buferr);
				break;
			}
		}
		float height_tenth = d * 10.0;

		wxRect rect_pixel;
		if ( musescore_def_xml )
		{
			rect_pixel.width = (int)(((width_tenth * millimeters) / tenths) * (resolution_dpi / inch)) ;
			rect_pixel.height = (int)(((height_tenth * millimeters) / tenths) * (resolution_dpi / inch)) ;
			rect_pixel.x = (int)(((x_tenth * millimeters) / tenths) * (resolution_dpi / inch)) ;
			rect_pixel.y = (int)(((y_tenth * millimeters) / tenths) * (resolution_dpi / inch)) - rect_pixel.height / 2 ;
			rect_pixel.width += 2 ;
			rect_pixel.height += 2 ;
		}
		else
		{
			rect_pixel.width = (int)(((width_tenth * millimeters) / tenths) * (resolution_dpi / inch)) ;
			rect_pixel.height = (int)(((height_tenth * millimeters) / tenths) * (resolution_dpi / inch)) ;
			rect_pixel.x = (int)(((x_tenth * millimeters) / tenths) * (resolution_dpi / inch)) ;
			rect_pixel.y = (int)(((y_tenth * millimeters) / tenths) * (resolution_dpi / inch)) - rect_pixel.height / 2 ;
			rect_pixel.width += 2 ;
			rect_pixel.height += 2 ;
		}
		xmlCompile->setPosEvent(nr_measure, t_480, page_nr, rect_pixel);
	}
	xmlCompile->setMeasureTurnEvent(0,true);
	int nbTurn = measureTurnPage.GetCount();
	for (int nrTurn = 0; nrTurn < nbTurn; nrTurn++)
	{
		int nrMeasureTurn = measureTurnPage[nrTurn];
		xmlCompile->setMeasureTurnEvent(nrMeasureTurn);
	}
	return returnCode;
}
void musicxmlscore::initRecordPlayback()
{
	xmlCompile->initRecordPlayback();
}
void musicxmlscore::initPlayback()
{
	xmlCompile->initPlayback();
}
void musicxmlscore::recordPlayback(wxLongLong time, int nr_device, int type_msg, int channel, int value1, int value2)
{
	xmlCompile->recordPlayback( time,  nr_device,  type_msg,  channel,  value1,  value2);
}
bool musicxmlscore::playback()
{
	return (xmlCompile->playback());
}
wxString musicxmlscore::getPlayback()
{
	return xmlCompile->getPlayback();
}
void musicxmlscore::setPlayVisible(wxString sin)
{
	if (!isOk())
		return;
	// analyse sin , to select tracks to be play/view
	// 2/4 : play 2 / view 4
	// 34 : play 3 & 4
	// /12 : view 1 & 2
	// 23/ : play/view 2 & 3
	// */ : play view all
	// * : play all
	// /* : view all
	wxString overwriteListPlay;
	wxString overwriteListVisible;
	bool overwritePlay = false;
	bool overwriteVisible = false;

	overwritePlay = true;
	overwriteVisible = false;
	overwriteListPlay.Empty();
	overwriteListVisible.Empty();

	if (sin.Contains("/"))
	{
		if (sin.Left(1) == "/")
		{
			overwritePlay = false;
			overwriteVisible = true;
			overwriteListPlay = "";
			overwriteListVisible = sin.Mid(1);
		}
		else
		{
			if (sin.Right(1) == "/")
			{
				overwritePlay = true;
				overwriteVisible = true;
				overwriteListPlay = sin.Mid(0, sin.Length() - 1);
				overwriteListVisible = overwriteListPlay;
			}
			else
			{
				overwritePlay = true;
				overwriteVisible = true;
				int pos = sin.Find('/');
				overwriteListPlay = sin.Left(pos);
				overwriteListVisible = sin.Mid(pos + 1);
			}
		}
	}
	else
	{
		overwritePlay = true;
		overwriteVisible = false;
		overwriteListPlay = sin;
		overwriteListVisible = "";
	}

	wxFileName txtFile = xmlCompile->getNameTxtFile();
	wxTextFile f;
	f.Open(txtFile.GetFullPath());
	if (f.IsOpened() == false)
		return;
	wxString line;
	wxString sectionName;
	int line_nb = f.GetLineCount();
	for (int line_nr = 0; line_nr < line_nb; line_nr++)
	{
		line = f.GetLine(line_nr);
		wxString s = line.Upper().Trim();
		// ret_code = true;
		if ((s.IsEmpty() == false) && (s.StartsWith(COMMENT_EXPRESSEUR) == false))
		{
			if (s.StartsWith(SET_MUSICXML_FILE))
			{
				sectionName = "";
			}
			else if (s.StartsWith(SET_TITLE))
			{
				sectionName = "";
			}
			else if (s.StartsWith(SET_PLAY_MARKS))
			{
				sectionName = "";
			}
			else if (s.StartsWith(SET_MARKS))
			{
				sectionName = "";
			}
			else if (s.StartsWith(SET_PARTS))
			{
				sectionName = SET_PARTS;
			}
			else if (s.StartsWith(SET_ORNAMENTS))
			{
				sectionName = "";
			}
			else if (s.StartsWith(SET_PLAYBACK))
			{
				sectionName = "";
			}
			else 
			{
				if (sectionName == SET_PARTS)
				{
					int partNr = xmlCompile->getPartNr(line);
					if ((partNr != wxNOT_FOUND) && (partNr >= 0) && (partNr < 9))
					{
						wxString spart;
						spart.Printf("%d", partNr + 1);
						bool changeLine = false;
						if (overwritePlay)
						{
							if ((overwriteListPlay.Contains(spart)) || (overwriteListPlay == "*"))
							{
								if (line.Replace(PART_NOT_PLAYED, PART_PLAYED, false) > 0)
								{
									changeLine = true;
								}
							}
							else
							{
								if (line.Contains(PART_NOT_PLAYED) == false)
								{
									if (line.Replace(PART_PLAYED, PART_NOT_PLAYED, false) > 0)
									{
										changeLine = true;
									}
								}
							}
						}
						if (overwriteVisible)
						{
							if ((overwriteListVisible.Contains(spart)) || (overwriteListVisible == "*"))
							{
								if (line.Replace(PART_NOT_VISIBLE, PART_VISIBLE, false) > 0)
								{
									changeLine = true;
								}
							}
							else
							{
								if (line.Contains(PART_NOT_VISIBLE) == false)
								{
									if (line.Replace(PART_VISIBLE, PART_NOT_VISIBLE, false) > 0)
									{
										changeLine = true;
									}
								}
							}
						}
						if (changeLine)
						{
							f.RemoveLine(line_nr);
							f.InsertLine(line, line_nr);
						}
					}
				}
			}
		}
	}
	f.Write();
	f.Close();
}

