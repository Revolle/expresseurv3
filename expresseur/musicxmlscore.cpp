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
#include <stdint.h>


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

#define FILE_OUT_XML "expresseur_out.xml" // musicXml export of Expresseur score
#define FILE_OUT_LILY "expresseur_out.ly" // Lilypond translation of FILE_OUT_XML
#define FILE_SRC_LILY "expresseur_src.ly" // Lilypond source file , adptated from FILE_OUT_LILY
#define FILE_OUT_PRESETLILY "expresseur_setting_template.ly" // template of Lilypond script to extract pos of Expreseur notes, and se the size of image
#define FILE_OUT_SETLILY "expresseur_setting.ly" // Lilypond script, adapted from FILE_OUT_PRESETLILY
#define FILE_LOG_LILY "expresseur_out.log" // Lilypond log during compilation of FILE_SRC_LILY
#define FILE_SCORE_PDF "expresseur_out-%d.pdf" // PDF (one per page) , output of lilypond generation
#define FILE_SCORE_PNG "expresseur_out-%d.png" // PNG (one per page) , output of lilypond generation
#define FILE_POS_LILY "expresseur_out_ly.notes" // position of Lilypond Expresseur notes, output of lilypond generation
#define FILE_POS_TXT "expresseur_out.notes" // position of Expresseur notes in FILE_SCORE_PNG
#define FILE_IN_XML "expresseur_in.xml"
#define RESOLUTION_PNG 72

#define PREFIX_CACHE "CACHE_EXPRESSEUR"
#define WIDTH_SEPARATOR_PAGE 10

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(l_posnotes);

uint32_t bswap32(uint32_t x) {
	return ((x & 0x000000FF) << 24) |
		((x & 0x0000FF00) << 8) |
		((x & 0x00FF0000) >> 8) |
		((x & 0xFF000000) >> 24);
}

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
	prevRectPos.SetWidth(0);

	this->SetBackgroundColour(*wxWHITE);
	
	basslua_call(moduleScore, functionScoreInitScore, "" );

	docOK = false;
	xmlCompile = NULL;

	cleanTmp();
	zoom(mConf->get(CONFIG_ZOOM_MUSICXML, 0));
}
musicxmlscore::~musicxmlscore()
{
	//basslua_call(moduleScore, functionScoreInitScore, "" );
	if (xmlCompile != NULL)
		delete xmlCompile;
	xmlCompile = NULL;
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
		wxArrayString fileToBeDeleted ;
		bool cont = dir.GetFirst(&filename, "expresseur*.png", wxDIR_FILES);
		while (cont)
		{
			ft.SetFullName(filename);
			fileToBeDeleted.Add(ft.GetFullPath());
			cont = dir.GetNext(&filename);
		}
		for(unsigned int i = 0 ; i < fileToBeDeleted.GetCount() ; i ++ )
		{
			wxRemoveFile(fileToBeDeleted[i]);
		}
	}
	wxFileName fp;
	fp.SetPath(mxconf::getTmpDir());
	fp.SetFullName(FILE_POS_TXT);
	if (fp.FileExists())
		wxRemoveFile(fp.GetFullPath());
	fp.SetFullName(FILE_OUT_XML);
	if (fp.FileExists())
		wxRemoveFile(fp.GetFullPath());
	fp.SetFullName(FILE_IN_XML);
	if (fp.FileExists())
		wxRemoveFile(fp.GetFullPath());
}
void musicxmlscore::cleanCache(int nbDayCache)
{
	wxDir dir(mxconf::getTmpDir());
	if (dir.IsOpened())
	{
		wxDateTime mlimitdate = wxDateTime::Now();
		if ( nbDayCache > 0 )
		{
			mlimitdate.Subtract(wxDateSpan(0, 0, 0, nbDayCache));
		}
		wxString filename;
		wxFileName ft;
		ft.SetPath(mxconf::getTmpDir());
		wxString sn;
		sn.Printf("%s*.*", PREFIX_CACHE);
		wxArrayString fileToBeDeleted ;
		bool cont = dir.GetFirst(&filename, sn , wxDIR_FILES);
		while (cont)
		{
			ft.SetFullName(filename);
			wxDateTime dateFile = ft.GetModificationTime();
			//wxMessageBox(mlimitdate.FormatDate() + ">?" + dateFile.FormatDate()  + " " + ft.GetName(),"Date expiration cache");
			if ((nbDayCache == -1 ) || ( dateFile.IsEarlierThan(mlimitdate)))
			{
				fileToBeDeleted.Add(ft.GetFullPath());
			}
			cont = dir.GetNext(&filename);
		}
		for(unsigned int i = 0 ; i < fileToBeDeleted.GetCount() ; i ++ )
		{
			wxRemoveFile(fileToBeDeleted[i]);
		}
	}
}
bool musicxmlscore::isOk()
{
	if (xmlCompile == NULL)
		return false;
	return (xmlCompile->isOk());
}
bool musicxmlscore::setFile(const wxFileName &lfilename)
{
	wxBusyCursor wait;

	if (xmlCompile != NULL)
		delete xmlCompile;
	xmlCompile = new musicxmlcompile();

	wxFileName fm;
	fm.SetPath(mxconf::getTmpDir());
	fm.SetFullName(FILE_IN_XML);
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
	case -3: fzoom = 11;  break;
	case -2: fzoom = 14;  break;
	case -1: fzoom = 17;  break;
	case 0: fzoom = 20;  break;
	case 1: fzoom = 25;  break;
	case 2: fzoom = 30;  break;
	case 3: fzoom = 35;  break;
	default: fzoom = 20;  break;
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
wxString musicxmlscore::getNamePage(int pageNr)
{
	// set the image file name according to the page-number
	wxString fn;
	fn.Printf(FILE_SCORE_PNG, pageNr);
	wxFileName fp;
	fp.SetPath(mxconf::getTmpDir());
	fp.SetName(fn);

	if (!fp.IsFileReadable())
	{
		// page does not exist ????
		mlog_in("error isFileReadable drawpage %s\n", (const char*)(fp.GetFullPath().c_str()));
		return wxEmptyString;
	}
	fn = fp.GetFullPath();
	return fn ;
}
bool musicxmlscore::getScorePosition(int *absolute_measure_nr, int *measure_nr, int *repeat , int *beat, int *t )
{
	int uid; 
	if (isOk() )
		return xmlCompile->getScorePosition(currentPos , absolute_measure_nr, measure_nr, repeat , beat, t, &uid );
	return true;
}
bool musicxmlscore::setPage(wxDC& dc, int pos , wxRect *rectPos, bool playing )
{
	if (!isOk() || !docOK || (pos < 0))	return false;

	// get the pagenr adn misc info
	int pageNr = 0 ;
	bool turnPage = false;
	int nr_ornament = 0;
	bool retPosEvent = xmlCompile->getPosEvent(pos, &pageNr, rectPos, &turnPage, &nr_ornament);
	if (!retPosEvent)
	{
		((wxFrame *)mParent)->SetStatusText(wxEmptyString, 2);
	}

	if ((pageNr < 1) || (pageNr > totalPages))
	{
		pageNr = totalPages;
		((wxFrame *)mParent)->SetStatusText(wxEmptyString, 2);
	}

	if (nr_ornament != -1)
	{
		prevNrOrnament = true;
		wxString sn;
		sn.Printf("*%d", nr_ornament + (playing?0:1));
		((wxFrame *)mParent)->SetStatusText(sn, 2);
	}
	else
	{
		if (prevNrOrnament)
		{
			((wxFrame *)mParent)->SetStatusText(wxEmptyString, 2);
			prevNrOrnament = false;
		}
	}
	// already the right page(s)
	if ((currentPageNr == pageNr) && (currentTurnPage == turnPage))	return retPosEvent;

	prevRectPos.SetWidth(0);

	// recreate the page
	currentPageNr = pageNr ;
	currentTurnPage = turnPage ;

	wxFileName fp;
	fp.SetPath(mxconf::getTmpDir());
	wxString fn = getNamePage(pageNr);
	if (fn.IsEmpty()) return false;
	wxBitmap fnbitmap(fn, wxBITMAP_TYPE_PNG);


	currentPageNrPartial = -1 ;

	if (turnPage)
	{
		{
			// half page on right
			wxDCClipper clipTurnPage(dc, wxRect(sizePage.GetWidth() / 2 + sizePage.GetWidth() / WIDTH_SEPARATOR_PAGE, 0, sizePage.GetWidth() / 2 - sizePage.GetWidth() / WIDTH_SEPARATOR_PAGE, sizePage.GetHeight()));
			dc.SetBackground(this->GetBackgroundColour());
			dc.Clear();
			dc.DrawBitmap(fnbitmap, 0, 0);
		}
		{
			// anticipate half of the next page
			{
				wxDCClipper clipTurnPage(dc, wxRect(0, 0, sizePage.GetWidth() / 2 , sizePage.GetHeight()));
				wxString fnturn = getNamePage(pageNr + 1);
				if (!fnturn.IsEmpty())
				{
					wxBitmap fnturnbitmap(fnturn, wxBITMAP_TYPE_PNG);
					dc.SetBackground(this->GetBackgroundColour());
					dc.Clear();
					dc.DrawBitmap(fnturnbitmap, 0, 0);
					currentPageNrPartial = pageNr + 1;
				}
			}
			{
				wxDCClipper clipTurnPage(dc, wxRect(sizePage.GetWidth() / 2, 0,  sizePage.GetWidth() / WIDTH_SEPARATOR_PAGE, sizePage.GetHeight()));
				dc.SetBackground(this->GetBackgroundColour());
				dc.Clear();
			}
		}
	}
	else
	{
		// full page
		dc.SetBackground(this->GetBackgroundColour());
		dc.Clear();
		dc.DrawBitmap(fnbitmap, 0, 0);
	}

	if (totalPages > 0)
	{
		// write the page indexes on the bottom
		wxSize sizePageNr = dc.GetTextExtent("0");
		buttonPage.SetHeight(sizePageNr.GetHeight());
		buttonPage.SetY(sizePage.GetHeight() - sizePageNr.GetHeight());
		buttonPage.SetX(0);
		buttonPage.SetWidth(sizePage.GetWidth());
		int widthNrPage = sizePage.GetWidth() / totalPages;
		wxDCClipper clipPageNr(dc, buttonPage);
		dc.SetBackground(this->GetBackgroundColour());
		dc.Clear();
		dc.SetTextForeground(*wxBLACK);
		//dc.SetTextBackground(*wxWHITE);
		for (int nrPage = 0; nrPage < totalPages; nrPage++)
		{
			wxString spage;
			if (nrPage == (pageNr -1))
				spage.Printf("[%d]", nrPage + 1);
			else
				spage.Printf("%d", nrPage + 1);
			wxSize sizeNrPage = dc.GetTextExtent(spage);
			int xsPage = widthNrPage * nrPage + widthNrPage / 2 - sizeNrPage.GetWidth() / 2;
			dc.DrawText(spage, xsPage, buttonPage.y);
		}
	}
	return retPosEvent;
}
void musicxmlscore::setCursor(wxDC& dc , int pos,bool playing )
{
	wxRect rectPos ;

	bool cont = setPage(dc, pos, &rectPos , playing );
	
	// nbSetPosition ++ ;
	int absolute_measure_nr, measure_nr, repeat, beat, t , uid;
	bool end_score = xmlCompile->getScorePosition(pos, &absolute_measure_nr, &measure_nr, &repeat, &beat, &t, &uid);
	if (absolute_measure_nr != prev_absolute_measure_nr)
	{
		prev_absolute_measure_nr = absolute_measure_nr;
		wxString spos;
		if (end_score)
			spos = _("end");
		else
		{
			switch (repeat)
			{
			case -1:
			case NULL_INT:
				break;
			case 0:
				spos.Printf(_("Expresseur measure %d / Score measure %d"), absolute_measure_nr, measure_nr);
				break;
			case 1:
				spos.Printf(_("Expresseur measure %d / Score measure %d (2nd time)"), absolute_measure_nr, measure_nr);
				break;
			case 2:
				spos.Printf(_("Expresseur measure %d / Score measure %d (3rd time)"), absolute_measure_nr, measure_nr);
				break;
			default:
				spos.Printf(_("Expresseur measure %d / Score measure %d (%dth time)"), absolute_measure_nr, measure_nr, repeat + 1);
				break;
			}
		}
		((wxFrame *)mParent)->SetStatusText(spos, 0);
	}

	// redraw the previous picture behind the cursor)
	if (prevRectPos.GetWidth() > 0)
	{
		wxDCClipper cursorclip(dc, prevRectPos);
		dc.SetBackground(this->GetBackgroundColour());
		dc.Clear();
	}
	if (!cont)
	{
		return;
	}

	// draw the cursor
	wxDCClipper cursorclip(dc, rectPos);
	if (playing)
		dc.SetBackground(*wxRED_BRUSH);
	else
		dc.SetBackground(*wxGREEN_BRUSH);
	dc.Clear();

	prevRectPos = rectPos;
}
void musicxmlscore::setPosition(int pos, bool playing )
{
	if (!isOk() || !docOK || (pos < 0))	return ;

	// onIdle : set the current pos
	newPaintPos = pos ;
	newPaintPlaying = playing ;
	if ((pos != prevPos) || (playing != prevPlaying))
	{
#ifdef RUN_MAC
		Refresh(false);
		Update();
#else
		//wxString sl ;
		//sl.Printf("setPosition pos=%d prevPos = %d newPaintPos=%d",pos,prevPos,newPaintPos);
		//mlog_in(sl);
		
		wxClientDC dc(this);
		setCursor(dc, pos, playing);
		currentPos = pos;
#endif
		prevPos = pos;
		prevPlaying = playing;
	}
}
void musicxmlscore::onPaint(wxPaintEvent& WXUNUSED(event))
{
	// onPaint
	wxPaintDC dc(this);
	if (!isOk() || !docOK  || (newPaintPos < 0))	return;
		
	currentPageNr = -1 ;
	prevRectPos.SetWidth(0);
	
	//wxString sl ;
	//sl.Printf("onpaint prevPos = %d newPaintPos=%d",prevPos,newPaintPos);
	//mlog_in(sl);
	
	setCursor(dc, newPaintPos, newPaintPlaying);
	currentPos = newPaintPos;
	// nbPaint ++ ;
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
void musicxmlscore::gotoPosition(wxString gotovalue)
{
	if (xmlCompile == NULL)
		return;
	wxString sgoto ;
	if (gotovalue.IsEmpty())
	{
		wxTextEntryDialog mdialog(NULL, "Expresseur measure (prefix !). eg !12\nScore measure (optional repetion with *). eg 6*2 ( measure 6 2nd time)\nA label as described in the text file. eg A*2 (label A 2nd time)", "Expresseur");
		if (mdialog.ShowModal() == wxID_OK)
			sgoto = mdialog.GetValue();
	}
	else
		sgoto = gotovalue ;
	if (! sgoto.IsEmpty() )
	{
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
	wxString xmlout, lilyscore, pythonexe, pythonscript, lilysrc, lilyexe, lilysetting, lilylog;
	wxString command_xmltolily , command_lilytopng;
	long lexec;
	wxTextFile fin, fout;


	// window big enough ?
	if ((sizeClient.GetWidth() < 200) || (sizeClient.GetHeight() < 100))
		return false;
	
	// XML score to display
	fm.SetPath(mxconf::getTmpDir());
	fm.SetFullName(FILE_OUT_XML);
	xmlout = fm.GetFullPath();
	//  file to store position of notes
	fm.SetFullName(FILE_POS_TXT);
	lilypos = fm.GetFullPath();
	// file to translate xml to lilypond
	fm.SetFullName(FILE_OUT_LILY);
	if (fm.FileExists())
		wxRemoveFile(fm.GetFullPath());
	lilyscore = fm.GetFullPath();
	// files to adapt lilypond xml translation
	fm.SetFullName(FILE_SRC_LILY);
	if (fm.FileExists())
		wxRemoveFile(fm.GetFullPath());
	lilysrc = fm.GetFullPath();
	// log file 
	fm.SetFullName(FILE_LOG_LILY);
	if (fm.FileExists())
		wxRemoveFile(fm.GetFullPath());
	lilylog = fm.GetFullPath();
	// settings for lilypond
	fm.SetFullName(FILE_OUT_SETLILY);
	if (fm.FileExists())
		wxRemoveFile(fm.GetFullPath());
	lilysetting = fm.GetFullPath(); 
	// lily python
	fm.SetPath(mxconf::getAppDir());
	fm.AppendDir("lilypond");
	fm.AppendDir("bin");
	fm.SetFullName("python.exe");
	pythonexe = fm.GetFullPath();
	// liliy translator 
	fm.SetFullName("musicxml2ly.py");
	pythonscript = fm.GetFullPath();
	// lilypond bin
	fm.SetFullName("lilypond.exe");
	lilyexe = fm.GetFullPath();

	// calculation for the page layout
	xmlCompile->isModified = false ;

	sizePage.SetWidth( sizeClient.GetX() );
	sizePage.SetHeight( sizeClient.GetY() );

	// clena useless temp files
	cleanTmp();


	// create the musicXml file to display
	xmlCompile->music_xml_displayed_file = xmlout;
	xmlCompile->compiled_score->write(xmlCompile->music_xml_displayed_file, true);

	// adapt lilypond settings
	fin.Open(FILE_OUT_PRESETLILY);
	fout.Create(lilysetting);
	fout.Open(lilysetting);
	if (fin.IsOpened() && fout.IsOpened())
	{
		wxString str;
		for (str = fin.GetFirstLine(); !fin.Eof(); str = fin.GetNextLine())
		{
			if (str.StartsWith("#(set-global-staff-size"))
			{
				wxString s1;
				s1.Printf("#(set-global-staff-size %d)", fzoom);
				fout.AddLine(s1);
				continue;
			}
			if (str.StartsWith("#(set! paper-alist"))
			{
				wxString s1;
				s1.Printf("#(set! paper-alist (cons '(\"myformat\" . (cons (* %d pt) (* %d pt))) paper-alist))", sizePage.GetWidth() , sizePage.GetHeight());
				fout.AddLine(s1);
				continue;
			}
			fout.AddLine(str);
		}
		fin.Close();
		fout.Write();
		fout.Close();
	}
	else
	{
		wxMessageBox("Cannot adapt Lilypond settings", "build score", wxOK | wxICON_ERROR);
		return false;
	}

	// prepare the command line to run lilypond
	command_xmltolily.Printf("%s %s --npl --nobeaming --output=%s %s", pythonexe , pythonscript , lilyscore , xmlout );
	command_lilytopng.Printf("%s -dlog-file=%s -dinclude-settings=%s -dresolution=%d  -dseparate-page-formats=pdf,png  %s", lilyexe , lilylog , lilysetting , RESOLUTION_PNG , lilyscore);

	bool alreadyAvailable = false;

	// calculate the CRC of this display
	crc_init();
	crc_cumulate_file(xmlout);
	crc_cumulate_file(lilysetting);
	crc_cumulate_string(command_xmltolily);
	crc_cumulate_string(command_lilytopng);
	wxString prefix_cache ;
	prefix_cache.Printf("%s_%s__",PREFIX_CACHE , crc_value);

	wxFileName poscache;
	poscache.SetPath(mxconf::getTmpDir());
	poscache.SetName( prefix_cache + FILE_POS_TXT);
	if (poscache.FileExists())
	{
		// Lilypond result already available in cache. Let's reuse it
		alreadyAvailable = true ;
		if ( ! wxCopyFile(poscache.GetFullPath(), lilypos) ) alreadyAvailable = false ;
		// copy pages
		wxFileName fsource;
		fsource.SetPath(mxconf::getTmpDir());
		wxFileName fdest;
		fdest.SetPath(mxconf::getTmpDir());
		wxString ffn;
		int pp = 1;
		while (true)
		{
			ffn.Printf(FILE_SCORE_PNG, pp);
			fsource.SetName( prefix_cache + ffn );
			if (!fsource.IsFileReadable())
				break;
			fdest.SetName(ffn);
			if ( ! wxCopyFile(fsource.GetFullPath(),fdest.GetFullPath()) ) alreadyAvailable = false ;
			pp++;
		}
		readPos();
	}
	if ( alreadyAvailable)
	{ 
		((wxFrame *)mParent)->SetStatusText("Score pages : read from cache",0);
	}
	else
	{
		((wxFrame *)mParent)->SetStatusText("Score pages : computation by Lilypond in progress", 0);

		lexec = wxExecute(command_xmltolily, wxEXEC_SYNC | wxEXEC_HIDE_CONSOLE);
		if (lexec < 0)
		{
			wxMessageBox("Cannot translate XML to Lilypond", "build score", wxOK | wxICON_ERROR);
			return false;
		}

		fin.Open(lilyscore);
		fout.Create(lilysrc);
		fout.Open(lilysrc);
		if (fin.IsOpened() && fout.IsOpened())
		{
			wxString str;
			for (str = fin.GetFirstLine(); !fin.Eof(); str = fin.GetNextLine())
			{
				if (!str.StartsWith("\\pointAndClickOff"))
					fout.AddLine(str);
			}
			fin.Close();
			fout.Write();
			fout.Close();
		}
		else
		{
			wxMessageBox("Cannot adapt Lilypond score", "build score", wxOK | wxICON_ERROR);
			return false;
		}

		// run the lilypond batch to build he pages and the positions
		long lexec = wxExecute(command_lilytopng , wxEXEC_SYNC | wxEXEC_HIDE_CONSOLE);
		if (lexec < 0)
		{
			wxMessageBox("Cannot convert Lilypond to PNG", "build score", wxOK | wxICON_ERROR);
			return false;
		}
		
		// TBD calculatepos

		/* wait for the position file, which should dclare the end of the MuseScore batch
		for(int nbTry=0; nbTry < 10 ; nbTry ++ )
		{
			if ( readPos() )
				break ;
			wxSleep(1);
		}
		*/

		((wxFrame *)mParent)->SetStatusText("Score pages : storing in cache", 0 );

		// cache this result for potential reuse
		wxCopyFile(lilypos , poscache.GetFullPath());

		// copy pages
		wxFileName fsource;
		fsource.SetPath(mxconf::getTmpDir());
		wxFileName fdest;
		fdest.SetPath(mxconf::getTmpDir());
		wxString ffn;
		int pp = 1;
		while (true)
		{
			ffn.Printf(FILE_SCORE_PNG, pp);
			fsource.SetName(ffn);
			if (!fsource.IsFileReadable())
				break;
			fdest.SetName(prefix_cache + ffn);
			wxCopyFile(fsource.GetFullPath(), fdest.GetFullPath());
			pp++;
		}
	}
		
	currentPageNr = -1;
	currentPageNrPartial = -1;
	prevPos = -1 ;
	prevPlaying = true ;
	return true ;
}
bool musicxmlscore::readlilypos()
{
	// from file fpos generated by event-litener.ly (modified for this syntax 
	// (ly:format "p:~a:~a:" 	(caddr origin) 		(cadr origin))
	// detect tokens  "<p:128:5:>" (the click-to-point outputed by event-listener.ly, generated by lilypond )
	// each token is added in lposnotes
	char ch;
	uint32_t nbposly, etat, nbint;
	cposnote mposnote;

	FILE* fp = fopen(FILE_POS_LILY, "rb");
	if (fp == NULL)
	{
		wxMessageBox("Cannot open Lilypond pos", "build score", wxOK | wxICON_ERROR);
		return false;
	}
	etat = 0;
	while ((ch = getc(fp)) != EOF)
	{
		switch (etat)
		{
			// p:
		case 0: if (ch == 'p') etat++; else etat = 0; break;
		case 1: if (ch == ':') { etat++; nbint = 0; mposnote.empty = true; }
			  else etat = 0; break;
			// line:column:
		case 2: if ((ch >= '0') && (ch <= '9')) { nbint *= 10; nbint += (ch - '0'); break; }
			  if (ch == ':') { etat++; mposnote.ply.line = nbint; nbint = 0; break; }
			  etat = 0; break;
		case 3: if ((ch >= '0') && (ch <= '9')) { nbint *= 10; nbint += (ch - '0'); break; }
			  if (ch == ':') { etat++; mposnote.ply.column = nbint; nbint = 0; break; }
			  etat = 0; break;
		case 4:
			lposnotes.Append( new cposnote(mposnote));
			etat = 0;
			break;
		default: etat = 0; break;
		}
	}
	fclose(fp);
	return true;
}
bool musicxmlscore::readlilypdf(uint32_t page, uint32_t xpng, uint32_t ypng)
{
	// detect structure "Rect [402.632 226.067 410.763 232.778]*.ly:128:5:6)" for the notes
	// and structure "MediaBox [0 0 798 598]" for the size of the page
	char ch;
	uint32_t etat, nbint;
	sfpos spdf ;
	float fletat, nbfloat;
	sposly mly;
	spdf.x1 = 0; spdf.y1 = 0; spdf.x2 = 0; spdf.y2 = 0;

	wxString fpdf;
	fpdf.Printf(FILE_SCORE_PDF, page);

	FILE* fp = fopen(fpdf, "rb");
	if (fp == NULL)
	{
		wxMessageBox("Cannot open Lilypond pdf", "build score", wxOK | wxICON_ERROR);
		return false;
	}
	etat = 0;
	while ((ch = getc(fp)) != EOF)
	{
		switch (etat)
		{
		case 0: if (ch == 'R') etat++; else { if (ch == 'M') etat = 101; else etat = 0; }; break;
		// seach for a small PDF rectangle sie. Pattern is "Rect [x1 y1 x2 y2]" 
		case 1: if (ch == 'e') etat++; else etat = 0; break;
		case 2: if (ch == 'c') etat++; else etat = 0; break;
		case 3: if (ch == 't') etat++; else etat = 0; break;
		case 4: if (ch == ' ') etat++; else etat = 0; break;
		case 5: if (ch == '[') { etat++; nbfloat = 0.0; }
			  else etat = 0; break;
			// x1_
		case 6: if ((ch >= '0') && (ch <= '9')) { nbfloat *= 10; nbfloat += (ch - '0'); break; }
			  if (ch == '.') { fletat = 0.1; etat++; break; }
			  if (ch == ' ') { etat += 2; spdf.x1 = nbfloat; nbfloat = 0.0; break; }
			  etat = 0; break;
		case 7: if ((ch >= '0') && (ch <= '9')) { nbfloat += (ch - '0') * fletat; fletat *= 0.1; break; }
			  if (ch == ' ') { etat++; spdf.x1 = nbfloat; nbfloat = 0.0; break; }
			  etat = 0; break;
			  // y1_
		case 8: if ((ch >= '0') && (ch <= '9')) { nbfloat *= 10; nbfloat += (ch - '0'); break; }
			  if (ch == '.') { fletat = 0.1; etat++; break; }
			  if (ch == ' ') { etat += 2; spdf.y1 = nbfloat; nbfloat = 0.0; break; }
			  etat = 0; break;
		case 9: if ((ch >= '0') && (ch <= '9')) { nbfloat += (ch - '0') * fletat; fletat *= 0.1; break; }
			  if (ch == ' ') { etat++; spdf.y1 = nbfloat; nbfloat = 0.0; break; }
			  etat = 0; break;
			  // x2_
		case 10: if ((ch >= '0') && (ch <= '9')) { nbfloat *= 10; nbfloat += (ch - '0'); break; }
			   if (ch == '.') { fletat = 0.1; etat++; break; }
			   if (ch == ' ') { etat += 2; spdf.x2 = nbfloat; nbfloat = 0.0; break; }
			   etat = 0; break;
		case 11: if ((ch >= '0') && (ch <= '9')) { nbfloat += (ch - '0') * fletat; fletat *= 0.1; break; }
			   if (ch == ' ') { etat++; spdf.x2 = nbfloat; nbfloat = 0.0; break; }
			   etat = 0; break;
			   // y2]
		case 12: if ((ch >= '0') && (ch <= '9')) { nbfloat *= 10.0; nbfloat += float(ch - '0'); break; }
			   if (ch == '.') { fletat = 0.1; etat++; break; }
			   if (ch == ']') { etat += 2; spdf.y2 = nbfloat; nbfloat = 0.0; break; }
			   etat = 0; break;
		case 13: if ((ch >= '0') && (ch <= '9')) { nbfloat += (ch - '0') * fletat; fletat *= 0.1; break; }
			   if (ch == ']') { etat++; spdf.y2 = nbfloat; nbfloat = 0.0; break; }
			   etat = 0; break;
			   // search for the lilipond point-and-click reference in teh PDF. Pattern "*.ly:line:column:width]"
		case 14:  if (ch == '.') { etat++; }
			   else { if (ch == '>' ) etat = 0; } break;
		case 15:  if (ch == 'l') { etat++; }
			   else { if (ch == '>') etat = 0; else etat = 14; } break;
		case 16:  if (ch == 'y') { etat++; }
			   else { if (ch == '>') etat = 0; else etat = 14; } break;
		case 17:  if (ch == ':') { etat++; nbint = 0; }
			   else { if (ch == '>') etat = 0; else etat = 14; } break;
			// posx:posy:l)
		case 18: if ((ch >= '0') && (ch <= '9')) { nbint *= 10; nbint += (ch - '0'); break; }
			   if (ch == ':') { etat++; mly.line = nbint; nbint = 0; break; }
			   etat = 0; break;
		case 19: if ((ch >= '0') && (ch <= '9')) { nbint *= 10; nbint += (ch - '0'); break; }
			   if (ch == ':') { etat++; mly.column = nbint; nbint = 0; break; }
			   etat = 0; break;
		case 20: if ((ch >= '0') && (ch <= '9')) { nbint *= 10; nbint += (ch - '0'); break; }
			   if (ch == ']') { etat++; nbint = 0; break; }
			   etat = 0; break;
			   // end Rect [402.632 226.067 410.763 232.778]....ly:128:5:6)
		case 21:
			etat = 0;
			// search this PDF object rectangle in the posnotes generated by liypond script
			for (l_posnotes::iterator iter_posnotes = lposnotes.begin(); iter_posnotes != lposnotes.end(); ++iter_posnotes)
			{
				cposnote* current_posnotes = *iter_posnotes;
				if ((current_posnotes->empty) && (mly.line = current_posnotes->ply.line) && (mly.column = current_posnotes->ply.column))
				{
					current_posnotes->empty = false;
					current_posnotes->page = page;
					current_posnotes->pdf.x1 = spdf.x1 ;
					current_posnotes->pdf.y1 = spdf.y1;
					current_posnotes->pdf.x2 = spdf.x2;
					current_posnotes->pdf.y1 = spdf.y2;
				}
			}
			break;
	    // size of the image-page in the pdf, pattern "MediaBox [0 0 798 598]"
		case 101: if (ch == 'e') etat++; else etat = 0; break;
		case 102: if (ch == 'd') etat++; else etat = 0; break;
		case 103: if (ch == 'i') etat++; else etat = 0; break;
		case 104: if (ch == 'a') etat++; else etat = 0; break;
		case 105: if (ch == ' ') etat++; else etat = 0; break;
		case 106: if (ch == 'B') etat++; else etat = 0; break;
		case 107: if (ch == 'o') etat++; else etat = 0; break;
		case 108: if (ch == 'x') etat++; else etat = 0; break;
		case 109: if (ch == ' ') etat++; else etat = 0; break;
		case 110: if (ch == '[') { etat++; nbint = 0; }
				else etat = 0; break;
		case 111: if ((ch >= '0') && (ch <= '9')) { nbint *= 10; nbint += (ch - '0'); break; }
				if (ch == ' ') { etat++; spdf.x1 = nbint; nbint = 0; break; }
				etat = 0; break;
		case 112: if ((ch >= '0') && (ch <= '9')) { nbint *= 10; nbint += (ch - '0'); break; }
				if (ch == ']') { etat = 0; spdf.y1 = nbint; break; }
				etat = 0; break;
		case 113: if ((ch >= '0') && (ch <= '9')) { nbint *= 10; nbint += (ch - '0'); break; }
				if (ch == ' ') { etat++; spdf.x2 = nbint; nbint = 0; break; }
				etat = 0; break;
		case 114: if ((ch >= '0') && (ch <= '9')) { nbint *= 10; nbint += (ch - '0'); break; }
				if (ch == ']') { etat = 0; spdf.y2 = nbint  ; break; }
				etat = 0; break;
		default: etat = 0; break;
		}
	}
	fclose(fp);
	if (spdf.x2 == 0)
	{
		wxMessageBox("Cannot read size Lilypond pdf", "build score", wxOK | wxICON_ERROR);
		return false;
	}
	float fxpng = (float)(xpng);
	float fypng = (float)(ypng);
	float fxpdf = (float)(spdf.x2 - spdf.x1);
	float fypdf = (float)(spdf.y2 - spdf.y1);
	// readjust teh size of pdf rectangle in the page to png rectangle 
	for (l_posnotes::iterator iter_posnotes = lposnotes.begin(); iter_posnotes != lposnotes.end(); ++iter_posnotes)
	{
		cposnote* current_posnotes = *iter_posnotes;
		if ((! current_posnotes->empty) && (page == current_posnotes->page))
		{
			current_posnotes->png.x1 = (int)(fxpng *current_posnotes->pdf.x1 / fxpdf);
			current_posnotes->png.y1 = (int)(fypng *current_posnotes->pdf.y1 / fypdf);
			current_posnotes->png.x2 = (int)(fxpng *current_posnotes->pdf.x2 / fxpdf);
			current_posnotes->png.y1 = (int)(fypng *current_posnotes->pdf.y2 / fypdf);
		}
	}

	return true;
}
bool musicxmlscore::readpngsize(uint32_t* xpng, uint32_t* ypng)
{
	// read size png
	*xpng = 0; *ypng = 0;
	wxString fpng;
	fpng.Printf(FILE_SCORE_PNG, 0);
	FILE* fp = fopen(fpng, "rb");
	if (fp == NULL)
	{
		wxMessageBox("Cannot open Lilypond png", "build score", wxOK | wxICON_ERROR);
		return false;
	}
	fseek(fp, 16, SEEK_SET);  // Move to the IHDR chunk's location
	fread(xpng, sizeof(uint32_t), 1, fp);
	fread(ypng, sizeof(uint32_t), 1, fp);

	// Convert from big-endian to host-endian
	*xpng = bswap32(*xpng);
	*ypng = bswap32(*ypng);

	fclose(fp);
	return true;
}
bool musicxmlscore::readlilypond(char* score, char* fpos)
{
	char files[1024];
	uint32_t pagenr = 0;
	uint32_t xpng, ypng;
	sprintf(files, "%s_%d.png", score, 0);
	if (!readpngsize(files, &xpng, &ypng))
	{
		// no png score
		wxMessageBox("no Lilypond png", "build score", wxOK | wxICON_ERROR); 
		return false;
	}

	while (true)
	{
		sprintf(files, "%s_%d.txt", fpos, pagenr);
		if (!readlilypos(files, pagenr))
		{
			// no more page
			break;
		}

		sprintf(files, "%s_%d.pdf", score, pagenr);
		if (!readlilypdf(files, pagenr,  xpng,  ypng))
		{
			// no png score
			wxMessageBox("no Lilypond pdf page correpsondance", "build score", wxOK | wxICON_ERROR);
			break;
		}

		pagenr++;
	}

	// output of the lilypond calculation to the lilypos file
	FILE* fp = fopen(lilypos, "w");
	if (fp == NULL)
	{
		wxMessageBox("Cannot write Lilypos", "build score", wxOK | wxICON_ERROR);
		return false;
	}

	for (l_posnotes::iterator iter_posnotes = lposnotes.begin(); iter_posnotes != lposnotes.end(); ++iter_posnotes)
	{
		cposnote* current_posnotes = *iter_posnotes;
		if (!current_posnotes->empty)
		{
		}
	}
	return true;
}


bool musicxmlscore::readPos()
{
	int previous_page_nr = -1;
	bool returnCode = false ;
	wxFileName fp;
	fp.SetPath(mxconf::getTmpDir());
	fp.SetFullName(FILE_POS_TXT);
	if (!fp.IsFileReadable())
	{
		wxMessageBox("No position score", "build score", wxOK | wxICON_ERROR);
		return false;
	}
	wxTextFile f;
	f.Open(fp.GetFullPath());
	if (f.IsOpened() == false)
	{
		wxMessageBox("Cannot open position score", "build score", wxOK | wxICON_ERROR);
		return false;
	}
	wxArrayInt measureTurnPage;
	wxString line;
	wxRect rect_pixel;
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
		int t480 = l;

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

		if (msplit[3].ToLong(&l) == false)
		{
			char buferr[1024];
			strcpy(buferr, msplit[3].c_str());
			mlog_in("musicxmlscore / readpos / err [3] tolong(%s)", buferr);
			break;
		}
		rect_pixel.x = l;

		if (msplit[4].ToLong(&l) == false)
		{
			char buferr[1024];
			strcpy(buferr, msplit[4].c_str());
			mlog_in("musicxmlscore / readpos / err [4] tolong(%s)", buferr);
			break;
		}
		rect_pixel.y = l;

		if (msplit[5].ToLong(&l) == false)
		{
			char buferr[1024];
			strcpy(buferr, msplit[5].c_str());
			mlog_in("musicxmlscore / readpos / err [5] tolong(%s)", buferr);
			break;
		}
		rect_pixel.y = l;

		if (msplit[6].ToLong(&l) == false)
		{
			char buferr[1024];
			strcpy(buferr, msplit[6].c_str());
			mlog_in("musicxmlscore / readpos / err [6] tolong(%s)", buferr);
			break;
		}
		rect_pixel.width = l;

		if (msplit[7].ToLong(&l) == false)
		{
			char buferr[1024];
			strcpy(buferr, msplit[7].c_str());
			mlog_in("musicxmlscore / readpos / err [7] tolong(%s)", buferr);
			break;
		}
		rect_pixel.height = l;

		rect_pixel.height /= 3;
		if (rect_pixel.height < 3)
			rect_pixel.height = 3;

		xmlCompile->setPosEvent(nr_measure, t480, page_nr, rect_pixel); // , mbitmap);
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
bool musicxmlscore::setPlayVisible(wxString sin)
{
	if (!isOk())
		return false;
	// analyse sin , to select tracks to be play/view
	// 2/4 : play 2 / view 4
	// 34 : play 3 & 4
	// /12 : view 1 & 2
	// 23/ : play/view 2 & 3
	// */ : play view all
	// * : play all
	// /* : view all
	// -2 : not play track 2. Other : not changed
	// +2 : play track 2. Other not changed

	bool changed = false;
	wxString overwriteListPlay;
	wxString overwriteListVisible;

	bool overwritePlay = false;
	bool overwriteVisible = false;
	int silenceTrack = -1;
	int playTrack = -1;
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
		if (sin.Left(1) == "+")
		{
			long l;
			if (sin.Mid(1).ToLong(&l))
			{
				playTrack = (int)l;
			}
			else return false;
		}
		else if (sin.Left(1) == "-")
		{
			long l;
			if (sin.Mid(1).ToLong(&l))
			{
				silenceTrack = (int)l;
			}
			else return false;
		}
		else
		{
			overwritePlay = true;
			overwriteVisible = false;
			overwriteListPlay = sin;
			overwriteListVisible = "";
		}
	}

	wxFileName txtFile = xmlCompile->getNameTxtFile();
	wxTextFile f;
	f.Open(txtFile.GetFullPath());
	if (f.IsOpened() == false)
		return false;
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
						if ((partNr + 1) == playTrack)
						{
							if (line.Replace(PART_NOT_PLAYED, PART_PLAYED, false) > 0)
							{
								changeLine = true;
							}
						}
						if ((partNr + 1) == silenceTrack)
						{
							if (line.Replace(PART_PLAYED, PART_NOT_PLAYED, false) > 0)
							{
								changeLine = true;
							}
						}
						if (changeLine)
						{
							f.RemoveLine(line_nr);
							f.InsertLine(line, line_nr);
							changed = true;
						}
					}
				}
			}
		}
	}
	f.Write();
	f.Close();
	return changed;
}

