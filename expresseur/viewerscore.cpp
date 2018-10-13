/////////////////////////////////////////////////////////////////////////////
// Name:        viewerscore.cpp
// Purpose:     virtual class to display the score /  expresseur V3
// Author:      Franck REVOLLE
// Modified by:
// Created:     27/07/2015
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

#include "global.h"
#include "mxconf.h"
#include "viewerscore.h"
#include "bitmapscore.h"


viewerscore::viewerscore(wxWindow *parent, wxWindowID id)
	: wxPanel(parent, id)
{

}
viewerscore::~viewerscore()
{
}
