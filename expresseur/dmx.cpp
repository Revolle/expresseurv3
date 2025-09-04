/////////////////////////////////////////////////////////////////////////////
// Name:        dmx.cpp
// Purpose:     modal dialog to edit the DMX /  expresseur V3
// Author:      Franck REVOLLE
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
#include "dmx.h"

enum
{
	IDM_DMX_1 = ID_DMX
};

wxBEGIN_EVENT_TABLE(editshortcut, wxDialog)
EVT_SIZE(dmx::OnSize)
EVT_CHOICE(IDM_DMX_1, luafile::OnLuaUserFile)
EVT_TEXT(IDM_DMX_2, luafile::OnLuaParameter)
wxEND_EVENT_TABLE()

dmx::dmx(wxFrame* parent, wxWindowID id, const wxString& title)
	: wxDialog(parent, id, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	mParent = parent;
	mThis = this;
	// c_eventMidi::OneIsProcessed = false ;
	// some sizerFlags commonly used
	sizerFlagMaximumPlace.Proportion(1);
	sizerFlagMaximumPlace.Expand();
	sizerFlagMaximumPlace.Border(wxALL, 2);

	sizerFlagMinimumPlace.Proportion(0);
	sizerFlagMinimumPlace.Border(wxALL, 2);

	wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
	wxFlexGridSizer* paramsizer = new wxFlexGridSizer(2, wxSize(5, 5));
	paramsizer->AddGrowableCol(1);

	wxArrayString lUserScript, lfUserScript;
	wxFileName fUser;
	fUser.Assign(getCwdDir());
	wxDir::GetAllFiles(fUser.GetPath(), &lUserScript, "*.lua", wxDIR_FILES);
	for (unsigned int i = 0; i < lUserScript.GetCount(); i++)
	{
		wxFileName fsUser(lUserScript[i]);
		lfUserScript.Add(fsUser.GetFullName());
	}
	fUser.Assign(getResourceDir());
	wxDir::GetAllFiles(fUser.GetPath(), &lUserScript, "*.lua", wxDIR_FILES);
	for (unsigned int i = 0; i < lUserScript.GetCount(); i++)
	{
		wxFileName fsUser(lUserScript[i]);
		lfUserScript.Add(fsUser.GetFullName());
	}
	paramsizer->Add(new wxStaticText(this, wxID_ANY, "LUA Script"), sizerFlagMaximumPlace);
	wxString luauserscriptfile = configGet(CONFIG_LUA_USER_SCRIPT, DEFAULT_LUA_USER_FILE);
	wxChoice* cLuaUserScript = new wxChoice(this, IDM_LUAFILE_LUA_USER_SCRIPT, wxDefaultPosition, wxDefaultSize, lfUserScript);
	if (lfUserScript.Index(luauserscriptfile) != wxNOT_FOUND)
		cLuaUserScript->SetSelection(lfUserScript.Index(luauserscriptfile));
	cLuaUserScript->SetToolTip(_("LUA script which manages user's features, in program and user's ressource dir"));
	paramsizer->Add(cLuaUserScript, sizerFlagMaximumPlace);

	wxString luascriptparameter = configGet(CONFIG_LUA_PARAMETER, DEFAULT_LUA_PARAMETER);

	paramsizer->Add(new wxStaticText(this, wxID_ANY, "LUA start parameter"), sizerFlagMaximumPlace);
	wxTextCtrl* mLuaParameter = new wxTextCtrl(this, IDM_LUAFILE_LUA_PARAMETER, luascriptparameter);
	mLuaParameter->SetToolTip("parameter for the LUA script.");
	paramsizer->Add(mLuaParameter, sizerFlagMaximumPlace);

	topsizer->Add(paramsizer, sizerFlagMaximumPlace);
	topsizer->Add(CreateButtonSizer(wxCLOSE), sizerFlagMaximumPlace);
	SetSizerAndFit(topsizer);

	SetReturnCode(0);
}
dmx::~dmx()
{
	//listMidi->Clear();
}
void dmx::OnSize(wxSizeEvent& WXUNUSED(event))
{
	Layout();
}

void dmx::scanMidi(int nr_device, int type_msg, int channel, int value1, int value2)
{
	wxString chTypeMsg;
	switch (type_msg)
	{
	case PROGRAM:
		chTypeMsg = sprogram;
		break;
	case NOTEON:
		if (value2 > 0)
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
	if (!chTypeMsg.IsEmpty())
	{
		wxString chMidiInEvent;
		chMidiInEvent.Printf("%12s device=%2d channel=%2d d1=%3d d2=%3d", chTypeMsg.c_str(), nr_device + 1, channel + 1, value1, value2);
		listMidi->Append(chMidiInEvent);
		listMidi->SetFirstItem(listMidi->GetCount() - 1);
	}
}
void dmx::OnMidi(wxCommandEvent& event)
{
	// format of the text midi event is defined in editshortcut::scanMidi

	wxString smessage = event.GetString();
	wxStringTokenizer tokenizer(smessage, " =", wxTOKEN_STRTOK);
	int nb = tokenizer.CountTokens();
	if (nb == 9)
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
		if (id != nameOpenDevice.end())
			ps = std::distance(nameOpenDevice.begin(), id);
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
