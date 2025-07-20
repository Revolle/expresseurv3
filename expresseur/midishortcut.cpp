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
	IDM_MIDISHORTCUT_DELETE = ID_MIDISHORTCUT,
	IDM_MIDISHORTCUT_ADD,
	IDM_MIDISHORTCUT_EDIT,
	IDM_MIDISHORTCUT_UP,
	IDM_MIDISHORTCUT_DOWN,
	IDM_MIDISHORTCUT_CLOSE,
	IDM_MIDISHORTCUT_LIST,
	IDM_MIDISHORTCUT_END,
	IDM_MIDISHORTCUT_ID_START = ID_MIDISHORTCUT + 100,
	IDM_MIDISHORTCUT_ID_END = IDM_MIDISHORTCUT_ID_START + 800
};



wxBEGIN_EVENT_TABLE(midishortcut, wxDialog)
EVT_SIZE(midishortcut::OnSize)
EVT_CLOSE(midishortcut::OnClose)
EVT_BUTTON(IDM_MIDISHORTCUT_DELETE, midishortcut::OnDelete)
EVT_BUTTON(IDM_MIDISHORTCUT_ADD, midishortcut::OnAdd)
EVT_BUTTON(IDM_MIDISHORTCUT_EDIT, midishortcut::OnEdit)
EVT_BUTTON(IDM_MIDISHORTCUT_UP, midishortcut::OnUp)
EVT_BUTTON(IDM_MIDISHORTCUT_DOWN, midishortcut::OnDown)
EVT_BUTTON(IDM_MIDISHORTCUT_CLOSE, midishortcut::OnClose)
wxEND_EVENT_TABLE()


midishortcut::midishortcut(wxFrame *parent, wxWindowID id, const wxString &title,wxArrayString inameAction, wxArrayString lMidiin, wxArrayString lOpenedMidiin)
: wxDialog(parent, id, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{

	mParent = parent;
	mThis = this;
	medit = NULL;
	nameAction = inameAction;
	changed = false;

	topsizer = NULL;
	// some sizerFlags commonly used
	sizerFlagMaximumPlace.Proportion(1);
	sizerFlagMaximumPlace.Expand();
	sizerFlagMaximumPlace.Border(wxALL, 2);

	sizerFlagMinimumPlace.Proportion(0);
	sizerFlagMinimumPlace.Border(wxALL, 1);

	nameDevice = lMidiin;
	nameOpenDevice.Clear();
	nameOpenDevice.Add(SALLMIDIIN);
	for (unsigned int i = 0; i < lOpenedMidiin.GetCount(); i++)
		nameOpenDevice.Add(lOpenedMidiin[i]);

	InitLists();

	topsizer = new wxFlexGridSizer(1, wxSize(5, 5));
	topsizer->AddGrowableRow(0);
	topsizer->AddGrowableCol(0);

	int widthColumn[MAX_COLUMN_SHORTCUT];
	for (int i = 0; i < MAX_COLUMN_SHORTCUT; i++)
	{
		widthColumn[i] = configGet(CONFIG_SHORTCUTWIDTHLIST, -1, false, wxString::Format("%d", i));
		if (widthColumn[i] < 5)
			widthColumn[i] = -1;
	}
	listShortchut = new wxListView(this, IDM_MIDISHORTCUT_LIST);
	listShortchut->AppendColumn(_("name"), wxLIST_FORMAT_LEFT, widthColumn[0]);
	listShortchut->AppendColumn(_("ALT+key"), wxLIST_FORMAT_LEFT, widthColumn[1]);
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
	configSet(CONFIG_SHORTCUTWIDTH, mrect.GetWidth());
	configSet(CONFIG_SHORTCUTHEIGHT, mrect.GetHeight());
	configSet(CONFIG_SHORTCUTX, mrect.GetX());
	configSet(CONFIG_SHORTCUTY, mrect.GetY());
	for (int i = 0; i < MAX_COLUMN_SHORTCUT; i++)
		configSet(CONFIG_SHORTCUTWIDTHLIST, listShortchut->GetColumnWidth(i), false, wxString::Format("%d", i));
}
void midishortcut::close()
{
#ifdef RUN_MAC
	EndModal(changed ? wxOK : wxCANCEL);
#else
	if ( IsModal() )
		EndModal(changed?wxOK:wxCANCEL);
#endif
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
	long nbSelector = configGet(CONFIG_SHORTCUTNB, 0, false);
	for (int nrSelector = 0; nrSelector < nbSelector; nrSelector++)
	{
		name = configGet(CONFIG_SHORTCUTNAME, "", false, wxString::Format("%d", nrSelector));
		action = configGet(CONFIG_SHORTCUTACTION, "", false, wxString::Format("%d", nrSelector));
		key = configGet(CONFIG_SHORTCUTKEY, "", false, wxString::Format("%d", nrSelector));
		key = key.Left(1);
		key.MakeUpper();
		if ((key.Len() > 0) && ((key[0] < 'A') || (key[0] > 'Z')))
		{
			key.Empty();
		}
		device = configGet(CONFIG_SHORTCUTDEVICENAME, "", false, wxString::Format("%d", nrSelector));
		channel = configGet(CONFIG_SHORTCUTCHANNEL, "", false, wxString::Format("%d", nrSelector));
		event = configGet(CONFIG_SHORTCUTEVENT, "", false, wxString::Format("%d", nrSelector));
		smin = configGet(CONFIG_SHORTCUTMIN, "", false, wxString::Format("%d", nrSelector));
		smax = configGet(CONFIG_SHORTCUTMAX, "", false, wxString::Format("%d", nrSelector));
		sstopOnMatch = configGet(CONFIG_STOPONMATCH, "", false, wxString::Format("%d", nrSelector));
		param = configGet(CONFIG_SHORTCUTPARAM, "", false, wxString::Format("%d", nrSelector));

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
	wxString s;
	wxString name, action, key;
	wxString sdevice, event, schannel, smin, smax, stopOnMatch , param;
	configSet(CONFIG_SHORTCUTNB, listShortchut->GetItemCount(), false);
	//mlog_in("midishortcut / saveShortcut / listShortchut->GetItemCount() : %d",listShortchut->GetItemCount());
	for (int nrItem = 0; nrItem < listShortchut->GetItemCount(); nrItem++)
	{
		name = listShortchut->GetItemText(nrItem, 0); 
		key = listShortchut->GetItemText(nrItem, 1); 
		key = key.Left(1);
		key.MakeUpper();
		if ((key.Len() > 0) && ((key[0] < 'A') || (key[0] > 'Z')))
		{
			key.Empty();
		}
		sdevice = listShortchut->GetItemText(nrItem, 2); 
		schannel = listShortchut->GetItemText(nrItem, 3);
		event = listShortchut->GetItemText(nrItem, 4); 
		smin = listShortchut->GetItemText(nrItem, 5);
		smax = listShortchut->GetItemText(nrItem, 6);
		action = listShortchut->GetItemText(nrItem, 7); 
		param = listShortchut->GetItemText(nrItem, 8); 
		stopOnMatch = listShortchut->GetItemText(nrItem, 9); 
		long channel = nameChannel.Index(schannel); channel--; 
		long min; smin.ToLong(&min); 
		long max;
		if (!smax.IsEmpty())
			smax.ToLong(&max);
		else
			max = min;
		

		configSet(CONFIG_SHORTCUTNAME, name, false, wxString::Format("%d", nrItem));
		configSet(CONFIG_SHORTCUTKEY, key, false, wxString::Format("%d", nrItem));
		configSet(CONFIG_SHORTCUTDEVICENAME, sdevice, false, wxString::Format("%d", nrItem));
		configSet(CONFIG_SHORTCUTCHANNEL, schannel, false, wxString::Format("%d", nrItem));
		configSet(CONFIG_SHORTCUTEVENT, event, false, wxString::Format("%d", nrItem));
		configSet(CONFIG_SHORTCUTMIN, smin, false, wxString::Format("%d", nrItem));
		configSet(CONFIG_SHORTCUTMAX, smax, false, wxString::Format("%d", nrItem));
		configSet(CONFIG_STOPONMATCH, stopOnMatch, false, wxString::Format("%d", nrItem));
		configSet(CONFIG_SHORTCUTACTION, action, false, wxString::Format("%d", nrItem));
		configSet(CONFIG_SHORTCUTPARAM, param, false, wxString::Format("%d", nrItem));
	}
}
void midishortcut::InitLists()
{
	wxString s;

	nameEvent.Clear();
	nameEvent.Add(_("(none)"));
	nameEvent.Add(snoteonoff);
	nameEvent.Add(snoteononly);
	nameEvent.Add(scontrol);
	nameEvent.Add(sprogram);

	nameChannel.Clear();
	nameChannel.Add(SALLCHANNEL);
	for (int i = 0; i < 16; i++)
	{
		s.Printf("%s#%d", _("channel"), i + 1);
		nameChannel.Add(s);
	}

	nameKey.Clear();
	nameKey.Add("");
	for (char i = 'A' ; i <= 'Z'; i++)
	{
		s.Printf("%c", i);
		nameKey.Add(s);
	}

	nameValueMax.Clear();
	nameValueMax.Add("");
	for (int i = 0; i < 128; i++)
	{
		s.Printf("%d", i);
		nameValueMin.Add(s);
		nameValueMax.Add(s);
		nameParam.Add(s);
	}

	nameStopOnMatch.Clear();
	nameStopOnMatch.Add(SSTOP);
	nameStopOnMatch.Add("continue");


}
void midishortcut::OnDelete(wxCommandEvent& WXUNUSED(event))
{
	changed = true;
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
	changed = true;
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
	changed = true;
	long i = listShortchut->GetFirstSelected();
	if (i == -1)
		return;
	edit(i);

	saveShortcut();
	reset();

}
void midishortcut::OnUp(wxCommandEvent& WXUNUSED(event))
{
	changed = true;
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
	changed = true;
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

	medit = new editshortcut (this, wxID_ANY, ("edit midishortcut"), 
		&lname,
		&laction, nameAction ,
		&lkey , nameKey ,
		&ldevice, nameDevice , nameOpenDevice ,
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
	valueName.Clear();
	valueAction.Clear();
	valueParam.Clear();
	valueKey.Clear();
	valueDevice.Clear();
	valueChannel.Clear();
	valueMin.Clear();
	valueEvent.Clear();
	// reset the selectors
	basslua_setSelector(0, -1, 'x', 0, 0, 0, NULL, 0, false , NULL);
	// set the selectors
	int nbSelector = configGet(CONFIG_SHORTCUTNB, 0);
	wxString name, saction, key;
	wxString sdevice, sevent, smin, smax, sparam, schannel, stopOnMatch;
	for (int nrSelector = 0; nrSelector < nbSelector; nrSelector++)
	{
		name = configGet(CONFIG_SHORTCUTNAME, "", false, wxString::Format("%d", nrSelector)); valueName.Add(name);
		saction = configGet(CONFIG_SHORTCUTACTION, "", false, wxString::Format("%d", nrSelector)); valueAction.Add(saction);
		key = configGet(CONFIG_SHORTCUTKEY, "", false, wxString::Format("%d", nrSelector)); valueKey.Add(key);
		sdevice = configGet(CONFIG_SHORTCUTDEVICENAME, SALLMIDIIN, false, wxString::Format("%d", nrSelector));
		schannel = configGet(CONFIG_SHORTCUTCHANNEL, SALLCHANNEL, false, wxString::Format("%d", nrSelector));
		sevent = configGet(CONFIG_SHORTCUTEVENT, "", false, wxString::Format("%d", nrSelector)); valueEvent.Add(sevent);
		smin = configGet(CONFIG_SHORTCUTMIN, "", false, wxString::Format("%d", nrSelector));
		smax = configGet(CONFIG_SHORTCUTMAX, "", false, wxString::Format("%d", nrSelector));
		stopOnMatch = configGet(CONFIG_STOPONMATCH, SSTOP,  false, wxString::Format("%d", nrSelector));
		sparam = configGet(CONFIG_SHORTCUTPARAM, "", false, wxString::Format("%d", nrSelector)); valueParam.Add(sparam);
		long min; smin.ToLong(&min); valueMin.Add(min);
		long nrAction = nameAction.Index(saction);
		long nrChannel = nameChannel.Index(schannel) - 1 ; valueChannel.Add(nrChannel);
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
			int nrDevice;
			if (nameOpenDevice.Index(sdevice) == 0)
				nrDevice = -1;
			else
			{
				nrDevice = nameDevice.Index(sdevice);
				if (nrDevice == wxNOT_FOUND)
					nrDevice = -2;
			}
			valueDevice.Add(nrDevice);

			//if (nrDevice != -2)
				basslua_setSelector(nrSelector, nrAction, 'b', nrDevice , nrChannel, bufevent, tp, 2, stopOnMatch.Contains("stop"), bufparam);
		}
	}
}
void midishortcut::write(wxTextFile *lfile)
{
	configWriteFile(lfile, CONFIG_SHORTCUTNB, 0);
	long nbSelector = configGet(CONFIG_SHORTCUTNB, 0, false);
	for (int nrSelector = 0; nrSelector < nbSelector; nrSelector++)
	{
		configWriteFile(lfile, CONFIG_SHORTCUTNAME, "", false, wxString::Format("%d", nrSelector));
		configWriteFile(lfile, CONFIG_SHORTCUTACTION, "", false, wxString::Format("%d", nrSelector));
		configWriteFile(lfile, CONFIG_SHORTCUTKEY, "", false, wxString::Format("%d", nrSelector));
		configWriteFile(lfile, CONFIG_SHORTCUTDEVICENAME, SALLMIDIIN, false, wxString::Format("%d", nrSelector));
		configWriteFile(lfile, CONFIG_SHORTCUTCHANNEL, SALLCHANNEL, false, wxString::Format("%d", nrSelector));
		configWriteFile(lfile, CONFIG_SHORTCUTEVENT, "", false, wxString::Format("%d", nrSelector));
		configWriteFile(lfile, CONFIG_SHORTCUTMIN, "", false, wxString::Format("%d", nrSelector));
		configWriteFile(lfile, CONFIG_SHORTCUTMAX, "", false, wxString::Format("%d", nrSelector));
		configWriteFile(lfile, CONFIG_STOPONMATCH, SSTOP, false, wxString::Format("%d", nrSelector));
		configWriteFile(lfile, CONFIG_SHORTCUTPARAM, "", false, wxString::Format("%d", nrSelector));
	}
}
void midishortcut::read(wxTextFile *lfile)
{
	if (configReadFile(lfile, CONFIG_SHORTCUTNB, 0))
	{
		long nbSelector = configGet(CONFIG_SHORTCUTNB, 0, false);
		for (int nrSelector = 0; nrSelector < nbSelector; nrSelector++)
		{
			configReadFile(lfile, CONFIG_SHORTCUTNAME, "", false, wxString::Format("%d", nrSelector));
			configReadFile(lfile, CONFIG_SHORTCUTACTION, "", false, wxString::Format("%d", nrSelector));
			configReadFile(lfile, CONFIG_SHORTCUTKEY, "", false, wxString::Format("%d", nrSelector));
			configReadFile(lfile, CONFIG_SHORTCUTDEVICENAME, SALLMIDIIN, false, wxString::Format("%d", nrSelector));
			configReadFile(lfile, CONFIG_SHORTCUTCHANNEL, SALLCHANNEL, false, wxString::Format("%d", nrSelector));
			configReadFile(lfile, CONFIG_SHORTCUTEVENT, "", false, wxString::Format("%d", nrSelector));
			configReadFile(lfile, CONFIG_SHORTCUTMIN, "", false, wxString::Format("%d", nrSelector));
			configReadFile(lfile, CONFIG_SHORTCUTMAX, "", false, wxString::Format("%d", nrSelector));
			configReadFile(lfile, CONFIG_STOPONMATCH, SSTOP, false, wxString::Format("%d", nrSelector));
			configReadFile(lfile, CONFIG_SHORTCUTPARAM, "", false, wxString::Format("%d", nrSelector));
		}
	}
}
wxArrayString midishortcut::getShortcuts()
{
	shortcutNrSelector.Clear();
	wxArrayString ls ;
	for (unsigned int nrSelector = 0; nrSelector < valueAction.GetCount(); nrSelector++)
	{
		wxString cs = valueKey[nrSelector] ;
		cs.MakeUpper() ;
		if (cs.Len() > 0)
		{
			wxChar c = cs[0];
			if ((c >= 'A') && (c <= 'Z'))
			{
				wxString s;
				s.Printf("%s\tALT+%c\n", valueName[nrSelector], c);
				ls.Add(s);
				shortcutNrSelector.Add(nrSelector);
			}
		}
	}
	return ls ;
}
void midishortcut::onShortcut(int nrShortcut)
{
	if ((nrShortcut < 0 ) || ( nrShortcut >= (int)(shortcutNrSelector.GetCount())))
		return ;
	
	if (prevShortcutNrSelector >= 0)
	{
		// off the previous shortcut
		if (valueEvent[prevShortcutNrSelector] == snoteonoff)
		{
			// simulation of note-off
			int nrDevice = valueDevice[prevShortcutNrSelector];
			int nrChannel = valueChannel[prevShortcutNrSelector];
			int pitch = valueMin[prevShortcutNrSelector];
			basslua_selectorTrigger(prevShortcutNrSelector, nrDevice, nrChannel, NOTEOFF, pitch, 0);
		}
	}
	prevShortcutNrSelector = -1 ; 
 
	// on this event
	int nrSelector = shortcutNrSelector[nrShortcut];
	int nrDevice = valueDevice[nrSelector];
	int nrChannel = valueChannel[nrSelector];
	int pitch = valueMin[nrSelector];
	int ievent = -1;
	wxString mevent = valueEvent[nrSelector];
	if (mevent == snoteonoff)
		ievent = NOTEON;
	else if (mevent == snoteononly)
		ievent = NOTEON;
	else if (mevent == scontrol)
		ievent = CONTROL;
	else if (mevent == sprogram)
		ievent = PROGRAM;
	int velo = 64;
	wxString mparameter = valueParam[nrSelector];
	wxString firstParameter = mparameter.BeforeFirst(' ');
	if (firstParameter.IsNumber() )
	{
		long l;
		if (firstParameter.ToLong(&l))
		{
			if ((l >= 0) && (l < 128))
			{
				velo = l;
			}
		}
	}
	// simulation of the event
	if (ievent != -1)
	{
		basslua_selectorTrigger(nrSelector,nrDevice, nrChannel, ievent, pitch, velo);
		prevShortcutNrSelector = nrSelector;
	}
}
