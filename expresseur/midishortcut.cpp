/////////////////////////////////////////////////////////////////////////////
// Name:        midishortcut.cpp
// Purpose:     modal dialog for the shortcuts /  expresseur V3
// Author:      Franck REVOLLE
// Modified by:
// Created:     28/04/2015
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
#include "wx/notebook.h"
#include "wx/bitmap.h"
#include "wx/tglbtn.h"
#include "wx/spinctrl.h"
#include "wx/textfile.h"
#include "wx/statline.h"
#include "wx/listctrl.h"
#include "wx/valgen.h"
#include "wx/listbox.h"
#include "wx/config.h"
#include "wx/dir.h"

#include "global.h"
#include "basslua.h"
#include "luabass.h"
#include "mxconf.h"
#include "editshortcut.h"
#include "midishortcut.h"

enum
{
	IDM_MIDISHORTCUT_DELETE = ID_MIDISHORTCUT ,
	IDM_MIDISHORTCUT_ADD,
	IDM_MIDISHORTCUT_EDIT,
	IDM_MIDISHORTCUT_UP,
	IDM_MIDISHORTCUT_DOWN,
	IDM_MIDISHORTCUT_CLOSE,
	IDM_MIDISHORTCUT_LIST,
	IDM_MIDISHORTCUT_PRECONFIG,
	IDM_MIDISHORTCUT_DESCRIPTION_DEVICE,
	IDM_MIDISHORTCUT_ID_START = ID_MIDISHORTCUT + 100,
	IDM_MIDISHORTCUT_ID_END = IDM_MIDISHORTCUT_ID_START + 800
};


wxBEGIN_EVENT_TABLE(midishortcut, wxDialog)
EVT_SIZE(midishortcut::OnSize)
EVT_CLOSE(midishortcut::OnClose)
EVT_CHOICE(IDM_MIDISHORTCUT_PRECONFIG, midishortcut::OnFunctionMidi)
EVT_BUTTON(IDM_MIDISHORTCUT_DELETE, midishortcut::OnDelete)
EVT_BUTTON(IDM_MIDISHORTCUT_ADD, midishortcut::OnAdd)
EVT_BUTTON(IDM_MIDISHORTCUT_EDIT, midishortcut::OnEdit)
EVT_BUTTON(IDM_MIDISHORTCUT_UP, midishortcut::OnUp)
EVT_BUTTON(IDM_MIDISHORTCUT_DOWN, midishortcut::OnDown)
EVT_BUTTON(IDM_MIDISHORTCUT_CLOSE, midishortcut::OnClose)
EVT_BUTTON(IDM_MIDISHORTCUT_DESCRIPTION_DEVICE, midishortcut::OnDescriptionFunctionMidi)
wxEND_EVENT_TABLE()


midishortcut::midishortcut(wxFrame *parent, wxWindowID id, const wxString &title, mxconf* lMxconf, wxArrayString inameAction)
: wxDialog(parent, id, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{

	mParent = parent;
	mThis = this;
	medit = NULL;
	mConf = lMxconf;
	nameAction = inameAction;

	topsizer = NULL;
	// some sizerFlags commonly used
	sizerFlagMaximumPlace.Proportion(1);
	sizerFlagMaximumPlace.Expand();
	sizerFlagMaximumPlace.Border(wxALL, 2);

	sizerFlagMinimumPlace.Proportion(0);
	sizerFlagMinimumPlace.Border(wxALL, 1);

	InitLists();

	topsizer = new wxFlexGridSizer(1, wxSize(5, 5));
	topsizer->AddGrowableRow(0);
	topsizer->AddGrowableCol(0);

	int widthColumn[MAX_COLUMN_SHORTCUT];
	for (int i = 0; i < MAX_COLUMN_SHORTCUT; i++)
	{
		widthColumn[i] = mConf->get(CONFIG_SHORTCUTWIDTHLIST, -1, false, wxString::Format("%d", i));
		if (widthColumn[i] < 5)
			widthColumn[i] = -1;
	}
	listShortchut = new wxListView(this, IDM_MIDISHORTCUT_LIST);
	listShortchut->AppendColumn(_("name"), wxLIST_FORMAT_LEFT, widthColumn[0]);
	listShortchut->AppendColumn(_("key"), wxLIST_FORMAT_LEFT, widthColumn[1]);
	listShortchut->AppendColumn(_("device"), wxLIST_FORMAT_LEFT, widthColumn[2]);
	listShortchut->AppendColumn(_("channel"), wxLIST_FORMAT_LEFT, widthColumn[3]);
	listShortchut->AppendColumn(_("event"), wxLIST_FORMAT_LEFT, widthColumn[4]);
	listShortchut->AppendColumn(_("min"), wxLIST_FORMAT_LEFT, widthColumn[5]);
	listShortchut->AppendColumn(_("max"), wxLIST_FORMAT_LEFT, widthColumn[6]);
	listShortchut->AppendColumn(_("action"), wxLIST_FORMAT_LEFT, widthColumn[7]);
	listShortchut->AppendColumn(_("param"), wxLIST_FORMAT_LEFT, widthColumn[8]);
	listShortchut->AppendColumn(_("on match"), wxLIST_FORMAT_LEFT, widthColumn[9]);

	topsizer->Add(listShortchut, sizerFlagMaximumPlace);

	wxBoxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);
	button_sizer->Add(new wxButton(this, IDM_MIDISHORTCUT_DELETE, _("Delete")), sizerFlagMinimumPlace.Border(wxALL, 10));
	button_sizer->Add(new wxButton(this, IDM_MIDISHORTCUT_ADD, _("Add")), sizerFlagMinimumPlace.Border(wxALL, 10));
	button_sizer->Add(new wxButton(this, IDM_MIDISHORTCUT_EDIT, _("Edit")), sizerFlagMinimumPlace.Border(wxALL, 10));
	button_sizer->Add(new wxButton(this, IDM_MIDISHORTCUT_UP, _("Up")), sizerFlagMinimumPlace.Border(wxALL, 10));
	button_sizer->Add(new wxButton(this, IDM_MIDISHORTCUT_DOWN, _("Down")), sizerFlagMinimumPlace.Border(wxALL, 10));
	button_sizer->Add(new wxButton(this, IDM_MIDISHORTCUT_CLOSE, _("Close")), sizerFlagMinimumPlace.Border(wxALL, 10));
	topsizer->Add(button_sizer, sizerFlagMaximumPlace);

	/*
	wxBoxSizer *device_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxArrayString arrayDeviceShort;
	wxString maskPreconfig;
	// These text files decribe the preconfigurations. 
	// The name of these files is used to load the lua script files : <name>_<onEventMidi>.lua
	// where <onEventMidi> can be OnControl, onNoteOn, onProgram ... 
	maskPreconfig.Printf("*.%s", SUFFIXE_PRECONFIG);
	arrayDeviceShort.Add(_("none"));
	wxString current_preconfig = mConf->get(CONFIG_MIDIFUNCTION, "");
	int nr_preconfig = wxNOT_FOUND;
	wxFileName fn;
	fn.AssignDir(mConf->get(CONFIG_DIR_RESSOURCE, ""));
	wxDir dir;
	dir.Open(fn.GetFullPath());
	if (dir.IsOpened())
	{
		int nr = 1;
		wxString filename;
		bool cont = dir.GetFirst(&filename, maskPreconfig, wxDIR_FILES);
		while (cont)
		{
			if (current_preconfig == filename)
				nr_preconfig = nr;
			arrayDeviceShort.Add(filename);
			cont = dir.GetNext(&filename);
			nr++;
		}
	}
	listFunctionMidi = new wxChoice(this, IDM_MIDISHORTCUT_PRECONFIG, wxDefaultPosition, wxDefaultSize, arrayDeviceShort);
	listFunctionMidi->SetSelection(nr_preconfig);
	device_sizer->Add(new wxStaticText(this, wxID_ANY , _("Pre-configuration : ")), sizerFlagMinimumPlace.Border(wxALL, 10));
	device_sizer->Add(listFunctionMidi, sizerFlagMinimumPlace.Border(wxALL, 10));
	device_sizer->Add(new wxButton(this, IDM_MIDISHORTCUT_DESCRIPTION_DEVICE, _("Description")), sizerFlagMinimumPlace.Border(wxALL, 10));
	topsizer->Add(device_sizer, sizerFlagMaximumPlace);
	*/

	loadShortcut();

	// resize all the frame and components
	SetSizerAndFit(topsizer);

}
midishortcut::~midishortcut()
{
}
void midishortcut::savePos()
{
	wxRect mrect = GetRect();
	mConf->set(CONFIG_SHORTCUTWIDTH, mrect.GetWidth());
	mConf->set(CONFIG_SHORTCUTHEIGHT, mrect.GetHeight());
	mConf->set(CONFIG_SHORTCUTX, mrect.GetX());
	mConf->set(CONFIG_SHORTCUTY, mrect.GetY());
	for (int i = 0; i < MAX_COLUMN_SHORTCUT; i++)
		mConf->set(CONFIG_SHORTCUTWIDTHLIST, listShortchut->GetColumnWidth(i), false, wxString::Format("%d", i));
}

void midishortcut::close()
{
}
void midishortcut::OnClose(wxCloseEvent& event)
{
	if (event.CanVeto())
	{
		Hide();
		event.Veto(true);
	}
	close();
}
void midishortcut::OnClose(wxCommandEvent& WXUNUSED(event))
{
	Hide();
	close();
}
void midishortcut::OnSize(wxSizeEvent& WXUNUSED(event))
{
	Layout();
}
void midishortcut::loadShortcut()
{
	wxString s  ;
	wxString name , action, key, channel ;
	wxString device, event, smin, smax, sstopOnMatch , param;
	int nrShortcut = 0 ;
	listShortchut->DeleteAllItems();
	long nbSelector = mConf->get(CONFIG_SHORTCUTNB, 0, false);
	for (int nrSelector = 0; nrSelector < nbSelector; nrSelector++)
	{
		name = mConf->get(CONFIG_SHORTCUTNAME, "", false, wxString::Format("%d", nrSelector));
		action = mConf->get(CONFIG_SHORTCUTACTION, "", false, wxString::Format("%d", nrSelector));
		key = mConf->get(CONFIG_SHORTCUTKEY, "", false, wxString::Format("%d", nrSelector));
		device = mConf->get(CONFIG_SHORTCUTDEVICENAME, "", false, wxString::Format("%d", nrSelector));
		channel = mConf->get(CONFIG_SHORTCUTCHANNEL, "", false, wxString::Format("%d", nrSelector));
		event = mConf->get(CONFIG_SHORTCUTEVENT, "", false, wxString::Format("%d", nrSelector));
		smin = mConf->get(CONFIG_SHORTCUTMIN, "", false, wxString::Format("%d", nrSelector));
		smax = mConf->get(CONFIG_SHORTCUTMAX, "", false, wxString::Format("%d", nrSelector));
		sstopOnMatch = mConf->get(CONFIG_STOPONMATCH, "", false, wxString::Format("%d", nrSelector));
		param = mConf->get(CONFIG_SHORTCUTPARAM, "", false, wxString::Format("%d", nrSelector));

		int nrItem = listShortchut->InsertItem(nrShortcut, name);

		listShortchut->SetItem(nrItem, 0, name);
		listShortchut->SetItem(nrItem, 1, key);
		listShortchut->SetItem(nrItem, 2, device);
		listShortchut->SetItem(nrItem, 3, channel);
		listShortchut->SetItem(nrItem, 4, event);
		listShortchut->SetItem(nrItem, 5, smin);
		listShortchut->SetItem(nrItem, 6, smax);
		listShortchut->SetItem(nrItem, 7, action);
		listShortchut->SetItem(nrItem, 8, param);
		listShortchut->SetItem(nrItem, 9, sstopOnMatch);

		nrShortcut++;
	}
	saveShortcut();
}
void midishortcut::saveShortcut()
{
	//mConf->set(CONFIG_MIDIFUNCTION, (listFunctionMidi->GetSelection() != wxNOT_FOUND)?listFunctionMidi->GetString(listFunctionMidi->GetSelection()):"", false);
	wxString s;
	wxString name, action, key;
	wxString sdevice, event, schannel, smin, smax, stopOnMatch , param;
	valueAction.Clear();
	valueParam.Clear();
	valueKey.Clear();
	valueDevice.Clear();
	valueChannel.Clear();
	valueMin.Clear();
	valueMax.Clear();
	valueStopOnMatch.Clear();
	valueEvent.Clear();
	valueHitkey = 0 ;
	mConf->set(CONFIG_SHORTCUTNB, listShortchut->GetItemCount(), false);
	//mlog_in("midishortcut / saveShortcut / listShortchut->GetItemCount() : %d",listShortchut->GetItemCount());
	for (int nrItem = 0; nrItem < listShortchut->GetItemCount(); nrItem++)
	{
		name = listShortchut->GetItemText(nrItem, 0);
		key = listShortchut->GetItemText(nrItem, 1); valueKey.Add(key);
		sdevice = listShortchut->GetItemText(nrItem, 2); 
		schannel = listShortchut->GetItemText(nrItem, 3);
		event = listShortchut->GetItemText(nrItem, 4); valueEvent.Add(event);
		smin = listShortchut->GetItemText(nrItem, 5);
		smax = listShortchut->GetItemText(nrItem, 6);
		action = listShortchut->GetItemText(nrItem, 7); valueAction.Add(action);
		param = listShortchut->GetItemText(nrItem, 8); valueParam.Add(param);
		stopOnMatch = listShortchut->GetItemText(nrItem, 9); valueStopOnMatch.Add(stopOnMatch);
		long nrDevice = nameDevice.Index(sdevice); nrDevice--;  valueDevice.Add(nrDevice);
		long channel = nameChannel.Index(schannel); channel--; valueChannel.Add(channel);
		long min; smin.ToLong(&min); valueMin.Add(min);
		long max;
		if (!smax.IsEmpty())
			smax.ToLong(&max);
		else
			max = min;
		valueMax.Add(max);
		

		mConf->set(CONFIG_SHORTCUTNAME, name, false, wxString::Format("%d", nrItem));
		mConf->set(CONFIG_SHORTCUTKEY, key, false, wxString::Format("%d", nrItem));
		mConf->set(CONFIG_SHORTCUTDEVICENAME, sdevice, false, wxString::Format("%d", nrItem));
		mConf->set(CONFIG_SHORTCUTCHANNEL, schannel, false, wxString::Format("%d", nrItem));
		mConf->set(CONFIG_SHORTCUTEVENT, event, false, wxString::Format("%d", nrItem));
		mConf->set(CONFIG_SHORTCUTMIN, smin, false, wxString::Format("%d", nrItem));
		mConf->set(CONFIG_SHORTCUTMAX, smax, false, wxString::Format("%d", nrItem));
		mConf->set(CONFIG_STOPONMATCH, stopOnMatch, false, wxString::Format("%d", nrItem));
		mConf->set(CONFIG_SHORTCUTACTION, action, false, wxString::Format("%d", nrItem));
		mConf->set(CONFIG_SHORTCUTPARAM, param, false, wxString::Format("%d", nrItem));
	}
}
void midishortcut::InitLists()
{
	wxString s;

	getMidiinDevices();

	nameEvent.Add(_("(none)"));
	nameEvent.Add(snoteonoff);
	nameEvent.Add(snoteononly);
	nameEvent.Add(scontrol);
	nameEvent.Add(sprogram);

	nameChannel.Add(SALLCHANNEL);
	for (int i = 0; i < 16; i++)
	{
		s.Printf("%s#%d",_("channel"), i + 1);
		nameChannel.Add(s);
	}

	nameValueMax.Add("");
	for (int i = 0; i < 128; i++)
	{
		s.Printf("%d", i);
		nameValueMin.Add(s);
		nameValueMax.Add(s);
		nameParam.Add(s);
	}

	nameStopOnMatch.Add(SSTOP);
	nameStopOnMatch.Add("continue");


}
void midishortcut::getMidiinDevices()
{
	wxString s;
	char ch[MAXBUFCHAR];
	nameDevice.Clear();
	nameDevice.Add(SALLMIDIIN);
	nbDevice = 1;
	for (int i = 0; i < MAX_MIDIIN_DEVICE; i++)
		valideMidiinDevice[i] = false;
	while (true)
	{
		basslua_call(moduleLuabass, sinGetMidiName, "i>s", nbDevice, ch);
		if ((*ch == '\0') || (nbDevice >= MAX_MIDIIN_DEVICE))
			break;
		bool valid = false;
		basslua_call(moduleGlobal, sinMidiIsValid, "s>b", ch, &valid);
		if (valid)
		{
			s.Printf("%s:%s", SMIDI , ch);
			nameDevice.Add(s);
			valideMidiinDevice[nbDevice] = true;
		}
		else
		{
			s.Printf("%s %s:%s", _("invalid"), SMIDI, ch);
			nameDevice.Add(s);
		}
		nbDevice++;
	}
	// mlog_in("midishircut / getMidiinDevices nbDevice=%d",nbDevice);
}
void midishortcut::OnDelete(wxCommandEvent& WXUNUSED(event))
{
	long i = listShortchut->GetFirstSelected();
	while (i != -1)
	{
		listShortchut->DeleteItem(i);
		i = listShortchut->GetNextSelected(i-1);
	}
	saveShortcut();
	reset();
}
void midishortcut::OnAdd(wxCommandEvent& WXUNUSED(event))
{
	int i = listShortchut->GetFirstSelected();
	if (i == -1)
		i = listShortchut->GetItemCount();
	wxString s;
	s.Printf("%s#%d", _("trigger"), i);
	i = listShortchut->InsertItem(i, s);
	listShortchut->SetItem(i, 2, SALLMIDIIN);
	listShortchut->SetItem(i, 3, SALLCHANNEL);
	listShortchut->SetItem(i, 9, SSTOP);
	int retCode = edit(i);
	if (retCode != wxID_OK)
		listShortchut->DeleteItem(i);

	saveShortcut();
	reset();

}
void midishortcut::OnEdit(wxCommandEvent& WXUNUSED(event))
{
	long i = listShortchut->GetFirstSelected();
	if (i == -1)
		return;
	edit(i);

	saveShortcut();
	reset();

}
void midishortcut::OnUp(wxCommandEvent& WXUNUSED(event))
{
	long i = listShortchut->GetFirstSelected();
	if (i < 1)
		return;
	for (int nrCol = 0; nrCol < MAX_COLUMN_SHORTCUT; nrCol++)
	{
		wxString i0 = listShortchut->GetItemText(i - 1,nrCol);
		wxString i1 = listShortchut->GetItemText(i, nrCol);
		listShortchut->SetItem(i - 1, nrCol, i1);
		listShortchut->SetItem(i, nrCol, i0);
	}
	saveShortcut();
	reset();

}
void midishortcut::OnDown(wxCommandEvent& WXUNUSED(event))
{
	long i = listShortchut->GetFirstSelected();
	if (i >= (listShortchut->GetItemCount() - 1))
		return;
	for (int nrCol = 0; nrCol < MAX_COLUMN_SHORTCUT; nrCol++)
	{
		wxString i0 = listShortchut->GetItemText(i, nrCol);
		wxString i1 = listShortchut->GetItemText(i + 1, nrCol);
		listShortchut->SetItem(i, nrCol, i1);
		listShortchut->SetItem(i + 1, nrCol, i0);
	}
	saveShortcut();
	reset();

}
void midishortcut::OnDescriptionFunctionMidi(wxCommandEvent& WXUNUSED(event))
{
	if (listFunctionMidi->GetSelection() == wxNOT_FOUND)
		return;
	wxString f = listFunctionMidi->GetString(listFunctionMidi->GetSelection());
	wxFileName fn;
	fn.AssignDir(mConf->get(CONFIG_DIR_RESSOURCE, ""));

	fn.SetFullName(f);
	wxString s = fn.GetFullPath();
	if (fn.IsFileReadable())
	{
		wxFile mfile(s);
		wxString t;
		mfile.ReadAll(&t);
		wxMessageBox(t, "Description of the preconfiguration");
	}
	else
		wxMessageBox("description not available", "Description of the preconfiguration" );

}
void midishortcut::OnFunctionMidi(wxCommandEvent& WXUNUSED(event))
{
	saveShortcut();
	reset();
}
int midishortcut::edit(long i)
{
	wxString lname = listShortchut->GetItemText(i, 0);


	wxString lkey = listShortchut->GetItemText(i, 1);

	wxString ldevice = listShortchut->GetItemText(i, 2);
	if (ldevice.IsEmpty()) ldevice = nameDevice[0];
	wxString lchannel = listShortchut->GetItemText(i, 3);

	wxString levent = listShortchut->GetItemText(i, 4);

	wxString lmin = listShortchut->GetItemText(i, 5);
	wxString lmax = listShortchut->GetItemText(i, 6);

	wxString laction = listShortchut->GetItemText(i, 7);
	if (laction.IsEmpty()) laction = nameAction[0];
	wxString lparam = listShortchut->GetItemText(i, 8);
	wxString lstopOnMatch = listShortchut->GetItemText(i, 9);

	openMidiIn(true);

	medit = new editshortcut (this, wxID_ANY, ("edit midishortcut"), 
		&lname,
		&laction, nameAction ,
		&lkey ,
		&ldevice, nameDevice ,
		&lchannel , nameChannel ,
		&levent, nameEvent ,
		&lmin, nameValueMin ,
		&lmax, nameValueMax,
		&lparam,
		&lstopOnMatch , nameStopOnMatch
		);
	int ret_code = medit->ShowModal();

	if (ret_code  == wxID_OK)
	{
		listShortchut->SetItem(i, 0, lname);
		listShortchut->SetItem(i, 1, lkey);
		listShortchut->SetItem(i, 2, ldevice);
		listShortchut->SetItem(i, 3, lchannel);
		listShortchut->SetItem(i, 4, levent);
		listShortchut->SetItem(i, 5, lmin);
		listShortchut->SetItem(i, 6, lmax);
		listShortchut->SetItem(i, 7, laction);
		listShortchut->SetItem(i, 8, lparam);
		listShortchut->SetItem(i, 9, lstopOnMatch);
		saveShortcut();
	}


	delete medit;
	medit = NULL;
	
	return ret_code;
}

void midishortcut::scanMidi(int nr_device, int type_msg, int channel, int value1, int value2)
{
	if ((medit) && (medit->IsActive()))
	{
		medit->scanMidi(nr_device,  type_msg,  channel,  value1,  value2);
	}
}
void midishortcut::reset()
{
	wxBusyCursor wait;
	// reset the preconfiguration
	wxString preconfig = mConf->get(CONFIG_MIDIFUNCTION, "");
	if (preconfig.EndsWith(SUFFIXE_PRECONFIG))
	{
		wxFileName fn;
		fn.AssignDir(mConf->get(CONFIG_DIR_RESSOURCE, ""));
		fn.SetFullName(preconfig);
		fn.SetExt("");
		wxString sfn = fn.GetFullPath();
		char buffn[MAXBUFCHAR];
		strcpy(buffn, sfn.c_str());
		basslua_addMidiFunction(buffn);
	}
	// reset the selectors
	basslua_setSelector(0, -1, 'x', 0, 0, 0, NULL, 0, false , NULL);
	// set the selectors
	int nbSelector = mConf->get(CONFIG_SHORTCUTNB, 0);
	wxString name, saction, key;
	wxString sdevice, sevent, smin, smax, sparam, schannel, stopOnMatch;
	for (int nrDevice = 0; nrDevice < MAX_MIDIIN_DEVICE; nrDevice++)
		deviceToOpen[nrDevice] = false;
	for (int nrSelector = 0; nrSelector < nbSelector; nrSelector++)
	{
		name = mConf->get(CONFIG_SHORTCUTNAME, "", false, wxString::Format("%d", nrSelector));
		saction = mConf->get(CONFIG_SHORTCUTACTION, "", false, wxString::Format("%d", nrSelector));
		key = mConf->get(CONFIG_SHORTCUTKEY, "", false, wxString::Format("%d", nrSelector));
		sdevice = mConf->get(CONFIG_SHORTCUTDEVICENAME, SALLMIDIIN, false, wxString::Format("%d", nrSelector));
		schannel = mConf->get(CONFIG_SHORTCUTCHANNEL, SALLCHANNEL, false, wxString::Format("%d", nrSelector));
		sevent = mConf->get(CONFIG_SHORTCUTEVENT, "", false, wxString::Format("%d", nrSelector));
		smin = mConf->get(CONFIG_SHORTCUTMIN, "", false, wxString::Format("%d", nrSelector));
		smax = mConf->get(CONFIG_SHORTCUTMAX, "", false, wxString::Format("%d", nrSelector));
		stopOnMatch = mConf->get(CONFIG_STOPONMATCH, SSTOP,  false, wxString::Format("%d", nrSelector));
		sparam = mConf->get(CONFIG_SHORTCUTPARAM, "", false, wxString::Format("%d", nrSelector));
		int nrDevice = nameDevice.Index(sdevice);
		//char bufsdevice[512];
		//strcpy(bufsdevice,sdevice.c_str());
		//mlog_in("midishortcut / reset , selector(%s)=%d",bufsdevice,nrDevice);
		if (nrDevice != wxNOT_FOUND)
		{
			if (nrDevice > 0)
			{
				deviceToOpen[nrDevice-1] = true;
			}
			else
			{
				// all midiin valid devices
				for (unsigned int nrDevice2 = 1; nrDevice2 < nameDevice.GetCount(); nrDevice2++)
				{
					wxString s = nameDevice[nrDevice2];
					if (s.BeforeFirst(':') == SMIDI)
					{
						//char bufs[512];
						//strcpy(bufs,s.c_str());
						//mlog_in("midishortcut / reset / tobeopened, nrDevice2#%d name=<%s>",nrDevice2,bufs);
						deviceToOpen[nrDevice2 - 1] = true;
					}
				}
			}
		}
		long min; smin.ToLong(&min);
		long nrAction = nameAction.Index(saction);
		long nrChannel = nameChannel.Index(schannel);
		long max;
		if (!smax.IsEmpty())
			smax.ToLong(&max);
		else
			max = min;
		if (nrAction >= 0)
		{
			int tp[10];
			tp[0] = min;
			tp[1] = max;
			char bufevent[MAXBUFCHAR];
			strcpy(bufevent, sevent.c_str());
			char bufparam[MAXBUFCHAR];
			strcpy(bufparam, sparam.c_str());
			basslua_setSelector(nrSelector, nrAction, 'b', nrDevice - 1, nrChannel - 1, bufevent, tp, 2, stopOnMatch.Contains("stop"), bufparam);
		}
	}
	openMidiIn(false);
}
void midishortcut::openMidiIn( bool allValid)
{
	if (allValid)
	{
		for (int nrDevice = 0; nrDevice < MAX_MIDIIN_DEVICE; nrDevice++)
			deviceToOpen[nrDevice] = false;
		// all midiin valid devices
		for (unsigned int nrDevice = 1; nrDevice < nameDevice.GetCount(); nrDevice++)
		{
			wxString s = nameDevice[nrDevice];
			if (s.BeforeFirst(':') == SMIDI)
				deviceToOpen[nrDevice-1] = true;
		}
	}
	// open the device in
	int nrDevicesToOpen[MAX_MIDIIN_DEVICE];
	int nbDevicesToOpen = 0;
	for (int nrDevice = 0; nrDevice < MAX_MIDIIN_DEVICE; nrDevice++)
	{
		if (deviceToOpen[nrDevice])
		{
			nrDevicesToOpen[nbDevicesToOpen] = nrDevice;
			nbDevicesToOpen++;
		}
	}
	basslua_openMidiIn(nrDevicesToOpen, nbDevicesToOpen);
}
void midishortcut::write(wxTextFile *lfile)
{
	mConf->writeFile(lfile, CONFIG_MIDIFUNCTION, "" );
	mConf->writeFile(lfile, CONFIG_SHORTCUTNB, 0);
	long nbSelector = mConf->get(CONFIG_SHORTCUTNB, 0, false);
	for (int nrSelector = 0; nrSelector < nbSelector; nrSelector++)
	{
		mConf->writeFile(lfile, CONFIG_SHORTCUTNAME, "", false, wxString::Format("%d", nrSelector));
		mConf->writeFile(lfile, CONFIG_SHORTCUTACTION, "", false, wxString::Format("%d", nrSelector));
		mConf->writeFile(lfile, CONFIG_SHORTCUTKEY, "", false, wxString::Format("%d", nrSelector));
		mConf->writeFile(lfile, CONFIG_SHORTCUTDEVICENAME, SALLMIDIIN, false, wxString::Format("%d", nrSelector));
		mConf->writeFile(lfile, CONFIG_SHORTCUTCHANNEL, SALLCHANNEL, false, wxString::Format("%d", nrSelector));
		mConf->writeFile(lfile, CONFIG_SHORTCUTEVENT, "", false, wxString::Format("%d", nrSelector));
		mConf->writeFile(lfile, CONFIG_SHORTCUTMIN, "", false, wxString::Format("%d", nrSelector));
		mConf->writeFile(lfile, CONFIG_SHORTCUTMAX, "", false, wxString::Format("%d", nrSelector));
		mConf->writeFile(lfile, CONFIG_STOPONMATCH, SSTOP, false, wxString::Format("%d", nrSelector));
		mConf->writeFile(lfile, CONFIG_SHORTCUTPARAM, "", false, wxString::Format("%d", nrSelector));
	}
}
void midishortcut::read(wxTextFile *lfile)
{
	mConf->readFile(lfile, CONFIG_MIDIFUNCTION, "");
	if (mConf->readFile(lfile, CONFIG_SHORTCUTNB, 0))
	{
		long nbSelector = mConf->get(CONFIG_SHORTCUTNB, 0, false);
		for (int nrSelector = 0; nrSelector < nbSelector; nrSelector++)
		{
			mConf->readFile(lfile, CONFIG_SHORTCUTNAME, "", false, wxString::Format("%d", nrSelector));
			mConf->readFile(lfile, CONFIG_SHORTCUTACTION, "", false, wxString::Format("%d", nrSelector));
			mConf->readFile(lfile, CONFIG_SHORTCUTKEY, "", false, wxString::Format("%d", nrSelector));
			mConf->readFile(lfile, CONFIG_SHORTCUTDEVICENAME, SALLMIDIIN, false, wxString::Format("%d", nrSelector));
			mConf->readFile(lfile, CONFIG_SHORTCUTCHANNEL, SALLCHANNEL, false, wxString::Format("%d", nrSelector));
			mConf->readFile(lfile, CONFIG_SHORTCUTEVENT, "", false, wxString::Format("%d", nrSelector));
			mConf->readFile(lfile, CONFIG_SHORTCUTMIN, "", false, wxString::Format("%d", nrSelector));
			mConf->readFile(lfile, CONFIG_SHORTCUTMAX, "", false, wxString::Format("%d", nrSelector));
			mConf->readFile(lfile, CONFIG_STOPONMATCH, SSTOP, false, wxString::Format("%d", nrSelector));
			mConf->readFile(lfile, CONFIG_SHORTCUTPARAM, "", false, wxString::Format("%d", nrSelector));
		}
	}
}
bool midishortcut::hitkey(wxChar c , bool on , wxFrame *parent)
{
	// simulate a selector , acording to the hit key
	bool found = false;
	if (on)
	{
		if (valueHitkey == c)
			return(false); // multi-on of the same key. Ignored
		valueHitkey = c ; // new on on this key
	}
	else
	{
		valueHitkey = c; 
	}
	wxString sValueKey(valueHitkey);
	sValueKey.MakeLower();
	char lowValueHitkey = sValueKey.at(0);
	for (unsigned int nrSelector = 0; nrSelector < valueAction.GetCount(); nrSelector++)
	{
		int pos ;
		if ((pos = valueKey[nrSelector].Find(lowValueHitkey)) != wxNOT_FOUND)
		{
			found = true; // a selector wants this key
			if (on)
			{
				int p = valueMin[nrSelector];
				int v = 64;
				int len = valueKey[nrSelector].length();
				if (valueMin[nrSelector] == valueMax[nrSelector])
				{
					// only one value : the range is translated to velocity
					if (len > 1)
					{
						if ((valueEvent[nrSelector] == snoteonoff) || (valueEvent[nrSelector] == snoteononly))
							v = 1 + (126 * ( pos + 1 )) / ( len + 1 );
						else
							v = (127 * (pos + 1)) / (len + 1);
					}
				}
				else
				{
					// more than one value : the range is translated to pitch
					if (len > 1)
						p = valueMin[nrSelector] + pos;
				}
				int nrDevice = valueDevice[nrSelector];
				int nrChannel = valueChannel[nrSelector];
				// simulation of the event
				basslua_selectorSearch(nrDevice, nrChannel, NOTEON, p, v);
				wxString strace;
				strace.Printf("shortcut %c : NoteOn %s %s , p=%d , v=%d", lowValueHitkey, 
					(nrDevice >= -1) ? nameDevice[nrDevice + 1] : nameDevice[0],
					(nrChannel >= -1) ? nameChannel[nrChannel + 1] : nameChannel[0],
					p, v);
				parent->SetStatusText(strace);
			}
			else
			{ // off
				if (valueEvent[nrSelector] == snoteonoff)
				{
					int p = valueMin[nrSelector];
					int v = 0;
					pos = valueKey[nrSelector].Find(lowValueHitkey);
					int len = valueKey[nrSelector].length();
					if (valueMin[nrSelector] != valueMax[nrSelector])
					{
						if (len > 1)
							p = valueMin[nrSelector] + pos;
					}
					// simulation of note-off
					int nrDevice = valueDevice[nrSelector];
					int nrChannel = valueChannel[nrSelector];
					basslua_selectorSearch(nrDevice, nrChannel, NOTEOFF, p, v);
					wxString strace;
					strace.Printf("shortcut %c : NoteOff %s %s , p=%d , v=%d", lowValueHitkey, 
						(nrDevice >= -1) ? nameDevice[nrDevice + 1] : nameDevice[0],
						(nrChannel >= -1) ? nameChannel[nrChannel + 1] : nameChannel[0],
						p, v);
					parent->SetStatusText(strace);
				}
				else
				{
					wxString strace;
					strace.Printf("shortcut %c : off", lowValueHitkey);
					parent->SetStatusText(strace);
				}
				valueHitkey = 0;
			}
		}
	}
	if (!found)
	{
		wxString strace;
		strace.Printf("shortcut %c : not recognized", lowValueHitkey);
		parent->SetStatusText(strace);
	}
	return found;
}
