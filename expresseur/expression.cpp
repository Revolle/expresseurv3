/////////////////////////////////////////////////////////////////////////////
// Name:        expression.cpp
// Purpose:     non-modal dialog for the expression /  expresseur V3
// Author:      Franck REVOLLE
// Modified by:
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
//#include "wx/notebook.h"
//#include "wx/bitmap.h"
//#include "wx/tglbtn.h"
#include "wx/spinctrl.h"
#include "wx/textfile.h"
//#include "wx/statline.h"
#include "wx/config.h"

#include "global.h"
#include "basslua.h"
#include "mxconf.h"
#include "expression.h"


enum
{
	IDM_EXPRESSION_VALUE = ID_EXPRESSION 
};


wxBEGIN_EVENT_TABLE(expression, wxDialog)
EVT_SIZE(expression::OnSize)
wxEND_EVENT_TABLE()

expression::expression(wxFrame *parent, wxWindowID id, const wxString &title)
: wxDialog(parent, id, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	mParent = parent;
	mThis = this;

	char name[MAXBUFCHAR];
	char help[MAXBUFCHAR] = "";
	nameValue.clear();
	helpValue.clear();
	unsigned int nrValue = 0;
	while (basslua_table(moduleGlobal, tableValues, nrValue, fieldName, name, NULL, tableGetKeyValue) == tableGetKeyValue)
	{
		nameValue.push_back(name);
		help[0] = '\0';
		basslua_table(moduleGlobal, tableValues, nrValue, fieldHelp, help, NULL, tableGetKeyValue);
		helpValue.push_back(help);
		nrValue++;
	}

	// some sizerFlags commonly used
	sizerFlagMaximumPlace.Proportion(1);
	sizerFlagMaximumPlace.Expand();
	sizerFlagMaximumPlace.Border(wxALL, 2);

	wxFlexGridSizer *paramsizer = new wxFlexGridSizer(2, wxSize(5, 5));
	paramsizer->AddGrowableCol(1);

	int v;
	wxString s  ;
	for (nrValue = 0; nrValue < nameValue.size(); nrValue++)
	{
		basslua_table(moduleGlobal, tableValues, nrValue, fielDefaultValue, NULL, &v, tableGetKeyValue);
		s = nameValue[nrValue];
		if (configExists(CONFIG_EXPRESSIONVALUE, false, nameValue[nrValue]))
			v = configGet(CONFIG_EXPRESSIONVALUE, v, false, nameValue[nrValue]);
		paramsizer->Add(new wxStaticText(this, wxID_ANY, s), sizerFlagMaximumPlace);
		basslua_table(moduleGlobal, tableValues, -1, nameValue[nrValue], NULL, &v, tableSetKeyValue);
		basslua_table(moduleGlobal, tableValues, nrValue, fieldCallFunction, NULL, &v, tableCallKeyFunction);
		configSet(CONFIG_EXPRESSIONVALUE, v, false, nameValue[nrValue]);
		mValue[nrValue] = new wxSlider(this, IDM_EXPRESSION_VALUE + nrValue , v, 0, 127, wxDefaultPosition, wxDefaultSize, 4L);
		mValue[nrValue]->Bind(wxEVT_SLIDER, &expression::OnValue, this);
		mValue[nrValue]->SetToolTip(helpValue[nrValue]);
		paramsizer->Add(mValue[nrValue], sizerFlagMaximumPlace);
	}
	txtValue = new wxStaticText(this, wxID_ANY, "");
	txtValue->SetForegroundColour(*wxLIGHT_GREY);
	paramsizer->Add(txtValue, sizerFlagMaximumPlace);
	paramsizer->Add(CreateButtonSizer(wxCLOSE), sizerFlagMaximumPlace);
	SetSizerAndFit(paramsizer);

}
expression::~expression()
{
}
void expression::OnSize(wxSizeEvent& WXUNUSED(event))
{
	Layout();
}
void expression::savePos()
{
	wxRect mrect = GetRect();
	configSet(CONFIG_EXPRESSIONWIDTH, mrect.GetWidth());
	configSet(CONFIG_EXPRESSIONHEIGHT, mrect.GetHeight());
	configSet(CONFIG_EXPRESSIONX, mrect.GetX());
	configSet(CONFIG_EXPRESSIONY, mrect.GetY());

}
void expression::scanValue()
{
	// scan the value which are currently in place, and update the GUI if necessary
	// this scan is called periodically by a global recurrent timer on main thread
	int v;
	unsigned int nb = nameValue.size();
	for (unsigned int nrValue = 0; nrValue < nb ; nrValue++)
	{
		basslua_table(moduleGlobal, tableValues, -1, nameValue[nrValue], NULL, &v, tableGetKeyValue);
		if (v != mValue[nrValue]->GetValue())
			mValue[nrValue]->SetValue(v);
	}
}
void expression::OnValue(wxEvent& event)
{
	int nrValue = event.GetId() - IDM_EXPRESSION_VALUE;

	int v = mValue[nrValue]->GetValue();
	wxString nameV = nameValue[nrValue];
	configSet(CONFIG_EXPRESSIONVALUE, v, false, nameV);

	basslua_table(moduleGlobal, tableValues, -1, nameV, NULL, &v, tableSetKeyValue);
	basslua_table(moduleGlobal, tableValues, nrValue, fieldCallFunction, NULL, &v, tableCallKeyFunction); 

	wxString s;
	s.Printf("%s %d ", nameV, v);
	txtValue->SetLabel(s);
}

void expression::reset()
{
	int v;
	for (unsigned int nrValue = 0; nrValue < nameValue.size(); nrValue++)
	{
		basslua_table(moduleGlobal, tableValues, nrValue, fielDefaultValue, NULL, &v, tableGetKeyValue);
		if (configExists(CONFIG_EXPRESSIONVALUE, false, nameValue[nrValue]))
			v = configGet(CONFIG_EXPRESSIONVALUE, v, false, nameValue[nrValue]);
		basslua_table(moduleGlobal, tableValues, -1, nameValue[nrValue], NULL, &v, tableSetKeyValue);
		basslua_table(moduleGlobal, tableValues, nrValue, fieldCallFunction, NULL, &v, tableCallKeyFunction);
	}
}
void expression::write(wxTextFile *lfile)
{
	for (unsigned int nrValue = 0; nrValue < nameValue.size(); nrValue++)
	{
		int v;
		basslua_table(moduleGlobal, tableValues, nrValue, fielDefaultValue, NULL, &v, tableGetKeyValue);
		configWriteFile(lfile, CONFIG_EXPRESSIONVALUE, v, false, nameValue[nrValue]);
	}
}
void expression::read(wxTextFile *lfile)
{
	for (unsigned int nrValue = 0; nrValue < nameValue.size(); nrValue++)
	{
		int v;
		basslua_table(moduleGlobal, tableValues, nrValue, fielDefaultValue, NULL, &v, tableGetKeyValue);
		configReadFile(lfile, CONFIG_EXPRESSIONVALUE, v, false, nameValue[nrValue]);
	}
}
