/////////////////////////////////////////////////////////////////////////////
// Name:        edishortcut.cpp
// Purpose:     modal dialog to edit the shortcut /  expresseur V3
// Author:      Franck REVOLLE
// Modified by:
// Created:     08/06/2015
// update : 23/11/2016 18:00
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
#include <vector>
#include <algorithm>


#include "wx/dialog.h"
#include "wx/filename.h"
#include "wx/stattext.h"
#include "wx/sizer.h"
#include "wx/bitmap.h"
#include "wx/tglbtn.h"
#include "wx/spinctrl.h"
#include "wx/textfile.h"
#include "wx/statline.h"
#include "wx/listctrl.h"
#include "wx/valgen.h"
#include "wx/listbox.h"
#include "wx/tokenzr.h"
#include "wx/config.h"

#include "global.h"
#include "basslua.h"
#include "editshortcut.h"

enum
{
	IDM_EDITSHORTCUT_LISTMIDI = ID_EDITSHORTCUT
}; 

wxBEGIN_EVENT_TABLE(editshortcut, wxDialog)
EVT_LISTBOX_DCLICK(IDM_EDITSHORTCUT_LISTMIDI, editshortcut::OnMidi )
wxEND_EVENT_TABLE()

editshortcut::editshortcut(wxWindow *parent, wxWindowID id, const wxString &title,
wxString *lname,
wxString *laction, std::vector <wxString> nameAction,
wxString *lkey, std::vector <wxString> nameKey,
wxString *ldevice, std::vector <wxString> lnameDevice, std::vector <wxString> lnameOpenDevice,
wxString *lchannel, std::vector <wxString> nameChannel,
wxString *levent, std::vector <wxString> nameEvent,
wxString *lmin, std::vector <wxString> nameValueMin,
wxString *lmax, std::vector <wxString> nameValueMax,
wxString *lparam ,
wxString *lstopOnMatch, std::vector <wxString> nameStopOnMatch
)
: wxDialog(parent, id, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	wxSizerFlags sizerFlagMaximumPlace;
	sizerFlagMaximumPlace.Proportion(1);
	sizerFlagMaximumPlace.Expand();
	sizerFlagMaximumPlace.Border(wxALL, 2);

	wxSizerFlags sizerFlagMinimumPlace;
	sizerFlagMinimumPlace.Proportion(0);
	sizerFlagMinimumPlace.Border(wxALL, 5);

	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	wxFlexGridSizer *secundsizer = new wxFlexGridSizer(2, wxSize(5, 5));
	secundsizer->AddGrowableCol(0);

	wxFlexGridSizer *fieldsizer = new wxFlexGridSizer(2, wxSize(5, 5));

	fieldsizer->Add(new wxStaticText(this, wxID_ANY, "Name"), sizerFlagMinimumPlace);
	wxTextCtrl *fName = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_EMPTY, lname));
	fName->SetToolTip("Any name. Free text");
	fieldsizer->Add(fName, sizerFlagMaximumPlace);

	fieldsizer->Add(new wxStaticText(this, wxID_ANY, _("ALT+Keyboard-shortcut")), sizerFlagMinimumPlace);
	wxChoice *fkey = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,nameKey, 0, wxGenericValidator(lkey));
	fkey->SetToolTip("a key from the computer keyboard, to be adde din the menu Action");
	fieldsizer->Add(fkey, sizerFlagMaximumPlace);

	nameOpenDevice = lnameOpenDevice;
	nameDevice = lnameDevice;
	fieldsizer->Add(new wxStaticText(this, wxID_ANY, "Device"), sizerFlagMinimumPlace);
	fTdevice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, nameOpenDevice, 0, wxGenericValidator(ldevice));
	fTdevice->SetToolTip("Midiin device trigger");
	fieldsizer->Add(fTdevice, sizerFlagMaximumPlace);

	fieldsizer->Add(new wxStaticText(this, wxID_ANY, "Channel"), sizerFlagMinimumPlace);
	fTchannel = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, nameChannel, 0, wxGenericValidator(lchannel));
	fTchannel->SetToolTip("Midiin channel trigger");
	fieldsizer->Add(fTchannel, sizerFlagMaximumPlace);

	fieldsizer->Add(new wxStaticText(this, wxID_ANY, "Event"), sizerFlagMinimumPlace);
	fEvent = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, nameEvent, 0, wxGenericValidator(levent));
	fEvent->SetToolTip("Midiin event trigger");
	fieldsizer->Add(fEvent, sizerFlagMaximumPlace);

	fieldsizer->Add(new wxStaticText(this, wxID_ANY, "Min Value"), sizerFlagMinimumPlace);
	fMin = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, nameValueMin, 0, wxGenericValidator(lmin));
	fMin->SetToolTip(_("Data1 of the midi-message.\nPitch of the note, control number, or program number"));
	fieldsizer->Add(fMin, sizerFlagMaximumPlace);

	fieldsizer->Add(new wxStaticText(this, wxID_ANY, "Max Value"), sizerFlagMinimumPlace);
	wxChoice *fMax = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, nameValueMax, 0, wxGenericValidator(lmax));
	fMax->SetToolTip(_("Maximum value (included ) for the pitch of the note, control number, or program number"));
	fieldsizer->Add(fMax, sizerFlagMaximumPlace);

	fieldsizer->Add(new wxStaticText(this, wxID_ANY, "Action"), sizerFlagMinimumPlace);
	wxChoice *fAction = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, nameAction, 0, wxGenericValidator(laction));
	fAction->SetToolTip("Action triggered");
	fieldsizer->Add(fAction, sizerFlagMaximumPlace);

	fieldsizer->Add(new wxStaticText(this, wxID_ANY, "On match"), sizerFlagMinimumPlace);
	wxChoice *fStopOnMatch = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, nameStopOnMatch, 0, wxGenericValidator(lstopOnMatch));
	fStopOnMatch->SetToolTip(_("On match, continue or stop the analysis of next selectors"));
	fieldsizer->Add(fStopOnMatch, sizerFlagMaximumPlace);

	fieldsizer->Add(new wxStaticText(this, wxID_ANY, "Parameter"), sizerFlagMinimumPlace);
	wxTextCtrl *fParameter = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, wxTextValidator(wxFILTER_NONE, lparam));
	fParameter->SetToolTip("Action parameter");
	fieldsizer->Add(fParameter, sizerFlagMaximumPlace);

	wxBoxSizer *thirdsizer = new wxBoxSizer(wxVERTICAL);
	thirdsizer->Add(new wxStaticText(this, wxID_ANY, "MIDI event detected"), sizerFlagMinimumPlace);
	listMidi = new wxListBox(this, IDM_EDITSHORTCUT_LISTMIDI);
	listMidi->SetToolTip("Double-click to copy");
	listMidi->SetFont(wxFont(8, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
	listMidi->Clear();
	listMidi->Append("Valid Midi-In ports are opened.");
	listMidi->Append("MIDI events are displayed. Double-click to fill automatically the form.");

	thirdsizer->Add(listMidi, sizerFlagMaximumPlace);

	secundsizer->Add(fieldsizer, sizerFlagMaximumPlace);
	secundsizer->Add(thirdsizer, sizerFlagMaximumPlace);
	topsizer->Add(secundsizer, sizerFlagMaximumPlace);
	topsizer->Add(new wxStaticLine(this), sizerFlagMinimumPlace);
	topsizer->Add(CreateButtonSizer(wxOK | wxCANCEL), sizerFlagMinimumPlace);
	SetSizerAndFit(topsizer);
}
editshortcut::~editshortcut()
{
	//listMidi->Clear();
}

void editshortcut::scanMidi(int nr_device, int type_msg, int channel, int value1, int value2)
{
	wxString chTypeMsg;
	switch (type_msg)
	{
	case PROGRAM:
		chTypeMsg = sprogram;
		break;
	case NOTEON:
		if ( value2 > 0 )
			chTypeMsg = snoteononly;
		else
			chTypeMsg = snoteonoff;
		break;
	case NOTEOFF:
		chTypeMsg = snoteonoff;
		break;
	case CONTROL:
		chTypeMsg = scontrol;
		break;
	default:
		break; 
	}
	if (! chTypeMsg.IsEmpty())
	{
		wxString chMidiInEvent;
		chMidiInEvent.Printf("%12s device=%2d channel=%2d d1=%3d d2=%3d", chTypeMsg.c_str(), nr_device + 1, channel + 1, value1, value2);
		listMidi->Append(chMidiInEvent);
		listMidi->SetFirstItem(listMidi->GetCount() - 1);
	}
}
void editshortcut::OnMidi(wxCommandEvent& event)
{
	// format of the text midi event is defined in editshortcut::scanMidi

	wxString smessage = event.GetString();
	wxStringTokenizer tokenizer(smessage, " =", wxTOKEN_STRTOK);
	int nb = tokenizer.CountTokens();
	if ( nb == 9 )
	{
		wxString token;
		
		//type msg
		token = tokenizer.GetNextToken();
		fEvent->SetStringSelection(token);

		//device
		token = tokenizer.GetNextToken();
		token = tokenizer.GetNextToken();
		long d; token.ToLong(&d);
		wxString ndev = nameDevice[d - 1];
		int ps = 0;
		auto id = std::find(nameOpenDevice.begin(), nameOpenDevice.end(), ndev);
		if ( id != nameOpenDevice.end())
			ps = std::distance(nameOpenDevice.begin(),id);
		fTdevice->SetSelection(ps);

		//channel
		token = tokenizer.GetNextToken();
		token = tokenizer.GetNextToken();
		long c; token.ToLong(&c);
		fTchannel->SetSelection(c);

		//data1
		token = tokenizer.GetNextToken();
		token = tokenizer.GetNextToken();
		fMin->SetStringSelection(token);

	}

}
