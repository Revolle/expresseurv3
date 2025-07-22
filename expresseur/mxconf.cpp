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
wxString mPrefix = "";
wxConfig mConfig(APP_NAME);

void setDir()
{
	if ( todoDir )
	{
		todoDir = false ;
		// Mac : stored in Library/Preferences/Expresseur... 
		mConfig.Write("creation", 1);
		if (!mConfig.Flush())
			wxMessageBox("error flush mxconf", "mxconf");
#ifdef RUN_WIN
		confPath = "regedit.exe HKEY_CURRENT_USER\\Software\\ExpresseurV3";
#else
		confPath = ((wxFileConfig*)mConfig)->GetPath();
#endif
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


		wxString s;

		wxFileName fuserDir;
		fuserDir.AssignDir(mpath.GetAppDocumentsDir());
		if (fuserDir.GetFullPath().Contains(APP_NAME) == false)
			fuserDir.AppendDir(APP_NAME);
		userDir = fuserDir.GetFullPath();
		if (!fuserDir.DirExists())
			wxFileName::Mkdir(userDir);
		if (!fuserDir.DirExists())
			wxMessageBox(userDir, "Directory user error");
		if (mConfig.Exists(CONFIG_USERDIRECTORY))
		{
			mConfig.Read(CONFIG_USERDIRECTORY, &s, userDir);
			if (s.length() > 3)
			{
				userDir = s;
			}
		}
		else
		{
			mConfig.Write(CONFIG_USERDIRECTORY, userDir);
		}

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
wxString getAppDir()
{
	setDir() ;
	return appDir ;
}
wxString getCwdDir()
{
	setDir();
	return cwdDir ;
}
wxString getTmpDir()
{
	setDir();
	return tmpDir ;
}
wxString getUserDir()
{
	setDir();
	return userDir;
}
wxString getConfPath()
{
	setDir();
	return confPath;
}
wxString getResourceDir()
{
	setDir();
	return resourceDir ;
}

wxConfig *configGet()
{
	return &mConfig;
}
void configErase()
{
	mConfig.DeleteAll() ;
}
void configSetPrefix(wxArrayString nameOpenMidiOutDevices)
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
wxString configPrefixKey(wxString key, bool prefix, wxString name)
{
	wxString s1;
	if (name.IsEmpty())
		s1.Printf("%s%s", prefix ? mPrefix : "", key);
	else
		s1.Printf("%s%s/%s", prefix ? mPrefix : "", key, name);
	return s1;
}
wxString configGet(wxString key, wxString defaultvalue, bool prefix, wxString name)
{
	wxString s;
	mConfig.Read(configPrefixKey(key, prefix, name), &s, defaultvalue);
	mConfig.Write(configPrefixKey(key, prefix, name), s);
	return s;
}
long configGet(wxString key, long defaultvalue, bool prefix, wxString name)
{
	long l;
	mConfig.Read(configPrefixKey(key, prefix, name), &l, defaultvalue);
	mConfig.Write(configPrefixKey(key, prefix, name), l);
	return(l);
}

void configSet(wxString key, wxString s, bool prefix, wxString name)
{
	mConfig.Write(configPrefixKey(key, prefix, name), s);
}
void configSet(wxString key, long l, bool prefix, wxString name)
{
	mConfig.Write(configPrefixKey(key, prefix, name), l);
}

void configRemove(wxString key, bool prefix, wxString name)
{
	mConfig.DeleteEntry(configPrefixKey(key, prefix = false, name));
}

long configWriteFile(wxTextFile *lfile, wxString key, long defaultvalue, bool prefix, wxString name)
{
	wxString s, v;
	long l = configGet(key, defaultvalue, prefix, name);
	s.Printf("%s=%ld", configPrefixKey( key , false, name), l);
	lfile->AddLine(s);
	return l;
}
wxString configWriteFile(wxTextFile *lfile, wxString key, wxString defaultvalue, bool prefix , wxString name)
{
	wxString s;
	wxString l = configGet(key, defaultvalue, prefix, name);
	s.Printf("%s=%s", configPrefixKey(key, false, name), l);
	lfile->AddLine(s);
	return l;
}
wxString configReadFileLines(wxTextFile *lfile, wxString key)
{
	wxString str;
	for (str = lfile->GetFirstLine(); !lfile->Eof(); str = lfile->GetNextLine())
	{
		if (str.StartsWith(key))
			return str;
	}
	return wxEmptyString;
}
bool configReadFile(wxTextFile *lfile, wxString key, wxString defaultvalue, bool prefix, wxString name)
{
	wxString s, s2;
	s = configReadFileLines(lfile, configPrefixKey(key, false, name));
	if (s.IsEmpty())
		return false;
	if (!s.StartsWith(configPrefixKey(key, false, name)))
		s2 = defaultvalue;
	else
		s2 = s.AfterFirst('=');
	configSet(key, s2, prefix , name);
	return true;
}
bool configReadFile(wxTextFile *lfile, wxString key, long defaultvalue, bool prefix, wxString name)
{
	wxString s, s2;
	long l;
	s = configReadFileLines(lfile, configPrefixKey(key, false, name));
	if (s.IsEmpty())
		return false;
	if (!s.StartsWith(configPrefixKey(key, false, name)))
		l = defaultvalue;
	else
	{
		s2 = s.AfterFirst('=');
		s2.ToLong(&l);
	}
	configSet(key, l, prefix , name);
	return(true);
}
bool configExists(wxString key, bool prefix, wxString name)
{
	return (mConfig.Exists(configPrefixKey(key, prefix, name)));
}

