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
	mImage = NULL;
	mBitmap = NULL;
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
}
bitmapscore::~bitmapscore()
{
	if ( mImage )
		delete mImage;
	if ( mBitmap )
		delete mBitmap ;

}
bool bitmapscore::isOk()
{
	return((mImage != NULL) && (mBitmap != NULL));
}
bool bitmapscore::setFile(const wxFileName &lfilename)
{
	// load the image
	wxFileName filename(lfilename);
	filename.SetExt(SUFFIXE_BITMAPCHORD);
	bool retcode = false;
	if (mImage)
		delete mImage;
	mImage = NULL;
	if (filename.IsFileReadable())
	{
		wxString sfilename = filename.GetFullPath();
		// mImage = new wxImage(sfilename, wxBITMAP_TYPE_ANY);
		mImage = new wxImage(sfilename, wxBITMAP_TYPE_PNG);
		if (mImage->IsOk())
		{
			retcode = true;
			// load the rect linked to the chords
			fileRectChord = filename;
			fileRectChord.SetExt("txb");
			readRectChord();
		}
	}
	if (retcode == false)
	{
		if (mImage)
			delete mImage;
		mImage = NULL;
	}
	newLayout() ;
	return retcode;
}
bool bitmapscore::displayFile(wxSize WXUNUSED(sizeClient))
{
	prevPos =1 ;
	prevPaintPos = -1 ;
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
void bitmapscore::newLayout()
{
	if (mImage == NULL)
	{
		return ;
	}
	if ( mBitmap )
		delete mBitmap ;
	mBitmap = NULL ;
	wxClientDC dc(this) ;
	wxSize sizeClient = dc.GetSize();
	wxSize sizeImage = mImage->GetSize();
	wxSize sizeDisplay;

	sizeDisplay.SetWidth(sizeClient.GetWidth());
	sizeDisplay.SetHeight((sizeImage.GetHeight()*sizeClient.GetWidth()) / sizeImage.GetWidth());
	if (sizeDisplay.GetHeight() > sizeClient.GetHeight())
	{
		sizeDisplay.SetHeight(sizeClient.GetHeight());
		sizeDisplay.SetWidth((sizeImage.GetWidth()*sizeClient.GetHeight()) / sizeImage.GetHeight());
	}
	xScale = (double)(sizeDisplay.GetWidth()) / (double)(sizeImage.GetWidth());
	yScale = (double)(sizeDisplay.GetHeight()) / (double)(sizeImage.GetHeight());
	wxImage lImage = mImage->Scale(sizeDisplay.GetWidth(), sizeDisplay.GetHeight(), wxIMAGE_QUALITY_HIGH);
	mBitmap = new wxBitmap(lImage);
}
void bitmapscore::refresh(wxDC& dc, int pos)
{
	if (! isOk() ) return ;

	// redraw the page
	dc.Clear();
	dc.DrawBitmap(*mBitmap,0,0);

	// draw the cursor
	if (!rectChord[pos].IsEmpty())
	{
		wxDCClipper clip(dc, rectChord[pos]);
		dc.SetUserScale(xScale, yScale);
		dc.SetPen(wxNullPen);
		dc.SetBrush(*wxWHITE_BRUSH);
		dc.SetLogicalFunction(wxXOR);
		dc.DrawBitmap(*mBitmap,0,0);
	}
}
void bitmapscore::setPosition(int pos, bool WXUNUSED( playing))
{
	// onIdle : set the current pos
	newPaintPos = pos ;
	if (pos != prevPos)
	{
		wxClientDC dc(this);
		refresh(dc,pos);
		prevPos = pos ;
	}
}
void bitmapscore::onPaint(wxPaintEvent& WXUNUSED(event))
{
	wxPaintDC dc(this);
	if (newPaintPos != prevPaintPos)
	{
		refresh(dc,newPaintPos);
		prevPaintPos = newPaintPos ;
	}
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
	if (((selectedRect.GetWidth() < 10) && (selectedRect.GetHeight() < 10)) || (nrChord == -1))
	{
		selectedRect.SetWidth(10);
		selectedRect.SetHeight(10);
		wxRect r;
		for (int nrRectChord = 0; nrRectChord < nbRectChord; nrRectChord++)
		{
			r = rectChord[nrRectChord] * selectedRect;
			if (!r.IsEmpty())
			{
				basslua_call(moduleChord, functionChordSetNrEvent, "i", nrRectChord);
				break;
			}
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
			case wxID_YES:
				alertSetRect = false;
			case wxID_NO:
				break;
			default:
				return;
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
