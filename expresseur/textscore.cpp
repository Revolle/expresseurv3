/////////////////////////////////////////////////////////////////////////////
// Name:        textscore.cpp
// Purpose:     display a text of the score /  expresseur V3
// Author:      Franck REVOLLE
// Modified by:
// Created:     27/07/2015
// update : 31 / 10 / 2016 10 : 00
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


#include "wx/toolbar.h"
#include "wx/log.h"
#include "wx/image.h"
#include "wx/filedlg.h"
#include "wx/colordlg.h"
#include "wx/srchctrl.h"
#include "wx/spinctrl.h"
#include "wx/textfile.h"
#include "wx/ffile.h"
#include "wx/filefn.h"
#include "wx/filename.h"
#include "wx/filehistory.h"
#include "wx/tglbtn.h"
#include "wx/config.h"
#include "wx/listctrl.h"
#include "wx/filepicker.h"
#include "wx/msgdlg.h"
#include "wx/scrolbar.h"
#include "wx/choicdlg.h"
#include "wx/xml/xml.h"
#include "wx/dynarray.h"
#include "wx/sstream.h"
#include "wx/protocol/http.h"
#include "wx/wizard.h"
#include "wx/clipbrd.h"
#include "wx/stdpaths.h"
#include "wx/dir.h"
#include "wx/dynlib.h"

#include "global.h"
#include "basslua.h"
#include "luabass.h"
#include "mxconf.h"
#include "viewerscore.h"
#include "mixer.h"
#include "editshortcut.h"
#include "midishortcut.h"
#include "expression.h"
#include "logError.h"
#include "emptyscore.h"
#include "bitmapscore.h"
#include "musicxml.h"
#include "musicxmlcompile.h"
#include "musicxmlscore.h"
#include "textscore.h"
#include "luafile.h"
#include "expresseur.h"


wxBEGIN_EVENT_TABLE(textscore, wxPanel)
EVT_SIZE(textscore::OnSize)
END_EVENT_TABLE()

textscore::textscore(wxWindow *parent, wxWindowID id, mxconf* lMxconf)
: wxTextCtrl(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxSUNKEN_BORDER | wxTE_RICH | wxTE_DONTWRAP)
{
	mParent = parent;
	mConf = lMxconf;
	
	textAttrRecognized.SetFontFamily(wxFONTFAMILY_TELETYPE);
	textAttrNormal.SetFontFamily(wxFONTFAMILY_TELETYPE);
	textAttrPosition.SetFontFamily(wxFONTFAMILY_TELETYPE);

	textAttrRecognized.SetTextColour(*wxBLUE);
	textAttrNormal.SetTextColour(*wxBLACK);
	textAttrPosition.SetTextColour(*wxRED);

	textAttrRecognized.SetFlags(wxTEXT_ATTR_FONT_FAMILY | wxTEXT_ATTR_FONT_POINT_SIZE | wxTEXT_ATTR_TEXT_COLOUR);
	textAttrNormal.SetFlags(wxTEXT_ATTR_FONT_FAMILY | wxTEXT_ATTR_FONT_POINT_SIZE | wxTEXT_ATTR_TEXT_COLOUR);
	textAttrPosition.SetFlags(wxTEXT_ATTR_FONT_FAMILY | wxTEXT_ATTR_FONT_POINT_SIZE | wxTEXT_ATTR_TEXT_COLOUR);

	SetDefaultStyle(textAttrNormal);

	oldchordStart = -1;
	oldchordEnd = -1;
	oldsectionStart = -1;
	oldsectionEnd = -1;
	prevInsertionPoint = -1;
}
textscore::~textscore()
{
	basslua_call(moduleChord, functionChordSetPosition, "i", 1);
}
void textscore::compileText()
{
	if (editMode) return;

	if (oldText == GetValue())
		return;

	bool userModification = IsModified();
	oldText = GetValue();
	char buf[5000];
	strcpy(buf, oldText.c_str());
	basslua_call(moduleChord, functionChordSetScore, "s", buf);
	int lenText = oldText.Length();
	SetStyle(0, lenText, textAttrNormal);
	int start = -1;
	int end = -1 ;
	basslua_call(moduleChord, functionChordGetRecognizedScore, ">ii", &start, &end);
	while (start >= 0)
	{
		if ((start > 0) && (end > 0))
		{
			SetStyle(start - 1, end, textAttrRecognized);
		}
		basslua_call(moduleChord, functionChordGetRecognizedScore, ">ii", &start, &end);
	}
	if (!userModification)
		DiscardEdits();
	
	oldchordStart = -1;
	oldchordEnd = -1;
	oldsectionStart = -1;
	oldsectionEnd = -1;
}
int textscore::scanPosition(bool editmode)
{
	int sectionStart, sectionEnd, chordStart, chordEnd , nrChord , posline;
	bool userModification = this->IsModified();
	basslua_call(moduleChord, functionChordGetPosition, ">iiiiii", &sectionStart, &sectionEnd, &chordStart, &chordEnd, &nrChord , &posline);
	mlog_in("scanPosition nrChord=%d nrline=%d", nrChord, posline);
	if (nrChord >= 0)
	{
		/*
		if ((sectionStart != oldsectionStart) || (sectionEnd != oldsectionEnd))
		{
			if (oldsectionStart > 0)
				SetStyle(oldsectionStart - 1, oldsectionEnd, textAttrRecognized);
			if (sectionStart > 0)
				SetStyle(sectionStart - 1, sectionEnd, textAttrPosition);
			oldsectionStart = sectionStart;
			oldsectionEnd = sectionEnd;
		}
		*/
		if ((chordStart != oldchordStart) || (chordEnd != oldchordEnd))
		{
			if (oldchordStart > 0)
				SetStyle(oldchordStart - 1, oldchordEnd, textAttrRecognized);
			if (chordStart > 0)
				SetStyle(chordStart - 1, chordEnd, textAttrPosition);
			oldchordStart = chordStart ;
			oldchordEnd = chordEnd;
/*			if (!editmode)
			{

				if (posline != previousLinePos)
				{
					ShowPosition(posline);
					previousLinePos = posline;
				}
			}
*/
		}
	}
	if (! userModification)
		DiscardEdits();
	return nrChord;
}
void textscore::scanTextPosition()
{
	if ( editMode )  return ;

	int insertionPoint = GetInsertionPoint();
	if ( insertionPoint != prevInsertionPoint)
	{
		wxString sl ;
		//sl.Printf("textscore::scanTextPosition %d %d",insertionPoint,prevInsertionPoint);
		// mlog_in(sl);
		basslua_call(moduleChord, functionChordSetPosition, "i", insertionPoint);
	}
	prevInsertionPoint = insertionPoint;
}
bool textscore::setFile(const wxFileName &filename)
{
	bool retcode = false;
	oldText.Empty();
	Clear();
	SetDefaultStyle(textAttrNormal);
	wxFileName filetext(filename);
	filetext.SetExt(SUFFIXE_TEXT);
	if (!filetext.FileExists())
	{
		wxTextFile f(filetext.GetFullPath());
		if (!f.Create())
			return false;
		f.Close();
	}
	if (LoadFile(filetext.GetFullPath()))
	{
		oldchordStart = -1;
		oldchordEnd = -1;
		oldsectionStart = -1;
		oldsectionEnd = -1;
		retcode = true;
	}
	SetModified(false);
	return retcode;
}
void textscore::saveFile(const wxFileName &filename)
{
	wxFileName f(filename);
	f.SetExt(SUFFIXE_TEXT);
	wxString s = f.GetFullPath();
	if (SaveFile(s))
	{
		SetModified(false);
	}
}
bool textscore::needToSave()
{
	return this->IsModified();
}
void textscore::noNeedToSave()
{
	SetModified(false);
}
void textscore::OnSize(wxSizeEvent& WXUNUSED(event))
{
	Refresh();
}
void textscore::setEditMode(bool ieditMode)
{
	editMode = ieditMode ;

	if (editMode)
		SetBackgroundColour(*wxWHITE);
	else
		SetBackgroundColour(wxColour(220, 220, 220, wxALPHA_OPAQUE));
	// Enable(editMode);
	if ( editMode)
		SetExtraStyle(! wxTE_READONLY) ;	
	else
		SetExtraStyle(wxTE_READONLY) ;	

}
void textscore::setFontSize(int s)
{
	sizeFont = s;
	if (sizeFont < 4)
		sizeFont = 4;
	if (sizeFont > 40)
		sizeFont = 40;


	textAttrRecognized.SetFontSize(sizeFont);
	textAttrNormal.SetFontSize(sizeFont);
	textAttrPosition.SetFontSize(sizeFont);

	SetDefaultStyle(textAttrNormal);
	
}
void textscore::zoom(int zoom)
{
	switch (zoom)
	{
	case -3: sizeFont = 6;  break;
	case -2: sizeFont = 8;  break;
	case -1: sizeFont = 10;  break;
	case 0: sizeFont = 12;  break;
	case 1: sizeFont = 14;  break;
	case 2: sizeFont = 16;  break;
	case 3: sizeFont = 18;  break;
	default: sizeFont = 12;  break;
	}

	setFontSize(sizeFont);
	wxTextAttr textAttr;
	textAttr.SetFontSize(sizeFont);
	textAttr.SetFontFamily(wxFONTFAMILY_TELETYPE);
	textAttr.SetFlags(wxTEXT_ATTR_FONT_FAMILY | wxTEXT_ATTR_FONT_POINT_SIZE);
	wxString stext = GetValue();
	SetStyle(0, stext.Len() , textAttr);
}
void textscore::savePlayback(wxString f)
{
	wxString m(SET_PLAYBACK);
	wxASSERT(m.StartsWith("~"));

	wxString s = this->GetValue();
	wxString p = s.BeforeLast('~');
	if (p.IsEmpty())
		p = s;
	p += f;
	this->SetValue(p);
	this->MarkDirty();
}