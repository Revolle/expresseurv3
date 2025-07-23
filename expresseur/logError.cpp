/////////////////////////////////////////////////////////////////////////////
// Name:        logerror.cpp
// Purpose:     non-modal dialog to log Midi activity /  expresseur V3
// Author:      Franck REVOLLE
// Modified by:
// Created:     08/09/2015
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
#include "wx/choice.h"
#include "wx/sizer.h"
#include "wx/listctrl.h"


#include "global.h"
#include "logError.h"
#include "basslua.h"

enum
{
	IDM_LOGERROR_CLEAR = ID_LOGERROR,
	IDM_LOGERROR_MIDIIN,
	IDM_LOGERROR_MIDIOUT,
	IDM_LOGERROR_TRACE,
	IDM_LOGERROR_CLOSE
};

wxBEGIN_EVENT_TABLE(logerror, logerror::wxDialog)
EVT_BUTTON(IDM_LOGERROR_CLEAR, logerror::OnLogerrorClear)
EVT_BUTTON(IDM_LOGERROR_CLOSE, logerror::OnLogerrorClose)
EVT_SIZE(logerror::OnSize)
wxEND_EVENT_TABLE()

logerror::logerror(wxFrame *parent, wxWindowID id, const wxString &title)
: wxDialog(parent, id, title, wxPoint(20, 20), wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	// some sizerFlags commonly used
	sizerFlagMaximumPlace.Proportion(1);
	sizerFlagMaximumPlace.Expand();
	sizerFlagMaximumPlace.Border(wxALL, 2);

	sizerFlagMinimumPlace.Proportion(0);
	sizerFlagMinimumPlace.Border(wxALL, 2);

	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);

	mlog = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	mlog->SetMinSize(wxSize(500, 400));
	topsizer->Add(mlog, sizerFlagMaximumPlace);

	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);

	wxButton *bClear = new wxButton(this, IDM_LOGERROR_CLEAR, "Clear");
	buttonSizer->Add(bClear, sizerFlagMaximumPlace);
	wxButton *bClose = new wxButton(this, IDM_LOGERROR_CLOSE, "Close");
	buttonSizer->Add(bClose, sizerFlagMaximumPlace);

	topsizer->Add(buttonSizer, sizerFlagMinimumPlace);

	SetSizerAndFit(topsizer);
	Layout();
}
logerror::~logerror()
{
	basslua_getLog(NULL);
}

void logerror::OnSize(wxSizeEvent& WXUNUSED(event))
{
	Layout();
}
void logerror::OnLogerrorClose(wxCommandEvent& WXUNUSED(event))
{
	Close();
}
void logerror::OnLogerrorClear(wxCommandEvent& WXUNUSED(event))
{
	mlog->Clear();
	mlog->Insert("Midi logs",0);
}
void logerror::scanLog()
{
	char buf[MAXBUFCHAR];
	while (basslua_getLog(buf))
		mlog->Insert(buf,0);
}
