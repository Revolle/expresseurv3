/////////////////////////////////////////////////////////////////////////////
// Name:        mxconf.cpp
// Purpose:     to read and write the configuration /  expresseur V3
// Author:      Franck REVOLLE
// Modified by:
// Created:     27/07/2015
// update : 13/11/2016 22:00
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
#include "wx/hash.h"
#include "wx/stdpaths.h"

#include "global.h"
#include "basslua.h"
#include "luabass.h"
#include "mxconf.h"

bool todoDir = true;
wxString appDir ;
wxString cwdDir ;
wxString tmpDir ;
wxString userDir;
wxString confPath;
wxString resourceDir ;

mxconf::mxconf()
{
	mConfig = new wxConfig(APP_NAME);
	// Mac : stored in Library/Preferences/Expresseur... 
	mConfig->Write("creation",1);
	if ( ! mConfig->Flush() )
		wxMessageBox("error flush mxconf","mxconf");
	mPrefix = "";
#ifdef RUN_WIN
	confPath = "regedit.exe HKEY_CURRENT_USER\\Software\\ExpresseurV3";
#else
	confPath = ((wxFileConfig *)mConfig)->GetPath() ;
#endif
}
mxconf::~mxconf()
{
	mConfig->Flush();
	delete mConfig;
}
void mxconf::setDir()
{
	if ( todoDir )
	{
		todoDir = false ;
		wxFileName fd(wxStandardPaths::Get().GetExecutablePath());
		fd.SetName("");
		fd.SetExt("");
		cwdDir = fd.GetFullPath();
#ifdef RUN_MAC
		fd.RemoveLastDir();
		fd.RemoveLastDir();
		fd.RemoveLastDir();
#endif
		appDir = fd.GetFullPath();

		wxStandardPaths mpath = wxStandardPaths::Get();
		wxFileName fuserDir;
		// 2025 04 27 pb de membe non static ????
		//if (! mConfig->Exists(CONFIG_USERDIRECTORY))
		//	mConfig->Write(CONFIG_USERDIRECTORY, mpath.GetAppDocumentsDir());
		//fuserDir.AssignDir(mConfig->read(CONFIG_USERDIRECTORY, mpath.GetAppDocumentsDir()));
		fuserDir.AssignDir(mpath.GetAppDocumentsDir());
		if (fuserDir.GetFullPath().Contains(APP_NAME) == false)
			fuserDir.AppendDir(APP_NAME);
		userDir = fuserDir.GetFullPath() ;
		if ( ! fuserDir.DirExists() )
			wxFileName::Mkdir(userDir) ;
		if ( ! fuserDir.DirExists() )
			wxMessageBox(userDir , "Directory user error");	

		wxFileName fresourcesDir;
		fresourcesDir.AssignDir(userDir);
		fresourcesDir.AppendDir(DIR_RESOURCES);
		resourceDir = fresourcesDir.GetFullPath() ;
		if ( ! fresourcesDir.DirExists() )
			wxFileName::Mkdir(resourceDir) ;
		if ( ! fresourcesDir.DirExists() )
			wxMessageBox(resourceDir , "Directory ressource error");	

		//wxFileName dtmp;
		//wxString stmp = wxFileName::GetTempDir() ;
		//dtmp.AssignDir(stmp);
		//dtmp.AppendDir(APP_NAME);
		wxFileName ftmpDir;
		ftmpDir.AssignDir(resourceDir);
		ftmpDir.AppendDir("tmp");
		tmpDir = ftmpDir.GetFullPath() ;
		if ( ! ftmpDir.DirExists() )
			wxFileName::Mkdir(tmpDir) ;
		if ( ! ftmpDir.DirExists() )
			wxMessageBox(tmpDir , "Directory tmp error");	

	}
}
wxString mxconf::getAppDir()
{
	setDir() ;
	return appDir ;
}
wxString mxconf::getCwdDir()
{
	setDir();
	return cwdDir ;
}
wxString mxconf::getTmpDir()
{
	setDir();
	return tmpDir ;
}
wxString mxconf::getUserDir()
{
	setDir();
	return userDir;
}
wxString mxconf::getConfPath()
{
	setDir();
	return confPath;
}
wxString mxconf::getResourceDir()
{
	setDir();
	return resourceDir ;
}

wxConfig *mxconf::getConfig()
{
	return mConfig;
}
void mxconf::deleteConf()
{
	mConfig->DeleteAll() ;
}
void mxconf::setPrefix(wxArrayString nameOpenMidiOutDevices)
{
	// the prefix is a checksum of all valid midiout's name
	wxString name;
	wxString names;
	wxString spipe;
	int nbOut = nameOpenMidiOutDevices.GetCount();
	if (nbOut == 0)
	{
		names = "no midi-out valid device";
	}
	else
	{
		for (int i = 0; i < nbOut; i++)
		{
			wxString s= nameOpenMidiOutDevices[i] ;
			if ((s.length() > 4) && (s.Mid(2, 2) == "- "))
				s = s.Mid(3); // suppress the prefix "X- "
			s = s.ToAscii(); 
			s.Replace("/", "_", true);
			names += spipe + s ;
			spipe = "|";
		}
	}
	mPrefix.Printf("%s/%s/", CONFIG_HARDWARE, names);
}

long mxconf::writeFile(wxTextFile *lfile, wxString key, long defaultvalue, bool prefix, wxString name)
{
	wxString s, v;
	long l = this->get(key, defaultvalue, prefix, name);
	s.Printf("%s=%ld", prefixKey( key , false, name), l);
	lfile->AddLine(s);
	return l;
}
wxString mxconf::writeFile(wxTextFile *lfile, wxString key, wxString defaultvalue, bool prefix , wxString name)
{
	wxString s;
	wxString l = this->get(key, defaultvalue, prefix, name);
	s.Printf("%s=%s", prefixKey(key, false, name), l);
	lfile->AddLine(s);
	return l;
}

wxString mxconf::readFileLines(wxTextFile *lfile, wxString key)
{
	wxString str;
	for (str = lfile->GetFirstLine(); !lfile->Eof(); str = lfile->GetNextLine())
	{
		if (str.StartsWith(key))
			return str;
	}
	return wxEmptyString;
}
bool mxconf::readFile(wxTextFile *lfile, wxString key, wxString defaultvalue, bool prefix, wxString name)
{
	wxString s, s2;
	s = readFileLines(lfile, prefixKey(key, false, name));
	if (s.IsEmpty())
		return false;
	if (!s.StartsWith(prefixKey(key, false, name)))
		s2 = defaultvalue;
	else
		s2 = s.AfterFirst('=');
	this->set(key, s2, prefix , name);
	return true;
}
bool mxconf::readFile(wxTextFile *lfile, wxString key, long defaultvalue, bool prefix, wxString name)
{
	wxString s, s2;
	long l;
	s = readFileLines(lfile, prefixKey(key, false, name));
	if (s.IsEmpty())
		return false;
	if (!s.StartsWith(prefixKey(key, false, name)))
		l = defaultvalue;
	else
	{
		s2 = s.AfterFirst('=');
		s2.ToLong(&l);
	}
	this->set(key, l, prefix , name);
	return(true);
}
bool mxconf::exists(wxString key, bool prefix, wxString name)
{
	return (mConfig->Exists(prefixKey(key, prefix, name)));
}
wxString mxconf::get(wxString key, wxString defaultvalue, bool prefix, wxString name)
{
	wxString s;
	mConfig->Read(prefixKey(key, prefix,name), &s, defaultvalue);
	mConfig->Write(prefixKey(key, prefix,name), s);
	return s;
}
long mxconf::get(wxString key, long defaultvalue, bool prefix, wxString name)
{
	long l;
	mConfig->Read(prefixKey(key, prefix, name), &l, defaultvalue);
	mConfig->Write(prefixKey(key, prefix, name), l);
	return(l);
}

void mxconf::set(wxString key, wxString s, bool prefix, wxString name)
{
	mConfig->Write(prefixKey(key, prefix, name), s);
}
void mxconf::set(wxString key, long l, bool prefix, wxString name)
{
	mConfig->Write(prefixKey(key, prefix, name), l);
}

void mxconf::remove(wxString key, bool prefix, wxString name)
{
	mConfig->DeleteEntry(prefixKey(key, prefix = false, name));
}
wxString mxconf::prefixKey(wxString key, bool prefix, wxString name)
{
	wxString s1;
	if (name.IsEmpty())
		s1.Printf("%s%s", prefix ? mPrefix : "", key);
	else
		s1.Printf("%s%s/%s", prefix ? mPrefix : "", key, name);
	return s1;
}
