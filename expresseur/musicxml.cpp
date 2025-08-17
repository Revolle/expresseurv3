/////////////////////////////////////////////////////////////////////////////
// Name:        musicxml.cpp
// Purpose:     classe for musicxml /  expresseur V3
// Author:      Franck REVOLLE
// Modified by:
// Created:     27/09/2015
// update : 20/11/2016 19:00
// Copyright:   (c) Franck REVOLLE Expresseur
// Licence:    Expresseur licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#include <vector>
#include <algorithm>

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
#include "wx/dcbuffer.h"
#include "wx/xml/xml.h"
#include "wx/filefn.h"
#include "wx/wfstream.h"

#include "global.h"

#include "musicxml.h"

static int gchord_counter = 0;
static int gtranspose = 0;

static int divisionsMax = 4 ;
static 	int ratioDivisions = 1 ;

wxString encodeXML(wxString s)
{
	wxString sw = s;
	sw.Replace("&", "&amp;");
	sw.Replace("\"", "&quot;");
	sw.Replace("\'", "&apos;");
	sw.Replace("<", "&lt;");
	sw.Replace(">", "&gt;");
	return sw;
}

int GetIntAttribute(wxXmlNode *xmlnode, wxString name)
{
	// return the integer value of an attribute in a node. Return NULL_INT if no attribute
	if ((xmlnode == NULL) || (!xmlnode->HasAttribute(name)))
		return NULL_INT;
	wxString s = xmlnode->GetAttribute(name,"nothing");
	long l = 0;
	if (s.ToLong(&l))
		return l;
	else
		return NULL_INT;
}
wxString GetStringAttribute(wxXmlNode *xmlnode, wxString name)
{
	// return the string value of an attribute in a node. Return NULL_STRING if no attribute
	if ((xmlnode == NULL) || (!xmlnode->HasAttribute(name)))
		return NULL_STRING;
	wxString s = xmlnode->GetAttribute(name);
	return s;
}
int GetIntContent(wxXmlNode *xmlnode)
{
	// return the integer value in a node. Return NULL_INT if no content
	if (xmlnode == NULL)
		return NULL_INT;
	wxString s = xmlnode->GetNodeContent();
	long l = 0;
	if (s.ToLong(&l))
		return l;
	else
		return NULL_INT;
}
int GetFloatContent(wxXmlNode *xmlnode)
{
	// return the float value in a node. Return NULL_INT if no content
	if (xmlnode == NULL)
		return NULL_INT;
	wxString s = xmlnode->GetNodeContent();
	double f = 0;
	if (s.ToDouble(&f))
		return f;
	else
		return NULL_INT;
}
wxString GetStringContent(wxXmlNode *xmlnode)
{
	// return the string value in a node. Return NULL_STRING if no content
	if (xmlnode == NULL)
		return NULL_STRING;
	wxString s = xmlnode->GetNodeContent();
	return s;
}
wxString GetCleanStringContent(wxXmlNode *xmlnode)
{
	// return the string value in a node, cleaned as alphanumeric. Return NULL_STRING if no content
	if (xmlnode == NULL)
		return NULL_STRING;
	wxString s = xmlnode->GetNodeContent();
	wxString sclean;
	for (int i = 0; i < s.length(); i++)
	{
		int c = static_cast<int>(s[i]);
		if (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9')) || (c == '_'))
			sclean += static_cast<char>(c);
	}
	return sclean;
}

// default attribute of a MusicXML object
//---------------------------------------
c_default_xy::c_default_xy()
{}
c_default_xy::c_default_xy(wxXmlNode *xmlnode)
{
	default_x = GetIntAttribute(xmlnode, "default-x");
	default_y = GetIntAttribute(xmlnode, "default-y");
	relative_x = GetIntAttribute(xmlnode, "relative-x");
	relative_y = GetIntAttribute(xmlnode, "relative-y");
}
c_default_xy::c_default_xy(const c_default_xy &default_xy)
{
	default_x = default_xy.default_x;
	default_y = default_xy.default_y;
	relative_x = default_xy.relative_x;
	relative_y = default_xy.relative_y;
}
void c_default_xy::write_xy(wxFFile *f)
{
	if (default_x != NULL_INT)
		f->Write(wxString::Format(" default-x=\"%d\" ", default_x));
	if (default_y != NULL_INT)
		f->Write(wxString::Format(" default-y=\"%d\" ", default_y));
	if (relative_x != NULL_INT)
		f->Write(wxString::Format(" relative-x=\"%d\" ", default_x));
	if (relative_y != NULL_INT)
		f->Write(wxString::Format(" relative-y=\"%d\" ", default_x));
}
// score-partwise/part-list/part
//------------------------------
c_score_part::c_score_part()
{}
c_score_part::c_score_part(wxXmlNode *xmlnode)
{
	id = GetStringAttribute(xmlnode,"id");
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "part-name")
		{
			part_name = GetCleanStringContent(child);
			part_alias = part_name;
		}
		else if (name == "part-abbreviation")
		{
			part_abbreviation = GetCleanStringContent(child);
			part_alias_abbreviation = part_abbreviation;
		}
		child = child->GetNext();
	}
}
c_score_part::c_score_part(wxString iid, wxString ipart_name, wxString ipart_abbreviation)
{
	id = iid;
	part_name = ipart_name;
	part_abbreviation = ipart_abbreviation;
	part_alias = part_name;
	part_alias_abbreviation = part_abbreviation;
	play = true;
	view = true;
}
c_score_part::c_score_part(const c_score_part & score_part)
{
	id = score_part.id;
	part_name = score_part.part_name;
	part_abbreviation = score_part.part_abbreviation;
	part_alias = score_part.part_alias;
	part_alias_abbreviation = score_part.part_alias_abbreviation;
	play = score_part.play;
	view = score_part.view;
}
void c_score_part::write(wxFFile *f)
{
	if (!view)
		return;
	f->Write(wxString::Format("<score-part "));
	if (id != NULL_STRING)
		f->Write(wxString::Format("id=\"%s\" ", id));
	f->Write(wxString::Format(" >\n"));
	if (part_name != NULL_STRING)
		f->Write(wxString::Format("<part-name>%s</part-name>\n",encodeXML(part_alias)));
	if (part_abbreviation != NULL_STRING)
		f->Write(wxString::Format("<part-abbreviation>%s</part-abbreviation>\n", encodeXML(part_alias_abbreviation)));
	f->Write(wxString::Format("</score-part>\n"));
}

// score-partwise/part-list
//-------------------------
c_part_list::c_part_list()
{
}
c_part_list::c_part_list(wxXmlNode *xmlnode)
{
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "score-part")
			score_parts.push_back(c_score_part(child));
		child = child->GetNext();
	}
}
c_part_list::c_part_list(const c_part_list &part_list)
{
	for (auto current_score_part : part_list.score_parts ) 
	{
		score_parts.push_back( c_score_part(current_score_part));
	}
}
c_part_list::~c_part_list()
{
	score_parts.clear();
}
void c_part_list::write(wxFFile *f)
{
	f->Write(wxString::Format("<part-list>\n"));
	for (auto current_score_part : score_parts ) 
	{
		current_score_part.write(f);
	}
	f->Write(wxString::Format("</part-list>\n"));
}

// score-partwise/list/part/measure/attributes/transpose
//--------------------------------------------
c_transpose::c_transpose()
{}
c_transpose::c_transpose(wxXmlNode *xmlnode)
{
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "diatonic")
			diatonic = GetIntContent(child);
		else if (name == "chromatic")
			chromatic = GetIntContent(child);
		else if (name == "octave-change")
			octave_change = GetIntContent(child);
		child = child->GetNext();
	}
}
c_transpose::c_transpose(const c_transpose & transpose)
{
	diatonic = transpose.diatonic;
	chromatic = transpose.chromatic;
	octave_change = transpose.octave_change;
}
void c_transpose::write(wxFFile *f)
{
	f->Write(wxString::Format("<transpose>\n"));
	if (diatonic != NULL_INT)
		f->Write(wxString::Format("<diatonic>%d</diatonic>\n", diatonic));
	if (chromatic != NULL_INT)
		f->Write(wxString::Format("<chromatic>%d</chromatic>\n", chromatic));
	if (octave_change != NULL_INT)
		f->Write(wxString::Format("<octave-change>%d</octave-change>\n", octave_change));
	f->Write(wxString::Format("</transpose>\n"));
}

// score-partwise/list/part/measure/attributes/time
//--------------------------------------------
c_time::c_time()
{}
c_time::c_time(wxXmlNode *xmlnode)
{
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "beats")
			beats = GetIntContent(child);
		else if (name == "beat-type")
			beat_type = GetIntContent(child);
		else if (name == "symbol")
			symbol = GetStringContent(child);
		child = child->GetNext();
	}
}
c_time::c_time(const c_time & time)
{
	beats = time.beats;
	beat_type = time.beat_type;
	symbol = time.symbol;
}
void c_time::write(wxFFile *f)
{
	f->Write(wxString::Format("<time"));
	if (symbol != NULL_STRING)
		f->Write(wxString::Format(" symbol=\"%s\" ", symbol));
	f->Write(wxString::Format(">\n"));
	if (beats != NULL_INT)
		f->Write(wxString::Format("<beats>%d</beats>\n", beats));
	if (beat_type != NULL_INT)
		f->Write(wxString::Format("<beat-type>%d</beat-type>\n", beat_type));
	f->Write(wxString::Format("</time>\n"));
}

// score-partwise/list/part/measure/attributes/key
//--------------------------------------------
c_key::c_key()
{}
c_key::c_key(wxXmlNode *xmlnode)
{
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "fifths")
			fifths = GetIntContent(child);
		else if (name == "mode")
			mode = child->GetNodeContent();
		child = child->GetNext();
	}
}
c_key::c_key(const c_key & key)
{
	fifths = key.fifths;
	mode = key.mode;
}
void c_key::write(wxFFile *f)
{
	f->Write(wxString::Format("<key>\n"));
	if (fifths != NULL_INT)
		f->Write(wxString::Format("<fifths>%d</fifths>\n", fifths));
	if (mode != NULL_STRING)
		f->Write(wxString::Format("<mode>%s</mode>\n", mode));
	f->Write(wxString::Format("</key>\n"));
}

// score-partwise/list/part/measure/attributes/clef
//--------------------------------------------
c_clef::c_clef()
{}
c_clef::c_clef(wxXmlNode *xmlnode)
{
	number = GetIntAttribute(xmlnode, "number");
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "line")
			line = GetIntContent(child);
		else if (name == "sign")
			sign = GetStringContent(child);
		else if (name == "clef-octave-change")
			clef_octave_change = GetIntContent(child);
		child = child->GetNext();
	}
}
c_clef::c_clef(const c_clef & clef)
{
	number = clef.number;
	line = clef.line;
	sign = clef.sign;
	clef_octave_change = clef.clef_octave_change;
}
void c_clef::write(wxFFile *f)
{
	f->Write(wxString::Format("<clef"));
	if (number != NULL_INT)
		f->Write(wxString::Format(" number=\"%d\" ", number));
	f->Write(wxString::Format(">\n"));
	if (sign != NULL_STRING)
		f->Write(wxString::Format("<sign>%s</sign>\n", sign));
	if (line != NULL_INT)
		f->Write(wxString::Format("<line>%d</line>\n", line));
	if (clef_octave_change != NULL_INT)
		f->Write(wxString::Format("<clef-octave-change>%d</clef-octave-change>\n", clef_octave_change));
	f->Write(wxString::Format("</clef>\n"));
}

// score-partwise/list/part/measure/attributes/staff-details
//----------------------------------------------------------
c_staff_details::c_staff_details()
{}
c_staff_details::c_staff_details(wxXmlNode *xmlnode)
{
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "staff-lines")
			staff_lines = GetIntContent(child);
		else if (name == "staff-size")
			staff_size = GetIntContent(child);
		child = child->GetNext();
	}
}
c_staff_details::c_staff_details(const c_staff_details & staff_details)
{
	staff_lines = staff_details.staff_lines;
	staff_size = staff_details.staff_size;
}
void c_staff_details::write(wxFFile *f)
{
	f->Write(wxString::Format("<staff-details>\n"));
	if (staff_lines != NULL_INT)
		f->Write(wxString::Format("<staff-lines>%d</staff-lines>\n", staff_lines));
	if (staff_size != NULL_INT)
		f->Write(wxString::Format("<staff-size>%d</staff-size>\n", staff_size));
	f->Write(wxString::Format("</staff-details>\n"));
}

// score-partwise/list/part/measure/attributes
//--------------------------------------------
c_attributes::c_attributes()
{}
c_attributes::c_attributes(wxXmlNode *xmlnode)
{
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "divisions")
			divisions = GetIntContent(child);
		else if (name == "key")
			key = new c_key(child);
		else if (name == "time")
			mtime = new c_time(child);
		else if (name == "staff-details")
			staff_details = new c_staff_details(child);
		else if (name == "staves")
			staves = GetIntContent(child);
		else if (name == "clef")
			clefs.push_back(c_clef(child));
		else if (name == "transpose")
			transpose = new c_transpose(child);
		child = child->GetNext();
	}
}
c_attributes::~c_attributes()
{
	clefs.clear();
	delete key;
	delete mtime;
	delete staff_details;
	delete transpose;
}
c_attributes::c_attributes(const c_attributes & attributes, bool withContent)
{
	divisions = attributes.divisions;
	if (attributes.mtime != NULL)
		mtime = new c_time(*(attributes.mtime));
	if (withContent)
	{
		staves = attributes.staves;
		if ((attributes.key != NULL) && (withContent))
			key = new c_key(*(attributes.key));
		for (auto & current : attributes.clefs) 
		{
			clefs.push_back(c_clef(current));
		}
		if (attributes.staff_details != NULL)
			staff_details = new c_staff_details(*(attributes.staff_details));
		if (attributes.transpose != NULL)
			transpose = new c_transpose(*(attributes.transpose));
	}
}
void c_attributes::write(wxFFile *f)
{
	f->Write(wxString::Format("<attributes>\n"));
	if (divisions != NULL_INT)
		f->Write(wxString::Format("<divisions>%d</divisions>\n", divisions));
	if (key != NULL)
		key->write(f);
	if (mtime != NULL)
		mtime->write(f);
	if (staves != NULL_INT)
		f->Write(wxString::Format("<staves>%d</staves>\n", staves));
	for (auto & current : clefs )
	{
		current.write(f);
	}
	if (staff_details != NULL)
		staff_details->write(f);
	if (transpose != NULL)
		transpose->write(f);
	f->Write(wxString::Format("</attributes>\n"));
}
void c_attributes::compile(bool twelved , c_measure *measure)
{
	if (transpose)
	{
		gtranspose = ((transpose->chromatic == NULL_INT) ? 0 : transpose->chromatic) + 12 * ((transpose->octave_change == NULL_INT) ? 0 : transpose->octave_change);
	}
	if (key)
	{
		measure->key_fifths = key->fifths;
	}
	if (mtime)
	{
		measure->beats = mtime->beats;
		measure->beat_type = mtime->beat_type;
	}
	if (divisions != NULL_INT)
	{
		if (twelved)
			divisions *= 12;
		measure->divisions = divisions;
		measure->division_quarter = divisions;
	}

	if ( divisionsMax < divisions )
		divisionsMax = divisions ;


	switch (measure->beat_type)
	{
	case 1:
		measure->division_beat = 4 * measure->division_quarter;
		measure->division_measure = 4 * measure->division_quarter * measure->beats;
		break;
	case 2:
		measure->division_beat = 2 * measure->division_quarter;
		measure->division_measure = 2 * measure->division_quarter * measure->beats;
		break;
	case 8:
		measure->division_beat = (3 * measure->division_quarter) / 2;
		measure->division_measure = measure->division_quarter * measure->beats / 2;
		break;
	case 4:
	default:
		measure->division_beat = measure->division_quarter;
		measure->division_measure = measure->division_quarter * measure->beats;
		break;
	}
}
void c_attributes::divisionsAlign()
{
	if (divisions != NULL_INT)
		divisions = divisionsMax ;
}


// score-partwise/list/part/measure/note/pitch
//--------------------------------------------
c_pitch::c_pitch()
{}
c_pitch::c_pitch(wxXmlNode *xmlnode , bool iunpitched)
{
	unpitched = iunpitched;
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if ((name == "step") || (name == "display-step"))
			step = child->GetNodeContent();
		else if (name == "alter")
			alter = GetIntContent(child);
		else if ((name == "octave") || (name == "display-octave"))
			octave = GetIntContent(child);
		child = child->GetNext();
	}
}
c_pitch::c_pitch(const c_pitch & pitch)
{
	step = pitch.step;
	octave = pitch.octave;
	alter = pitch.alter;
	transpose = pitch.transpose;
}
void c_pitch::write(wxFFile *f)
{
	if (unpitched)
	{
		f->Write(wxString::Format("<unpitched>\n"));
		if (step != NULL_STRING)
			f->Write(wxString::Format("<display-step>%s</display-step>\n", step));
		if (octave != NULL_INT)
			f->Write(wxString::Format("<display-octave>%d</display-octave>\n", octave));
		f->Write(wxString::Format("</unpitched>\n"));
	}
	else
	{
		f->Write(wxString::Format("<pitch>\n"));
		if (step != NULL_STRING)
			f->Write(wxString::Format("<step>%s</step>\n", step));
		if (alter != NULL_INT)
			f->Write(wxString::Format("<alter>%d</alter>\n", alter));
		if (octave != NULL_INT)
			f->Write(wxString::Format("<octave>%d</octave>\n", octave));
		f->Write(wxString::Format("</pitch>\n"));
	}
}
void c_pitch::compile()
{
	transpose = gtranspose;
}
int c_pitch::toMidiPitch()
{
	int pitch;
	if (step.IsEmpty())
		return 64;
	wxChar cstep = step.Upper()[0];
	switch (cstep)
	{
	case 'C': pitch = 0; break;
	case 'D': pitch = 2; break;
	case 'E': pitch = 4; break;
	case 'F': pitch = 5; break;
	case 'G': pitch = 7; break;
	case 'A': pitch = 9; break;
	case 'B': pitch = 11; break;
	default: pitch = 0; break;
	}
	if (octave == NULL_INT)
		pitch += 12 * 5;
	else
		pitch += 12 * (octave+1);
	if (alter != NULL_INT)
		pitch += alter;
	pitch += transpose;
	return pitch;
}
int c_pitch::shiftPitch(int p, int up, int fifths)
{
	// shift pitch <p>, with <up> diatonic degrees, according to tone described by <fifths> in [-1..0..7]
	int z; // tone
	if (fifths >= 0)
		z = (fifths * 7) % 12;
	else
		z = 12 - ((-7 * fifths) % 12);
	int d; // degree in tone
	d = (p - z) % 12;
	int i = up;
	while (i != 0)
	{
		if (i > 0 )
		{
			switch (d)
			{
			case 0: p += 2; d = 2;  break;
			case 1: p += 1; d = 2;  break;
			case 2: p += 2; d = 4;  break;
			case 3: p += 1; d = 4;  break;
			case 4: p += 1; d = 5;  break;
			case 5: p += 2; d = 7;  break;
			case 6: p += 1; d = 7;  break;
			case 7: p += 2; d = 9;  break;
			case 8: p += 1; d = 9;  break;
			case 9: p += 2; d = 11;  break;
			case 10: p += 1; d = 11;  break;
			case 11: p += 1; d = 0;  break;
			default: break;
			}
			i--;
		}
		else
		{
			switch (d)
			{
			case 0: p -= 1; d = 11 ; break;
			case 1: p -= 1; d = 0 ; break;
			case 2: p -= 2; d = 0 ; break;
			case 3: p -= 1; d = 2 ; break;
			case 4: p -= 2; d = 2 ; break;
			case 5: p -= 1; d = 4 ; break;
			case 6: p -= 1; d = 5 ; break;
			case 7: p -= 2; d = 5 ; break;
			case 8: p -= 1; d = 7 ; break;
			case 9: p -= 2; d = 7 ; break;
			case 10: p -= 1; d = 9 ; break;
			case 11: p -= 2; d = 9 ; break;
			default: break;
			}
			i++;
		}
	}
	return (p);
}
bool c_pitch::isEqual(const c_pitch & pitch)
{
	if (!step.IsSameAs(pitch.step))
		return false;
	if (!(octave == pitch.octave))
		return false;
	if (!(alter == pitch.alter))
		return false;
	return true;
}
// score-partwise/list/part/measure/note/rest
//--------------------------------------------
c_rest::c_rest()
{}
c_rest::c_rest(wxXmlNode *xmlnode)
{
	measure = GetStringAttribute(xmlnode, "measure");
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "display-step")
			display_step = child->GetNodeContent();
		else if (name == "display-octave")
			display_octave = GetIntContent(child);
		child = child->GetNext();
	}
}
c_rest::c_rest(const c_rest & rest)
{
	measure = rest.measure;
	display_step = rest.display_step;
	display_octave = rest.display_octave;
}
void c_rest::write(wxFFile *f)
{
	if ((measure == NULL_STRING) && (display_step == NULL_STRING) && (display_octave == NULL_INT))
	{
		f->Write(wxString::Format("<rest/>\n"));
		return;
	}
	f->Write(wxString::Format("<rest"));
	if (measure != NULL_STRING)
		f->Write(wxString::Format(" measure=\"%s\"", measure));
	f->Write(wxString::Format(">\n"));
	if (display_step != NULL_STRING)
		f->Write(wxString::Format("<display-step>%s</display-step>\n", display_step));
	if (display_octave != NULL_INT)
		f->Write(wxString::Format("<display-octave>%d</display-octave>\n", display_octave));
	f->Write(wxString::Format("</rest>\n"));
}

// score-partwise/list/part/measure/note/time-modification
//--------------------------------------------------------
c_time_modification::c_time_modification()
{}
c_time_modification::c_time_modification(wxXmlNode *xmlnode)
{
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "actual-notes")
			actual_notes = GetIntContent(child);
		else if (name == "normal-notes")
			normal_notes = GetIntContent(child);
		child = child->GetNext();
	}
}
c_time_modification::c_time_modification(const c_time_modification & time_modification)
{
	actual_notes = time_modification.actual_notes;
	normal_notes = time_modification.normal_notes;
}
void c_time_modification::write(wxFFile *f)
{
	f->Write(wxString::Format("<time-modification>\n"));
	if (actual_notes != NULL_INT)
		f->Write(wxString::Format("<actual-notes>%d</actual-notes>\n", actual_notes));
	if (normal_notes != NULL_INT)
		f->Write(wxString::Format("<normal-notes>%d</normal-notes>\n", normal_notes));
	f->Write(wxString::Format("</time-modification>\n"));
}

// score-partwise/list/part/measure/note/lyric
//--------------------------------------------
c_lyric::c_lyric() : c_default_xy()
{}
c_lyric::c_lyric(wxXmlNode *xmlnode) : c_default_xy(xmlnode)
{
	number = GetIntAttribute(xmlnode,"number");
	placement = GetStringAttribute(xmlnode, "placement");
	name = GetStringAttribute(xmlnode,"name");
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString mname = child->GetName();
		if (mname == "text")
			text = child->GetNodeContent();
		else if (mname == "syllabic")
			syllabic = child->GetNodeContent();
		else if (mname == "extend")
			extend_type = child->GetAttribute("type");
		child = child->GetNext();
	}
}
c_lyric::c_lyric(const c_lyric & lyric) : c_default_xy(lyric)
{
	number = lyric.number;
	placement = lyric.placement;
	name = lyric.name;
	text = lyric.text;
	syllabic = lyric.syllabic;
	extend_type = lyric.extend_type;
}
void c_lyric::write(wxFFile *f)
{
	f->Write(wxString::Format("<lyric "));
	write_xy(f);
	if (number != NULL_INT)
		f->Write(wxString::Format(" number=\"%d\" ", number));
	if (placement != NULL_STRING)
		f->Write(wxString::Format(" placement=\"%s\" ", placement));
	if (name != NULL_STRING)
		f->Write(wxString::Format(" name=\"%s\" ", name));
	f->Write(wxString::Format(">\n"));
	if (syllabic != NULL_STRING)
		f->Write(wxString::Format("<syllabic>%s</syllabic>\n", syllabic));
	if (extend_type != NULL_STRING)
		f->Write(wxString::Format("<extend type=\"%s\" />\n", extend_type));
	if (text != NULL_STRING)
		f->Write(wxString::Format("<text>%s</text>\n", encodeXML(text)));
	f->Write(wxString::Format("</lyric>\n"));
}

// score-partwise/list/part/measure/note/beam
//-------------------------------------------
c_beam::c_beam()
{}
c_beam::c_beam(wxXmlNode *xmlnode)
{
	number = GetIntAttribute(xmlnode, "number");
	value = xmlnode->GetNodeContent();
}
c_beam::c_beam(const c_beam &beam)
{
	value = beam.value ;
	number = beam.number;
}
void c_beam::write(wxFFile *f)
{
	if ( number == NULL_INT)
		f->Write(wxString::Format("<beam>%s</beam>\n", value));
	else
		f->Write(wxString::Format("<beam number=\"%d\">%s</beam>\n",number, value));
}

// score-partwise/list/part/measure/note/notations/arpeggiate
//-----------------------------------------------------------
c_arpeggiate::c_arpeggiate()
{}
c_arpeggiate::c_arpeggiate(wxXmlNode *xmlnode)
{
	direction = GetStringAttribute(xmlnode,"direction");
	number = GetIntAttribute(xmlnode, "number");
}
c_arpeggiate::c_arpeggiate(const c_arpeggiate & arpeggiate)
{
	direction = arpeggiate.direction;
	number = arpeggiate.number;
}
void c_arpeggiate::write(wxFFile *f)
{
	f->Write(wxString::Format("<arpeggiate "));
	if (direction != NULL_STRING)
		f->Write(wxString::Format(" direction=\"%s\" ", direction));
	if (number != NULL_INT)
		f->Write(wxString::Format(" number=\"%d\" ", number));
	f->Write(wxString::Format(" />\n"));
}

// score-partwise/list/part/measure/note/notations/articulations
//--------------------------------------------------------------
c_articulations::c_articulations()
{}
c_articulations::c_articulations(wxXmlNode *xmlnode)
{
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		articulations.push_back(name);
		placements.push_back(GetStringAttribute(child,"placement"));
		default_xs.push_back(GetIntAttribute(child, "default-x"));
		default_ys.push_back(GetIntAttribute(child, "default-y"));
		child = child->GetNext();
	}
}
c_articulations::c_articulations(const c_articulations & carticulations)
{
	unsigned int nb = carticulations.articulations.size();
	for (unsigned int i = 0; i < nb; i++)
	{
		articulations.push_back(carticulations.articulations[i]);
		placements.push_back(carticulations.placements[i]);
		default_xs.push_back(carticulations.default_xs[i]);
		default_ys.push_back(carticulations.default_ys[i]);
	}
}
void c_articulations::write(wxFFile *f)
{
	unsigned int nb = articulations.size();
	if (nb > 0)
	{
		f->Write("<articulations>\n");
		for (unsigned int i = 0; i < nb; i++)
		{
			f->Write(wxString::Format("<%s ", articulations[i]));
			if (placements[i] != NULL_STRING)
				f->Write(wxString::Format(" placement=\"%s\" ", placements[i]));
			if (default_xs[i] != NULL_INT)
				f->Write(wxString::Format(" default-x=\"%d\" ", default_xs[i]));
			if (default_ys[i] != NULL_INT)
				f->Write(wxString::Format(" default-y=\"%d\" ", default_ys[i]));
			f->Write(wxString::Format(" />\n"));
		}
		f->Write("</articulations>\n");
	}
}

// score-partwise/list/part/measure/note/notations/lOrnaments
//--------------------------------------------------------------
c_ornaments::c_ornaments()
{}
c_ornaments::c_ornaments(wxXmlNode *xmlnode)
{
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		lOrnaments.push_back(name);
		placements.push_back(GetStringAttribute(child,"placement"));
		default_xs.push_back(GetIntAttribute(child, "default-x"));
		default_ys.push_back(GetIntAttribute(child, "default-y"));
		child = child->GetNext();
	}
}
c_ornaments::c_ornaments(const c_ornaments & cornaments)
{
	unsigned int nb = cornaments.lOrnaments.size();
	for (unsigned int i = 0; i < nb; i++)
	{
		lOrnaments.push_back(cornaments.lOrnaments[i]);
		placements.push_back(cornaments.placements[i]);
		default_xs.push_back(cornaments.default_xs[i]);
		default_ys.push_back(cornaments.default_ys[i]);
	}
}
void c_ornaments::write(wxFFile *f)
{
	unsigned int nb = lOrnaments.size();
	if (nb > 0)
	{
		f->Write("<ornaments>\n");
		for (unsigned int i = 0; i < nb; i++)
		{
			f->Write(wxString::Format("<%s ", lOrnaments[i]));
			if (placements[i] != NULL_STRING)
				f->Write(wxString::Format(" placement=\"%s\" ", placements[i]));
			if (default_xs[i] != NULL_INT)
				f->Write(wxString::Format(" default-x=\"%d\" ", default_xs[i]));
			if (default_ys[i] != NULL_INT)
				f->Write(wxString::Format(" default-y=\"%d\" ", default_ys[i]));
			f->Write(wxString::Format(" />\n"));
		}
		f->Write("</ornaments>\n");
	}
}


// score-partwise/list/part/measure/note/notations/dynamics
// score - partwise / list / part / measure / direction / direction-type / dynamics
//---------------------------------------------------------------------------------
c_dynamics::c_dynamics() : c_default_xy()
{}
c_dynamics::c_dynamics(wxXmlNode *xmlnode) : c_default_xy(xmlnode)
{
	placement = GetStringAttribute(xmlnode,"placement");
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		dynamic = child->GetName();
		child = child->GetNext();
	}
}
c_dynamics::c_dynamics(const c_dynamics &dynamics) : c_default_xy(dynamics)
{
	placement = dynamics.placement;
	dynamic = dynamics.dynamic;
}
void c_dynamics::write(wxFFile *f)
{
	f->Write(wxString::Format("<dynamics "));
	write_xy(f);
	if (placement != NULL_STRING)
		f->Write(wxString::Format("placement=\"%s\" ", placement));
	f->Write(wxString::Format(" >\n"));
	if (dynamic != NULL_STRING)
		f->Write(wxString::Format("<%s/>\n", dynamic));
	f->Write(wxString::Format("</dynamics>\n"));
}

// score-partwise/list/part/measure/note/notations/fermata
//---------------------------------------------------------
c_fermata::c_fermata() : c_default_xy()
{}
c_fermata::c_fermata(wxXmlNode *xmlnode) : c_default_xy(xmlnode)
{
	placement = GetStringAttribute(xmlnode,"placement");
	type = GetStringAttribute(xmlnode,"type");
}
c_fermata::c_fermata(const c_fermata &fermata) : c_default_xy(fermata)
{
	type = fermata.type;
	placement = fermata.placement;
}
void c_fermata::write(wxFFile *f)
{
	f->Write(wxString::Format("<fermata "));
	write_xy(f);
	if (type != NULL_STRING)
		f->Write(wxString::Format("type=\"%s\" ", type));
	if (placement != NULL_STRING)
		f->Write(wxString::Format("placement=\"%s\" ", placement));
	f->Write(wxString::Format(" />\n"));
}
// score-partwise/list/part/measure/note/notations/glissando
//---------------------------------------------------------
c_glissando::c_glissando() : c_default_xy()
{}
c_glissando::c_glissando(wxXmlNode *xmlnode) : c_default_xy(xmlnode)
{
	type = GetStringAttribute(xmlnode,"type");
	number = GetIntAttribute(xmlnode, "number");
}
c_glissando::c_glissando(const c_glissando &glissando) : c_default_xy(glissando)
{
	type = glissando.type;
	number = glissando.number;
}
void c_glissando::write(wxFFile *f)
{
	f->Write(wxString::Format("<glissando "));
	write_xy(f);
	if (type != NULL_STRING)
		f->Write(wxString::Format("type=\"%s\" ", type));
	if (number != NULL_INT)
		f->Write(wxString::Format("number=\"%d\" ", number));
	f->Write(wxString::Format(" />\n"));
}

// score-partwise/list/part/measure/note/notations/slide
//---------------------------------------------------------
c_slide::c_slide() : c_default_xy()
{}
c_slide::c_slide(wxXmlNode *xmlnode) : c_default_xy(xmlnode)
{
	type = GetStringAttribute(xmlnode,"type");
	number = GetIntAttribute(xmlnode, "number");
}
c_slide::c_slide(const c_slide &slide) : c_default_xy(slide)
{
	type = slide.type;
	number = slide.number;
}
void c_slide::write(wxFFile *f)
{
	f->Write(wxString::Format("<slide "));
	write_xy(f);
	if (type != NULL_STRING)
		f->Write(wxString::Format("type=\"%s\" ", type));
	if (number != NULL_INT)
		f->Write(wxString::Format("number=\"%d\" ", number));
	f->Write(wxString::Format(" />\n"));
}

// score-partwise/list/part/measure/note/notations/slur
//---------------------------------------------------------
c_slur::c_slur() : c_default_xy()
{}
c_slur::c_slur(wxXmlNode *xmlnode) : c_default_xy(xmlnode)
{
	placement = GetStringAttribute(xmlnode,"placement");
	type = GetStringAttribute(xmlnode,"type");
	number = GetIntAttribute(xmlnode, "number");
}
c_slur::c_slur(const c_slur &slur) : c_default_xy(slur)
{
	placement = slur.placement;
	type = slur.type;
	number = slur.number;
}
void c_slur::write(wxFFile *f)
{
	f->Write(wxString::Format("<slur "));
	write_xy(f);
	if (placement != NULL_STRING)
		f->Write(wxString::Format("placement=\"%s\" ", placement));
	if (type != NULL_STRING)
		f->Write(wxString::Format("type=\"%s\" ", type));
	if (number != NULL_INT)
		f->Write(wxString::Format("number=\"%d\" ", number));
	f->Write(wxString::Format(" />\n"));
}

// score-partwise/list/part/measure/note/notations/tied
//---------------------------------------------------------
c_tied::c_tied()
{}
c_tied::c_tied(wxXmlNode *xmlnode) 
{
	complete(xmlnode);
}
void c_tied::complete(wxXmlNode *xmlnode)
{
	wxString type = GetStringAttribute(xmlnode,"type");
	if (type.IsSameAs("start", false))
		start = true;
	if (type.IsSameAs("stop", false))
		stop = true;
}
c_tied::c_tied(const c_tied &tied)
{
	start = tied.start;
	stop = tied.stop;
}
void c_tied::write(wxFFile *f)
{
	if (stop)
		f->Write(wxString::Format("<tied type=\"stop\" />\n"));
	if (start)
		f->Write(wxString::Format("<tied type=\"start\" />\n"));
}
// score-partwise/list/part/measure/note/notations/tie
//---------------------------------------------------------
c_tie::c_tie()
{}
c_tie::c_tie(wxXmlNode *xmlnode)
{
	complete(xmlnode);
}
void c_tie::complete(wxXmlNode *xmlnode)
{
	wxString type = GetStringAttribute(xmlnode,"type");
	if (type.IsSameAs("start", false))
		start = true;
	if (type.IsSameAs("stop", false))
		stop = true;
}
c_tie::c_tie(const c_tie &tie)
{
	start = tie.start;
	stop = tie.stop;
}
void c_tie::write(wxFFile *f)
{
	if (stop)
		f->Write(wxString::Format("<tie type=\"stop\" />\n"));
	if (start)
		f->Write(wxString::Format("<tie type=\"start\" />\n"));
}

// score-partwise/list/part/measure/note/notations/tuplet
//---------------------------------------------------------
c_tuplet::c_tuplet() : c_default_xy()
{}
c_tuplet::c_tuplet(wxXmlNode *xmlnode) : c_default_xy(xmlnode)
{
	placement = GetStringAttribute(xmlnode,"placement");
	type = GetStringAttribute(xmlnode,"type");
	number = GetIntAttribute(xmlnode, "number");
}
c_tuplet::c_tuplet(const c_tuplet & tuplet) : c_default_xy(tuplet)
{
	placement = tuplet.placement;
	type = tuplet.type;
	number = tuplet.number;
}
void c_tuplet::write(wxFFile *f)
{
	f->Write(wxString::Format("<tuplet "));
	write_xy(f);
	if (placement != NULL_STRING)
		f->Write(wxString::Format("placement=\"%s\" ", placement));
	if (type != NULL_STRING)
		f->Write(wxString::Format("type=\"%s\" ", type));
	if (number != NULL_INT)
		f->Write(wxString::Format("number=\"%d\" ", number));
	f->Write(wxString::Format(" />\n"));
}

// score-partwise/list/part/measure/note/notations
//------------------------------------------------
c_notations::c_notations()
{}
c_notations::c_notations(wxXmlNode *xmlnode)
{
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "arpeggiate")
		{
			if (arpeggiate != NULL)
				delete arpeggiate;
			arpeggiate = new c_arpeggiate(child);
		}
		else if (name == "articulations")
		{
			if (articulations != NULL)
				delete articulations;
			articulations = new c_articulations(child);
		}
		else if (name == "ornaments")
		{
			if (lOrnaments != NULL)
				delete lOrnaments;
			lOrnaments = new c_ornaments(child);
		}
		else if (name == "dynamics")
		{
			if (dynamics != NULL)
				delete dynamics;
			dynamics = new c_dynamics(child);
		}
		else if (name == "fermata")
		{
			if (fermata != NULL)
				delete fermata;
			fermata = new c_fermata(child);
		}
		else if (name == "glissando")
		{
			if (glissando != NULL)
				delete glissando;
			glissando = new c_glissando(child);
		}
		else if (name == "slide")
		{
			if (slide != NULL)
				delete slide;
			slide = new c_slide(child);
		}
		else if (name == "tuplet")
		{
			if (tuplet != NULL)
				delete tuplet;
			tuplet = new c_tuplet(child);
		}
		else if (name == "slur")
			slurs.push_back(c_slur(child));
		else if (name == "lOrnaments")
		{
			if (tied == NULL)
				tied = new c_tied(child);
			else
				tied->complete(child);
		}
		else if (name == "tied")
		{
			if (tied == NULL)
				tied = new c_tied(child);
			else
				tied->complete(child);
		}
		child = child->GetNext();
	}
}
c_notations::~c_notations()
{
	delete arpeggiate;
	delete articulations;
	delete lOrnaments;
	delete dynamics;
	delete fermata;
	delete glissando;
	delete slide;
	delete tied;
	delete tuplet;
	slurs.clear();
}
c_notations::c_notations(const c_notations & notations)
{
	if (notations.arpeggiate != NULL)
		arpeggiate = new c_arpeggiate(*(notations.arpeggiate));
	if (notations.articulations != NULL)
		articulations = new c_articulations(*(notations.articulations));
	if (notations.lOrnaments != NULL)
		lOrnaments = new c_ornaments(*(notations.lOrnaments));
	if (notations.dynamics != NULL)
		dynamics = new c_dynamics(*(notations.dynamics));
	if (notations.fermata != NULL)
		fermata = new c_fermata(*(notations.fermata));
	if (notations.glissando != NULL)
		glissando = new c_glissando(*(notations.glissando));
	if (notations.slide != NULL)
		slide = new c_slide(*(notations.slide));
	if (notations.tied != NULL)
		tied = new c_tied(*(notations.tied));
	if (notations.tuplet != NULL)
		tuplet = new c_tuplet(*(notations.tuplet));
	for (auto & current : notations.slurs )
	{
		slurs.push_back(c_slur(current));
	}
}
void c_notations::write(wxFFile *f)
{
	f->Write(wxString::Format("<notations>\n"));
	if (arpeggiate != NULL)
		arpeggiate->write(f);
	if (articulations != NULL)
		articulations->write(f);
	if (lOrnaments != NULL)
		lOrnaments->write(f);
	if (dynamics != NULL)
		dynamics->write(f);
	if (fermata != NULL)
		fermata->write(f);
	if (glissando != NULL)
		glissando->write(f);
	if (slide != NULL)
		slide->write(f);
	if (tied != NULL)
		tied->write(f);
	if (tuplet != NULL)
		tuplet->write(f);
	for (auto & current : slurs )
	{
		current.write(f);
	}
	f->Write(wxString::Format("</notations>\n"));
}

// score-partwise/list/part/measure/note
//--------------------------------------------
c_note::c_note() : c_default_xy()
{}
c_note::c_note(wxXmlNode *xmlnode) : c_default_xy(xmlnode)
{
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "pitch")
			pitch = new c_pitch(child);
		if (name == "unpitched")
			pitch = new c_pitch(child,true);
		else if (name == "rest")
			rest = new c_rest(child);
		else if (name == "time-modification")
			time_modification = new c_time_modification(child);
		else if (name == "notations")
			notations = new c_notations(child);
		else if (name == "lyric")
			lyrics.push_back(c_lyric(child));
		else if (name == "beam")
			beams.push_back(c_beam(child));
		else if (name == "duration")
			duration = GetIntContent(child);
		else if (name == "voice")
			voice = GetIntContent(child);
		else if (name == "type")
			mtype = child->GetNodeContent();
		else if (name == "stem")
			stem = child->GetNodeContent();
		else if (name == "notehead")
		{
			notehead = child->GetNodeContent();
			notecolor = GetStringAttribute(child,"color");
		}
		else if (name == "accidental")
			accidental = child->GetNodeContent();
		else if (name == "staff")
			staff = GetIntContent(child);
		else if (name == "chord")
			chord = true;
		else if (name == "dot")
			dots ++;
		else if (name == "grace")
			grace = true;
		else if (name == "tie")
		{
			if (tie == NULL)
				tie = new c_tie(child);
			else
				tie->complete(child);
		}
		else if (name == "cue")
			cue = true;
		child = child->GetNext();
	}
}
c_note::~c_note()
{
	beams.clear();
	lyrics.clear();
	delete pitch;
	delete rest;
	delete time_modification;
	delete notations;
	delete tie;
}
c_note::c_note(const c_note & note) : c_default_xy(note)
{
	if (note.pitch != NULL)
		pitch = new c_pitch(*(note.pitch));
	if (note.rest != NULL)
		rest = new c_rest(*(note.rest));
	if (note.time_modification != NULL)
		time_modification = new c_time_modification(*(note.time_modification));
	if (note.notations != NULL)
		notations = new c_notations(*(note.notations));
	if (note.tie != NULL)
		tie = new c_tie(*(note.tie));
	for (auto & current : note.lyrics )
	{
		lyrics.push_back(c_lyric(current));
	}
	for (auto & current : note.beams )
	{
		beams.push_back(c_beam(current));
	}
	duration = note.duration;
	voice = note.voice;
	mtype = note.mtype;
	stem = note.stem;
	notehead = note.notehead;
	notecolor = note.notecolor;
	accidental = note.accidental;
	staff = note.staff;
	chord = note.chord;
	chord_order = note.chord_order;
	dots = note.dots;
	grace = note.grace;
	cue = note.cue;
	partNr = note.partNr;
}
void c_note::write(wxFFile *f)
{
	if (cue)
		return;

	f->Write(wxString::Format("<note"));
	write_xy(f);
	f->Write(wxString::Format(">\n"));
	if (grace)
		f->Write(wxString::Format("<grace/>\n"));	
	if (chord)
		f->Write(wxString::Format("<chord/>\n"));
	if (pitch != NULL)
		pitch->write(f);
	if (rest != NULL)
		rest->write(f);
	if (duration != NULL_INT)
		f->Write(wxString::Format("<duration>%d</duration>\n", duration));
	if (tie != NULL)
		tie->write(f);	
	if (voice != NULL_INT)
		f->Write(wxString::Format("<voice>%d</voice>\n", voice));
	if (mtype != NULL_STRING)
		f->Write(wxString::Format("<type>%s</type>\n", mtype));
	if (dots > 0)
	{
		for (int i = 0; i < dots; i ++)
			f->Write(wxString::Format("<dot/>\n"));
	}
	if (accidental != NULL_STRING)
		f->Write(wxString::Format("<accidental>%s</accidental>\n", accidental));
	if (time_modification != NULL)
		time_modification->write(f);
	if (stem != NULL_STRING)
		f->Write(wxString::Format("<stem>%s</stem>\n", stem));
	if (notehead != NULL_STRING)
	{
		if ( notecolor == NULL_STRING)
			f->Write(wxString::Format("<notehead>%s</notehead>\n", notehead));
		else
			f->Write(wxString::Format("<notehead color=\"%s\">%s</notehead>\n", notecolor, notehead));
	}
	if (staff != NULL_INT)
		f->Write(wxString::Format("<staff>%d</staff>\n", staff));
	for (auto & current : beams )
	{
		current.write(f);
	}
	if (notations != NULL)
		notations->write(f);
	for (auto & current : lyrics )
	{
		current.write(f);
	}
	f->Write(wxString::Format("</note>\n"));
}
void c_note::compile(int ipartNr , bool twelved)
{
	partNr = ipartNr;
	if ( chord)
		gchord_counter ++ ;
	else
		gchord_counter = 0;
	chord_order = gchord_counter;
	if (pitch != NULL)
		pitch->compile();
	if (twelved)
	{
		if (duration != NULL_INT)
			duration *= 12 ;
	}
}
void c_note::divisionsAlign(int ratio)
{
	duration *= ratio ;
}
// score-partwise/list/part/measure/backup
//--------------------------------------------
c_backup::c_backup()
{}
c_backup::c_backup(wxXmlNode *xmlnode)
{
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "duration")
			duration = GetIntContent(child);
		child = child->GetNext();
	}
}
c_backup::c_backup(const c_backup &backup)
{
	duration = backup.duration;
}
void c_backup::write(wxFFile *f)
{
	f->Write(wxString::Format("<backup>\n"));
	if (duration != NULL_INT)
		f->Write(wxString::Format("<duration>%d</duration>\n", duration));
	f->Write(wxString::Format("</backup>\n"));
}
void c_backup::compile(bool twelved)
{
	if (twelved && (duration != NULL_INT))
		duration *= 12 ;
}
void c_backup::divisionsAlign(int ratio)
{
	if (duration != NULL_INT)
		duration *= ratio ;
}
// score-partwise/list/part/measure/forward
//--------------------------------------------
c_forward::c_forward()
{}
c_forward::c_forward(wxXmlNode *xmlnode)
{
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "duration")
			duration = GetIntContent(child);
		child = child->GetNext();
	}
}
c_forward::c_forward(const c_forward &forward)
{
	duration = forward.duration;
}
void c_forward::write(wxFFile *f)
{
	f->Write(wxString::Format("<forward>\n"));
	if (duration != NULL_INT)
		f->Write(wxString::Format("<duration>%d</duration>\n", duration));
	f->Write(wxString::Format("</forward>\n"));
}
void c_forward::compile(bool twelved)
{
	if (twelved && (duration != NULL_INT))
		duration *= 12 ;

}
void c_forward::divisionsAlign(int ratio)
{
	if (duration != NULL_INT)
		duration *= ratio ;
}
// score-partwise/list/part/measure/barline/repeat
//------------------------------------------------
c_repeat::c_repeat()
{}
c_repeat::c_repeat(wxXmlNode *xmlnode)
{
	direction = GetStringAttribute(xmlnode,"direction" );
	times = GetStringAttribute(xmlnode,"times" );
}
c_repeat::c_repeat(const c_repeat & repeat)
{
	direction = repeat.direction;
	times = repeat.times;
}
void c_repeat::write(wxFFile *f)
{
	f->Write(wxString::Format("<repeat"));
	if (direction != NULL_STRING)
		f->Write(wxString::Format(" direction=\"%s\"", direction));
	if (times != NULL_STRING)
		f->Write(wxString::Format(" times=\"%s\"", times));
	f->Write(wxString::Format("/>\n"));
}

// score-partwise/list/part/measure/barline/ending
//------------------------------------------------
c_ending::c_ending() : c_default_xy()
{}
c_ending::c_ending(wxXmlNode *xmlnode) : c_default_xy(xmlnode)
{
	end_length = GetIntAttribute(xmlnode, "end-length");
	number = GetStringAttribute(xmlnode,"number");
	type = GetStringAttribute(xmlnode,"type");
	value = xmlnode->GetNodeContent();
}
c_ending::c_ending(const c_ending &ending) : c_default_xy(ending) 
{
	end_length = ending.end_length;
	number = ending.number;
	type = ending.type;
	value = ending.value;
}
void c_ending::write(wxFFile *f)
{
	f->Write(wxString::Format("<ending"));
	write_xy(f);
	if (end_length != NULL_INT)
		f->Write(wxString::Format(" end_length=\"%d\"", end_length));
	if (number != NULL_STRING)
		f->Write(wxString::Format(" number=\"%s\"", number));
	if (type != NULL_STRING)
		f->Write(wxString::Format(" type=\"%s\"", type));
	if ((value == NULL_STRING) || (value.IsEmpty()))
		f->Write(wxString::Format("/>\n"));
	else
		f->Write(wxString::Format(">%s</ending>\n", value));
}

// score-partwise/list/part/measure/barline
//-----------------------------------------
c_barline::c_barline()
{}
c_barline::c_barline(wxXmlNode *xmlnode)
{
	location = GetStringAttribute(xmlnode,"location");
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "bar-style")
			bar_style = child->GetNodeContent();
		else if (name == "segno")
			segno = true;
		else if (name == "coda")
			coda = true;
		else if (name == "fermata")
			fermata = true;
		else if (name == "repeat")
			repeat = new c_repeat(child);
		else if (name == "ending")
			ending = new c_ending(child);
		child = child->GetNext();
	}
}
c_barline::~c_barline()
{
	delete ending;
	delete repeat;
}
c_barline::c_barline(const c_barline & barline)
{
	location = barline.location;
	bar_style = barline.bar_style;
	segno = barline.segno;
	coda = barline.coda;
	fermata = barline.fermata;
	if ( barline.repeat)
		repeat = new c_repeat(*barline.repeat);
	if (barline.ending)
		ending = new c_ending(*barline.ending);
}
void c_barline::write(wxFFile *f)
{
	f->Write(wxString::Format("<barline"));
	if (location != NULL_STRING)
		f->Write(wxString::Format(" location=\"%s\"", location));
	f->Write(wxString::Format(">\n"));
	if (bar_style != NULL_STRING)
		f->Write(wxString::Format("<bar-style>%s</bar-style>\n", bar_style));
	if (segno)
		f->Write(wxString::Format("<segno/>"));
	if (coda)
		f->Write(wxString::Format("<coda/>"));
	if (fermata)
		f->Write(wxString::Format("<fermata/>"));
	if (ending != NULL)
		ending->write(f);
	if (repeat != NULL)
		repeat->write(f);
	f->Write(wxString::Format("</barline>\n"));
}

// score-partwise/list/part/measure/harmony/root
//----------------------------------------------
c_root::c_root()
{}
c_root::c_root(wxXmlNode *xmlnode)
{
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "root-step")
			root_step = child->GetNodeContent();
		else if (name == "root-alter")
			root_alter = GetIntContent(child);
		child = child->GetNext();
	}
}
c_root::c_root(const c_root & root)
{
	root_step = root.root_step;
	root_alter = root.root_alter;
}
void c_root::write(wxFFile *f)
{
	f->Write(wxString::Format("<root>\n"));
	if (root_step != NULL_STRING)
		f->Write(wxString::Format("<root-step>%s</root-step>\n", root_step));
	if (root_alter != NULL_INT)
		f->Write(wxString::Format("<root-alter>%d</root-alter>\n", root_alter));
	f->Write(wxString::Format("</root>\n"));
}

// score-partwise/list/part/measure/harmony/bass
//----------------------------------------------
c_bass::c_bass()
{}
c_bass::c_bass(wxXmlNode *xmlnode)
{
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "bass-step")
			bass_step = child->GetNodeContent();
		else if (name == "bass-alter")
			bass_alter = GetIntContent(child);
		child = child->GetNext();
	}
}
c_bass::c_bass(const c_bass &bass)
{
	bass_step = bass.bass_step;
	bass_alter = bass.bass_alter;
}
void c_bass::write(wxFFile *f)
{
	f->Write(wxString::Format("<bass>\n"));
	if (bass_step != NULL_STRING)
		f->Write(wxString::Format("<bass-step>%s</bass-step>\n", bass_step));
	if (bass_alter != NULL_INT)
		f->Write(wxString::Format("<bass-alter>%d</bass-alter>\n", bass_alter));
	f->Write(wxString::Format("</bass>\n"));
}

// score-partwise/list/part/measure/harmony/kind
//----------------------------------------------
c_kind::c_kind()
{}
c_kind::c_kind(wxXmlNode *xmlnode)
{
	use_symbols = GetStringAttribute(xmlnode,"use-symbols");
	text = GetStringAttribute(xmlnode,"text");
	value = xmlnode->GetNodeContent();
}
c_kind::c_kind(const c_kind &kind)
{
	use_symbols = kind.use_symbols;
	text = kind.text;
	value = kind.value;
}
void c_kind::write(wxFFile *f)
{
	f->Write(wxString::Format("<kind "));
	if (use_symbols != NULL_STRING)
		f->Write(wxString::Format("use-symbols=\"%s\"", use_symbols));
	if (text != NULL_STRING)
		f->Write(wxString::Format("text=\"%s\"", text));
	f->Write(wxString::Format(">\n"));
	if (value != NULL_STRING)
		f->Write(wxString::Format("%s", value));
	f->Write(wxString::Format("</kind>\n"));
}

// score-partwise/list/part/measure/harmony
//------------------------------------------
c_harmony::c_harmony() :c_default_xy()
{}
c_harmony::c_harmony(wxXmlNode *xmlnode) :c_default_xy(xmlnode)
{
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "function")
			function = child->GetNodeContent();
		else if (name == "inversion")
			inversion = GetIntContent(child);
		else if (name == "root")
			root = new c_root(child);
		else if (name == "bass")
			bass = new c_bass(child);
		else if (name == "kind")
			kind = new c_kind(child);
		child = child->GetNext();
	}
}
c_harmony::c_harmony(const c_harmony &harmony) :c_default_xy(harmony)
{
	function = harmony.function;
	inversion = harmony.inversion;
	if (harmony.root != NULL)
		root = new c_root(*(harmony.root));
	if (harmony.bass != NULL)
		bass = new c_bass(*(harmony.bass));
	if (harmony.kind != NULL)
		kind = new c_kind(*(harmony.kind));
}
c_harmony::~c_harmony()
{
	delete root;
	delete bass;
	delete kind;
}
void c_harmony::write(wxFFile *f)
{
	f->Write(wxString::Format("<harmony "));
	write_xy(f);
	f->Write(wxString::Format(" >\n"));
	if (function != NULL_STRING)
		f->Write(wxString::Format("<function>%s</function>", function));
	if (inversion != NULL_INT)
		f->Write(wxString::Format("<inversion>%d</inversion>", inversion));
	if (root != NULL)
		root->write(f);
	if (bass != NULL)
		bass->write(f);
	if (kind != NULL)
		kind->write(f);
	f->Write(wxString::Format("</harmony>\n"));
}
// score - partwise / list / part / measure / direction / direction-type / pedal
//------------------------------------------------------------------------------
c_pedal::c_pedal() :c_default_xy()
{}
c_pedal::c_pedal(wxXmlNode *xmlnode) :c_default_xy(xmlnode)
{
	type = GetStringAttribute(xmlnode,"type");
	line = GetStringAttribute(xmlnode,"line");
	sign = GetStringAttribute(xmlnode,"sign");
}
c_pedal::c_pedal(const c_pedal &pedal) :c_default_xy(pedal)
{
	type = pedal.type;
	line = pedal.line;
	sign = pedal.sign;
}
void c_pedal::write(wxFFile *f)
{
	f->Write(wxString::Format("<pedal "));
	write_xy(f);
	if (type != NULL_STRING)
		f->Write(wxString::Format("type=\"%s\" ", type));
	if (line != NULL_STRING)
		f->Write(wxString::Format("line=\"%s\" ", line));
	if (sign != NULL_STRING)
		f->Write(wxString::Format("sign=\"%s\" ", sign));
	f->Write(wxString::Format(" />\n"));
}

// score - partwise / list / part / measure / direction / direction-type / octave_shift
//-------------------------------------------------------------------------------------
c_octave_shift::c_octave_shift() :c_default_xy()
{}
c_octave_shift::c_octave_shift(wxXmlNode *xmlnode) :c_default_xy(xmlnode)
{
	type = GetStringAttribute(xmlnode,"type");
	number = GetIntAttribute(xmlnode, "number");
	size = GetIntAttribute(xmlnode, "size");
}
c_octave_shift::c_octave_shift(const c_octave_shift &octave_shift) :c_default_xy(octave_shift)
{
	type = octave_shift.type;
	number = octave_shift.number;
	size = octave_shift.size;
}
void c_octave_shift::write(wxFFile *f)
{
	f->Write(wxString::Format("<octave-shift "));
	write_xy(f);
	if (type != NULL_STRING)
		f->Write(wxString::Format("type=\"%s\" ", type));
	if (number != NULL_INT)
		f->Write(wxString::Format("number=\"%d\" ", number));
	if (size != NULL_INT)
		f->Write(wxString::Format("size=\"%d\" ", size));
	f->Write(wxString::Format(" />\n"));
}

// score - partwise / list / part / measure / direction / direction-type / rehearsal
//----------------------------------------------------------------------------------
c_rehearsal::c_rehearsal() :c_default_xy()
{}
c_rehearsal::c_rehearsal(wxXmlNode *xmlnode) : c_default_xy(xmlnode)
{
	value = xmlnode->GetNodeContent();
}
c_rehearsal::c_rehearsal(const c_rehearsal &rehearsal) : c_default_xy(rehearsal)
{
	value = rehearsal.value;
}
void c_rehearsal::write(wxFFile *f)
{
	if (value.IsEmpty() == false)
	{
		f->Write(wxString::Format("<rehearsal"));
		write_xy(f);
		f->Write(wxString::Format(">%s</rehearsal>\n", value));
	}
}

// score - partwise / list / part / measure / direction / direction-type / words
//----------------------------------------------------------------------------------
c_words::c_words() :c_default_xy()
{}
c_words::c_words(wxXmlNode *xmlnode) : c_default_xy(xmlnode)
{
	value = xmlnode->GetNodeContent();
}
c_words::c_words(const c_words &words) : c_default_xy(words)
{
	value = words.value;
}
void c_words::write(wxFFile *f)
{
	if (value.IsEmpty() == false)
	{
		f->Write(wxString::Format("<words"));
		write_xy(f);
		f->Write(wxString::Format(">%s</words>\n", encodeXML(value)));
	}
}

// score - partwise / list / part / measure / direction / direction-type / wedge
//------------------------------------------------------------------------------
c_wedge::c_wedge() :c_default_xy()
{}
c_wedge::c_wedge(wxXmlNode *xmlnode) : c_default_xy(xmlnode)
{
	type = GetStringAttribute(xmlnode,"type");
	spread = GetIntAttribute(xmlnode, "spread");
}
c_wedge::c_wedge(const c_wedge &wedge) : c_default_xy(wedge)
{
	type = wedge.type;
	spread = wedge.spread;
}
void c_wedge::write(wxFFile *f)
{
	f->Write(wxString::Format("<wedge "));
	write_xy(f);
	if (type != NULL_STRING)
		f->Write(wxString::Format("type=\"%s\" ", type));
	if (spread != NULL_INT)
		f->Write(wxString::Format("spread=\"%d\" ", spread));
	f->Write(wxString::Format(" />\n"));
}

// score - partwise / list / part / measure / direction / direction-type / coda
//------------------------------------------------------------------------------
c_coda::c_coda() :c_default_xy()
{}
c_coda::c_coda(wxXmlNode *xmlnode) : c_default_xy(xmlnode)
{}
c_coda::c_coda(c_coda const &coda) : c_default_xy(coda)
{}
void c_coda::write(wxFFile *f)
{
	f->Write(wxString::Format("<coda "));
	write_xy(f);
	f->Write(wxString::Format("/> \n"));
}

// score - partwise / list / part / measure / direction / direction-type / segno
//------------------------------------------------------------------------------
c_segno::c_segno() : c_default_xy()
{}
c_segno::c_segno(wxXmlNode *xmlnode) : c_default_xy(xmlnode)
{}
c_segno::c_segno(c_segno const &segno) : c_default_xy(segno)
{}
void c_segno::write(wxFFile *f)
{
	f->Write(wxString::Format("<segno "));
	write_xy(f);
	f->Write(wxString::Format("/> \n"));
}

// score - partwise / list / part / measure / direction / direction-type
//----------------------------------------------------------------------
c_direction_type::c_direction_type()
{}
c_direction_type::c_direction_type(wxXmlNode *xmlnode)
{
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "pedal")
		{
			pt = (void*)(new c_pedal(child));
			type = t_pedal;
		}
		else if (name == "octave-shift")
		{
			pt = (void*)(new c_octave_shift(child));
			type = t_octave_shift;
		}
		else if (name == "rehearsal")
		{
			pt = (void*)(new c_rehearsal(child));
			type = t_rehearsal;
		}
		else if (name == "wedge")
		{
			pt = (void*)(new c_wedge(child));
			type = t_wedge;
		}
		else if (name == "coda")
		{
			pt = (void*)(new c_coda(child));
			type = t_coda;
		}
		else if (name == "dynamics")
		{
			pt = (void*)(new c_dynamics(child));
			type = t_dynamics;
		}
		else if (name == "segno")
		{
			pt = (void*)(new c_segno(child));
			type = t_segno;
		}
		else if (name == "words")
		{
			pt = (void*)(new c_words(child));
			type = t_words;
		}
		child = child->GetNext();
	}
}
c_direction_type::~c_direction_type()
{
	if (pt != NULL)
	{
		switch (type)
		{
		case t_pedal: delete ((c_pedal*)(pt)); break;
		case t_octave_shift: delete ((c_octave_shift*)(pt)); break;
		case t_rehearsal: delete ((c_rehearsal*)(pt)); break;
		case t_wedge: delete ((c_wedge*)(pt)); break;
		case t_coda: delete ((c_coda*)(pt)); break;
		case t_dynamics: delete ((c_dynamics*)(pt)); break;
		case t_segno: delete ((c_segno*)(pt)); break;
		case t_words: delete ((c_words*)(pt)); break;
		default:  break;
		}
	}
}
c_direction_type::c_direction_type(c_direction_type const &direction_type)
{
	type = direction_type.type;
	switch (type)
	{
	case t_pedal: pt = (void *)(new c_pedal(*((c_pedal*)(direction_type.pt)))); break;
	case t_octave_shift: pt = (void *)(new c_octave_shift(*((c_octave_shift*)(direction_type.pt)))); break;
	case t_rehearsal: pt = (void *)(new c_rehearsal(*((c_rehearsal*)(direction_type.pt)))); break;
	case t_wedge: pt = (void *)(new c_wedge(*((c_wedge*)(direction_type.pt)))); break;
	case t_coda: pt = (void *)(new c_coda(*((c_coda*)(direction_type.pt)))); break;
	case t_dynamics: pt = (void *)(new c_dynamics(*((c_dynamics*)(direction_type.pt)))); break;
	case t_segno: pt = (void *)(new c_segno(*((c_segno*)(direction_type.pt)))); break;
	case t_words: pt = (void *)(new c_words(*((c_words*)(direction_type.pt)))); break;
	default: break;
	}
}
void c_direction_type::write(wxFFile *f)
{
	switch (type)
	{
	case t_pedal: ((c_pedal*)(pt))->write(f); break;
	case t_octave_shift: ((c_octave_shift*)(pt))->write(f); break;
	case t_rehearsal: ((c_rehearsal*)(pt))->write(f); break;
	case t_wedge: ((c_wedge*)(pt))->write(f); break;
	case t_coda: ((c_coda*)(pt))->write(f); break;
	case t_dynamics: ((c_dynamics*)(pt))->write(f); break;
	case t_segno: ((c_segno*)(pt))->write(f); break;
	case t_words: ((c_words*)(pt))->write(f); break;
	default:  break;
	}
}

// score - partwise / list / part / measure / direction / sound
//-------------------------------------------------------------
c_sound::c_sound()
{}
c_sound::c_sound(wxXmlNode *xmlnode)
{
	wxXmlAttribute *xmlAttribute =xmlnode->GetAttributes();
	if (xmlAttribute)
	{
		name = xmlAttribute->GetName();
		value = xmlAttribute->GetValue();
	}
}
c_sound::c_sound(c_sound const &sound)
{
	name = sound.name;
	value = sound.value;
}
void c_sound::write(wxFFile *f)
{
	f->Write(wxString::Format("<sound %s=\"%s\" />", name,encodeXML(value)));
}
// score-partwise/list/part/measure/direction
//-------------------------------------------
c_direction::c_direction()
{
}
c_direction::c_direction(wxXmlNode *xmlnode)
{
	placement = GetStringAttribute(xmlnode,"placement");
	directive = GetStringAttribute(xmlnode,"directive");
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "direction-type")
			direction_types.push_back(c_direction_type(child));
		else if (name == "sound")
			sounds.push_back(c_sound(child));
		child = child->GetNext();
	}
}
c_direction::c_direction(const c_direction &direction)
{
	placement = direction.placement;
	directive = direction.directive;
	for (auto & current: direction.direction_types ) 
	{
		direction_types.push_back(c_direction_type(current));
	}
	for (auto & current : direction.sounds )
	{
		sounds.push_back(c_sound(current));
	}
}
c_direction::~c_direction()
{
	direction_types.clear();
	sounds.clear();
}
void c_direction::write(wxFFile *f)
{
	if ((direction_types.size() == 0) && (sounds.size() == 0))
		return;
	f->Write(wxString::Format("<direction"));
	if (placement != NULL_STRING)
		f->Write(wxString::Format(" placement=\"%s\"", placement));
	if (directive != NULL_STRING)
		f->Write(wxString::Format(" directive=\"%s\"", directive));
	f->Write(wxString::Format(">\n"));
	if (direction_types.size() > 0)
	{
		f->Write(wxString::Format("<direction-type>\n"));
		for (auto & current : direction_types )
		{
			current.write(f);
		}
		f->Write(wxString::Format("</direction-type>\n"));
	}
	if (sounds.size() > 0)
	{
		for (auto & current : sounds )
		{
			current.write(f);
		}
	}
	f->Write(wxString::Format("</direction>\n"));
}

// score-partwise/list/part/measure/xsequencex
//--------------------------------------------
c_measure_sequence::c_measure_sequence()
{}
c_measure_sequence::c_measure_sequence(void *ipt, int itype)
{
	type = itype;
	pt = ipt;
}
c_measure_sequence::~c_measure_sequence()
{
	switch (type)
	{
	case t_note: delete ((c_note*)(pt)); break;
	case t_harmony: delete ((c_harmony*)(pt)); break;
	case t_backup: delete ((c_backup*)(pt)); break;
	case t_forward: delete ((c_forward*)(pt)); break;
	case t_barline: delete ((c_barline*)(pt)); break;
	case t_direction: delete ((c_direction*)(pt)); break;
	case t_attributes: delete ((c_attributes*)(pt)); break;
	default: wxASSERT(false);  break;
	}
}
c_measure_sequence::c_measure_sequence(const c_measure_sequence & measure_sequence, bool withContent)
{
	type = measure_sequence.type;
	switch (type)
	{
	case t_note: pt = (void *)(new c_note(*((c_note*)(measure_sequence.pt)))); break;
	case t_harmony: pt = (void *)(new c_harmony(*((c_harmony*)(measure_sequence.pt)))); break;
	case t_backup: pt = (void *)(new c_backup(*((c_backup*)(measure_sequence.pt)))); break;
	case t_forward: pt = (void *)(new c_forward(*((c_forward*)(measure_sequence.pt)))); break;
	case t_barline: pt = (void *)(new c_barline(*((c_barline*)(measure_sequence.pt)))); break;
	case t_direction: pt = (void *)(new c_direction(*((c_direction*)(measure_sequence.pt)))); break;
	case t_attributes: pt = (void *)(new c_attributes(*((c_attributes*)(measure_sequence.pt)), withContent)); break;
	default:wxASSERT(false);   break;
	}
}
void c_measure_sequence::write(wxFFile *f)
{
	switch (type)
	{
	case t_note: ((c_note*)(pt))->write(f); break;
	case t_harmony: ((c_harmony*)(pt))->write(f); break;
	case t_backup: ((c_backup*)(pt))->write(f); break;
	case t_forward: ((c_forward*)(pt))->write(f); break;
	case t_barline: ((c_barline*)(pt))->write(f); break;
	case t_direction: ((c_direction*)(pt))->write(f); break;
	case t_attributes: ((c_attributes*)(pt))->write(f); break;
	default: wxASSERT(false);   break;
	}
}
void c_measure_sequence::compile(int partNr , bool twelved , c_measure *measure)
{
	switch (type)
	{
	case t_note: ((c_note*)(pt))->compile(partNr , twelved); break;
	//case t_harmony: ((c_harmony*)(pt))->compile(twelved); break;
	case t_backup: ((c_backup*)(pt))->compile(twelved); break;
	case t_forward: ((c_forward*)(pt))->compile(twelved); break;
	//case t_barline: ((c_barline*)(pt))->compile(twelved); break;
	//case t_direction: ((c_direction*)(pt))->compile(twelved); break;
	case t_attributes: ((c_attributes*)(pt))->compile(twelved , measure); break;
	default: break;
	}
}
void c_measure_sequence::divisionsAlign(int ratio)
{
	switch (type)
	{
	case t_note: ((c_note*)(pt))->divisionsAlign(ratio); break;
	//case t_harmony: ((c_harmony*)(pt))->divisionsAlign(ratio); break;
	case t_backup: ((c_backup*)(pt))->divisionsAlign(ratio); break;
	case t_forward: ((c_forward*)(pt))->divisionsAlign(ratio); break;
	//case t_barline: ((c_barline*)(pt))->divisionsAlign(ratio); break;
	//case t_direction: ((c_direction*)(pt))->divisionsAlign(ratio); break;
	case t_attributes: ((c_attributes*)(pt))->divisionsAlign(); break;
	default: break;
	}
}
// score-partwise/list/part/measure
//---------------------------------
c_measure::c_measure()
{}
c_measure::c_measure(int inumber , int iwidth = NULL_INT)
{
	number = inumber;
	original_number = number;
	repeat = 0;
	width = iwidth;
}
c_measure::c_measure(wxXmlNode *xmlnode)
{
	number = GetIntAttribute(xmlnode,"number");
	original_number = number;
	repeat = 0;
	width = GetIntAttribute(xmlnode, "width");
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "note")
		{
			// c_measure_sequence *mnote = new c_measure_sequence(new c_note(child), t_note);
			measure_sequences.push_back(c_measure_sequence(new c_note(child), t_note));
		}
		else if (name == "harmony")
		{
			measure_sequences.push_back(c_measure_sequence(new c_harmony(child), t_harmony));
		}
		else if (name == "backup")
		{
			measure_sequences.push_back(c_measure_sequence(new c_backup(child), t_backup));
		}
		else if (name == "forward")
		{
			measure_sequences.push_back( c_measure_sequence(new c_forward(child), t_forward));
		}
		else if (name == "barline")
		{
			measure_sequences.push_back( c_measure_sequence(new c_barline(child), t_barline));
		}
		else if (name == "direction")
		{
			measure_sequences.push_back( c_measure_sequence(new c_direction(child), t_direction));
		}
		else if (name == "attributes")
		{
			measure_sequences.push_back( c_measure_sequence(new c_attributes(child), t_attributes));
		}
		child = child->GetNext();
	}
}
c_measure::c_measure(const c_measure &measure , bool withContent)
{
	number = measure.number;
	width = measure.width;
	division_beat = measure.division_beat;
	division_measure = measure.division_measure;
	division_quarter = measure.division_quarter;
	original_number = measure.original_number;
	repeat = measure.repeat;
	key_fifths = measure.key_fifths;
	for (auto & current : measure.measure_sequences )
	{
		int type = current.type;
		switch (type)
		{
		//case t_note: ((c_note*)(pt))->compile(partNr, twelved); break;
		//case t_harmony: ((c_harmony*)(pt))->compile(twelved); break;
		//case t_backup: ((c_backup*)(pt))->compile(twelved); break;
		//case t_forward: ((c_forward*)(pt))->compile(twelved); break;
		//case t_barline: ((c_barline*)(pt))->compile(twelved); break;
		//case t_direction: ((c_direction*)(pt))->compile(twelved); break;
		case t_attributes: measure_sequences.push_back(c_measure_sequence(current,withContent)); break;
		default: if (withContent) measure_sequences.push_back(c_measure_sequence(current, true));  break;
		}
	}
}
c_measure::~c_measure()
{
	measure_sequences.clear();
}
void c_measure::write(wxFFile *f, bool layout = true)
{
	f->Write(wxString::Format("<measure"));
	if (number != NULL_INT)
		f->Write(wxString::Format(" number=\"%d\"", number));
	if (layout && (width != NULL_INT))
		f->Write(wxString::Format(" width=\"%d\"", width));
	f->Write(wxString::Format(">\n"));
	for (auto & current : measure_sequences )
	{
		current.write(f);
	}
	f->Write(wxString::Format("</measure>\n"));
}
void c_measure::compile(c_measure *previous_measure , int partNr, bool twelved)
{
	if (previous_measure)
	{
		division_quarter = previous_measure->division_quarter;
		division_beat = previous_measure->division_beat;
		division_measure = previous_measure->division_measure;
		key_fifths = previous_measure->key_fifths;
		beats = previous_measure->beats;
		beat_type = previous_measure->beat_type;
	}
	if (previous_measure == NULL)
		original_number = 1;
	else
		original_number = previous_measure->original_number + 1;
	repeat = 0;
	for (auto & current : measure_sequences )
	{
		current.compile(partNr , twelved , this);
	}
}
void c_measure::divisionsAlign()
{
	if (divisions != NULL_INT)
		ratioDivisions = divisionsMax / divisions ;
	if (ratioDivisions == 1)
		return ;

	divisions *= ratioDivisions;
	division_quarter *= ratioDivisions;
	division_beat *= ratioDivisions;
	division_measure *= ratioDivisions;

	for (auto & current : measure_sequences )
	{
		current.divisionsAlign(ratioDivisions);
	}
}

// score-partwise/part
//-------------------------
c_part::c_part()
{}
c_part::c_part(wxString iid)
{
	id = iid;
}
c_part::c_part(wxXmlNode *xmlnode)
{
	wxXmlNode *child = xmlnode->GetChildren();
	id = GetStringAttribute(xmlnode,"id");
	while (child)
	{
		wxString name = child->GetName();
		if (name == "measure")
			measures.push_back(c_measure(child));
		child = child->GetNext();
	}
}
c_part::c_part(const c_part &part, bool withMeasures)
{
	id = part.id;
	if (withMeasures)
	{
		for (auto & current : part.measures )
		{
			measures.push_back(c_measure(current));
		}
	}
}
c_part::~c_part()
{
	measures.clear();
}
void c_part::write(wxFFile *f, bool layout = true)
{
	f->Write(wxString::Format("<part "));
	if (id != NULL_STRING)
		f->Write(wxString::Format(" id=\"%s\" ", id));
	f->Write(wxString::Format(" >\n"));
	for (auto & current : measures )
	{
		current.write(f, layout);
	}
	f->Write(wxString::Format("</part>\n"));
}
void c_part::compile(int ipartNr, bool twelved)
{
	gchord_counter = 0;
	gtranspose = 0;
	partNr = ipartNr;
	c_measure *previous_measure = NULL;
	for (auto & current : measures )
	{
		current.compile(previous_measure, partNr, twelved);
		previous_measure = & current;
	}
}
void c_part::divisionsAlign()
{
	for (auto & current : measures )
	{
		current.divisionsAlign();
	}
}

// score-partwise/work
//--------------------
c_work::c_work()
{}
c_work::c_work(wxXmlNode *xmlnode)
{
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "work-title")
			work_title = child->GetNodeContent();
		child = child->GetNext();
	}
}
c_work::c_work(const c_work &work)
{
	work_title = work.work_title;
}
void c_work::write(wxFFile *f)
{
	f->Write(wxString::Format("<work>\n"));
	if (work_title != NULL_STRING)
	f->Write(wxString::Format("<work-title>%s</work-title>\n", encodeXML(work_title)));
	f->Write(wxString::Format("</work>\n"));
}

// score-partwise/defaults/scaling
//--------------------
c_scaling::c_scaling()
{}
c_scaling::c_scaling(const c_scaling &scaling)
{
	millimeters = scaling.millimeters;
	tenths = scaling.tenths;
}
void c_scaling::read(wxXmlNode *xmlnode)
{
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "millimeters")
		{
			if (GetFloatContent(child) != NULL_INT)
				millimeters = GetFloatContent(child);
		}

		if (name == "tenths")
		{
			if (GetFloatContent(child) != NULL_INT)
				tenths = GetFloatContent(child);
		}
		child = child->GetNext();
	}
}
char* fint(float f, char*buf)
{
	// to avoid regionalisation in float printf (comma, point, .. )
	sprintf(buf,"%d.%05d\n",(int)(f), (int)(100000*(f-(float)((int)f))));
	return buf;
}
void c_scaling::write(wxFFile *f)
{
	char buffint[256];
	f->Write(wxString::Format("<scaling>\n"));
	f->Write(wxString::Format("<millimeters>%s</millimeters>\n", fint(millimeters,buffint)));
	f->Write(wxString::Format("<tenths>%s</tenths>\n", fint(tenths,buffint)));
	f->Write(wxString::Format("</scaling>\n"));
}

// score-partwise/defaults/page-layout
//--------------------
c_page_layout::c_page_layout()
{}
c_page_layout::c_page_layout(const c_page_layout &page_layout)
{
	page_height = page_layout.page_height;
	page_width = page_layout.page_width;
}
void c_page_layout::read(wxXmlNode *xmlnode)
{
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "page-height")
		{
			if (GetFloatContent(child) != NULL_INT)
				page_height = GetFloatContent(child);
		}

		if (name == "page-width")
		{
			if (GetFloatContent(child) != NULL_INT)
				page_width = GetFloatContent(child);
		}
		child = child->GetNext();
	}
}
void c_page_layout::write(wxFFile *f)
{
	char buffint[256];
	f->Write(wxString::Format("<page-layout>\n"));
	f->Write(wxString::Format("<page-height>%s</page-height>\n", fint(page_height,buffint)));
	f->Write(wxString::Format("<page-width>%s</page-width>\n", fint(page_width,buffint)));
	f->Write(wxString::Format("<page-margins type = \"even\">\n"));
	f->Write(wxString::Format("<left-margin>%s</left-margin>\n", fint(margin,buffint)));
	f->Write(wxString::Format("<right-margin>%s</right-margin>\n", fint(margin,buffint)));
	f->Write(wxString::Format("<top-margin>%s</top-margin>\n", fint(margin,buffint)));
	f->Write(wxString::Format("<bottom-margin>%s</bottom-margin>\n", fint(margin,buffint)));
	f->Write(wxString::Format("</page-margins>\n"));
	f->Write(wxString::Format("<page-margins type = \"odd\">\n"));
	f->Write(wxString::Format("<left-margin>%s</left-margin>\n", fint(margin,buffint)));
	f->Write(wxString::Format("<right-margin>%s</right-margin>\n", fint(margin,buffint)));
	f->Write(wxString::Format("<top-margin>%s</top-margin>\n", fint(margin,buffint)));
	f->Write(wxString::Format("<bottom-margin>%s</bottom-margin>\n", fint(margin,buffint)));
	f->Write(wxString::Format("</page-margins>\n"));
	f->Write(wxString::Format("</page-layout>\n"));
}

// score-partwise/defaults
//--------------------
c_defaults::c_defaults()
{}
c_defaults::c_defaults(wxXmlNode *xmlnode)
{
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "scaling")
			scaling.read(child);
		if (name == "page-layout")
			page_layout.read(child);
		child = child->GetNext();
	}
}
c_defaults::c_defaults(const c_defaults &defaults)
{
	scaling = defaults.scaling;
	page_layout = defaults.page_layout;
}
void c_defaults::write(wxFFile *f)
{
	f->Write(wxString::Format("<defaults>\n"));
	scaling.write(f);
	page_layout.write(f);
	f->Write(wxString::Format("</defaults>\n"));
}

// score-partwise
//---------------
c_score_partwise::c_score_partwise()
{}
c_score_partwise::c_score_partwise(wxXmlNode *xmlnode)
{
	wxXmlNode *child = xmlnode->GetChildren();
	while (child)
	{
		wxString name = child->GetName();
		if (name == "work")
			work = new c_work(child);
		else if (name == "part-list")
			part_list = new c_part_list(child);
		else if (name == "part")
			parts.push_back(c_part(child));
		child = child->GetNext();
	}
}
c_score_partwise::c_score_partwise(const c_score_partwise &score_partwise, bool withMeasures)
{
	if (score_partwise.work != NULL)
		work = new c_work(*(score_partwise.work));
	if (score_partwise.part_list != NULL)
		part_list = new c_part_list(*(score_partwise.part_list));
	for (auto & current : score_partwise.parts)
	{
		parts.push_back(c_part(current, withMeasures));
	}
	already_twelved = score_partwise.already_twelved;
}
c_score_partwise::~c_score_partwise()
{
	parts.clear();
	delete work;
	delete part_list;
}
void c_score_partwise::write(wxString filename , bool layout = true)
{
	wxFFile xf(filename,"w");
	if (!xf.IsOpened())
		return;
	xf.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE score-partwise PUBLIC \"-//Recordare//DTD MusicXML 3.0 Partwise//EN\" \"http://www.musicxml.org/dtds/partwise.dts\" >\n");
	xf.Write("<score-partwise>\n");
	if (work != NULL)
		work->write(&xf);
	defaults.write(&xf);
	if (part_list != NULL)
		part_list->write(&xf);
	for (size_t i = 0 ; i < parts.size(); i++ )
	{
		c_score_part *current_score_part = & (part_list->score_parts[i] );
		c_part *current_part = &(parts[i]);
		if (current_score_part->view)
			current_part->write(&xf, layout);
	}
	xf.Write("</score-partwise>\n");
	xf.Close();
}

void c_score_partwise::compile(bool twelved)
{
	divisionsMax = -1 ;
	bool tobe_twelved = false;
	if (twelved) // to increase the reolution of the duration by 3*2*2
	{
		if (!already_twelved)
		{
			tobe_twelved = true;
			already_twelved = true;
		}
	}
	for (size_t i = 0; i < parts.size(); i++)
	{
		parts[i].compile(i, tobe_twelved);
	}

	// to align divisions for each measure
	for (auto & current : parts )
	{
		current.divisionsAlign();
	}
}
