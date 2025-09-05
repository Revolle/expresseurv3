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
#include <vector>
#include <algorithm>

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

#include "wx/frame.h"
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
#include "wx/bitmap.h"


#include "global.h"
#include "basslua.h"
#include "mxconf.h"
#include "viewerscore.h"
#include "expression.h"
#include "luafile.h"

#include "musicxml.h"
#include "musicxmlcompile.h"
#include "musicxmlscore.h"

uint32_t bswap32(uint32_t x) {
	return ((x & 0x000000FF) << 24) |
		((x & 0x0000FF00) << 8) |
		((x & 0x00FF0000) >> 8) |
		((x & 0xFF000000) >> 24);
}

// CRC tool to optimize calcualtion of score pages

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
std::uint64_t crc_poly = 0xC96C5795D7870F42;

// input is dividend: as 0000000000000000000000000000000000000000000000000000000000000000<8-bit byte>
// where the lsb of the 8-bit byte is the coefficient of the highest degree term (x^71) of the dividend
// so division is really for input byte * x^64

// you may wonder how 72 bits will fit in 64-bit data type... well as the shift-right occurs, 0's are supplied
// on the left (most significant) side ... when the 8 shifts are done, the right side (where the input
// byte was placed) is discarded

// when done, table[XX] (where XX is a byte) is equal to the CRC of 00 00 00 00 00 00 00 00 XX
//
std::uint64_t crc_table[256];

void crc_generate_table()
{
    for(unsigned int i=0; i<256; ++i)
    {
    	std::uint64_t crc = i;

    	for(unsigned int j=0; j<8; ++j)
    	{
            // is current coefficient set?
			std::uint64_t alreadySet = crc & 1;
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
std::uint64_t crc_value = 0 ;
void crc_cumulate(char *stream, unsigned int n)
{
	// cumulate CRC of stream (lenght=n)
    for(unsigned int i=0; i<n; ++i)
    {
    	std::uint64_t mc = std::uint64_t(stream[i]);
        std::uint64_t index = mc ^ crc_value;
		std::uint64_t ff = 0xFFFF;
		std::uint64_t ii = (index & ff);
		std::uint64_t i256 = 256;
		ii = ii % i256;
		int si = ii;
        std::uint64_t lookup = crc_table[si];

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

musicxmlscore::musicxmlscore(wxWindow *parent, wxWindowID id , bool log )
: viewerscore(parent, id)
{
	crc_generate_table();


	mParent = parent;
	nrChord = -1;
	xmlName.Clear();
	prevRectPos.SetWidth(0);

	this->SetBackgroundColour(*wxWHITE);
	
	basslua_call(moduleScore, functionScoreInitScore, "" );

	docOK = false;
	xmlCompile = NULL;

	logLilypond = log;

	cleanTmp();
}
musicxmlscore::~musicxmlscore()
{
	//basslua_call(moduleScore, functionScoreInitScore, "" );
	if (xmlCompile != NULL)
		delete xmlCompile;
	xmlCompile = NULL;
}
void musicxmlscore::cleanTmp()
{
	wxFileName ft;
	ft.SetPath(getTmpDir());
	wxString filename;
	std::vector<wxString> masqs = { "ex*.png" , "ex*.pdf", "ex*.ly*" , "ex*.pos" , "ex*.xml" , "ex*.log"};
	std::vector<wxString> fileToBeDeleted;
	wxDir dir(getTmpDir());
	if (dir.IsOpened())
	{
		for (auto & masq : masqs)
		{
			bool cont = dir.GetFirst(&filename, masq, wxDIR_FILES);
			while (cont)
			{
				ft.SetFullName(filename);
				fileToBeDeleted.push_back(ft.GetFullPath());
				cont = dir.GetNext(&filename);
			}
		}
		for (auto f : fileToBeDeleted)
		{
			wxRemoveFile(f);
		}
	}
}
void musicxmlscore::cleanCache(int nbDayCache)
{
	wxDir dir(getTmpDir());
	if (dir.IsOpened())
	{
		wxDateTime mlimitdate = wxDateTime::Now();
		if ( nbDayCache > 0 )
		{
			mlimitdate.Subtract(wxDateSpan(0, 0, 0, nbDayCache));
		}
		wxString filename;
		wxFileName ft;
		ft.SetPath(getTmpDir());
		wxString sn;
		sn.Printf("%s*.*", PREFIX_CACHE);
		std::vector<wxString> fileToBeDeleted;
		bool cont = dir.GetFirst(&filename, sn , wxDIR_FILES);
		while (cont)
		{
			ft.SetFullName(filename);
			wxDateTime dateFile = ft.GetModificationTime();
			//wxMessageBox(mlimitdate.FormatDate() + ">?" + dateFile.FormatDate()  + " " + ft.GetName(),"Date expiration cache");
			if ((nbDayCache == -1 ) || ( dateFile.IsEarlierThan(mlimitdate)))
			{
				fileToBeDeleted.push_back(ft.GetFullPath());
			}
			cont = dir.GetNext(&filename);
		}
		for(auto f : fileToBeDeleted)
		{
			wxRemoveFile(f);
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
	xmlName = lfilename;
	if (xmlCompile != NULL)
		delete xmlCompile;
	xmlCompile = new musicxmlcompile();

	wxFileName fm;
	fm.SetPath(getTmpDir());
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
		s.Printf("Error opening stream file %s", f.GetFullName());
		wxMessageBox(s);
		return false;
	}

	wxZipInputStream zip(in);
	if (!zip.IsOk())
	{
		wxString s;
		s.Printf("Error reading zip structure of %s", f.GetFullName());
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
				s.Printf("Error reading zip entry %s of %s", name, f.GetFullName());
				wxMessageBox(s);
				return false;
			}
			zip.Read(stream_out);
			if (zip.LastRead() < 10)
			{
				wxString s;
				s.Printf("Error content in zip entry %s of %s", name, f.GetFullName());
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
	fn.Printf(FILE_SCORE_PNG, pageNr + 1);
	wxFileName fp;
	fp.SetPath(getTmpDir());
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
bool musicxmlscore::drawPage(int pageNr, bool turnPage)
{
	//mlog_in("drawPage %d sec/ pageNr=%d / turnPage=%d", wxGetLocalTimeMillis().GetValue() ,  pageNr, turnPage);
	//prevRectPos.SetWidth(0);
	scoreBitmap.Create(sizePage.GetWidth(), sizePage.GetHeight());
	// recreate the page
	wxMemoryDC memDC(scoreBitmap);
	//mlog_in("setPage start bitmap(%s) memDC(%s)", (scoreBitmap.IsOk()) ? "OK" : "!!KO!!" , (memDC.IsOk()) ? "OK" : "!!KO!!");

	currentPageNr = pageNr;
	currentTurnPage = turnPage;

	wxFileName fp;
	fp.SetPath(getTmpDir());
	wxString fn = getNamePage(pageNr);
	if (fn.IsEmpty()) return false;
	wxBitmap fnbitmap(fn, wxBITMAP_TYPE_PNG);
	//mlog_in("setPage bitmap=%s (%s)", (const char*)(fn.c_str()), (fnbitmap.IsOk()) ? "OK" : "!!KO!!" );

	currentPageNrPartial = -1;

	if (turnPage)
	{
		{
			// half page on right
			wxDCClipper clipTurnPage(memDC, wxRect(sizePage.GetWidth() / 2 + sizePage.GetWidth() / WIDTH_SEPARATOR_PAGE, 0, sizePage.GetWidth() / 2 - sizePage.GetWidth() / WIDTH_SEPARATOR_PAGE, sizePage.GetHeight()));
			memDC.SetBackground(this->GetBackgroundColour());
			memDC.Clear();
			memDC.DrawBitmap(fnbitmap, 0, 0);
		}
		{
			// anticipate half of the next page
			{
				wxDCClipper clipTurnPage(memDC, wxRect(0, 0, sizePage.GetWidth() / 2, sizePage.GetHeight()));
				wxString fnturn = getNamePage(pageNr + 1);
				if (!fnturn.IsEmpty())
				{
					wxBitmap fnturnbitmap(fnturn, wxBITMAP_TYPE_PNG);
					memDC.SetBackground(this->GetBackgroundColour());
					memDC.Clear();
					memDC.DrawBitmap(fnturnbitmap, 0, 0);
					currentPageNrPartial = pageNr + 1;
				}
			}
			{
				wxDCClipper clipTurnPage(memDC, wxRect(sizePage.GetWidth() / 2, 0, sizePage.GetWidth() / WIDTH_SEPARATOR_PAGE, sizePage.GetHeight()));
				memDC.SetBackground(this->GetBackgroundColour());
				memDC.Clear();
			}
		}
	}
	else
	{
		// full page
		memDC.SetBackground(this->GetBackgroundColour());
		memDC.Clear();
		memDC.DrawBitmap(fnbitmap, 0, 0);
		//mlog_in("setPage DrawBitmap");
	}

	if (totalPages > 0)
	{
		// write the page indexes on the bottom
		wxSize sizePageNr = memDC.GetTextExtent("0");
		buttonPage.SetHeight(sizePageNr.GetHeight());
		buttonPage.SetY(sizePage.GetHeight() - sizePageNr.GetHeight());
		buttonPage.SetX(0);
		buttonPage.SetWidth(sizePage.GetWidth());
		int widthNrPage = sizePage.GetWidth() / totalPages;
		wxDCClipper clipPageNr(memDC, buttonPage);
		memDC.SetBackground(this->GetBackgroundColour());
		memDC.Clear();
		memDC.SetTextForeground(*wxBLACK);
		//memDC.SetTextBackground(*wxWHITE);
		for (int nrPage = 0; nrPage < totalPages; nrPage++)
		{
			wxString spage;
			if (nrPage == pageNr)
				spage.Printf("[%d]", nrPage + 1);
			else
				spage.Printf("%d", nrPage + 1);
			wxSize sizeNrPage = memDC.GetTextExtent(spage);
			int xsPage = widthNrPage * nrPage + widthNrPage / 2 - sizeNrPage.GetWidth() / 2;
			memDC.DrawText(spage, xsPage, buttonPage.y);
			//mlog_in("setPage DrawText=%s", (const char*)(spage.c_str()));

		}
	}
	memDC.SelectObject(wxNullBitmap);
	//mlog_in("setPage end (%s) ",(scoreBitmap.IsOk()) ? "OK" : "!!KO!!");
	return true;
}
bool musicxmlscore::setPage(int pageNr, bool turnPage, bool redraw)
{
	if (!isOk() || !docOK )	return false;

	// mlog_in("setPage pageNr=%d (%d) / turnPage=%d (%d) / %d\n", pageNr , currentPageNr , turnPage, currentTurnPage , redraw);
	if ((pageNr == -1) || ((currentPageNr == pageNr) && (currentTurnPage == turnPage)))
	{
		if ( redraw )
		{
			// redraw the page
			return drawPage( pageNr,  turnPage);
		}
	}
	else
	{ 
		return drawPage( pageNr,  turnPage);
	}
	return false;
}
bool musicxmlscore::setCursor(int pos,bool playing, bool redraw )
{
	wxRect rectPos ;
	int nr_ornament = 0;

	// get the pagenr adn misc info
	int pageNr = 0;
	bool turnPage = false;
	bool retPosEvent = xmlCompile->getPosEvent(pos, &pageNr, &rectPos, &turnPage, &nr_ornament);
	if (!retPosEvent)
	{
		((wxFrame*)mParent)->SetStatusText(wxEmptyString, 2);
	}

	if ((pageNr < 0) || (pageNr >= totalPages))
	{
		((wxFrame*)mParent)->SetStatusText(wxEmptyString, 2);
	}
	if (nr_ornament != -1)
	{
		prevNrOrnament = true;
		wxString sn;
		sn.Printf("*%d", nr_ornament + (playing ? 0 : 1));
		((wxFrame*)mParent)->SetStatusText(sn, 2);
	}
	else
	{
		if (prevNrOrnament)
		{
			((wxFrame*)mParent)->SetStatusText(wxEmptyString, 2);
			prevNrOrnament = false;
		}
	}

	setPage( pageNr , turnPage , redraw );
	
	// nbSetPosition ++ ;
	int absolute_measure_nr, measure_nr, repeat, beat, t , uid;
	bool end_score = xmlCompile->getScorePosition(pos, &absolute_measure_nr, &measure_nr, &repeat, &beat, &t, &uid);
	if (absolute_measure_nr != prev_absolute_measure_nr)
	{
		prev_absolute_measure_nr = absolute_measure_nr;
		wxString spos;
		if (end_score)
			spos = "end";
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
		((wxFrame*)mParent)->SetStatusText(spos, 0);
	}
	wxMemoryDC memDC(scoreBitmap);

	// redraw the previous picture behind the cursor)
	if (! prevRectPos.IsEmpty() )
	{
		wxDCClipper cursorclip(memDC, prevRectPos);
		memDC.SetBackground(this->GetBackgroundColour());
		memDC.Clear();
	}

	if ( ! rectPos.IsEmpty() )
	{
		// draw the cursor
		wxDCClipper cursorclip(memDC, rectPos);
		if (playing)
			memDC.SetBackground(*wxRED_BRUSH);
		else
			memDC.SetBackground(*wxGREEN_BRUSH);		
		memDC.Clear();
	}

	prevRectPos = rectPos;
	return true;
}
void musicxmlscore::setPosition(int pos, bool playing)
{
	if (!isOk() || !docOK || (pos < 0))	return;

	// onIdle : set the current pos
	if ((pos != prevPos) || (playing != prevPlaying))
	{
		prevPos = pos;
		prevPlaying = playing;
		Refresh();
	}
}

void musicxmlscore::onPaint(wxPaintEvent& WXUNUSED(event))
{
	// onPaint
	wxPaintDC dc(this);
	if (!isOk() || !docOK  || (prevPos < 0))	return;
	wxRegionIterator upd(GetUpdateRegion()); // get the update rect list
	//int vX, vY, vW, vH;
	bool redraw = false;
	while (upd)
	{
		redraw = true;
		break;
		/*
		vX = upd.GetX();
		vY = upd.GetY();
		vW = upd.GetW();
		vH = upd.GetH();
		wxString sl ;
		sl.Printf("onpaint v %d %d %d %d",vX,vY,vW,vH);
		mlog_in(sl);
		// Repaint this rectangle
		upd++;
		*/
	}
	setCursor(prevPos, prevPlaying, redraw);
	dc.DrawBitmap(scoreBitmap, 0, 0, false);
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
	if ((pageNr < 0) || (pageNr >= totalPages))
		return;
	int nrEvent = xmlCompile->pageToEventNr(pageNr);
	if (nrEvent != -1)
	{
		// currentPageNr = pageNr;	
		basslua_call(moduleScore, functionScoreGotoNrEvent, "i", nrEvent + 1);
	}
}
void musicxmlscore::OnLeftDown(wxMouseEvent& event)
{
	if (!isOk())
		return;
	wxClientDC dc(this);
	wxPoint mPoint = event.GetLogicalPosition(dc);
	if (buttonPage.Contains(mPoint) && ( totalPages > 0 ))
	{
		int nrPage = mPoint.x / (sizePage.GetWidth()  / totalPages )  ;
		if ((nrPage < 0) || (nrPage >= totalPages))
			return;
		int nrEvent;
		nrEvent = xmlCompile->pageToEventNr(nrPage);
		if (nrEvent == -1)
			return;
		prevPos = -1;
		prevPlaying = true;
		//currentPageNr = nrPage;	
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
std::uint64_t musicxmlscore::crc_cumulate_file(wxString fname)
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
std::uint64_t musicxmlscore::crc_cumulate_string(wxString s)
{
	// cumulate the 64bits CRC of the string
	crc_cumulate(s.char_str(),s.Len());
	return crc_value ;
}
bool musicxmlscore::newLayout(wxSize sizeClient)
{
	//// NOP
	return false;

	if (!isOk())
		return false;
	wxBusyCursor waitcursor;
	mlog_in("newLayout file %s" , (const char*)(xmlName.GetFullPath().c_str()));
	wxFileName fm;
	wxString xmlout, lilyscore, pythonexe, pythonscript,lilyexe, lilysetting, lilypresetting , lilylog , lilybar ;
	wxString command_xmltolily , command_lilytopng;
	wxString overridexpr;
	long lexec;
	wxTextFile fin, fout;


	// window big enough ?
	int xs = sizeClient.GetWidth();
	int ys = sizeClient.GetHeight();
	if ((xs < SIZECLIENTMINWIDTH) || (ys < SIZECLIENTMINHEIGHT))
	{
		wxMessageBox("Window too small for music display", "build score", wxOK | wxICON_ERROR);
		return false;
	}
	
	// XML score to display
	fm.SetPath(getTmpDir());
	fm.SetFullName(FILE_OUT_XML);
	xmlout = fm.GetFullPath();
	//  file to store lilypond position of notes
	fm.SetFullName(FILE_POS_LILY);
	lilypos = fm.GetFullPath();
	if (fm.Exists())
		wxRemoveFile(lilypos);
	//  file to store position of notes
	fm.SetFullName(FILE_POS_TXT);
	expresseurpos = fm.GetFullPath();
	if (fm.Exists())
		wxRemoveFile(expresseurpos);
	// file to translate xml to lilypond
	fm.SetFullName(FILE_OUT_LILY);
	lilyscore = fm.GetFullPath();
	if (fm.Exists())
		wxRemoveFile(lilyscore);
	// files to adapt lilypond xml translation
	fm.SetFullName(FILE_SRC_LILY);
	lilysrc = fm.GetFullPath();
	if (fm.Exists())
		wxRemoveFile(lilysrc);
	// log file 
	fm.SetFullName(FILE_LOG_LILY);
	lilylog = fm.GetFullPath();
	if (fm.Exists())
		wxRemoveFile(lilylog);
	// barres de mesure
	fm.SetFullName(FILE_BAR_LILY);
	lilybar = fm.GetFullPath();
	// settings for lilypond
	fm.SetFullName(FILE_OUT_SETLILY);
	lilysetting = fm.GetFullPath();
	if (fm.Exists())
		wxRemoveFile(lilysetting);
	fm.SetPath(getCwdDir());
	// presettings for lilypond
	fm.SetFullName(FILE_IN_PRESETLILY);
	lilypresetting = fm.GetFullPath();
#ifdef RUN_MAC
	// liliy translator 
	fm.SetPath(getCwdDir());
	fm.AppendDir("lilypond-2.25.28");
	fm.AppendDir("libexec");
	fm.SetFullName("musicxml2ly");
	pythonscript = fm.GetFullPath();
	// lilypond bin
	fm.SetPath(getCwdDir());
	fm.AppendDir("lilypond-2.25.28");
	fm.AppendDir("bin");
	fm.SetFullName("lilypond");
	lilyexe = fm.GetFullPath();
#else
#ifdef _DEBUG
	fm.SetPath("C:\\Users\\franc\\Documents\\lilypond\\lilypond-2.25.28");
#else
	fm.SetPath(getCwdDir());
	fm.AppendDir("lilypond");
#endif
	fm.AppendDir("bin");
	fm.SetFullName("python.exe");
	pythonexe = fm.GetFullPath();
	// liliy translator 
	fm.SetFullName("musicxml2ly.py");
	pythonscript = pythonexe + "\" \"" + fm.GetFullPath();
	// lilypond bin
	fm.SetFullName("lilypond.exe");
	lilyexe = fm.GetFullPath();
#endif

	// calculation for the page layout
	xmlCompile->isModified = false ;

	sizePage.SetWidth( sizeClient.GetX() );
	sizePage.SetHeight( sizeClient.GetY() );

	// clean useless temp files
	// cleanTmp();


	// create the musicXml file to display
	xmlCompile->music_xml_displayed_file = xmlout;
	xmlCompile->compiled_score->write(xmlCompile->music_xml_displayed_file);

	// adapt lilypond settings
	fin.Open(lilypresetting);
	fout.Create(lilysetting);
	fout.Open(lilysetting);
	if (fin.IsOpened() && fout.IsOpened())
	{
		wxString str;
		int mzoom;
		mzoom = configGet(CONFIG_ZOOM_MUSICXML, 0);
		if (mzoom < -3) mzoom = -3;
		if (mzoom > 3 ) mzoom = 3;
		int nrzoom = -3;
		for (str = fin.GetFirstLine(); !fin.Eof(); str = fin.GetNextLine())
		{
			if (str.StartsWith("%%%%%%%%override Expresseur:"))
			{
				overridexpr = str.Mid(strlen("%%%%%%%%override Expresseur:"));
			}
			if (str.StartsWith("%%%%%%%%translate_xml_to_ly:"))
			{
				command_xmltolily.Printf(str, pythonscript, FILE_OUT_LILY , FILE_OUT_XML);
				fout.AddLine(command_xmltolily);
				command_xmltolily.Replace("%%%%translate_xml_to_ly:", "");
				continue;
			}
			if (str.StartsWith("%%%%%%%%translate_ly_to_png:"))
			{
				command_lilytopng.Printf(str, lilyexe, FILE_LOG_LILY, FILE_OUT_SETLILY, FILE_SRC_LILY );
				fout.AddLine(command_lilytopng);
				command_lilytopng.Replace("%%%%translate_ly_to_png:", "");
				continue;
			}
			if (str.StartsWith("#(set-global-staff-size"))
			{
				if (nrzoom == mzoom)
					fout.AddLine(str);
				nrzoom++;
				continue;
			}
			if (str.StartsWith("#(set! paper-alist"))
			{
				wxString s1;
				s1.Printf(str, sizePage.GetWidth() , sizePage.GetHeight());
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
		wxString serr;
		serr.Printf("Cannot adapt Lilypond settings from %s", FILE_IN_PRESETLILY);
		wxMessageBox(serr, "build score", wxOK | wxICON_ERROR);
		return false;
	}

	bool alreadyAvailable = false;

	// calculate the CRC of this display
	crc_init();
	crc_cumulate_file(xmlout);
	crc_cumulate_file(lilysetting);
	crc_cumulate_file(lilybar);
	crc_cumulate_string(command_xmltolily);
	crc_cumulate_string(command_lilytopng);
	wxString prefix_cache ;
	prefix_cache.Printf("%s_%llu_", PREFIX_CACHE, crc_value);

	wxFileName poscache;
	poscache.SetPath(getTmpDir());
	poscache.SetFullName( prefix_cache + FILE_POS_TXT);
	if (poscache.FileExists())
	{
		// Lilypond result already available in cache. Let's reuse it
		alreadyAvailable = true ;
		if ( ! wxCopyFile(poscache.GetFullPath(), expresseurpos) ) alreadyAvailable = false ;
		// copy pages
		wxFileName fsource;
		fsource.SetPath(getTmpDir());
		wxFileName fdest;
		fdest.SetPath(getTmpDir());
		wxString ffn;
		int pp = 1;
		while (true)
		{
			ffn.Printf(FILE_SCORE_PNG, pp);
			fsource.SetFullName( prefix_cache + ffn );
			if (!fsource.IsFileReadable())
				break;
			fdest.SetFullName(ffn);
			if ( ! wxCopyFile(fsource.GetFullPath(),fdest.GetFullPath()) ) alreadyAvailable = false ;
			pp++;
		}
	}

	if ( alreadyAvailable)
	{ 
		mlog_in("Score pages : read from cache");
		((wxFrame *)mParent)->SetStatusText("Score pages : read from cache",0);
	}
	else
	{
		((wxFrame *)mParent)->SetStatusText("Score pages : computation by Lilypond in progress", 0);

		wxExecuteEnv execenv;
		execenv.cwd = getTmpDir();
		wxArrayString stdouput;
		wxArrayString stderror;
		mlog_in((const char*)(command_xmltolily.c_str()));
		lexec = wxExecute(command_xmltolily, stdouput , stderror , wxEXEC_SYNC | wxEXEC_HIDE_CONSOLE  , &execenv);
		if (lexec < 0)
		{
			wxMessageBox("Cannot translate XML to Lilypond", "build score", wxOK | wxICON_ERROR);
			mlog_in("Cannot translate XML to Lilypond");
			return false;
		}
		if ( stdouput.GetCount() > 0 )
			mlog_in("stdout translate XML to Lilypond");
		for (size_t i = 0; i < stdouput.GetCount(); ++i)
		{
			mlog_in("%s", (const char*)(stdouput[i].c_str()));
		}
		if ( stderror.GetCount() > 0 )
			mlog_in("stderror translate XML to Lilypond");
		for (size_t i = 0; i < stderror.GetCount(); ++i)
		{
			mlog_in("%s", (const char*)(stderror[i].c_str()));
		}
		fin.Open(lilyscore);
		fout.Create(lilysrc);
		fout.Open(lilysrc);
		bool expresseurstaff = false;
		int score = 0;
		bool firstf8 = true;
		if (fin.IsOpened() && fout.IsOpened())
		{
			wxString str;
			for (str = fin.GetFirstLine(); !fin.Eof(); str = fin.GetNextLine())
			{
				if (str.StartsWith("\\version"))
				{
					// rajout des barres de mesure
					wxTextFile fbar;
					fbar.Open(lilybar);
					if (fbar.IsOpened())
					{
						wxString sbar;
						for (sbar = fbar.GetFirstLine(); !fbar.Eof(); sbar = fbar.GetNextLine())
						{
							fout.AddLine(sbar);
						}
						fbar.Close();
					}
				}
				if (str.Contains("PartExpresseurMusicXMLPartIdVoiceTwo"))
				{
					// fin de la partie expresseur
					expresseurstaff = false;
				}
				if (str.StartsWith("\\pointAndClickOff"))
						continue;
				if (str.StartsWith("PartExpresseurMusicXMLPartIdVoiceOne"))
				{
					expresseurstaff = true;
				}
				if (str.StartsWith("\\score"))
				{
					expresseurstaff = false;
				}
				if (expresseurstaff)
				{
					if (str.Contains("staffLines \"treble\" 2"))
					{
						str.Replace("staffLines \"treble\" 2", "staffLines \"percussion\" 2");
					}
					if (firstf8)
					{
						if (str.Contains("\\once \\stemUp f"))
						{
							str.Replace("\\once \\stemUp f", overridexpr + " f", false);
							firstf8 = false;
						}
					}
					if (str.Contains("\\once \\stemUp f"))
					{
						str.Replace("\\once \\stemUp f", "f", true);
						firstf8 = false;
					}
					fout.AddLine(str);
					continue;
				}
				else
				{
					str.Replace("\\stemUp", "");
					str.Replace("\\stemDown", "");
				}
				switch (score)
				{
				case 0: 
					if (str.StartsWith("\\score"))
						score = 1;
					break;
				case 1:
					if (str.Contains("\\PartExpresseurMusicXMLPartIdVoiceOne"))
						score = 2;
					break;
				case 2:
					if (str.Contains("}"))
						score = 3;
					break;
				case 3:
					fout.AddLine("\\context Voice = ""barmarks"" { \\barmarks }");
					score = 4;
					break;
				default: break;
				}
				fout.AddLine(str);
			}
			fin.Close();
			fout.Write();
			fout.Close();
		}
		else
		{
			wxString serr;
			serr.Printf("Cannot adapt Lilypond score from %s", lilyscore);
			wxMessageBox(serr, "build score", wxOK | wxICON_ERROR);
			return false;
		}

		// run the lilypond batch to build he pages and the positions
		mlog_in((const char*)(command_lilytopng.c_str()));
		lexec = wxExecute(command_lilytopng, stdouput, stderror, wxEXEC_SYNC | wxEXEC_HIDE_CONSOLE, &execenv);
		if (lexec < 0)
		{
			wxMessageBox("Cannot translate Lilypond to PNG", "build score", wxOK | wxICON_ERROR);
			mlog_in("Cannot translate Lilypond to PNG : %s\n", (const char*)(command_lilytopng.c_str()));
			return false;
		}
		if (stdouput.GetCount() > 0)
			mlog_in("stdout translate Lilypond to PNG");
		for (size_t i = 0; i < stdouput.GetCount(); ++i)
		{
			mlog_in("%s", (const char*)(stdouput[i].c_str()));
		}
		if (stderror.GetCount() > 0)
			mlog_in("stderror translate Lilypond to PNG");
		for (size_t i = 0; i < stderror.GetCount(); ++i)
		{
			mlog_in("%s", (const char*)(stderror[i].c_str()));
		}
		
		readlilypond();

		((wxFrame *)mParent)->SetStatusText("Score pages : stored in cache", 0 );

		// cache this result for potential reuse
		wxCopyFile(expresseurpos , poscache.GetFullPath());

		// copy pages in CACHE
		wxFileName fsource;
		fsource.SetPath(getTmpDir());
		wxFileName fdest;
		fdest.SetPath(getTmpDir());
		wxString ffn;
		int pp = 1;
		while (true)
		{
			ffn.Printf(FILE_SCORE_PNG, pp);
			fsource.SetFullName(ffn);
			if (!fsource.IsFileReadable())
				break;
			fdest.SetFullName(prefix_cache + ffn);
			wxCopyFile(fsource.GetFullPath(), fdest.GetFullPath());
			pp++;
		}
	}
		
	readPos();
	currentPageNr = -1;
	currentPageNrPartial = -1;
	prevPos = -1 ;
	prevPlaying = true ;
	return true ;
}
bool musicxmlscore::readlilypos()
{
	// detect line column of tokens \once in expressur_out.ly 
	// each token is added in lposnotes array

	mlog_in("Read Lilypond index positions from %s", (const char*)(lilysrc.c_str()));

	char ch;
	char chexpr[] = "PartExpresseurMusicXMLPartIdVoiceOne = \\relative ";
	uint32_t etat = 0;
	uint32_t nrline = 0; // line number in the file
	uint32_t nrcolumn = 0; // column number in the file
	uint32_t memnrline = 0; // line number in the file
	uint32_t memnrcolumn = 0; // column number in the file

	int nbwaitf8 = 0;
	cposnote mposnote;
	mposnote.ply.line = 0; // column

	int fp = open(lilysrc, O_RDONLY);
	if (fp == -1)
	{
		wxMessageBox("Cannot open Lilypond source file expresseur_out.ly", "build score", wxOK | wxICON_ERROR);
		mlog_in("Error reading Lilypond index positions from %s", (const char*)(lilysrc.c_str()));
		return false;
	}
	etat = 0;
	while ((read(fp, &ch, 1)) > 0)
	{
		switch (ch) 
		{
		case '\n' : 
			nrline++;
			nrcolumn = 0;
			continue;
		case '\r':
			// ignore \r
			continue;
		default:
			// normal character
			nrcolumn++;
			break;
		}


		switch (etat)
		{
		default: 
			if (ch == chexpr[etat])
			{
				if (etat == (strlen(chexpr) - 1))
					etat = 1000;
				else
					etat++;
			}
			  else
				etat = 0 ;
			break;
		case 1000: 
			if (ch == 'f')
			{
				memnrline = nrline+1;
				memnrcolumn = nrcolumn - 1; // -1 because of the f
				etat++;
				break;
			}
			if (ch == '\\')
			{
				memnrline = nrline+1;
				memnrcolumn = nrcolumn - 1; // -1 because of the f
				etat = 2000;
				break;
			}
			etat = 1000;	
			break;
		case 1001: 
			if ((ch >= '0') && (ch <= '9'))
			{
				mposnote.ply.line = memnrline;
				mposnote.ply.column = memnrcolumn;
				mposnote.empty = true;
				lposnotes.push_back(cposnote(mposnote));
				if (logLilypond)
					mlog_in("  l=%d c=%d", mposnote.ply.line, mposnote.ply.column);	
			}
			etat = 1000; 
			break;
		case 2000: if (ch == 't') etat++; else etat = 1000; break;
		case 2001: if (ch == 'w') etat++; else etat = 1000; break;
		case 2002: if (ch == 'e') etat++; else etat = 1000; break;
		case 2003: if (ch == 'a') etat++; else etat = 1000; break;
		case 2004: 
			if (ch == 'k')
			{
				nbwaitf8 = 0;
				etat++;
			}
			else 
				etat = 1000; 
			break;
		case 2005: 
			if (ch == ' ')
			{
				etat ++;
				break;
			}
			if (nbwaitf8++ > 50)
			{
				// too many wait for f8. Reset the state
				nbwaitf8 = 0;
				etat = 1000;
				break;
			}
			break;
		case 2006:
			if (ch == 'f')
			{
				etat++;
				break;
			}
			etat = 2005;
			break;
		case 2007:
			if ((ch >= '0') && (ch <= '9'))
			{
				mposnote.ply.line = memnrline;
				mposnote.ply.column = memnrcolumn;
				mposnote.empty = true;
				lposnotes.push_back(cposnote(mposnote));
				if (logLilypond)
					mlog_in("  l=%d c=%d", mposnote.ply.line , mposnote.ply.column);
				etat = 1000;
				break;
			}
			etat = 2005;
			break;
		}
	}
	close(fp);
	return true;
}
bool musicxmlscore::readlilypdf(uint32_t page, uint32_t xpng, uint32_t ypng)
{
	// detect structure "Rect [402.632 226.067 410.763 232.778]*.ly:128:5:6)" for the notes
	// and structure "MediaBox [0 0 798 598]" for the size of the page
	char ch;
	uint32_t etat = 0;
	uint32_t nbint = 0 ;
	int i = 0;
	sfpos spdf, pagepdf;
	float fletat = 0.1 ;
	float nbfloat = 0.0 ;
	sposly mly;
	mly.column = 0; mly.line = 0;
	spdf.x1 = 0; spdf.y1 = 0; spdf.x2 = 0; spdf.y2 = 0;
	pagepdf.x1 = 0; pagepdf.y1 = 0; pagepdf.x2 = 0; pagepdf.y2 = 0;

	wxFileName fpdf;
	fpdf.SetPath(getTmpDir());
	wxString s;
	s.Printf(FILE_SCORE_PDF, page + 1);
	//s.Printf(FILE_SCORE_PDF, 0);
	fpdf.SetFullName(s);
#ifdef RUN_WIN
	if (logLilypond)
		mlog_in("Read Lilypond PDF  try _open _O_RDONLY | _O_BINARY");
	int fp = _open(fpdf.GetFullPath(), _O_RDONLY | _O_BINARY);
#else
	if (logLilypond)
		mlog_in("Read Lilypond PDF  try open _O_RDONLY");
	int fp = open(fpdf.GetFullPath(), O_RDONLY);
#endif
	if (fp == -1)
	{
		return false;
	}
	if (logLilypond)
		mlog_in("Read Lilypond PDF positions from %s", (const char*)(fpdf.GetFullPath().c_str()));
	etat = 0;
#ifdef RUN_WIN
	while ((_read(fp, &ch, 1)) > 0)
#else
	while ((read(fp, &ch, 1)) > 0)
#endif
	{
		switch (etat)
		{
		case 0: 
			if (ch == 'R') 
				etat++; 
			else 
			{ 
				if (ch == 'M') 
					etat = 101; 
				else etat = 0; 
			}
			break;
		// seach for a small PDF rectangle sie. Pattern is "Rect [x1 y1 x2 y2]" 
		case 1: if (ch == 'e') etat++; else etat = 0; break;
		case 2: if (ch == 'c') etat++; else etat = 0; break;
		case 3: if (ch == 't') etat++; else etat = 0; break;
		case 4: if (ch == ' ') etat++; else etat = 0; break;
		case 5: if (ch == '[') { etat++; nbfloat = 0.0; }
			  else etat = 0; break;
			// x1_
		case 6: if ((ch >= '0') && (ch <= '9')) { nbfloat = nbfloat * 10.0 + (float)(ch - '0'); break; }
			  if (ch == '.') { fletat = 0.1; etat++; break; }
			  if (ch == ' ') { etat += 2; spdf.x1 = nbfloat; nbfloat = 0.0; break; }
			  etat = 0; break;
		case 7: if ((ch >= '0') && (ch <= '9')) { nbfloat += (float)(ch - '0') * fletat; fletat *= 0.1; break; }
			  if (ch == ' ') { etat++; spdf.x1 = nbfloat; nbfloat = 0.0; break; }
			  etat = 0; break;
			  // y1_
		case 8: if ((ch >= '0') && (ch <= '9')) { nbfloat = nbfloat * 10.0 + (float)(ch - '0'); break; }
			  if (ch == '.') { fletat = 0.1; etat++; break; }
			  if (ch == ' ') { etat += 2; spdf.y1 = nbfloat; nbfloat = 0.0; break; }
			  etat = 0; break;
		case 9: if ((ch >= '0') && (ch <= '9')) { nbfloat += (ch - '0') * fletat; fletat *= 0.1; break; }
			  if (ch == ' ') { etat++; spdf.y1 = nbfloat; nbfloat = 0.0; break; }
			  etat = 0; break;
			  // x2_
		case 10: if ((ch >= '0') && (ch <= '9')) { nbfloat = nbfloat * 10.0 + (float)(ch - '0'); break; }
			   if (ch == '.') { fletat = 0.1; etat++; break; }
			   if (ch == ' ') { etat += 2; spdf.x2 = nbfloat; nbfloat = 0.0; break; }
			   etat = 0; break;
		case 11: if ((ch >= '0') && (ch <= '9')) { nbfloat += (ch - '0') * fletat; fletat *= 0.1; break; }
			   if (ch == ' ') { etat++; spdf.x2 = nbfloat; nbfloat = 0.0; break; }
			   etat = 0; break;
			   // y2]
		case 12: if ((ch >= '0') && (ch <= '9')) { nbfloat = nbfloat * 10.0 + (float)(ch - '0'); break; }
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
		case 18: if ((ch >= '0') && (ch <= '9')) { nbint = nbint * 10 + (int)(ch - '0'); break; }
			   if (ch == ':') { etat++; mly.line = nbint; nbint = 0; break; }
			   etat = 0; break;
		case 19: if ((ch >= '0') && (ch <= '9')) { nbint = nbint * 10 + (int)(ch - '0'); break; }
			   if (ch == ':') { etat++; mly.column = nbint; nbint = 0; break; }
			   etat = 0; break;
		case 20: if ((ch >= '0') && (ch <= '9')) { nbint = nbint * 10 + (int)(ch - '0'); break; }
			   if (ch == ')') { etat++; nbint = 0; break; }
			   etat = 0; break;
			   // end Rect [402.632 226.067 410.763 232.778]....ly:128:5:6)
		case 21:
			etat = 0;
			if (logLilypond)
				mlog_in("  PDF rect x1=%f y1=%f x2=%f y2=%f for l=%d c=%d", spdf.x1, spdf.y1, spdf.x2, spdf.y2, mly.line, mly.column);
			// search this PDF object rectangle in the posnotes generated by liypond script
			i = -1;
			for (auto & current_posnotes : lposnotes )
			{
				i++;
				if ((current_posnotes.empty) && (mly.line == current_posnotes.ply.line) && (mly.column == current_posnotes.ply.column))
				{
					current_posnotes.empty = false;
					current_posnotes.page = page;
					current_posnotes.pdf.x1 = spdf.x1 ;
					current_posnotes.pdf.y1 = spdf.y1;
					current_posnotes.pdf.x2 = spdf.x2;
					current_posnotes.pdf.y2 = spdf.y2;
					if (logLilypond)
						mlog_in("  lposnotes %d : page=%d x1=%f y1=%f x2=%f y2=%f", i , current_posnotes.page, current_posnotes.pdf.x1, current_posnotes.pdf.y1, current_posnotes.pdf.x2, current_posnotes.pdf.y2);
				}
			}
			break;
	    // size of the image-page in the pdf, pattern "MediaBox [0 0 798 598]"
		case 101: if (ch == 'e') etat++; else etat = 0; break;
		case 102: if (ch == 'd') etat++; else etat = 0; break;
		case 103: if (ch == 'i') etat++; else etat = 0; break;
		case 104: if (ch == 'a') etat++; else etat = 0; break;
		case 105: if (ch == 'B') etat++; else etat = 0; break;
		case 106: if (ch == 'o') etat++; else etat = 0; break;
		case 107: if (ch == 'x') etat++; else etat = 0; break;
		case 108: if (ch == ' ') etat++; else etat = 0; break;
		case 109: if (ch == '[') { etat++; nbint = 0; }
				else etat = 0; break;
		case 110: if ((ch >= '0') && (ch <= '9')) { nbint = nbint * 10 + (int)(ch - '0'); break; }
				if (ch == ' ') { etat++; pagepdf.x1 = nbint; nbint = 0; break; }
				etat = 0; break;
		case 111: if ((ch >= '0') && (ch <= '9')) { nbint = nbint * 10 + (int)(ch - '0'); break; }
				if (ch == ' ') { etat++; pagepdf.y1 = nbint; break; }
				etat = 0; break;
		case 112: if ((ch >= '0') && (ch <= '9')) { nbint = nbint * 10 + (int)(ch - '0'); break; }
				if (ch == ' ') { etat++; pagepdf.x2 = nbint; nbint = 0; break; }
				etat = 0; break;
		case 113: if ((ch >= '0') && (ch <= '9')) { nbint = nbint * 10 + (int)(ch - '0'); break; }
				if (ch == ']') { etat = 0; pagepdf.y2 = nbint  ; break; }
				etat = 0; break;
		default: etat = 0; break;
		}
	}
#ifdef RUN_WIN
	_close(fp);
#else
	close(fp);
#endif
	if (pagepdf.x2 == 0)
	{
		wxMessageBox("Cannot read size Lilypond pdf", "build score", wxOK | wxICON_ERROR);
		mlog_in("Error Lilypond PDF analyse size");
		return false;
	}
	float fxpng_pdf = (float)(xpng) / (float)(pagepdf.x2 - pagepdf.x1);
	float fypng_pdf = (float)(ypng) / (float)(pagepdf.y2 - pagepdf.y1);
	// readjust the size of pdf rectangle in the page to png rectangle 
	for (auto & current_posnotes : lposnotes )
	{
		if ((! current_posnotes.empty) && (page == current_posnotes.page))
		{
			current_posnotes.png.x1 = (int)(fxpng_pdf *current_posnotes.pdf.x1);
			current_posnotes.png.x2 = (int)(fxpng_pdf *current_posnotes.pdf.x2);
			current_posnotes.png.y1 = pagepdf.y2 - pagepdf.y1 - (int)(fypng_pdf *current_posnotes.pdf.y2);
			current_posnotes.png.y2 = (pagepdf.y2 - pagepdf.y1) - (int)(fypng_pdf * current_posnotes.pdf.y1);
			if (logLilypond)
				mlog_in("  p=%d x1=%d y1=%d x2=%d y2=%d", current_posnotes.page, current_posnotes.png.x1, current_posnotes.png.y1, current_posnotes.png.x2, current_posnotes.png.y2);
		}
	}

	return true;
}
bool musicxmlscore::readpngsize(uint32_t* xpng, uint32_t* ypng)
{
	// read size png
	*xpng = 0; *ypng = 0;
	wxFileName fpng;
	fpng.SetPath(getTmpDir());
	wxString s;
	s.Printf(FILE_SCORE_PNG, (0+1));
	fpng.SetFullName(s);
	if (logLilypond)
		mlog_in("Read Lilypond PNG size from %s", (const char*)(fpng.GetFullPath().c_str()));
	FILE* fp = fopen(fpng.GetFullPath(), "rb");
	if (fp == NULL)
	{
		wxMessageBox("Cannot open Lilypond png", "build score", wxOK | wxICON_ERROR);
		mlog_in("Error Lilypond PNG size from %s", (const char*)(fpng.GetFullPath().c_str()));
		return false;
	}
	fseek(fp, 16, SEEK_SET);  // Move to the IHDR chunk's location
	fread(xpng, sizeof(uint32_t), 1, fp);
	fread(ypng, sizeof(uint32_t), 1, fp);

	// Convert from big-endian to host-endian
	*xpng = bswap32(*xpng);
	*ypng = bswap32(*ypng);
	if (logLilypond)
		mlog_in("   x=%d pixels , y=%d pixels ", *xpng , *ypng );

	fclose(fp);
	return true;
}
bool musicxmlscore::readlilypond()
{
	uint32_t xpng, ypng;
	if (!readpngsize(&xpng, &ypng))
	{
		// no png score
		wxMessageBox("no Lilypond png", "build score", wxOK | wxICON_ERROR); 
		return false;
	}

	lposnotes.clear();
	if (!readlilypos())
	{
		// no pos file
		wxMessageBox("no Lilypond pos", "build score", wxOK | wxICON_ERROR);
		return false;
	}

	uint32_t pagenr = 0;
	while (true)
	{

		if (!readlilypdf(pagenr,  xpng,  ypng))
		{
			// no more pdf page
			break;
		}
		pagenr++;
	}

	wxTextFile fout;
	if (logLilypond)
		mlog_in("Write Expresseur PNG positions in %s", (const char*)(expresseurpos.c_str()));
	if (logLilypond)
		mlog_in("  p:<page>:<x1>:<y1>:<x2>:<y2>");
	fout.Create(expresseurpos);
	for (auto & current_posnotes : lposnotes )
	{
		wxString s;
		s.Printf("%c:%u:%u:%u:%u:%u:", current_posnotes.empty?'?':'p' , current_posnotes.page, current_posnotes.png.x1, current_posnotes.png.y1, current_posnotes.png.x2, current_posnotes.png.y2);
		if (logLilypond)
			mlog_in("  %s", (const char*)(s.c_str()));
		fout.AddLine(s);
	}
	fout.Write();
	fout.Close();

	return true;
}


bool musicxmlscore::readPos()
{

	int fp = open(expresseurpos, O_RDONLY);
	if (fp == -1)
	{
		wxMessageBox("Cannot open expresseur pos", "build score", wxOK | wxICON_ERROR);
		return false;
	}
	int etat = 0;
	int nbint = 0;
	char ch;
	wxRect rect;
	std::vector <int> measureTurnPage;
	int nrpage = 0;
	int nrmeasure = 0 ;
	int nrnote = 0;
	int previous_nrpage = -1;
	while ((read(fp, &ch, 1)) > 0)
	{
		switch (etat)
		{
		// p:
		case 0: 
			if (ch == 'p') { etat++; break; }
			break;
		case 1: 
			if (ch == ':') {etat++; nbint = 0; break;}
			etat = 0; 	break;
			// nrpage:x1:y1:x2:y2:
		case 2: 
			  if ((ch >= '0') && (ch <= '9')) { nbint = nbint * 10 + (int)(ch - '0'); break; }
			  if (ch == ':') { etat++; nrpage = nbint; nbint = 0; break; }
			  etat = 0; break;
		case 3: if ((ch >= '0') && (ch <= '9')) { nbint = nbint * 10 + (int)(ch - '0'); break; }
			  if (ch == ':') { etat++; rect.x = nbint; nbint = 0; break; }
			  etat = 0; break;
		case 4: if ((ch >= '0') && (ch <= '9')) { nbint = nbint * 10 + (int)(ch - '0'); break; }
			  if (ch == ':') { etat++; rect.y = nbint; nbint = 0; break; }
			  etat = 0; break;
		case 5: if ((ch >= '0') && (ch <= '9')) { nbint = nbint * 10 + (int)(ch - '0'); break; }
			  if (ch == ':') { etat++; rect.width = nbint - rect.x ; nbint = 0; break; }
			  etat = 0; break;
		case 6: if ((ch >= '0') && (ch <= '9')) { nbint = nbint * 10 + (int)(ch - '0'); break; }
			  if (ch == ':') { etat ++; rect.height = nbint - rect.y ; nbint = 0; break; }
			  etat = 0; break;
		case 7:
			rect.y += 1.2 * rect.height;
			rect.height /= 2;
			nrmeasure = xmlCompile->setPosEvent(nrnote , nrpage , rect);
			nrnote++;	
			if (nrpage != previous_nrpage)
			{
				measureTurnPage.push_back(nrmeasure - 1);
			}
			previous_nrpage = nrpage;
			totalPages = nrpage + 1;
			etat = 0; break;
		default: etat = 0; break;
		}
	}
	close(fp);

	xmlCompile->setMeasureTurnEvent(0,true);
	for (auto & nrMeasureTurn : measureTurnPage ) //int nrTurn = 0; nrTurn < nbTurn; nrTurn++)
	{
		xmlCompile->setMeasureTurnEvent(nrMeasureTurn);
	}
	return true;
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

