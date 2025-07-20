/////////////////////////////////////////////////////////////////////////////
// Name:        emptyscore.cpp
// Purpose:     display an empty score /  expresseur V3
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
#include "mxconf.h"
#include "editshortcut.h"
#include "midishortcut.h"
#include "expression.h"
#include "viewerscore.h"
#include "emptyscore.h"
#include "luafile.h"

wxBEGIN_EVENT_TABLE(emptyscore, wxPanel)
EVT_PAINT(emptyscore::onPaint)
EVT_SIZE(emptyscore::OnSize)
wxEND_EVENT_TABLE()

emptyscore::emptyscore(wxWindow *parent, wxWindowID id)
: viewerscore(parent, id)
{
	mParent = parent;
}
emptyscore::~emptyscore()
{
}
bool emptyscore::isOk()
{
	return false ;
}
bool emptyscore::setFile(const wxFileName & WXUNUSED(lfilename))
{
	wxClientDC dc(this);
	dc.Clear();
	dc.DrawText(_("No image score"), 20, 20);
	if (configGet(CONFIG_EMPTYSCOREWARNING, 1) == 1)
	{
		wxMessageDialog *mDialog = new wxMessageDialog(this, _("No score available"), _("Score viewer"), wxYES | wxNO | wxHELP | wxICON_INFORMATION | wxCENTRE);
		mDialog->SetYesNoLabels(_("OK"), _("Don't show again this message"));
		switch (mDialog->ShowModal())
		{
		case wxID_NO:
			configSet(CONFIG_EMPTYSCOREWARNING, 0);
			break;
		case wxID_HELP:
			wxLaunchDefaultBrowser("http://www.expresseur.com/help/imagechord.html");
			break;
		default:
			break;
		}
	}
	return true ;
}
bool emptyscore::displayFile(wxSize WXUNUSED(sizeClient))
{
	return true;
}
int emptyscore::getTrackCount()
{
	return 0;
}
wxString emptyscore::getTrackName(int WXUNUSED(trackNr) )
{
	return wxEmptyString;
}

void emptyscore::OnSize(wxSizeEvent& WXUNUSED(event))
{
	Refresh();
}
void emptyscore::onPaint(wxPaintEvent& WXUNUSED(event))
{
	wxPaintDC dc(this);

	dc.Clear();
	dc.DrawText(_("No image score"), 20, 20);
}
void emptyscore::setPosition(int WXUNUSED(pos), bool WXUNUSED(playing))
{
}
void emptyscore::newLayout()
{
}
void emptyscore::zoom(int WXUNUSED(dzoom))
{

}
void emptyscore::gotoPosition(wxString gotovalue)
{

}
void emptyscore::gotoNextPage(bool WXUNUSED(forward))
{

}
