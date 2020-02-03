/////////////////////////////////////////////////////////////////////////////
// Name:        luafile.cpp
// Purpose:     non-modal dialog to select lua files /  expresseur V3
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

#include "wx/dialog.h"
#include "wx/filename.h"
#include "wx/stattext.h"
#include "wx/sizer.h"
#include "wx/notebook.h"
#include "wx/bitmap.h"
#include "wx/tglbtn.h"
#include "wx/spinctrl.h"
#include "wx/textfile.h"
#include "wx/textctrl.h"
#include "wx/statline.h"
#include "wx/config.h"
#include "wx/filepicker.h"
#include "wx/filefn.h"
#include "wx/dir.h"
#include "wx/thread.h"

#include "global.h"
#include "mxconf.h"
#include "basslua.h"
#include "luafile.h"

#define DEFAULT_LUA_FILE "expresseur.lua"
#define DEFAULT_LUA_USER_FILE "luauser.lua"
#define DEFAULT_LUA_PARAMETER "--preopen_midiout"

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(l_eventMidi);
l_eventMidi g_lEventMidis;
wxCriticalSection g_CriticalSection ;

bool c_eventMidi::OneIsProcessed = false ;
c_eventMidi::c_eventMidi(	wxLongLong itime , int inr_device, int itype_msg, int ichannel, int ivalue1, int ivalue2, bool iisProcessed)
{ 
	time = itime; nr_device = inr_device;  type_msg = itype_msg; channel = ichannel; value1 = ivalue1;  value2 = ivalue2; isProcessed = iisProcessed;
	if ( iisProcessed )
		OneIsProcessed = true; 
}

enum
{
	IDM_LUAFILE_LUA_SCRIPT = ID_LUAFILE ,
	IDM_LUAFILE_LUA_USER_SCRIPT,
	IDM_LUAFILE_LUA_PARAMETER
};

wxBEGIN_EVENT_TABLE(luafile, luafile::wxDialog)
EVT_SIZE(luafile::OnSize)
EVT_CHOICE(IDM_LUAFILE_LUA_SCRIPT, luafile::OnLuaFile)
EVT_CHOICE(IDM_LUAFILE_LUA_USER_SCRIPT, luafile::OnLuaUserFile)
EVT_TEXT(IDM_LUAFILE_LUA_PARAMETER, luafile::OnLuaParameter)
wxEND_EVENT_TABLE()

luafile::luafile(wxFrame *parent, wxWindowID id, const wxString &title, mxconf* lMxconf)
: wxDialog(parent, id, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	mParent = parent;
	mThis = this;
	mConf = lMxconf;
	// c_eventMidi::OneIsProcessed = false ;
	// some sizerFlags commonly used
	sizerFlagMaximumPlace.Proportion(1);
	sizerFlagMaximumPlace.Expand();
	sizerFlagMaximumPlace.Border(wxALL, 2);

	sizerFlagMinimumPlace.Proportion(0);
	sizerFlagMinimumPlace.Border(wxALL, 2);

	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	wxFlexGridSizer *paramsizer = new wxFlexGridSizer(2, wxSize(5, 5));
	paramsizer->AddGrowableCol(1);

	wxArrayString lScript, lfScript;
	wxFileName f;
	f.Assign(mxconf::getCwdDir());
	wxDir::GetAllFiles(f.GetPath(), &lScript, "*.lua", wxDIR_FILES);
	for (unsigned int i = 0; i < lScript.GetCount(); i++)
	{
		wxFileName fs(lScript[i]);
		lfScript.Add(fs.GetFullName());
	}


	paramsizer->Add(new wxStaticText(this, wxID_ANY, _("LUA main Script")), sizerFlagMaximumPlace);
	wxString luascriptfile = mConf->get(CONFIG_LUA_SCRIPT, DEFAULT_LUA_FILE);
	wxChoice *cLuaScript = new wxChoice(this, IDM_LUAFILE_LUA_SCRIPT, wxDefaultPosition, wxDefaultSize, lfScript);
	if (lfScript.Index(luascriptfile) != wxNOT_FOUND)
		cLuaScript->SetSelection(lfScript.Index(luascriptfile));
	cLuaScript->SetToolTip(_("LUA script which manages technically midi inputs, midi outputs, ..."));
	paramsizer->Add(cLuaScript, sizerFlagMaximumPlace);

	wxArrayString lUserScript, lfUserScript;
	wxFileName fUser;
	fUser.Assign(mxconf::getResourceDir());
	wxDir::GetAllFiles(fUser.GetPath(), &lUserScript, "*.lua", wxDIR_FILES);
	for (unsigned int i = 0; i < lUserScript.GetCount(); i++)
	{
		wxFileName fsUser(lUserScript[i]);
		lfUserScript.Add(fsUser.GetFullName());
	}
	paramsizer->Add(new wxStaticText(this, wxID_ANY, _("LUA User Script")), sizerFlagMaximumPlace);
	wxString luauserscriptfile = mConf->get(CONFIG_LUA_USER_SCRIPT, DEFAULT_LUA_USER_FILE);
	wxChoice *cLuaUserScript = new wxChoice(this, IDM_LUAFILE_LUA_USER_SCRIPT, wxDefaultPosition, wxDefaultSize, lfUserScript);
	if (lfUserScript.Index(luauserscriptfile) != wxNOT_FOUND)
		cLuaUserScript->SetSelection(lfUserScript.Index(luauserscriptfile));
	cLuaUserScript->SetToolTip(_("LUA script which manages user's features, in user's ressource dir"));
	paramsizer->Add(cLuaUserScript, sizerFlagMaximumPlace);

	wxString luascriptparameter = mConf->get(CONFIG_LUA_PARAMETER, DEFAULT_LUA_PARAMETER);

	paramsizer->Add(new wxStaticText(this, wxID_ANY, _("LUA start parameter")), sizerFlagMaximumPlace);
	wxTextCtrl *mLuaParameter = new wxTextCtrl(this, IDM_LUAFILE_LUA_PARAMETER, luascriptparameter);
	mLuaParameter->SetToolTip(_("parameter for the LUA script"));
	paramsizer->Add(mLuaParameter, sizerFlagMaximumPlace);


	topsizer->Add(paramsizer, sizerFlagMaximumPlace);
	topsizer->Add(CreateButtonSizer(wxCLOSE), sizerFlagMaximumPlace);
	SetSizerAndFit(topsizer);

	SetReturnCode(0);

}
luafile::~luafile()
{
}
void luafile::OnSize(wxSizeEvent& WXUNUSED(event))
{
	Layout();
}
void luafile::OnLuaFile(wxCommandEvent& event)
{
	wxString f = event.GetString();
	mConf->set(CONFIG_LUA_SCRIPT, f);

	SetReturnCode(1);
}
void luafile::OnLuaUserFile(wxCommandEvent& event)
{
	wxString f = event.GetString();
	mConf->set(CONFIG_LUA_USER_SCRIPT, f);

	SetReturnCode(1);
}
void luafile::OnLuaParameter(wxCommandEvent&  event)
{
	wxString p = event.GetString();
	mConf->set(CONFIG_LUA_PARAMETER, p);

	SetReturnCode(1);
}
void luafile::reset(mxconf* mConf, bool all, int timerDt)
{
	wxBusyCursor wait;
	wxFileName f;
	f.Assign(mxconf::getCwdDir());
	wxFileName fuser;
	fuser.Assign(mxconf::getResourceDir());
	wxString luascriptfile;
	wxString luauserscriptfile;
	wxString luascriptparameter;

	luascriptfile = mConf->get(CONFIG_LUA_SCRIPT, DEFAULT_LUA_FILE);
	luauserscriptfile = mConf->get(CONFIG_LUA_USER_SCRIPT, DEFAULT_LUA_USER_FILE);
	luascriptparameter = mConf->get(CONFIG_LUA_PARAMETER, DEFAULT_LUA_PARAMETER);

	f.SetFullName(luascriptfile);
	long dd = 0;
	if (f.IsFileReadable())
	{
		wxDateTime d = f.GetModificationTime();
		dd = d.GetTicks();
	}
	wxString slua = f.GetFullPath();

	fuser.SetFullName(luauserscriptfile);
	long dduser = 0;
	if (fuser.IsFileReadable())
	{
		wxDateTime d = fuser.GetModificationTime();
		dduser = d.GetTicks();
	}
	wxString sluauser = fuser.GetName();
	if (dduser > dd)
		dd = dduser;

	wxFileName dtmp(mxconf::getTmpDir());
	dtmp.SetName("expresseur_log");
	wxString slog = dtmp.GetFullPath();
	wxFileName dressource(mxconf::getResourceDir());
	wxString sressource = dressource.GetFullPath();
	wxFileName::SetCwd(mxconf::getCwdDir()) ;
	basslua_open(slua.c_str(), sluauser.c_str() , luascriptparameter.c_str(), all, dd, functioncallback, slog.c_str(), sressource.c_str(),true, timerDt);
}
void luafile::write(mxconf* mConf, wxTextFile *lfile)
{
	mConf->writeFile(lfile, CONFIG_LUA_SCRIPT, DEFAULT_LUA_FILE);
	mConf->writeFile(lfile, CONFIG_LUA_USER_SCRIPT, DEFAULT_LUA_USER_FILE);
	mConf->writeFile(lfile, CONFIG_LUA_PARAMETER, DEFAULT_LUA_PARAMETER);
}
void luafile::read(mxconf* mConf , wxTextFile *lfile)
{
	mConf->readFile(lfile, CONFIG_LUA_SCRIPT, DEFAULT_LUA_FILE);
	mConf->readFile(lfile, CONFIG_LUA_USER_SCRIPT, DEFAULT_LUA_USER_FILE);
	mConf->readFile(lfile, CONFIG_LUA_PARAMETER, DEFAULT_LUA_PARAMETER);
}
void luafile::functioncallback(double time , int nr_device , int type_msg , int channel , int value1 , int value2 , bool isProcessed )
{
	if (time == NULL_INT)
		return;
	wxCriticalSectionLocker locker(g_CriticalSection);
	g_lEventMidis.Append(new c_eventMidi((wxLongLong)(time*1000.0) ,  nr_device ,  type_msg ,  channel ,  value1 ,  value2 ,  isProcessed));
}
bool luafile::isCalledback(wxLongLong *time , int *nr_device , int *type_msg , int *channel , int *value1 , int *value2 , bool *isProcessed , bool *oneIsProcessed )
{
	wxCriticalSectionLocker locker(g_CriticalSection);
	if (g_lEventMidis.IsEmpty())
		return false;
	l_eventMidi::iterator iter_eventMidi;
	iter_eventMidi = g_lEventMidis.begin();
	c_eventMidi *m_eventMidi = *iter_eventMidi ;
	*time = m_eventMidi->time ;
	*nr_device = m_eventMidi->nr_device ;
	*type_msg = m_eventMidi->type_msg ;
	*channel = m_eventMidi->channel ;
	*value1 = m_eventMidi->value1 ; 
	*value2 = m_eventMidi->value2 ;
	*isProcessed = m_eventMidi->isProcessed;
	*oneIsProcessed = c_eventMidi::OneIsProcessed;
	g_lEventMidis.DeleteContents(true) 	;
	g_lEventMidis.pop_front();
	if (g_lEventMidis.IsEmpty())
		c_eventMidi::OneIsProcessed = false ;
	return true;
}
