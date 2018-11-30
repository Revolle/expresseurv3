/////////////////////////////////////////////////////////////////////////////
// Name:        bitmapscore.cpp
// Purpose:     display a bitmap of the score /  expresseur V3
// Author:      Franck REVOLLE
// Modified by:
// Created:     27/07/2015
// update : 15/11/2016 18:00
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


#include "wx/dialog.h"
#include "wx/filename.h"
#include "wx/stattext.h"
#include "wx/sizer.h"
#include "wx/bitmap.h"
#include "wx/tglbtn.h"
#include "wx/spinctrl.h"
#include "wx/textfile.h"
#include "wx/listctrl.h"
#include "wx/valgen.h"
#include "wx/listbox.h"
#include "wx/tokenzr.h"
#include "wx/config.h"
#include "wx/dcclient.h"
#include "wx/msgdlg.h"
#include "wx/image.h"
#include "wx/filehistory.h"

#include "global.h"
#include "basslua.h"
#include "mxconf.h"
#include "viewerscore.h"
#include "expression.h"
#include "bitmapscore.h"

wxBEGIN_EVENT_TABLE(bitmapscore, wxPanel)
EVT_PAINT(bitmapscore::onPaint)
EVT_LEFT_DOWN(bitmapscore::OnLeftDown)
EVT_LEFT_UP(bitmapscore::OnLeftUp)
EVT_MOUSE_EVENTS(bitmapscore::OnMouse)
wxEND_EVENT_TABLE()

bitmapscore::bitmapscore(wxWindow *parent, wxWindowID id, mxconf* lMxconf)
: viewerscore(parent, id)
{
	mParent = parent;
	mConf = lMxconf;
	currentDC = NULL;
	mPointStart = wxDefaultPosition;
	alertSetRect = true;
	selectedRect.SetWidth(0);
	nbRectChord = 0;
	nrChord = -1; 
	for (int i = 0; i < MAX_RECTCHORD; i++)
	{
		rectChord[i].SetX(0);
		rectChord[i].SetY(0);
		rectChord[i].SetWidth(0);
		rectChord[i].SetHeight(0);
	}
	xScale = 1.0;
	yScale = 1.0;
	prevNrChord = -1;
	prevPaintNrChord = -1;
}
bitmapscore::~bitmapscore()
{
	if (currentDC)
		delete currentDC;
}
bool bitmapscore::isOk()
{
	return(filename.IsOk());
}
bool bitmapscore::setFile(const wxFileName &lfilename)
{
	filename = lfilename;
	// load the image
	filename.SetExt(SUFFIXE_BITMAPCHORD);
	if (filename.FileExists() && filename.IsFileReadable())
	{
		fileRectChord = filename;
		fileRectChord.SetExt("txb");
		return true;
	}
	filename.Clear();
	return false;

}
bool bitmapscore::displayFile(wxSize sizeClient)
{
	if (!isOk()) return false;
	newLayout(sizeClient);
	return true;
}
int bitmapscore::getTrackCount()
{
	return 0;
}
wxString bitmapscore::getTrackName(int WXUNUSED(trackNr))
{
	return wxEmptyString;
}
bool bitmapscore::newLayout(wxSize sizeClient)
{
	if (!isOk()) return false;

	sizePage = sizeClient;
	fileInDC.Clear();

	// load the rect linked to the chords
	readRectChord();

	prevNrChord = -1;
	prevPaintNrChord = -1;
	return true;

}
bool bitmapscore::setPage()
{
	if (fileInDC == filename)
		return true;

	sizePage = this->GetSize();

	if ((sizePage.GetHeight() < 100) || (sizePage.GetWidth() < 100))
		return false;

	wxImage mImage(filename.GetFullPath(), wxBITMAP_TYPE_PNG);
	if (!mImage.IsOk()) return false;

	wxSize sizeDisplay;
	wxSize wxSizeImage = mImage.GetSize();

	sizeDisplay.SetWidth(sizePage.GetWidth());
	sizeDisplay.SetHeight((wxSizeImage.GetHeight()*sizePage.GetWidth()) / wxSizeImage.GetWidth());
	if (sizeDisplay.GetHeight() > sizePage.GetHeight())
	{
		sizeDisplay.SetHeight(sizePage.GetHeight());
		sizeDisplay.SetWidth((wxSizeImage.GetWidth()*sizePage.GetHeight()) / wxSizeImage.GetHeight());
	}
	xScale = (double)(sizeDisplay.GetWidth()) / (double)(wxSizeImage.GetWidth());
	yScale = (double)(sizeDisplay.GetHeight()) / (double)(wxSizeImage.GetHeight());

	wxBitmap emptyBitmap(sizePage);
	if (currentDC)
		delete currentDC;
	currentDC = new wxMemoryDC(emptyBitmap);
	if (!currentDC->IsOk())
	{
		delete currentDC;
		return false;
	}
	currentDC->SetBackground(this->GetBackgroundColour());
	currentDC->Clear();

	wxImage rimage = mImage.Scale(sizeDisplay.GetWidth(), sizeDisplay.GetHeight() , wxIMAGE_QUALITY_HIGH);
	wxBitmap mBitmap(rimage);
	if (! mBitmap.IsOk()) return false;

	currentDC->DrawBitmap(mBitmap,0,0);
	if ( ! currentDC->IsOk() )
	{
		delete currentDC;
		return false;
	}

	fileInDC = filename;
	prevNrChord = -1;
	prevPaintNrChord = -1;

	return true;
}
void bitmapscore::setCursor(wxDC& dc, int pos)
{
	sizePage = this->GetSize();
	
	if ( ! setPage()) return ;

	// redraw the page(s)
	dc.Clear();
	dc.Blit(0, 0, sizePage.GetWidth(), sizePage.GetHeight(), currentDC, 0, 0);

	// draw the cursor
	if (!rectChord[pos].IsEmpty())
	{
		wxRect mrect , nrect;
		nrect = rectChord[pos];
		// scale the rectangle
		mrect.SetX(nrect.GetX() * xScale);
		mrect.SetY(nrect.GetY() * yScale);
		mrect.SetWidth(nrect.GetWidth() * xScale);
		mrect.SetHeight(nrect.GetHeight() * yScale);
		// underscore of the rectangle
		mrect.SetY(mrect.GetY() + mrect.GetHeight());
		mrect.SetHeight(5);
		// clip on the underscore and clear(paint)
		wxDCClipper clip(dc, mrect);
		dc.SetBackground(*wxRED_BRUSH);
		dc.Clear();
	}
}
void bitmapscore::setPosition(int pos, bool WXUNUSED( playing))
{
	if (!isOk()) return;

	nrChord = pos - 1 ;

	// onIdle : set the current pos
	newPaintNrChord = nrChord;
	if (nrChord != prevNrChord)
	{
		wxClientDC dc(this);
		setCursor(dc, nrChord);
		prevNrChord = nrChord;
		// nbSetPosition ++ ;
	}
}
void bitmapscore::onPaint(wxPaintEvent& WXUNUSED(event))
{
	// onPaint
	wxPaintDC dc(this);
	if (!isOk()) return;

	if (newPaintNrChord != prevPaintNrChord) 
	{
		setCursor(dc, newPaintNrChord);
		prevPaintNrChord = newPaintNrChord;
		// nbPaint ++ ;
	}
}
int bitmapscore::getNbPaint()
{
	int i = nbPaint ;
	nbPaint = 0 ;
	return i;
}
int bitmapscore::getNbSetPosition()
{
	int i =  nbSetPosition;
	nbSetPosition = 0 ;
	return i ;
}

void bitmapscore::OnLeftDown(wxMouseEvent& event)
{
	wxClientDC mDC(this);
	mDC.SetUserScale(xScale, yScale);
	mPointStart = event.GetLogicalPosition(mDC);
}
void bitmapscore::OnLeftUp(wxMouseEvent& event)
{
	wxClientDC mDC(this);
	mDC.SetUserScale(xScale, yScale);
	mPointEnd = event.GetLogicalPosition(mDC);
	selectedRect = highlight(false, mPointStart, mPointEnd, mDC);
	mPointStart = wxDefaultPosition;
	if (((selectedRect.GetWidth() < 5) && (selectedRect.GetHeight() < 5)) || (nrChord == -1))
	{
		selectedRect.SetWidth(10);
		selectedRect.SetHeight(10);
		wxRect r;
		int dmax = 100000;
		int bestrect = -1;
		for (int nrRectChord = 0; nrRectChord < nbRectChord; nrRectChord++)
		{
			r = rectChord[nrRectChord] * selectedRect;
			if (!r.IsEmpty())
			{
				bestrect = nrRectChord;
				break;
			}
			int dx = rectChord[nrRectChord].GetX() - selectedRect.GetX();
			dx *= dx;
			int dy = rectChord[nrRectChord].GetY() - selectedRect.GetY();
			dy *= dy;
			int d = dx;
			if (d < dy)
				d = dy;
			if (d < dmax)
			{
				bestrect = nrRectChord;
				dmax = d;
			}
		}
		if (bestrect != -1)
		{
			basslua_call(moduleChord, functionChordSetNrEvent, "i", bestrect + 1);
			nrChord = bestrect;
		}
	}
	else
	{
		if (alertSetRect)
		{
			wxMessageDialog *mDialog = new wxMessageDialog(this, _("link this rectangle to the current chord ?"), _("Score link"), wxYES | wxNO | wxCANCEL | wxICON_QUESTION | wxCENTRE);
			mDialog->SetYesNoCancelLabels(_("Always link"), _("Link now"), _("Cancel"));
			switch (mDialog->ShowModal())
			{
			case wxID_YES:	alertSetRect = false; break;
			case wxID_NO:	break;	
			default: return;
			}
			delete mDialog;
		}
		if ( nrChord >= 0 )
		{
			rectChord[nrChord] = selectedRect;
			if (nrChord >= nbRectChord)
				nbRectChord = nrChord + 1;
			writeRectChord();
		}
	}
}
wxRect bitmapscore::highlight(bool on, wxPoint start, wxPoint end , wxDC& dc)
{
	wxRect mRect;
	if (end.x >= mPointStart.x)
	{
		mRect.SetX(start.x);
		mRect.SetWidth(end.x - mPointStart.x);
	}
	else
	{
		mRect.SetX(end.x);
		mRect.SetWidth(mPointStart.x - end.x);
	}
	if (end.y >= mPointStart.y)
	{
		mRect.SetY(start.y);
		mRect.SetHeight(end.y - mPointStart.y);
	}
	else
	{
		mRect.SetY(end.y);
		mRect.SetHeight(mPointStart.y - end.y);
	}
	if (on)
	{
		dc.SetLogicalFunction(wxINVERT);
		if (! prevRect.IsEmpty())
			dc.DrawRectangle(prevRect);
		dc.DrawRectangle(mRect);
		prevRect = mRect;
	}
	else
	{
		dc.SetLogicalFunction(wxINVERT);
		dc.DrawRectangle(prevRect);
		prevRect.SetWidth(0);
	}
	return mRect;
}
void bitmapscore::OnMouse(wxMouseEvent& event)
{
	if ((event.Dragging()) && ( mPointStart != wxDefaultPosition))
	{
		wxClientDC mDC(this);
		mDC.SetUserScale(xScale, yScale);
		wxPoint mPos = event.GetLogicalPosition(mDC);
		highlight(true, mPointStart, mPos, mDC);
	}
}
void bitmapscore::readRectChord()
{
	wxTextFile      tfile;
	nbRectChord = 0;
	if (fileRectChord.IsFileReadable() == true)
	{

		tfile.Open(fileRectChord.GetFullPath());
		if (tfile.IsOpened() == false)
			return;
		wxString str = tfile.GetFirstLine(); // LIST_RECT
		str = tfile.GetNextLine();
		wxString token;
		long l;
		while (!tfile.Eof())
		{
			wxStringTokenizer tokenizer(str, ";");
			if (tokenizer.CountTokens() == 4)
			{
				token = tokenizer.GetNextToken();
				token.ToLong(&l);
				rectChord[nbRectChord].SetX(l);
				token = tokenizer.GetNextToken();
				token.ToLong(&l);
				rectChord[nbRectChord].SetY(l);
				token = tokenizer.GetNextToken();
				token.ToLong(&l);
				rectChord[nbRectChord].SetWidth(l);
				token = tokenizer.GetNextToken();
				token.ToLong(&l);
				rectChord[nbRectChord].SetHeight(l);
			}
			if ((nbRectChord == 0) && (rectChord[nbRectChord].GetX() == 0) && (rectChord[nbRectChord].GetY() == 0) && (rectChord[nbRectChord].GetWidth() == 0) && (rectChord[nbRectChord].GetHeight() == 0))
				nbRectChord = 0 ; // problem compatibilite bug V3.0
			else
				nbRectChord++;
			str = tfile.GetNextLine();
		}
		tfile.Close();
	}

	if (nbRectChord == 0 )
	{
		if (mConf->get(CONFIG_BITMAPSCOREWARNINGTAGIMAGE, 1) == 1)
		{
			wxMessageDialog *mDialog = new wxMessageDialog(this, _("No chord tagged in the image"), _("Score image"), wxYES | wxNO | wxHELP | wxICON_INFORMATION | wxCENTRE);
			mDialog->SetYesNoLabels(_("OK"), _("Don't show again this message"));
			switch (mDialog->ShowModal())
			{
			case wxID_NO:
				mConf->set(CONFIG_BITMAPSCOREWARNINGTAGIMAGE, 0);
				break;
			case wxID_HELP:
				wxLaunchDefaultBrowser("http://www.expresseur.com/help/imagechord.html");
				break;
			default:
				break;
			}
		}
	}
}
void bitmapscore::writeRectChord()
{
	wxString s;
	wxFileName f;
	wxTextFile tfile;
	if (fileRectChord.IsFileWritable() == false)
		tfile.Create(fileRectChord.GetFullPath());
	tfile.Open(fileRectChord.GetFullPath());
	if (tfile.IsOpened() == false)
		return;
	tfile.Clear();
	tfile.AddLine(LIST_RECT);
	for (int nrRectChord = 0; nrRectChord < nbRectChord; nrRectChord++)
	{
		s.Printf("%d;%d;%d;%d", rectChord[nrRectChord].GetX(), rectChord[nrRectChord].GetY(), rectChord[nrRectChord].GetWidth(), rectChord[nrRectChord].GetHeight());
		tfile.AddLine(s);
	}
	tfile.Write();
	tfile.Close();
}
void bitmapscore::zoom(int WXUNUSED(dzoom))
{

}
void bitmapscore::gotoNextPage(bool WXUNUSED(forward))
{

}
void bitmapscore::gotoPosition()
{

}
