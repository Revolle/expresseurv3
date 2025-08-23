/////////////////////////////////////////////////////////////////////////////
// Name:        musicxmlcompile.cpp
// Purpose:     class to compile musicxml-source-file
// output :     musicxml-Expresseur-file MUSICXML_FILE + structure-to-play
// Author:      Franck REVOLLE
// Modified by:
// Created:     27/11/2015
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
#include "wx/tokenzr.h"
#include "wx/xml/xml.h"
#include "wx/filefn.h"
#include "wx/wfstream.h"
#include "wx/dynarray.h"
#include "wx/arrstr.h"
#include "wx/textfile.h"
#include "wx/config.h"
#include <wx/list.h>

#include "global.h"

#include "basslua.h"

#include "mxconf.h"
#include "musicxml.h"
#include "musicxmlcompile.h"


//#include <wx/arrimpl.cpp>
//WX_DEFINE_SORTED_ARRAY(ArrayOfMusicxmlevents);
//WX_DEFINE_SORTED_ARRAY(musicxmlevent *, SortedArrayOfMusicxmlevents);

enum ornamentType
{
	o_divisions = 0, // divisions per quarter note, used to count in a measure. For information to the end-user
	o_dynamic,
	o_random_delay,
	o_pedal_bar,
	o_pedal,
	o_lua,
	o_text,
	o_pianissimo,
	o_piano,
	o_mesopiano,
	o_mesoforte,
	o_forte,
	o_fortissimo,
	o_crescendo,
	o_diminuendo,
	o_tenuto,
	o_staccato,
	o_breath_mark,
	o_fermata,
	o_accent,
	o_grace,
	o_mordent,
	o_turn,
	o_btrill,
	o_trill,
	o_arpeggiate,
	o_transpose,
	o_delay,
	o_before,
	o_after,
	o_flagend
};
char const *ornamentName[] =
{
	"divisions_per_quarter",
	"dynamic",
	"random_delay",
	"pedal_bar",
	"pedal",
	"lua",
	"text",
	"pianissimo",
	"piano",
	"mesopiano",
	"mesoforte",
	"forte",
	"fortissimo",
	"crescendo",
	"diminuendo",
	"tenuto",
	"staccato",
	"breath_mark",
	"fermata",
	"accent",
	"grace",
	"mordent",
	"turn",
	"btrill",
	"trill",
	"arpeggiate",
	"transpose",
	"delay",
	"before",
	"after",
	"flagend"
};



static bool musicXmlEventsCompareStart(const c_musicxmlevent& a, const c_musicxmlevent& b)
{
	// funtion used to sort the list of musicxmlevent, using start time : l_musicxmlevent
	if (a.start_measureNr < b.start_measureNr)
		return true;
	if (a.start_measureNr >b.start_measureNr)
		return false;

	if (a.start_t < b.start_t)
		return true;
	if (a.start_t >b.start_t)
		return false;
	if (a.start_order < b.start_order)
		return true;
	if (a.start_order > b.start_order)
		return false;


	if (a.stop_measureNr < b.stop_measureNr)
		return true;
	if (a.stop_measureNr > b.stop_measureNr)
		return false;
	if (a.stop_t < b.stop_t)
		return true;
	if (a.stop_t > b.stop_t)
		return false;
	if (a.stop_order < b.stop_order)
		return true;
	if (a.stop_order > b.stop_order)
		return false;

	if (a.partNr < b.partNr)
		return true;
	if (a.partNr > b.partNr)
		return false;

	if (a.voice < b.voice)
		return true;
	if (a.voice > b.voice)
		return false;

	if (a.pitch < b.pitch)
		return true;
	if (a.pitch >b.pitch)
		return false;

	return false;
}
static int musicXmlEventsCompareStop(const c_musicxmlevent& a, const c_musicxmlevent& b)
{
	// funtion used to sort the list of c_musicxmlevent, using stop time : l_musicxmlevent

	if (a.stop_measureNr < b.stop_measureNr)
		return true;
	if (a.stop_measureNr > b.stop_measureNr)
		return false;

	if (a.stop_t < b.stop_t)
		return true;
	if (a.stop_t > b.stop_t)
		return false;

	if (a.stop_order < b.stop_order)
		return true;
	if (a.stop_order > b.stop_order)
		return false;

	if ((!(a.tenuto)) && (b.tenuto))
		return true;
	if ((a.tenuto) && (!(b.tenuto)))
		return false;

	if (a.start_measureNr < b.start_measureNr)
		return false;
	if (a.start_measureNr > b.start_measureNr)
		return true;

	if (a.start_t < b.start_t)
		return false;
	if (a.start_t > b.start_t)
		return true;

	if (a.start_order < b.start_order)
		return false;
	if (a.start_order > b.start_order)
		return true;

	if (a.nr < b.nr)
		return true;
	if (a.nr > b.nr)
		return false;

	return false;
}
static bool measureMarkCompare(const c_measureMark& a, const c_measureMark& b)
{
	return (a.number < b.number );
}
static bool ornamentCompare(const c_ornament& a, const c_ornament& b)
{
	// compare function for the list of l_ornament
	if (a.measureNumber < b.measureNumber)
		return true;
	if (a.measureNumber > b.measureNumber)
		return false;
	if (a.t < b.t)
		return true;
	if (a.t > b.t)
		return false;
	if (a.partNr < b.partNr)
		return true;
	if (a.partNr > b.partNr)
		return false;
	return false;
}
static bool ornamentEgal(const c_ornament& a, const c_ornament& b)
{
	bool r =
		(a.type == b.type) &&
		(a.partNr == b.partNr) &&
		(a.staffNr == b.staffNr) &&
		(a.measureNumber == b.measureNumber) &&
		(a.t == b.t) &&
		(a.chord_order == b.chord_order) &&
		(a.repeat == b.repeat) &&
		(a.before == b.before) &&
		(a.value == b.value) &&
		(a.absolute_measureNr == b.absolute_measureNr) &&
		(a.mark_prefix == b.mark_prefix);
	return r;
};

static bool arpeggiateCompare(const c_arpeggiate_toapply& a, const c_arpeggiate_toapply& b)
{
	// compare function for the list of l_ornament
	if (a.nr < b.nr)
		return true;
	if (a.nr > b.nr)
		return false;
	if (a.musicxmlevent->pitch < b.musicxmlevent->pitch)
		return ((a.down && !(a.before)) || (!(a.down) && a.before) ? false : true);
	if (a.musicxmlevent->pitch > b.musicxmlevent->pitch)
		return ((a.down && !(a.before)) || (!(a.down) && a.before) ? true : false);
	return false;
}
static bool pedal_barCompare(const c_pedal_bar_toapply& a, const c_pedal_bar_toapply& b)
{
	// compare function for the list of l_pedal_bar_toapply
	return (a.measureNr < b.measureNr);
}
void musicxmlcompile::fillStartStopNext()
{

	// add a fake element at the end 



	// lMusicxmlevents  : index nr sorted by the Start-time
	std::sort(lMusicxmlevents.begin(), lMusicxmlevents.end(),musicXmlEventsCompareStart);
	int nr_musicxmlevent = -1 ;
	for (auto& current_musicxmlevent : lMusicxmlevents)
	{
		nr_musicxmlevent++;
		nb_measure = current_musicxmlevent.stop_measureNr;
		current_musicxmlevent.nr = nr_musicxmlevent;
	}

	c_musicxmlevent last_musicxmlevent;
	last_musicxmlevent.visible = false;
	last_musicxmlevent.played = false;
	last_musicxmlevent.start_measureNr = nb_measure + 1;
	last_musicxmlevent.stop_measureNr = nb_measure + 1;
	last_musicxmlevent.start_t = 0;
	last_musicxmlevent.stop_t = 0;
	last_musicxmlevent.end_score = true;
	lMusicxmlevents.push_back(last_musicxmlevent);

	nbEvents = lMusicxmlevents.size() ;

	// index Stop : indes sorted by the Stop-time
	std::sort(lMusicxmlevents.begin(), lMusicxmlevents.end(), musicXmlEventsCompareStop);
	indexStop.clear();
	for (auto& current_musicxmlevent : lMusicxmlevents)
	{
		indexStop.push_back(current_musicxmlevent.nr);
	}

	std::sort(lMusicxmlevents.begin(), lMusicxmlevents.end(), musicXmlEventsCompareStart);
	int nrEvent = -1;
	for (std::vector<c_musicxmlevent>::iterator current_musicxmlevent = lMusicxmlevents.begin()  ;
		current_musicxmlevent != lMusicxmlevents.end(); 
		current_musicxmlevent++ )
	{
		nrEvent++;
		int nrEventNext = nrEvent;
		for (std::vector<c_musicxmlevent>::iterator next_musicxmlevent = lMusicxmlevents.begin() + nrEvent + 1 ;
			next_musicxmlevent != lMusicxmlevents.end() ; 
			next_musicxmlevent ++)
		{
			nrEventNext++;
			if (!next_musicxmlevent->played)	
				continue;
			if (next_musicxmlevent->start_measureNr < current_musicxmlevent->stop_measureNr) 
				continue;
			if (next_musicxmlevent->start_measureNr > current_musicxmlevent->stop_measureNr)
			{
				current_musicxmlevent->nextNr = nrEventNext;
				break;
			}
			if (next_musicxmlevent->start_t < current_musicxmlevent->stop_t) 
				continue;
			if (next_musicxmlevent->start_t > current_musicxmlevent->stop_t)
			{
				current_musicxmlevent->nextNr = nrEventNext;
				break;
			}
			if (next_musicxmlevent->start_order < current_musicxmlevent->stop_order) 
				continue;
			if (next_musicxmlevent->start_order >= current_musicxmlevent->stop_order)
			{
				current_musicxmlevent->nextNr = nrEventNext;
				break;
			}
		}	
	}

}
void musicxmlcompile::dump_musicxmlevents()
{
	/*
	int nrEvent = -1 ;
	for (auto & current_musicxmlevent : lMusicxmlevents )
	{
		nrEvent ++ ;
		int nr = current_musicxmlevent.nr;
		int start = current_musicxmlevent.start_t;
		int stop = current_musicxmlevent.stop_t;
		int track = current_musicxmlevent.partNr;
		int pitch = current_musicxmlevent.pitch;
		int duration = current_musicxmlevent.duration;
		int will_stop_index = current_musicxmlevent.will_stop_index;
		int stop_index = current_musicxmlevent.stop_index;
		int nextNr = current_musicxmlevent.nextNr;
		int i = 1;
	}
	*/	
}
musicxmlcompile::musicxmlcompile()
{
	// slMusicxmlevents = new SortedArrayOfMusicxmlevents(ComparemusicXmlEvents);
	score = NULL;
	compiled_score = NULL;
	lMusicxmlevents.clear();
	lMeasureMarks.clear();
	lOrnaments.clear();
}
musicxmlcompile::~musicxmlcompile()
{
	lMusicxmlevents.clear();
	lMeasureMarks.clear();
	lArpeggiate_toapply.clear();
	lPedal_bar_toapply.clear();
	lOrnaments.clear();
	lOrnamentsMusicxmlevents.clear();

	lEventPlaybacks.clear() ;
	lEventPlaybacks.clear();

	if (score != NULL)
		delete score;
	if (compiled_score != NULL)
		delete compiled_score;
}
wxFileName musicxmlcompile::loadTxtFile(wxFileName itxtFile)
{

	// just extract the musicxml file from the txtfile
	txtFile = itxtFile;

	readMarks(false);
	return musicxmlFile;
}
void musicxmlcompile::setNameFile(wxFileName itxtFile, wxFileName ixmlFile)
{

	txtFile = itxtFile;
	musicxmlFile = ixmlFile;
}
wxFileName musicxmlcompile::getNameTxtFile()
{
	return txtFile;
}
wxFileName musicxmlcompile::getNameXmlFile()
{
	return musicxmlFile;
}
bool musicxmlcompile::loadXmlFile(wxString xmlfilein, bool useMarkFile)
{

	// load the musicxml musicxmlFile, compile it, and generate the MUSICXML_FILE for the musicxml-viewer

	// load the input muscixml file in the C++ score structure
	xmlLoad(xmlfilein);

	if (!isOk())
		return false;

	// compile the C++ score structure into :
	// - a musicxml file to diplay 
	// - a set of events to play
	// - a file of parameters ( lOrnaments, repetitions, .. )
	
	compileScore(useMarkFile);

	return isOk();
}
void musicxmlcompile::xmlLoad(wxString xmlfilein)
{

	// load the inupt muscixml file in the C++ score structure

	isModified = true ;

	if (score != NULL)
		delete score;
	score = NULL;

	wxXmlDocument *xmlDoc = new wxXmlDocument();
	if (!xmlDoc->Load(xmlfilein))
	{
		delete xmlDoc;
		return;
	}

	wxXmlNode *root = xmlDoc->GetRoot();
	wxString name = root->GetName();
	if (name != "score-partwise")
	{
		wxMessageBox("Only musicXML score-partwise is accepted", "MusicXML load", wxICON_ERROR);
		delete xmlDoc;
		return ;
	}

	// load xml in the C++ score-structure
	score = new c_score_partwise(root);
	delete xmlDoc;

	if (!isOk())
	{
		delete score;
		score = NULL;
	}

}
void musicxmlcompile::compileScore(bool useMarkFile)
{
	// compile the C++ score structure into :
	// - a musicxml file to display 
	// - a set of events to play
	// - a file of parameters ( lOrnaments, repetitions, .. )

	if (!isOk())
		return;

	// create a new score, from the original one, without the measures
	if (compiled_score != NULL)
		delete compiled_score;
	compiled_score = new c_score_partwise(*score, false);

	isModified = true ;

	lMusicxmlevents.clear();
	lMeasureMarks.clear();
	lArpeggiate_toapply.clear();
	lPedal_bar_toapply.clear();
	lOrnaments.clear();
	lOrnamentsMusicxmlevents.clear();

	// compile the score to calculate additional information ( beat/measure, transposition, .. ), and to increase resolution "divisions*12"
	score->compile(true);

	// analyse the default repeat-sequence from "score", fill "measureMark" and "markList"
	analyseMeasure();

	// remove an existing Expresseur Part
	removeExpresseurPart();

	// overload the parameters read from the score, with the optional data available in the text file
	if (useMarkFile)
		readMarks();

	// create the list of measures, according to repetitions
	createListMeasures();

	// build the sequence of measures in the compiled parts
	addExpresseurPart();

	buildMeasures();

	// save the parameters in the text file
	if (!useMarkFile)
		writeMarks();

	// compile the score and build the notes to play in lMusicxmlevents
	compilePlayedScore();

	//  add lOrnaments in the notes to play in lMusicxmlevents
	addOrnaments();
	// lMusicxmlevents contains the notes to play. Compile lMusicxmlevents
	compileMusicxmlevents();

	// add the part for Expresseur, according to lMusicxmlevents
	compileExpresseurPart();

	// push the events to play to the LUA-script
	pushLuaMusicxmlevents();

}
bool musicxmlcompile::isOk(bool check_compiled_score)
{
	// return true if the C++ score is OK

	if (score == NULL)
		return false;
	if (score->part_list == NULL)
		return false;
	if (score->parts.size() == 0)
		return false;
	if (score->parts[0].measures.size() == 0)
		return false;
	if (check_compiled_score)
	{
		if (compiled_score == NULL)
			return false;
		if (compiled_score->part_list == NULL)
			return false;
		if (compiled_score->parts.size() == 0)
			return false;
		if (compiled_score->part_list->score_parts.size() == 0)
			return false;
	}
	return true;
}
void musicxmlcompile::pushLuaMusicxmlevents()
{
	// push to LUA the musicXemEvents compiled
	basslua_call(moduleScore, functionScoreInitScore, "");
	
	std::sort(lMusicxmlevents.begin(), lMusicxmlevents.end() , musicXmlEventsCompareStart);
	getMarkNr(-1);
	getMeasureNr(-1);
	int nr_musicxmlevent = -1;
	for (auto& m : lMusicxmlevents )
	{
		nr_musicxmlevent++;
		char buflua[256];
		strcpy(buflua, m.lua.c_str());

		int markNr = getMarkNr(m.original_measureNr);
		int measureNr = getMeasureNr(m.original_measureNr);
		int measureLength = m.division_measure ;
		if ( m.cross )
			m.velocity = 1 ;
		// push the event itself
#define pushLUAparameters "iiiiiiiiisiiiiiiiiiii"
		basslua_call(moduleScore, functionScoreAddEvent, pushLUAparameters,
			m.played , m.visible, 
			(m.partNr) + 1, m.pitch, m.velocity, m.delay ,
			m.dynamic, m.random_delay, m.pedal,
			buflua ,
			(m.will_stop_index) + 1, (m.stop_index) + 1,
			markNr, measureNr, measureLength,
			m.start_measureNr,m.start_t , m.start_order, 
			m.stop_measureNr, m.stop_t , m.stop_order			
			);
		// push the starts of the event
		int nb = m.starts.size();
		for (int n = 0; n < nb; n++)
		{
			if (m.starts[n] >= 0)
				basslua_call(moduleScore, functionScoreAddEventStarts, "i", (m.starts[n]) + 1);
		}
		// push the stops of the event
		nb = m.stops.size();
		for (int n = 0; n < nb; n++)
		{
			if (m.stops[n] >= 0)
				basslua_call(moduleScore, functionScoreAddEventStops, "i", (m.stops[n]) + 1);
		}
	}
	// push the tracks to LUA
	int nb_tracks = getTracksCount();
	for (int n = 0; n < nb_tracks; n++)
	{
		wxString s = getTrackName(n);
		char buf[256];
		strcpy(buf, s.c_str());
		basslua_call(moduleScore, functionScoreAddTrack, "s", buf);
	}
	// finish the push
	basslua_call(moduleScore, functionScoreFinishScore, "");
}
void musicxmlcompile::clearLuaScore()
{
	basslua_call(moduleScore, functionScoreInitScore, "");
}
void musicxmlcompile::analyseMeasure()
{
	// analyse the default repeat-sequence from "score" to "measureMark" and "markList"

	analyseMeasureMarks();

	sortMeasureMarks();
	
	analyseList();

	int nbMarks = lMeasureMarks.size();
	int nbList = markList.size();
	if ((nbMarks < 2) && (nbList < 2))
	{
		lMeasureMarks.clear();
		markList.clear();
	}
	// for debug :
	//for (auto & current_measureMark : lMeasureMarks)
	//{
	//	int i = current_measureMark->number;
	//	wxString s = current_measureMark->name;
	//}
	//std::vector <int>::iterator iter_markList;
	//for (auto & current_markList : markList)
	//{
	//	int i = current_markList;
	//}
}
int musicxmlcompile::getMarkNr(int measureNr)
{
	static int cgetMarkNr = 1;
	static int measureNrgetMarkNr = -1;
	if (measureNr < 0)
	{
		cgetMarkNr = 1;
		measureNrgetMarkNr = 1;
		return false;
	}
	if (measureNr == measureNrgetMarkNr)
		return cgetMarkNr;
	measureNrgetMarkNr = measureNr;
	for (auto & measureMark : lMeasureMarks)
	{
		if (measureMark.number == measureNr)
		{
			cgetMarkNr++;
			return cgetMarkNr;
		}
	}
	return cgetMarkNr;
}
int musicxmlcompile::getMeasureNr(int measureNr)
{
	static int cMeasureNr = 0;
	static int pMeasureNr = -1;
	if (measureNr < 0)
	{
		cMeasureNr = 0;
		pMeasureNr = -1;
		return 0;
	}
	if (measureNr != pMeasureNr)
		cMeasureNr++;
	pMeasureNr = measureNr;
	return cMeasureNr;
}
void musicxmlcompile::analyseMeasureMarks()
{
	// extract from the current score xmlfilein: rehearsal, special barlines, repeat, coda, segno ..., filling :
	//   - measureMark : marks read from barlines and rehearsal 
	//   - markList : list of marks to play , acording to repeat signs

	// extract from the current score lOrnaments list
	bool mark; 
	int partNr = -1;
	for (auto & part : score->parts )
	{
		partNr++;
		mark = (partNr == 0);
		int nbMeasure = part.measures.size();
		for (auto & measure : part.measures)
		{
			int timeMeasure = 0;
			/* if ((measure->divisions != NULL_INT) && (partNr == 0))
			{
				c_ornament *ornament = new c_ornament(o_divisions, measure->number, 0, -1, -1, -1, false, wxString::Format("%d", measure->divisions));
				lOrnaments.Append(ornament);
			}*/
			for (auto & sequence : measure.measure_sequences)
			{
				c_measureMark measureMark(measure.number);
				switch (sequence.type)
				{
				case t_barline:
					if ( partNr == 0 )
					{
						c_barline *barline = (c_barline *)(sequence.pt);
						// bar_style in { regular, dotted, dashed, heavy, light-light, light-heavy, heavy-light, heavy-heavy, tick, short, none }
						wxChar c = barline->bar_style[0];
						switch (c)
						{
						case 'h':
						case 'H':
						case 'l':
						case 'L':
							mark = true;
							break;
						default:break;
						}
						if (barline->location == "right")
						{
							measureMark.changeMeasure(measure.number + 1);
							if (measure.number == nbMeasure)
							{
								measureMark.name = END_OF_THE_SCORE;
							}
						}
						if ((barline->repeat) && (barline->repeat->direction == "forward"))
						{
							mark = true;
							measureMark.repeatForward = true;
						}
						if ((barline->repeat) && (barline->repeat->direction == "backward"))
						{
							mark = true;
							measureMark.repeatBackward = true;
						}
						if ((barline->ending) && (barline->ending->type == "start") && (barline->ending->number[0] == '1'))
						{
							mark = true;
							measureMark.jumpnext = true;
						}
					}
					break;
				case t_direction:
				{
					c_direction *direction = (c_direction *)(sequence.pt);
					for (auto& direction_type : direction->direction_types)
					{
						switch (direction_type.type)
						{
						case t_rehearsal:
							mark = true;
							measureMark.name = ((c_rehearsal*)(direction_type.pt))->value;
							measureMark.rehearsal = true;
						break;
						case t_wedge:
						{
							c_wedge *current_wedge = (c_wedge*)(direction_type.pt);
							wxString type = current_wedge->type.MakeLower();
							if (type == "crescendo")
							{
								lOrnaments.push_back(c_ornament(o_crescendo, measure.original_number, timeMeasure, partNr, -1, -1, false, ""));
							}
							else if (type == "diminuendo")
							{
								lOrnaments.push_back(c_ornament(o_diminuendo, measure.original_number, timeMeasure, partNr, -1, -1 , false, ""));
							}
						}
						break;
						case t_pedal:
						{
							c_pedal *pedal = (c_pedal*)(direction_type.pt);
							wxString s = pedal->type.Lower();
							if (s == "start")
								lOrnaments.push_back(c_ornament(o_pedal, measure.original_number, timeMeasure, partNr, -1, -1, false, ""));
						}
						break;
						case t_dynamics:
						{
							c_dynamics *dynamics = (c_dynamics*)(direction_type.pt);
							wxString s = dynamics->dynamic.Lower();
							int o = -1;
							if (s == "pp") o = o_pianissimo;
							else if (s == "p") o = o_piano;
							else if (s == "mp") o = o_mesopiano;
							else if (s == "mf") o = o_mesoforte;
							else if (s == "f") o = o_forte;
							else if (s == "ff") o = o_fortissimo;
							if ( o != -1 )
								lOrnaments.push_back(c_ornament(o, measure.original_number, timeMeasure, partNr, -1, -1, false, ""));
						}
						default: break;
						}
					}
					for (auto & sound : direction->sounds)
					{
						mark = true;
						if (sound.name == "dacapo")
						{
							measureMark.changeMeasure(measure.number + 1);
							measureMark.dacapo = true;
						}
						else if (sound.name == "coda")
						{
							measureMark.name = "coda";
							measureMark.coda = true;
						}
						else if (sound.name == "segno")
						{
							measureMark.segno = true;
							measureMark.name = "segno";
						}
						else if (sound.name == "fine")
						{
							measureMark.changeMeasure(measure.number + 1);
							measureMark.fine = true;
						}
						else if (sound.name == "tocoda")
						{
							measureMark.changeMeasure(measure.number + 1);
							measureMark.tocoda = true;
						}
						else if (sound.name == "dalsegno")
						{
							measureMark.changeMeasure(measure.number + 1);
							measureMark.dalsegno = true;
						}
					}
				}
					break;
				case t_note:
				{
					c_note *note = (c_note *)(sequence.pt);
					if (note->chord)
						timeMeasure -= (note->duration == NULL_INT) ? 0 : note->duration;
					analyseNoteOrnaments(note, measure.number, timeMeasure);
					timeMeasure += (note->duration == NULL_INT) ? 0 : note->duration;
				}
					break;
				case t_backup:
				{
					c_backup *backup = (c_backup *)(sequence.pt);
					timeMeasure -= backup->duration;
				}
					break;
				case t_forward:
				{
					c_forward *forward = (c_forward *)(sequence.pt);
					timeMeasure += forward->duration;
				}
					break;
				default:
					break;
				}
				if (mark)
					lMeasureMarks.push_back(measureMark);
				mark = false;
			}
		}
	}
}
void musicxmlcompile::analyseNoteOrnaments(c_note *note, int measureNumber, int t)
{
	// extract the list of arnaments from the note read in the muscixml source file
	if (note->grace)
	{
		wxString s;
		wxString sep;
		wxString alter;
		if (grace.empty() == false)
		{
			if (note->chord)
				sep = "+";
			else
				sep = ",";
		}
		switch (note->pitch->alter)
		{
		case -2: alter = "bb"; break;
		case -1: alter = "b"; break;
		case 1: alter = "#"; break;
		case 2: alter = "##"; break;
		default: alter = ""; break;
		}
		s.Printf("%s%s%s%d", sep, note->pitch->step, alter, note->pitch->octave);
		grace.Append(s);
	}
	else
	{
		if (grace.empty() == false)
		{
			lOrnaments.push_back(c_ornament(o_grace, measureNumber, t, note->partNr, note->staff, -1, false, grace));
			grace.Empty();
		}
	}

	if (note && (note->notations))
	{
		if (note->notations->arpeggiate)
		{
			if (note->notations->arpeggiate->direction == "down")
				lOrnaments.push_back(c_ornament(o_arpeggiate, measureNumber, t, note->partNr, note->staff, -1, false, "down"));
			else
				lOrnaments.push_back(c_ornament(o_arpeggiate, measureNumber, t, note->partNr, note->staff, -1, false, "up"));
		}
		if (note->notations->fermata)
		{
			lOrnaments.push_back(c_ornament(o_fermata, measureNumber, t, note->partNr, note->staff, -1, false, ""));
		}
		if ((note->notations->lOrnaments) && (note->notations->lOrnaments->lOrnaments.size() > 0))
		{
			wxString stype = note->notations->lOrnaments->lOrnaments.front() ;
			if (stype == "inverted-mordent")
			{
				lOrnaments.push_back(c_ornament(o_mordent, measureNumber, t,  note->partNr, note->staff, -1, false, "inverted"));
			}
			else if (stype == "mordent")
			{
				lOrnaments.push_back(c_ornament(o_mordent, measureNumber, t, note->partNr, note->staff, -1, false, ""));
			}
			else if (stype == "inverted-turn")
			{
				lOrnaments.push_back(c_ornament(o_turn, measureNumber, t, note->partNr, note->staff, -1, false, "inverted"));
			}
			else if (stype == "turn")
			{
				lOrnaments.push_back(c_ornament(o_turn, measureNumber, t,  note->partNr, note->staff, -1, false, ""));
			}
			/*
			else if (stype == "delayed-inverted-turn")
			{
				lOrnaments.push_back(c_ornament(o_delayed_turn, measureNumber, t, partNr, -1, false, "inverted"));
			}
			else if (stype == "delayed-turn")
			{
				lOrnaments.push_back(c_ornament(o_delayed_turn, measureNumber, t, partNr, -1, false, ""));
			}
			*/
			else if (stype == "trill-mark")
			{
				if (((note->mtype == "whole") || (note->mtype == "half")) && (note->dots == 0))
					lOrnaments.push_back(c_ornament(o_trill, measureNumber, t, note->partNr, note->staff, -1, false, "8"));
				else if (((note->mtype == "whole") || (note->mtype == "half")) && (note->dots > 0))
					lOrnaments.push_back(c_ornament(o_trill, measureNumber, t, note->partNr, note->staff, -1, false, "12"));
				else if (note->dots > 0)
					lOrnaments.push_back(c_ornament(o_trill, measureNumber, t, note->partNr, note->staff, -1, false, "6"));
				else
					lOrnaments.push_back(c_ornament(o_trill, measureNumber, t, note->partNr, note->staff, -1, false, "4"));
			}
		}
		if ((note->notations->articulations) && (note->notations->articulations->articulations.size() > 0))
		{
			for (auto & stype : note->notations->articulations->articulations)
			{
				stype.Lower();
				if (stype.Contains("accent"))
				{
					lOrnaments.push_back(c_ornament(o_accent, measureNumber, t, note->partNr, note->staff, -1, false, ""));
				}
				else if (stype == "tenuto")
				{
					lOrnaments.push_back(c_ornament(o_tenuto, measureNumber, t, note->partNr, note->staff, -1, false, ""));
				}
				else if (stype == "staccato")
				{
					lOrnaments.push_back(c_ornament(o_staccato, measureNumber, t, note->partNr, note->staff, -1, false, "display"));
				}
				else if (stype == "breath-mark")
				{
					lOrnaments.push_back(c_ornament(o_breath_mark, measureNumber, t, note->partNr, note->staff, -1, false, ""));
				}
			}
		}
	}
}
void musicxmlcompile::analyseList()
{
	// capture list of marks in meausureMarks, into markList

	unsigned int markPlayed = 0; // start at the beginning of the score
	bool inDacapo = false;
	int inRepeat = 0;
	unsigned int markRepeat = 0;
	int markSegno = 0;
	int antiloop = 0;
	while (true)
	{
		// specific markers
		if (lMeasureMarks[markPlayed].segno)
			markSegno = markPlayed;
		if ((inRepeat == 2) && (lMeasureMarks[markPlayed].repeatBackward) && (markPlayed != markRepeat))
		{
			inRepeat = 0;
		}
		if ((inRepeat == 0) && (inDacapo == false) && (lMeasureMarks[markPlayed].repeatForward))
		{
			markRepeat = markPlayed;
			inRepeat = 1;
		}

		if ((inRepeat != 1) && (inDacapo == false) && (lMeasureMarks[markPlayed].dacapo))
		{
			markPlayed = 0;
			inDacapo = true;
		}
		else if ((inRepeat != 1) && (inDacapo == false) && (lMeasureMarks[markPlayed].dalsegno))
		{
			markPlayed = markSegno;
			inDacapo = true;
		}
		else if ((inRepeat == 1) && (lMeasureMarks[markPlayed].repeatBackward) && (markPlayed != markRepeat))
		{
			markPlayed = markRepeat;
			inRepeat = 2;
		}
		else if ((inRepeat == 2) && (lMeasureMarks[markPlayed].jumpnext))
		{
			markPlayed++;
			inRepeat = 0;
		}
		else if ((inDacapo) && (lMeasureMarks[markPlayed].fine))
		{
			break;
		}
		else if ((inDacapo) && (lMeasureMarks[markPlayed].tocoda))
		{
			bool found = false;
			int nrMark = -1;
			for (auto & current_measureMark : lMeasureMarks)
			{
				nrMark++;
				if (current_measureMark.coda)
				{
					found = true;
					markPlayed = nrMark;
				}
			}
			if (found == false)
				break;
		}
		else
		{
			markList.push_back(markPlayed);
			markPlayed++;
		}
		if (markPlayed >= lMeasureMarks.size())
			break;
		if ((antiloop++) > 1000)
		{
			wxASSERT(false);
			break;
		}
	}
}
void musicxmlcompile::sortMeasureMarks()
{
	// sort the list of markers, by measure number 

	std::sort(lMeasureMarks.begin() , lMeasureMarks.end(), measureMarkCompare);
	// suppress markers with same measure number
	int prev_number = -1;
	c_measureMark *prev_measureMark = NULL;
	for (auto & measureMark : lMeasureMarks)
	{
		if (measureMark.number == prev_number)
		{
			if (measureMark.rehearsal != NULL)
			{
				measureMark.merge(*prev_measureMark);
				prev_measureMark->toBeDeleted = true;
			}
			else
			{
				prev_measureMark->merge(measureMark);
				measureMark.toBeDeleted = true;

			}
		}
		else
		{
			prev_number = measureMark.number;
			prev_measureMark = &measureMark;
		}
	}
	// remove the markers to be deleted
	lMeasureMarks.erase(
		std::remove_if(lMeasureMarks.begin(), lMeasureMarks.end(), 
		[](const c_measureMark& p) {
		return p.toBeDeleted;
		}),
		lMeasureMarks.end());
}
void musicxmlcompile::singleOrnaments()
{
	// clear the ornaments duplicated 
	if (lOrnaments.size() < 2)
		return;
	std::sort(lOrnaments.begin(), lOrnaments.end(), ornamentCompare);
	lOrnaments.erase(
		std::unique(lOrnaments.begin(), lOrnaments.end(), ornamentEgal), 
		lOrnaments.end());
}
void musicxmlcompile::clearOrnaments()
{
	// clear the ornaments, except the divisions 
	lOrnaments.clear();
}
void musicxmlcompile::writeMarks()
{
	// write in a text file <inputfilename> with txt extension :
	// FILE
	// musicXml file
	// SET_MARKS
	// measure_number=label ..
	// PLAY_MARKS
	// markList ...
	// SET_TRACKS
	// trackname : view play
	// SET_ORNAMENTS
	// meausureNr[.beat[.time[.chord_order]]][*repeat][/track][<]=ornament_name
	// PLAYBACK
	// time,type_msg,channel,value1,value2

	wxTextFile f;
	wxString sf = txtFile.GetFullPath();
	if (! txtFile.FileExists())
		f.Create(sf);
	f.Open(sf);
	if (!f.IsOpened())
	{
		wxLogError("writeMarks : Error on file %s", sf);
		return;
	}
	f.Clear();

	wxString s;

	s.Printf("%s : %s", SET_MUSICXML_FILE, musicxmlFile.GetFullName());
	f.AddLine(s);

	f.AddLine("");
	if ( compiled_score->work )
		s.Printf("%s : %s", SET_TITLE, compiled_score->work->work_title.c_str() );
	else
		s.Printf("%s : %s", SET_TITLE, musicxmlFile.GetFullName() );
	f.AddLine(s);

	f.AddLine("");
	s.Printf("%s :", SET_MARKS);
	f.AddLine(s);
	sortMeasureMarks();
	if (lMeasureMarks.size() == 0)
	{
		f.AddLine("-- nothing defined.");
		f.AddLine("-- Example with \"A\" on measure 1, and \"B\" on measure 9 :");
		f.AddLine("--   1:A");
		f.AddLine("--   9:B");
	}
	else
	{
		for (auto & measureMark : lMeasureMarks)
		{
			if (!measureMark.name.IsSameAs(END_OF_THE_SCORE))
			{
				s.Printf("%d:%s", measureMark.number, measureMark.name);
				f.AddLine(s);
			}
		}
	}

	f.AddLine("");

	s.Printf("%s :", SET_PLAY_MARKS);
	f.AddLine(s);
	if (lMeasureMarks.size() == 0)
	{
		f.AddLine("-- Nothing defined.");
		f.AddLine("-- Example to organize MARKS A and B : ");
		f.AddLine("--  A");
		f.AddLine("--  B");
		f.AddLine("--  A");
	}
	else
	{
		for (auto & mark : markList)
		{
			if ((mark >= 0) && (mark < (int)(lMeasureMarks.size())))
			{
				s = lMeasureMarks[mark].name;
				if (!s.IsSameAs(END_OF_THE_SCORE))
				{
					f.AddLine(s);
				}
			}
		}
	}

	f.AddLine("");

	s.Printf("%s :", SET_PARTS);
	f.AddLine(s);

	// list the parts
	int partNr = -1;
	for (auto & current_part : compiled_score->part_list->score_parts )
	{
		partNr++;
		if (current_part.id != ExpresseurId)
		{
			wxString alias;
			if (current_part.part_alias != current_part.part_name)
			{
				alias.Printf(",alias=%s/%s", current_part.part_alias, current_part.part_alias_abbreviation);
			}
			s.Printf("P%d_%s:%s/%s%s", partNr + 1, current_part.part_name,
				current_part.play ? PART_PLAYED : PART_NOT_PLAYED, current_part.view ? PART_VISIBLE : PART_NOT_VISIBLE,
				alias);
			f.AddLine(s);
		}
	}

	f.AddLine("");

	s.Printf("%s :", SET_ORNAMENTS);
	f.AddLine(s);

	singleOrnaments();
	for (auto & ornament : lOrnaments ) 
	{
		if (ornament.type != o_divisions)
		{
			wxString sr , st , srepeatNr ,  strack , sbefore , sname;
			int tInBeat , beat ;
			int division_beat, division_quarter, division_measure;
			division_beat = getDivision(ornament.measureNumber, &division_quarter, &division_measure);
			beat = ornament.t / division_beat;
			tInBeat = ornament.t % division_beat;
			if ((tInBeat != 0) || (ornament.chord_order > 0))
			{
				if (ornament.chord_order <= 0)
					st.Printf(".%d.%d", beat + 1, (int)(tInBeat / 12) ); // 1/12 : to display back  the original division
				else
					st.Printf(".%d.%d.%d", beat + 1, (int)(tInBeat / 12) , ornament.chord_order + 1);
			}
			else
				st.Printf(".%d", beat + 1);
			if (ornament.repeat != -1)
				srepeatNr.Printf("*%d", (ornament.repeat + 1));
			if (ornament.partNr != -1)
			{
				if (( ornament.staffNr != -1) && (ornament.staffNr != NULL_INT))
					strack.Printf("@P%d_%s.%d", ornament.partNr + 1, getTrackName(ornament.partNr), ornament.staffNr);
				else
					strack.Printf("@P%d_%s", ornament.partNr + 1, getTrackName(ornament.partNr));
			}
			if (ornament.before)
				sbefore = "<";
			if (ornament.value.empty() == false)
			{
				/*if (ornament.type == o_divisions)
				{
					// display the original division ( / 12 )
					long lv = 1 ;
					ornament.value.ToLong(&lv);
					sname.Printf("%s=%d", ornamentName[ornament.type], (int)(lv/12));
				}
				else*/
					sname.Printf("%s=%s", ornamentName[ornament.type], ornament.value);
			}
			else
				sname = ornamentName[ornament.type];
			if (ornament.absolute_measureNr)
				sr = "!";
			else
			{
				if ((ornament.mark_prefix >= 0) && (ornament.mark_prefix < (int)(lMeasureMarks.size())))
					sr.Printf("%s.", lMeasureMarks[ornament.mark_prefix].name);
			}
			s.Printf("%s%d%s%s%s%s:%s",sr, ornament.measureNumber, st, srepeatNr, sbefore, strack, sname);
			f.AddLine(s);
		}
	}


	f.Write();
	f.Close();
}
bool musicxmlcompile::readMarkLine(wxString s, wxString sectionName)
{
	bool ret_code = true;
	if (sectionName == SET_MARKS)
	{
		wxString s_name;
		wxString s_number;
		bool number_ok;
		long l_number = 0;
		if (s.Contains(":"))
		{
			s_number = s.BeforeFirst(':', &s_name).Trim(true).Trim(false);
			number_ok = s_number.ToLong(&l_number);
		}
		else
		{
			s_number = s.Trim(true).Trim(false);
			number_ok = s.ToLong(&l_number);
		}
		if (!number_ok)
			return false;
		c_measureMark measureMark(l_number);
		s_name.Trim(true).Trim(false);
		if (s_name.empty() == false)
			measureMark.name = s_name;
		lMeasureMarks.push_back(measureMark);
		return true;
	}
	if (sectionName == SET_PLAY_MARKS)
	{
		s.Trim(true).Trim(false);
		if (s.empty() == false)
		{
			int i = -1;
			for (auto & measureMark : lMeasureMarks)
			{
				i++;
				if (measureMark.name == s)
				{
					markList.push_back(i); //  lMeasureMarks.IndexOf(measureMark));
					return true;
				}
			}
			return false;
		}
	}
	if (sectionName == SET_PARTS)
	{
		// nr_name :[played]/[visible]
		wxString r;
		wxString id_name = s.BeforeLast(':',&r);
		int partNb = 0;
		int partNr = getPartNr(id_name , &partNb );
		if (partNr == wxNOT_FOUND)
			return false;
		c_score_part *current = & (compiled_score->part_list->score_parts[partNr]);
		current->view = ! ( r.Contains(PART_NOT_VISIBLE)); // view
		current->play = !(r.Contains(PART_NOT_PLAYED)); // play
		wxString alias = s.AfterFirst('=');
		if (!(alias.empty()))
		{
			wxString abbreviation;
			wxString part_alias = alias.BeforeFirst('/', &abbreviation);
			current->part_alias = part_alias;
			if (!(abbreviation.empty()))
				current->part_alias_abbreviation = abbreviation;
		}
		return true;
	}
	if (sectionName == SET_PLAYBACK)
	{
		if (s.StartsWith(SET_RATIO))
		{
			wxString r = s.AfterFirst('=');
			if ( r.empty())
				r = s.AfterFirst(':');
			if (!r.empty())
			{
				long rl;
				if (r.ToLong(&rl))
				{
					if ((rl > 20) && (rl < 400))
						ratioPlayback = rl;
				}
			}
		}
		else
		{
			// dt:device,type_msg,channel,value1,value2
			wxStringTokenizer tokenizer(s, ":,");
			if (tokenizer.CountTokens() == 6)
			{
				wxString token;
				wxLongLong_t dtl;
				long dt, nr_device, type_msg, channel, value1, value2;
				token = tokenizer.GetNextToken();
				token.ToLong(&dt);
				dt = (dt * 100 ) / ratioPlayback;
				dtl = dt;
				timeReadPlayback += dtl;
				token = tokenizer.GetNextToken();
				token.ToLong(&nr_device);
				token = tokenizer.GetNextToken();
				token.ToLong(&type_msg);
				token = tokenizer.GetNextToken();
				token.ToLong(&channel);
				token = tokenizer.GetNextToken();
				token.ToLong(&value1);
				token = tokenizer.GetNextToken();
				token.ToLong(&value2);
				lEventPlaybacks.push_back(c_eventPlayback(timeReadPlayback, nr_device, type_msg, channel, value1, value2));
			}
		}
		return true;
	}
	if (sectionName == SET_ORNAMENTS)
	{
		// [!|mark.]measureNr[.beat[.time[.orderChord]]][*repeat][@track[.staff]][<]:ornament_name[=value]
		wxString sornament_name_value, sornament_name, sornament_value;
		wxString position;
		position = s.BeforeFirst(':', &sornament_name_value);
		if (sornament_name_value.Contains("="))
			sornament_name = sornament_name_value.BeforeFirst('=', &sornament_value);
		else
			sornament_name = sornament_name_value;
		bool absolute = false;
		if (position.StartsWith("!"))
		{
			position = position.Mid(1);
			absolute = true;
		}
		int nr_marker = -1 ;
		if (position.Find('.') != wxNOT_FOUND)
		{
			wxString mmarker = position.BeforeFirst('.');
			int nr = -1;
			for (auto & measureMark : lMeasureMarks) 
			{
				nr++;
				if (measureMark.name.IsSameAs(mmarker))
				{
					nr_marker = nr ;
					mmarker = position.BeforeFirst('.',&position);
					break;
				}
			}
		}

		bool before = false;
		if (position.EndsWith("<"))
		{
			position = position.BeforeLast('<');
			before = true;
		}
		int trackNr = -1;
		int staffNr = -1;
		if (position.Contains("@"))
		{
			wxString strack;
			position = position.BeforeLast('@', &strack);
			if (strack.Contains("."))
			{
				wxString sstaff;
				strack = strack.BeforeLast('.', &sstaff);
				long l;
				if (sstaff.ToLong(&l))
					staffNr = l;
				else 
					return false;
			}
			int n = getPartNr(strack);
			if (n == wxNOT_FOUND)
				return false;
			trackNr = n;
		}
		int repeat = -1;
		if (position.Contains("*"))
		{
			wxString srepeat;
			position = position.BeforeLast('*', &srepeat);
			long l;
			if (srepeat.ToLong(&l))
				repeat = l - 1;
			else
				return false;
		}
		int beat = 0;
		int tInBeat = 0;
		int chord_order = -1;
		if (position.Contains("."))
		{
			wxString stime;
			position = position.BeforeFirst('.', &stime);
			if (stime.Contains("."))
			{
				wxString sBeat, stInBeat;
				sBeat = stime.BeforeFirst('.', &stInBeat);
				long l;
				if (sBeat.ToLong(&l))
					beat = l;
				if (stInBeat.Contains("."))
				{
					wxString sChordOrder, stInBeat2;
					stInBeat2 = stInBeat.BeforeFirst('.', &sChordOrder);
					if (stInBeat2.ToLong(&l))
						tInBeat = l;
					if (sChordOrder.ToLong(&l))
						chord_order = l - 1;
				}
				else
				{
					if (stInBeat.ToLong(&l))
						tInBeat = l;
				}
			}
			else
			{
				long l;
				if (stime.ToLong(&l))
					beat = l;
			}
		}
		int measure_nr = -1;
		long l;
		if (position.ToLong(&l))
			measure_nr = l;
		else
			return false;
		int nr_ornament = -1;
		sornament_name.MakeLower().Trim();
		sornament_value.Trim();
		for (int i = 0; i < o_flagend; i++)
		{
			if (ornamentName[i] == sornament_name)
			{
				nr_ornament = i;
				break;
			}
		}
		if ((nr_ornament == -1) || (measure_nr < 0))
			return false;
		if (nr_ornament != o_divisions)
		{
			c_ornament ornament(nr_ornament, measure_nr, NULL_INT, trackNr, staffNr, repeat, before, sornament_value);
			ornament.tInBeat = tInBeat * 12 ; // displayed time is the original one, not multiplied by 12
			ornament.beat = beat;
			ornament.chord_order = chord_order;
			ornament.absolute_measureNr = absolute;
			ornament.mark_prefix = nr_marker;
			lOrnaments.push_back(ornament);
		}
	}
	return ret_code;
}
int musicxmlcompile::getDivision(int measure_nr, int *division_quarter, int *division_measure)
{
	c_part *part = & (compiled_score->parts[0] );
	int lnb_measure = part->measures.size();
	if ((measure_nr < 1) || (measure_nr > lnb_measure))
	{
		*division_quarter = 1;
		*division_measure = 4;
		return false;
	}
	c_measure *measure = & ( part->measures[measure_nr-1] );
	*division_quarter = measure->division_quarter;
	*division_measure = measure->division_measure;
	return measure->division_beat;
}
int musicxmlcompile::getPartNr(wxString ispart , int *partNb)
{
	// spart = id_name , return partnr
	int nbPart = score->part_list->score_parts.size();
	if (partNb)
		*partNb = nbPart;
	wxString spart = ispart.BeforeFirst(':');
	wxString s1 , r;
	if (spart.StartsWith("P", &r) || spart.StartsWith("p", &r))
	{
		long l;
		bool ret = r.ToLong(&l);
		if (ret && (l > 0) && (l <= nbPart))
			return (l - 1);
		int partNr = -1;
		for (auto & current_part : score->part_list->score_parts )
		{
			partNr++;
			s1.Printf("P%d_%s", partNr + 1, current_part.part_name);
			if ((spart == s1) || (current_part.part_alias == spart ))
				return(partNr);
		}
	}
	return(wxNOT_FOUND);
}
void musicxmlcompile::readMarks(bool full)
{
	// read optional parameters, from a text file <inputfilename> with txt extension :
	// SET_MARKS
	// measure_number=label ..
	// PLAY_MARKS
	// markList ...
	// SET_ORNAMENTS
	// measureNr[.time[*repeat[/track]]][<]=ornament_name
	// PLAYBACK
	// time,type_msg,channel,value1,value2

	wxTextFile f;
	f.Open(txtFile.GetFullPath());
	if (f.IsOpened() == false)
		return;
	wxString line;
	wxString sectionName;
	musicxmlFile.Clear();
	bool err = false;
	int line_nb = f.GetLineCount();
	for (int line_nr = 0; line_nr < line_nb; line_nr++)
	{
		line = f.GetLine(line_nr);
		int posComment = line.Find(COMMENT_EXPRESSEUR);
		if (posComment != wxNOT_FOUND)
			line.Truncate(posComment);
		wxString s = line.Upper().Trim();
		// ret_code = true;
		if (s.empty() == false )
		{
			if (s.StartsWith(SET_MUSICXML_FILE))
			{
				sectionName = "";
				wxString file = line.AfterFirst(':').Trim(true).Trim(false);
				musicxmlFile =  txtFile;
				musicxmlFile.SetFullName(file);
			}
			else if (s.StartsWith(SET_TITLE))
			{
				sectionName = "";
				if (full)
				{
					wxString title = line.AfterFirst(':').Trim(true).Trim(false);
					if (compiled_score->work == NULL)
						compiled_score->work = new c_work();
					compiled_score->work->work_title = title;
				}
			}
			else if ( s.StartsWith(SET_PLAY_MARKS))
			{
				sectionName = "";
				if (full)
				{
					sectionName = SET_PLAY_MARKS;
					sortMeasureMarks();
					markList.clear();
				}
			}
			else if (s.StartsWith(SET_MARKS))
			{
				sectionName = "";
				if (full)
				{
					sectionName = SET_MARKS;
					lMeasureMarks.clear();
					lMeasureMarks.clear();
				}
			}
			else if (s.StartsWith(SET_PARTS))
			{
				sectionName = "";
				if ( full )
					sectionName = SET_PARTS;
			}
			else if (s.StartsWith(SET_ORNAMENTS))
			{
				sectionName = "";
				if (full)
				{
					sectionName = SET_ORNAMENTS;
					clearOrnaments();
				}
			}
			else if (s.StartsWith(SET_PLAYBACK))
			{
				sectionName = "";
				if (full)
				{
					sectionName = SET_PLAYBACK;
					timeReadPlayback = 0;
					ratioPlayback = 100;
					lEventPlaybacks.clear();
					lEventPlaybacks.clear();
				}
			}
			else if (full)
			{
				if (!readMarkLine(line, sectionName))
				{
					f.RemoveLine(line_nr);
					f.InsertLine("-- error : " + line, line_nr);
					err = true;
				}
			}
		}
	}
	if ( full )
		sortMeasureMarks();
	if ( err )
		f.Write();
	f.Close();
}
int musicxmlcompile::compileNote(c_part *part, c_note *note, int measureNr, int originalMeasureNr, int t, int division_measure, int division_beat, int division_quarter, int repeat, int key_fifths)
{

	// compile a note in the lMusicxmlevents

	// calcul du position dans le temps de la note
	if ((note->grace)  || (note->rest) || (note->cue) || ((note->tie) && ((note->tie->stop) || (note->tie->compiled))))
	{
		if (! note->chord)
			t += (note->duration == NULL_INT) ? 0 : note->duration;
		return t;
	}

	if (note->chord)
		t -= (note->duration == NULL_INT) ? 0 : note->duration;

	int pitch = 64;
	if (note->pitch)
	{
		pitch = note->pitch->toMidiPitch();
	}

	int stop_measureNr, stop_t;
	stop_measureNr = measureNr; 
	stop_t = t;
	compileTie(part, note, &stop_measureNr, &stop_t, division_measure);

	c_musicxmlevent mmusicxmlevent (part->idNr,note->staff, note->voice, measureNr, originalMeasureNr, t, stop_measureNr, stop_t, pitch, division_measure, division_beat, division_quarter, repeat, 0, key_fifths);
	mmusicxmlevent.chord_order = note->chord_order;
	if (note->notehead.IsSameAs("x",false))
	{
		mmusicxmlevent.velocity = 1;
		mmusicxmlevent.cross = true;
	}

	lMusicxmlevents.push_back(mmusicxmlevent);

	t += (note->duration == NULL_INT) ? 0 : note->duration;

	return t;
}
void musicxmlcompile::compileTie(c_part *part, c_note *note, int *measureNr , int *t , int division_measure )
{
	// compile the note->tie in the part, to calculate the realDuration
	// it updates measureNr and t with the end of the note, according to note->tie, which ties additional next notes

	int current_division_measure = division_measure;
	*t += note->duration;
	if (*t >= current_division_measure)
	{
		*t = 0;
		(*measureNr)++;
	}
	if ((note->tie == NULL) || (note->tie->start == false))
		return;
	// scan a note which can be tied to current note
	for (auto & current_measure : part->measures )
	{
		int current_t = 0;
		if (current_measure.number > (*measureNr))
			return ;
		if (current_measure.number == (*measureNr))
		{
			current_division_measure = current_measure.division_measure;
			for (auto & current_measure_sequence :  current_measure.measure_sequences )
			{
				switch (current_measure_sequence.type)
				{
				case t_note:
				{
					c_note *current_note = (c_note *)(current_measure_sequence.pt);
					// process the note 
					if (current_note->chord)
						current_t -= current_note->duration;
					if (note->pitch && current_note->pitch && (note->pitch->isEqual(*(current_note->pitch))) && (note->voice == current_note->voice))
					{
						if (((*measureNr) == current_measure.number) && ((*t) == current_t))
						{
							if ((current_note->tie) && (current_note->tie->stop))
							{
								current_note->tie->compiled = true;
								compileTie(part, current_note, measureNr, t, current_division_measure);
								return;
							}
						}
					}
					current_t += current_note->duration;
				}
					break;
				case t_backup:
				{
					c_backup *current_backup = (c_backup *)(current_measure_sequence.pt);
					current_t -= current_backup->duration;
				}
					break;
				case t_forward:
				{
					c_forward *current_forward = (c_forward *)(current_measure_sequence.pt);
					current_t += current_forward->duration;
				}
					break;
				default:
					break;
				}
			}
		}
	}
	return ;
}
void musicxmlcompile::compilePedalBar()
{
	if (lPedal_bar_toapply.size() == 0)
		return;
	std::sort(lPedal_bar_toapply.begin(), lPedal_bar_toapply.end() , pedal_barCompare);
	auto pedal_bar_toapply = lPedal_bar_toapply.begin();
	int start_measureNr = pedal_bar_toapply->measureNr;
	int value_pedal = pedal_bar_toapply->value;
	pedal_bar_toapply++;
	std::vector <c_musicxmlevent>  pedalPending;
	int pMeasureNr = -1;
	while(true)
	{
		int stop_measureNr = 99999;
		if (pedal_bar_toapply != lPedal_bar_toapply.end())
		{
			stop_measureNr = pedal_bar_toapply->measureNr;
		}
		pMeasureNr = -1;
		// add a pedal on each bar
		for (auto & musicxmlevent : lMusicxmlevents ) 
		{
			if (musicxmlevent.start_measureNr >= stop_measureNr)
				break;
			if (musicxmlevent.start_measureNr >= start_measureNr)
			{
				if (musicxmlevent.start_measureNr != pMeasureNr)
				{
					pMeasureNr = musicxmlevent.start_measureNr;
					c_musicxmlevent m(musicxmlevent);
					m.played = false;
					m.visible = false;
					m.start_order = -128;
					m.pedal = value_pedal;
					pedalPending.push_back(m);
					if (value_pedal == 0)
						break;
				}
			}
		}

		if (pedal_bar_toapply == lPedal_bar_toapply.end())
			break;
		start_measureNr = pedal_bar_toapply->measureNr;
		value_pedal = pedal_bar_toapply->value;
		pedal_bar_toapply++;
	} 
	for (auto & musicxmlevent : pedalPending )
	{
		lMusicxmlevents.push_back(musicxmlevent);
	}
	pedalPending.clear();
}
void musicxmlcompile::compileCrescendo()
{
	int startMusicxmlevent[MAX_SCORE_PART];
	int pVelocity[MAX_SCORE_PART];
	for (int i = 0; i < MAX_SCORE_PART; i++)
		pVelocity[i] = 64;

	// fix all velocities, not yet defined by an ornament-velocity
	for (auto & musicxmlevent : lMusicxmlevents ) 
	{
		if (musicxmlevent.nuance != NULL_INT)
			pVelocity[musicxmlevent.partNr] = musicxmlevent.nuance;
		musicxmlevent.velocity = pVelocity[musicxmlevent.partNr];
		musicxmlevent.velocity += musicxmlevent.accent;
		if (musicxmlevent.velocity > 127)
			musicxmlevent.velocity = 127;
		if (musicxmlevent.velocity < 1)
			musicxmlevent.velocity = 1;
	}

	bool crescendo_pending[MAX_SCORE_PART];
	for (int i = 0; i < MAX_SCORE_PART; i++)
	{
		crescendo_pending[i] = false;
		pVelocity[i] = 64;
	}
	int nrEvent = -1;
	// fix crescendos
	for (auto & musicxmlevent : lMusicxmlevents)
	{
		nrEvent++;
		if ((crescendo_pending[musicxmlevent.partNr] == false) && (musicxmlevent.nuance != NULL_INT))
			pVelocity[musicxmlevent.partNr] = musicxmlevent.nuance;
		if (musicxmlevent.crescendo)
		{
			// marker on this crescendo, on this part
			startMusicxmlevent[musicxmlevent.partNr] = nrEvent;
			crescendo_pending[musicxmlevent.partNr] = true;
		}
		else
		{
			if ((crescendo_pending[musicxmlevent.partNr]) && (musicxmlevent.nuance != NULL_INT))
			{
				// crescendo pending : applied up this nuance
				crescendo_pending[musicxmlevent.partNr] = false;
				int cPartNr = musicxmlevent.partNr;
				c_musicxmlevent musicxmleventStart = lMusicxmlevents[startMusicxmlevent[cPartNr]];
				c_musicxmlevent musicxmleventStop = musicxmlevent;
				int dt = (musicxmleventStop.start_measureNr - musicxmleventStart.start_measureNr)*(musicxmleventStart.division_measure) + (musicxmleventStop.start_t - musicxmleventStart.start_t);
				int start_velocity = pVelocity[musicxmlevent.partNr];
				int stop_velocity = musicxmlevent.nuance;
				for (int i = startMusicxmlevent[cPartNr]; i < nrEvent; i++)
				{
					c_musicxmlevent *musicxmlevent2 = & (lMusicxmlevents[i]) ;
					if (musicxmlevent2->partNr == cPartNr)
					{
						int dtl = (musicxmlevent2->start_measureNr - musicxmleventStart.start_measureNr)*(musicxmleventStart.division_measure) + (musicxmlevent2->start_t - musicxmleventStart.start_t);
						musicxmlevent2->velocity = start_velocity + ((stop_velocity - start_velocity) * dtl) / dt;
					}
				}
			}
		}
	}
}
void musicxmlcompile::compileTransposition()
{
	// fix transposition
	int pTransposition[MAX_SCORE_PART];
	for (int i = 0; i < MAX_SCORE_PART; i++)
		pTransposition[i] = 0;
	for (auto & musicxmlevent : lMusicxmlevents )
	{
		if (musicxmlevent.transpose != NULL_INT)
			pTransposition[musicxmlevent.partNr] = musicxmlevent.transpose;
		musicxmlevent.pitch += pTransposition[musicxmlevent.partNr];
	}
}
void musicxmlcompile::compileArppegio()
{
	// proccess pending arpegiatte
	if (lArpeggiate_toapply.size() == 0)
		return;
	std::sort(lArpeggiate_toapply.begin() , lArpeggiate_toapply.end() , arpeggiateCompare);
	int dt = 0;
	int pnr = -1;
	for (auto & arpeggiate_toapply : lArpeggiate_toapply ) // .begin(), iter_arpeggiate_toapply++; iter_arpeggiate_toapply != lArpeggiate_toapply.end(); ++iter_arpeggiate_toapply)
	{
		if (arpeggiate_toapply.nr == pnr)
		{
			dt += (arpeggiate_toapply.before) ? -1 : 1;
			arpeggiate_toapply.musicxmlevent->start_order = dt;
			arpeggiate_toapply.musicxmlevent->visible = false;
		}
		else
		{
			pnr = arpeggiate_toapply.nr;
			dt = 0;
		}
	}

}
wxString musicxmlcompile::pitchToString(int pitch)
{
	const  char *cpitch[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "Bb", "B" };
	int p = pitch % 12;
	int o = pitch / 12 - 1;
	wxString s;
	s.Printf("%s%d", cpitch[p], o);
	return s;
}
wxString musicxmlcompile::pitchToString(std::vector <int> pitches)
{
	wxString s;
	bool first = true;
	for (auto & p : pitches )
	{
		wxString sp = pitchToString(p);
		if ( ! first )
			s.Append(",");
		s.Append(sp);
	}
	return s;
}
std::vector <int> musicxmlcompile::stringToPitch(wxString s, int *nbChord)
{
	std::vector <int> ap;

	// return array of pitches ( a chord ), extracted from string s in first call, chord by chord
	// e.g. stringToPitch("C4/E4,G4") returns first 64,68 , and nbChord=2
	//      then stringToPitch("") returns 71 
	//      then stringToPitch("") return empty arrary
	static std::vector <int> t;
	static unsigned int pt;
	if (s.empty() == false)
	{
		*nbChord = 0;
		t.clear();
		pt = 0;
		wxStringTokenizer tokenizer(s, "+,", wxTOKEN_STRTOK);
		while (tokenizer.HasMoreTokens())
		{
			wxChar sep = tokenizer.GetLastDelimiter();
			wxString token = tokenizer.GetNextToken();
			char cp = token.GetChar(0);
			int p = 0;
			switch (cp)
			{
			case 'C': p = 0; break;
			case 'D': p = 2; break;
			case 'E': p = 4; break;
			case 'F': p = 5; break;
			case 'G': p = 7; break;
			case 'A': p = 9; break;
			case 'B': p = 11; break;
			default: break;
			}
			if (token.Contains("bb"))
				p -= 2;
			else if (token.Contains("b"))
				p--;
			else if (token.Contains("##"))
				p += 2;
			else if (token.Contains("#"))
				p++;
			cp = token.Last();
			switch (cp)
			{
			case '0': p += 1 * 12; break;
			case '1': p += 2 * 12; break;
			case '2': p += 3 * 12; break;
			case '3': p += 4 * 12; break;
			case '4': p += 5 * 12; break;
			case '5': p += 6 * 12; break;
			case '6': p += 7 * 12; break;
			case '7': p += 8 * 12; break;
			case '8': p += 9 * 12; break;
			case '9': p += 10* 12; break;
			default: break;
			}
			while (p < 0)
				p += 12;
			while (p > 127)
				p -= 12;
			if (sep == '+')
				p *= (-1);
			else
				(*nbChord)++;
			t.push_back(p);
		}
	}
	if (pt >= t.size())
		return ap;
	ap.push_back(abs(t[pt]));
	pt++;
	while (true)
	{
		if ((pt >= t.size()) || (t[pt] > 0))
			return ap;
		ap.push_back(abs(t[pt]));
		pt++;
	}
	return ap;
}
void musicxmlcompile::addGraces(std::vector <int> gracePitches, bool before, c_musicxmlevent *musicxmlevent)
{
	// add grace notes, using a array of grace integer pitches. 
	int nbChord = gracePitches.size() ;
	c_musicxmlevent mtemplate(*musicxmlevent);
	musicxmlevent->visible = true;
	mtemplate.visible = false;
	mtemplate.stop_measureNr = mtemplate.start_measureNr;
	mtemplate.stop_t = mtemplate.start_t;
	int start_order = musicxmlevent->start_order;
	if (before)
		musicxmlevent->start_order = start_order + 0;
	else
		musicxmlevent->start_order = start_order + 2 * nbChord;
	for (int nrChord = 0; nrChord < nbChord; nrChord++)
	{
		if (before)
		{
			mtemplate.start_order = start_order - 2 * (nbChord - nrChord);
			mtemplate.stop_order = start_order - 2 * (nbChord - nrChord) + 1;
		}
		else
		{
			mtemplate.start_order = start_order + 2 * nrChord;
			mtemplate.stop_order = start_order + 2 * nrChord + 1;
		}
		c_musicxmlevent m(mtemplate);
		m.pitch = gracePitches[nrChord];
		lOrnamentsMusicxmlevents.push_back(m);
	}
}
void musicxmlcompile::addGraces(wxString gracePitches, bool before, c_musicxmlevent *musicxmlevent)
{

	// add grace notes, using a text of grace notes. e.g. "C4/E4,G4" adds chord C4/E4 and G4
	std::vector <int> pitchChord;
	int nbChord ;
	pitchChord = stringToPitch(gracePitches, &nbChord);
	int nrChord = 0;
	c_musicxmlevent mtemplate(*musicxmlevent);
	musicxmlevent->visible = true;
	mtemplate.visible = false;
	mtemplate.stop_measureNr = mtemplate.start_measureNr;
	mtemplate.stop_t = mtemplate.start_t;
	int start_order = musicxmlevent->start_order;
	if (before)
		musicxmlevent->start_order = start_order + 0;
	else
		musicxmlevent->start_order = start_order + 2 * nbChord;
	while (pitchChord.size() == false)
	{
		if (before)
		{
			mtemplate.start_order = start_order - 2 * (nbChord - nrChord);
			mtemplate.stop_order = start_order - 2 * (nbChord - nrChord) + 1;
		}
		else
		{
			mtemplate.start_order = start_order + 2 * nrChord;
			mtemplate.stop_order = start_order + 2 * nrChord + 1;
		}
		int nbPitch = pitchChord.size();
		for (int nrPitch = 0; nrPitch < nbPitch; nrPitch++)
		{
			c_musicxmlevent m(mtemplate);
			m.pitch = pitchChord[nrPitch];
			lOrnamentsMusicxmlevents.push_back(m);
		}
		nrChord++;
		pitchChord = stringToPitch("", NULL);
	}
}
void musicxmlcompile::addOrnament(c_ornament *ornament, c_musicxmlevent *musicxmlevent, int nr_ornament)
{

	// add the ornament in the musicxmlevent
	bool btrill = false;
	switch (ornament->type)
	{
	case o_delay:
	{
		long l;
		int v = 10;
		if (ornament->value.ToLong(&l))
			v = l;
		musicxmlevent->delay = v;
		break;
	}
	case o_transpose:
	{
		long l;
		int v = 10;
		if (ornament->value.ToLong(&l))
			v = l;
		musicxmlevent->transpose = v;
		break;
	}
	case o_pianissimo:
		musicxmlevent->nuance = 10;
		break;
	case o_piano:
		musicxmlevent->nuance = 30;
		break;
	case o_mesopiano:
		musicxmlevent->nuance = 50;
		break;
	case o_mesoforte:
		musicxmlevent->nuance = 70;
		break;
	case o_forte:
		musicxmlevent->nuance = 90;
		break;
	case o_fortissimo:
		musicxmlevent->nuance = 100;
		break;
	case o_crescendo:
		musicxmlevent->crescendo = true;
		break;
	case o_diminuendo:
		musicxmlevent->crescendo = true;
		break;
	case o_tenuto:
		musicxmlevent->tenuto = true;
		break;
	case o_breath_mark:
	{
		c_musicxmlevent m(*musicxmlevent);
		m.played = false;
		m.visible = false;
		m.start_order = -128;
		m.breath_mark = true;
		lOrnamentsMusicxmlevents.push_back(m);
		break;
	}
	case o_fermata:
	{
		c_musicxmlevent m(*musicxmlevent);
		m.played = false;
		m.visible = false;
		m.start_order = -128;
		m.fermata = true;
		lOrnamentsMusicxmlevents.push_back(m);
		break;
	}
	case o_staccato:
	{
		bool display_only = false;
		int d = musicxmlevent->duration;
		if (ornament->value == "2/3")
			d = (d * 2) / 3;
		else if (ornament->value == "1/3")
			d = d / 3;
		else if (ornament->value == "3/4")
			d = (d * 3) / 4;
		else if (ornament->value == "1/2")
			d = d / 2;
		else
			display_only = true;
		if (display_only)
		{
			c_musicxmlevent m(*musicxmlevent);
			m.played = false;
			m.visible = false;
			m.start_order = -128;
			m.staccato = true;
			lOrnamentsMusicxmlevents.push_back(m);
		}
		else
		{
			musicxmlevent->duration = d;
			musicxmlevent->stop_t = musicxmlevent->start_t + d;
		}
		break;
	}
	case o_accent:
	{
		long l;
		int v = 10;
		if (ornament->value.ToLong(&l))
			v = l;
		musicxmlevent->accent = v;
		break;
	}
	case o_grace:
	{
		std::vector <int> gracePitches;
		if (ornament->value.empty())
		{
			// just one grace note one diatonic tone upper
			gracePitches.push_back(c_pitch::shiftPitch(musicxmlevent->pitch, 1, musicxmlevent->fifths));
			addGraces(gracePitches, ornament->before, musicxmlevent);
		}
		else if (ornament->value.StartsWith("i"))
		{
			// "i"nverted : just one grace note one diatonic tone lower
			gracePitches.push_back(c_pitch::shiftPitch(musicxmlevent->pitch, 1, musicxmlevent->fifths));
			addGraces(gracePitches, ornament->before, musicxmlevent);
		}
		else
			// a text of grace notes. e.g. "C4/E4,G4" adds chord C4/E4 and G4
			addGraces(ornament->value, ornament->before, musicxmlevent);
		break;
	}
	case o_mordent:
	{
	    std::vector <int> gracePitches;
		gracePitches.push_back(musicxmlevent->pitch);
		gracePitches.push_back(c_pitch::shiftPitch(musicxmlevent->pitch, (ornament->value.StartsWith('i'/*inverted*/)) ? -1 : 1, musicxmlevent->fifths));
		addGraces(gracePitches, ornament->before, musicxmlevent); 
		break;
	}
	case o_turn:
	{
		std::vector <int> gracePitches;
		gracePitches.push_back(c_pitch::shiftPitch(musicxmlevent->pitch, (ornament->value.StartsWith('i'/*inverted*/)) ? -1 : 1, musicxmlevent->fifths));
		gracePitches.push_back(musicxmlevent->pitch);
		gracePitches.push_back(c_pitch::shiftPitch(musicxmlevent->pitch, (ornament->value.StartsWith('i'/*inverted*/)) ? 1 : -1, musicxmlevent->fifths));
		addGraces(gracePitches, ornament->before, musicxmlevent);
		break;
	}
	case o_btrill:
		btrill = true;
	case o_trill:
	{
		std::vector <int> gracePitches;
		int p0, p1;
		if (btrill)
		{
			p0 = c_pitch::shiftPitch(musicxmlevent->pitch, 1, musicxmlevent->fifths);
			p1 = musicxmlevent->pitch;
		}
		else
		{
			p0 = musicxmlevent->pitch;
			p1 = c_pitch::shiftPitch(musicxmlevent->pitch, 1, musicxmlevent->fifths);
		}
		long l;
		int v = 4;
		if (ornament->value.ToLong(&l))
			v = l;
		v /= 2;
		for (int i = 0; i < v; i++)
		{
			gracePitches.push_back(p0);
			if (i != (v - 1))
				gracePitches.push_back(p1);
		}
		addGraces(gracePitches, ornament->before, musicxmlevent);
		musicxmlevent->pitch = p1;
		break;
	}
	case o_arpeggiate:
		lArpeggiate_toapply.push_back(c_arpeggiate_toapply(100 * musicxmlevent->repeat + nr_ornament, (ornament->value == "down"), ornament->before, musicxmlevent));
		break;
	case o_dynamic:
	{
		c_musicxmlevent m(*musicxmlevent);
		m.played = false;
		m.visible = false;
		m.start_order = -128;
		long l;
		int v = 100;
		if (ornament->value.ToLong(&l))
			v = l;
		m.dynamic = v;
		lOrnamentsMusicxmlevents.push_back(m);
		break;
	}
	case o_random_delay:
	{
		c_musicxmlevent m(*musicxmlevent);
		m.played = false;
		m.visible = false;
		m.start_order = -128;
		long l;
		int v = 0;
		if (ornament->value.ToLong(&l))
			v = l;
		m.random_delay = v;
		lOrnamentsMusicxmlevents.push_back(m);
		break;
	}
	case o_text :
	{
		c_musicxmlevent m(*musicxmlevent);
		m.played = false;
		m.visible = false;
		m.start_order = -128;
		m.text = ornament->value;
		lOrnamentsMusicxmlevents.push_back(m);
		break;
	}
	case o_pedal:
	{
		c_musicxmlevent m(*musicxmlevent);
		m.played = false;
		m.visible = false;
		m.start_order = -128;
		long l;
		int v = 65;
		if (ornament->value.ToLong(&l))
			v = l;
		m.pedal = v;
		lOrnamentsMusicxmlevents.push_back(m);
		break;
	}
	case o_lua:
	{
		c_musicxmlevent m(*musicxmlevent);
		m.played = false;
		m.visible = false;
		m.start_order = -128;
		m.lua = ornament->value;
		lOrnamentsMusicxmlevents.push_back(m);
		break;
	}
	case o_after:
		(musicxmlevent->start_order) += 2;
		break;
	case o_before:
		(musicxmlevent->start_order) -= 2;
		break;
	case o_pedal_bar:
	{
		long l;
		int v = 65;
		if (ornament->value.ToLong(&l))
			v = l;
		lPedal_bar_toapply.push_back(c_pedal_bar_toapply(v, musicxmlevent));
		break;
	}
	default: break;
	}

}
void musicxmlcompile::createImperativePartOrnament(c_ornament *ornament,int nrPart, int nrStaff, int end_measure,int end_start_t, int division_measure, int division_beat , int division_quarter )
{

	c_musicxmlevent m(nrPart, nrStaff, 0,
		ornament->measureNumber, ornament->measureNumber, ornament->t, 
		end_measure,end_start_t , 0,
		division_measure, division_beat, division_quarter,
		0, -128, 0);
	m.played = false;
	m.visible = false;
	addOrnament(ornament, &m,0);
	lOrnamentsMusicxmlevents.push_back(m);
}
void musicxmlcompile::createImperativeOrnament(c_ornament *ornament)
{
	// look for next start, to finish a new virtual note for the ornament
	c_musicxmlevent* after_musicxmlevent = NULL;
	for (auto & musicxmlevent : lMusicxmlevents ) 
	{
		if ((musicxmlevent.start_measureNr > ornament->measureNumber) || ((musicxmlevent.start_measureNr == ornament->measureNumber) && (musicxmlevent.start_t > ornament->t)))
		{
			after_musicxmlevent = &musicxmlevent;
			break;
		}
	}
	if (!after_musicxmlevent)
		return;
	if (ornament->t >= after_musicxmlevent->division_measure)
		return;
	if (ornament->partNr >= 0 )
	{
		if ( ornament->staffNr >= 0 )
		{
			createImperativePartOrnament(ornament,ornament->partNr,ornament->staffNr, after_musicxmlevent->start_measureNr,after_musicxmlevent->start_t,
					after_musicxmlevent->division_measure, after_musicxmlevent->division_beat,after_musicxmlevent->division_quarter);
			return ;
		}
		for(int s = 0 ; s < 2 ; s++ )
		{
			ornament->staffNr = s ;
			createImperativePartOrnament(ornament,ornament->partNr,s , after_musicxmlevent->start_measureNr,after_musicxmlevent->start_t,
					after_musicxmlevent->division_measure, after_musicxmlevent->division_beat,after_musicxmlevent->division_quarter);
		}
		ornament->staffNr = -1 ;
		return ;
	}
	int nbPart = score->parts.size() - 1;
	for(int p = 0 ; p < nbPart ; p++)
	{
		ornament->partNr = p ;
		for(int s = 0 ; s < 2 ; s ++ )
		{
			ornament->staffNr = s ;
			createImperativePartOrnament(ornament,p,s , after_musicxmlevent->start_measureNr,after_musicxmlevent->start_t,
					after_musicxmlevent->division_measure, after_musicxmlevent->division_beat,after_musicxmlevent->division_quarter);
		}
	}
	ornament->partNr = -1 ;
	ornament->staffNr = -1 ;
}
void musicxmlcompile::addOrnaments()
{
	//  add lOrnaments in the lMusicxmlevents
	std::sort(lMusicxmlevents.begin(), lMusicxmlevents.end() , musicXmlEventsCompareStart);
	std::sort(lOrnaments.begin(), lOrnaments.end() , ornamentCompare);
	lOrnamentsMusicxmlevents.clear();

	// fill time information for ornaments which are not yet completed
	for (auto & ornament : lOrnaments )
	{
		if (ornament.t == NULL_INT)
		{
			int division_beat, division_quarter, division_measure;
			division_beat  = getDivision(ornament.measureNumber, &division_quarter, &division_measure);
			ornament.t = ornament.tInBeat + (ornament.beat - 1) * division_beat;
		}
	}

	// create musicxmlevents for imperative ornaments
	for (auto & ornament : lOrnaments)
	{
		switch (ornament.type)
		{
			case	o_dynamic :
			case	o_random_delay :
			case	o_pedal_bar :
			case	o_pedal :
			case	o_pianissimo :
			case	o_piano :
			case	o_mesopiano :
			case	o_mesoforte :
			case	o_forte :
			case	o_fortissimo :
			case	o_crescendo :
			case	o_diminuendo :
			case	o_transpose :
					ornament.processed = true ;
					createImperativeOrnament(&ornament);
					break;
			default: break;
		}
	}

	// add ornaments in the muiscXmlEvents, at the right timing
	for (auto & musicxmlevent : lMusicxmlevents )
	{
		int nr_ornament = -1;
		for (auto & ornament : lOrnaments ) 
		{
			nr_ornament++;
			if ( ! (ornament.processed) )
			{
				bool measureOk = false;
				if (ornament.absolute_measureNr)
				{
					// an absolute measure-nr ( starting with ! ) 
					measureOk = (musicxmlevent.start_measureNr == ornament.measureNumber);
				}
				else
				{
					if (ornament.mark_prefix ==  -1 )
					{
						// a relative measure-nr ( a number without ! prefix )
						measureOk = (musicxmlevent.original_measureNr == ornament.measureNumber);
					}
					else
					{
						// a mark free-text, which refer to a meauser-nr
						if ((ornament.mark_prefix >= 0) && (ornament.mark_prefix < (int)(lMeasureMarks.size())))
							measureOk = (musicxmlevent.original_measureNr == ornament.measureNumber + lMeasureMarks[ornament.mark_prefix].number - 1);
					}
				}
				if
				(		measureOk
						 && (musicxmlevent.start_t == ornament.t)
						 && ((ornament.chord_order < 0) || (musicxmlevent.chord_order == ornament.chord_order))
						 && ((ornament.partNr < 0) || (musicxmlevent.partNr == ornament.partNr))
						 && ((ornament.staffNr < 0) || (musicxmlevent.staffNr == NULL_INT) || (musicxmlevent.staffNr == ornament.staffNr))
						 && ((ornament.repeat < 0) || (musicxmlevent.repeat == ornament.repeat))
					)
				{
					// right measure, repeatition, time, staff, part, chord-order : calculate the ornament in the ornaments_musicXmlEvents
					addOrnament(&ornament, &musicxmlevent,nr_ornament);
				}
			}
		}
	}

	/* ? don't know what it does ...
	for (iter_ornament = lOrnaments.begin(); iter_ornament != lOrnaments.end(); ++iter_ornament)
	{
		c_ornament *ornament = *iter_ornament;
		if	(ornament->processed == false)
		{
			switch (ornament->type)
			{
			case o_dynamic:
			case o_random_delay:
			case o_pedal_bar :
			case o_pedal :
			case o_lua:
			case o_transpose:
			case o_text:
					createOrnament(ornament);
					break;
			default: break;
			}
		}
	}
	*/

	// finnaly, add the calculated ornament_musicXmlEvents in the global list lMusicxmlevents
	for (auto & ornament_musicxmlevent : lOrnamentsMusicxmlevents)
	{
		lMusicxmlevents.push_back(ornament_musicxmlevent);
	}
	lOrnamentsMusicxmlevents.clear();

	std::sort(lMusicxmlevents.begin() , lMusicxmlevents.end() , musicXmlEventsCompareStart);

	// finalize the calculation of the ornaments in the ornamenet_musicXmlEvents
	compileCrescendo();
	compileTransposition();
	compileArppegio();
	compilePedalBar();

}
void musicxmlcompile::compileMusicxmlevents()
{
	// lMusicxmlevents contains the notes to play. Compile lMusicxmlevents

	// set musicxmlevent->nr in the order
	fillStartStopNext();

	// links the starts and stops in the list of Musicxmlevents to play

	int p_MeasureNr = -1;
	int p_t = -1;
	int p_order = -1;
	bool p_tenuto = false;
	int previous_i = -1;


	// link synchronous-stops : fill musicxmlevent->stops[] (list of synchronous OFF events )
	for (int nrStopEvent = 0; nrStopEvent <  nbEvents; nrStopEvent++)
	{
		// c_musicxmlevent* current_musicxmlevent = &(lMusicxmlevents[indexStop[nrStopEvent]]);
		int current_i = indexStop[nrStopEvent];
		if (!lMusicxmlevents[current_i].played) continue ;
		if ((lMusicxmlevents[current_i].stop_measureNr != p_MeasureNr)
			||(lMusicxmlevents[current_i].stop_t != p_t)
			|| (lMusicxmlevents[current_i].stop_order != p_order)
			|| (lMusicxmlevents[current_i].tenuto != p_tenuto))
			previous_i = current_i;
		if (previous_i >= 0)
		{
			lMusicxmlevents[previous_i].stops.push_back(lMusicxmlevents[current_i].nr);
			if (previous_i != current_i)
				lMusicxmlevents[current_i].stop_orpheline = false;
		}
		p_MeasureNr = lMusicxmlevents[current_i].stop_measureNr;
		p_t = lMusicxmlevents[current_i].stop_t;
		p_order = lMusicxmlevents[current_i].stop_order;
		p_tenuto = lMusicxmlevents[current_i].tenuto;
	}

	// link synchronous-starts : fill musicxmlevent->starts[] (list of synchronous ON events)
	p_MeasureNr = -1;
	p_t = -1;
	p_order = -1;
	previous_i = -1 ;
	for (int current_i = 0; current_i < nbEvents; current_i++)
	{
		if (! lMusicxmlevents[current_i].played) continue ;
		if ((lMusicxmlevents[current_i].start_measureNr != p_MeasureNr) 
			|| (lMusicxmlevents[current_i].start_t != p_t) 
			|| (lMusicxmlevents[current_i].start_order != p_order))
			previous_i = current_i;
		if (previous_i>= 0)
			lMusicxmlevents[previous_i].starts.push_back(lMusicxmlevents[current_i].nr);

		p_MeasureNr = lMusicxmlevents[current_i].start_measureNr;
		p_t = lMusicxmlevents[current_i].start_t;
		p_order = lMusicxmlevents[current_i].start_order;
	}

	// links starts with stops-non-tenuto : fill musicxmlevent->will_stop_index ( event which will be stopped on trigger-OFF )
	p_MeasureNr = -1;
	p_t = -1;
	p_order = -1;
	p_tenuto = false;
	previous_i = -1;
	dump_musicxmlevents();
	for (int current_i = 0; current_i < nbEvents ; current_i++)
	{
		if (! lMusicxmlevents[current_i].played) continue;
		if (lMusicxmlevents[current_i].starts.empty()) continue;
		if (lMusicxmlevents[current_i].will_stop_index != -1) continue;
		bool triggerOnFound = false; // become true if a trigger-on event appears before the end of this event
		for (int nrTriggerEvent_i = current_i + 1 ; nrTriggerEvent_i < nbEvents ; nrTriggerEvent_i ++)
		{
			if (!lMusicxmlevents[nrTriggerEvent_i].played) continue;
			if (lMusicxmlevents[nrTriggerEvent_i].starts.empty()) continue;
			if (lMusicxmlevents[nrTriggerEvent_i].start_measureNr > lMusicxmlevents[current_i].stop_measureNr) break;
			if (lMusicxmlevents[nrTriggerEvent_i].start_measureNr < lMusicxmlevents[current_i].stop_measureNr) { triggerOnFound = true; break; }
			if (lMusicxmlevents[nrTriggerEvent_i].start_t > lMusicxmlevents[current_i].stop_t) break;
			if (lMusicxmlevents[nrTriggerEvent_i].start_t < lMusicxmlevents[current_i].stop_t) { triggerOnFound = true; break; }
			if (lMusicxmlevents[nrTriggerEvent_i].start_order > lMusicxmlevents[current_i].stop_order) break;
			if (lMusicxmlevents[nrTriggerEvent_i].start_order < lMusicxmlevents[current_i].stop_order) { triggerOnFound = true; break; }
		}
		if (triggerOnFound) continue;
		// reverse search for a trigger-off synchronous
		for(int nrReverseEvent_i = current_i; nrReverseEvent_i >= 0; nrReverseEvent_i--)
		{
			if (! lMusicxmlevents[nrReverseEvent_i].played) continue;
			if (lMusicxmlevents[nrReverseEvent_i].stops.empty()) continue;
			if (lMusicxmlevents[nrReverseEvent_i].tenuto) continue;
			if (lMusicxmlevents[nrReverseEvent_i].stop_measureNr != lMusicxmlevents[current_i].stop_measureNr) continue;
			if (lMusicxmlevents[nrReverseEvent_i].stop_t != lMusicxmlevents[current_i].stop_t) continue;
			if (lMusicxmlevents[nrReverseEvent_i].stop_order != lMusicxmlevents[current_i].stop_order) continue;
			lMusicxmlevents[nrReverseEvent_i].will_stop_index = lMusicxmlevents[current_i].nr;
			lMusicxmlevents[current_i].stop_orpheline = false;
			break;
		} 
	}
	
	// links stops-tenuto and orpheline-stops with synchronous starts : fill musicxmlevent->stop_index ( event which are stopped on trigger-ON )
	p_MeasureNr = -1;
	p_t = -1;
	p_order = -1;
	p_tenuto = false;
	for (int current_i = 0; current_i < nbEvents; current_i++)
	{
		if (!lMusicxmlevents[current_i].played) continue;
		if (!lMusicxmlevents[current_i].stop_orpheline) continue;
		lMusicxmlevents[lMusicxmlevents[current_i].nextNr].stop_index = current_i;
	}

	// count ornaments per note
	int prev_start_t = -1;
	int prev_start_measureNr = -1;
	int nb_order_start_blind = 0;
	std::vector <c_musicxmlevent> lmusicxmlevents_visible;
	for (int current_i = 0; current_i <  nbEvents; current_i++)
	{
		if ((lMusicxmlevents[current_i].start_t != prev_start_t) || (lMusicxmlevents[current_i].start_measureNr != prev_start_measureNr))
		{
			if (lmusicxmlevents_visible.size() > 0)
			{
				for (auto & current_musicxmlevent_visible : lmusicxmlevents_visible ) 
				{
					current_musicxmlevent_visible.nb_ornaments = nb_order_start_blind - 1;
				}
				nb_order_start_blind = 0;
				lmusicxmlevents_visible.clear();
			}
			prev_start_t = lMusicxmlevents[current_i].start_t;
			prev_start_measureNr = lMusicxmlevents[current_i].start_measureNr;
		}
		//if (lMusicxmlevents[current_i].visible)
		lmusicxmlevents_visible.push_back(lMusicxmlevents[current_i]);
		//else
		//{
		if (lMusicxmlevents[current_i].starts.size() > 0)
		{
			lMusicxmlevents[current_i].nr_ornament = nb_order_start_blind;
			nb_order_start_blind++;
		}
		//}
	}
}
void musicxmlcompile::removeExpresseurPart()
{
	// remove an existing Expresseur Part

	compiled_score->part_list->score_parts.erase(
		std::remove_if(compiled_score->part_list->score_parts.begin(), compiled_score->part_list->score_parts.end(),
			[](c_score_part s) { return (s.id == ExpresseurId); }),
		compiled_score->part_list->score_parts.end());

	compiled_score->parts.erase(
		std::remove_if(compiled_score->parts.begin(), compiled_score->parts.end(),
			[](c_part s) { return (s.id == ExpresseurId); }),
		compiled_score->parts.end());
}
void musicxmlcompile::createListMeasures()
{

	// create the list of measures, according to repetitions

	c_part *first_part = & (score->parts[0] );
	if (markList.size() == 0)
	{
		// no mark list. Play all measures straight forward
		for (unsigned int i = 0; i < first_part->measures.size(); i++)
			measureList.push_back(i + 1);
		return;
	}
	sortMeasureMarks();
	// follow mark list instructions
	int nbMeasureMarks = lMeasureMarks.size();
	for (auto & nrMark :markList )
	{
		if ((nrMark >= 0) && (nrMark < nbMeasureMarks))
		{
			int measureStart = lMeasureMarks[nrMark].number; // .Item(nrMark)->GetData()->number;
			int measureEnd = first_part->measures.size() + 1;
			if (nrMark < (nbMeasureMarks - 1))
			{
				measureEnd = lMeasureMarks[nrMark + 1].number; // .Item(nrMark + 1)->GetData()->number;
			}
			for (int i = measureStart; i < measureEnd; i++)
			{
				measureList.push_back(i);
			}
		}
	}
}
void musicxmlcompile::addExpresseurPart()
{
	score->part_list->score_parts.push_back(c_score_part(ExpresseurId, PART_EXPRESSEUR_LONG, PART_EXPRESSEUR_SHORT));
	score->parts.push_back(c_part (ExpresseurId));
	c_part& part_expresseur = score->parts[score->parts.size() - 1];
	// fill the Expresseur part with empty measures
	for (auto & measure : score->parts[0].measures)
	{
		c_measure newMeasure(measure, false);
		if ( measure.number == 1)
		{
			c_attributes *current_attributes = NULL;
			for (auto & measure_sequence : newMeasure.measure_sequences)
			{
				if (measure_sequence.type == t_attributes)
				{
					current_attributes = (c_attributes *)(measure_sequence.pt);
					break;
				}
			}
			if (current_attributes == NULL)
			{
				current_attributes = new c_attributes();
				c_measure_sequence mmeasure_sequence;
				mmeasure_sequence.type = t_attributes;
				mmeasure_sequence.pt = current_attributes;
				newMeasure.measure_sequences.push_back(mmeasure_sequence);
			}
			wxASSERT(current_attributes != NULL);
			if (current_attributes->key)
				delete (current_attributes->key);
			current_attributes->key = NULL;
			if (current_attributes->staff_details)
				delete (current_attributes->staff_details);
			current_attributes->staff_details = new c_staff_details();
			current_attributes->staff_details->staff_lines = 2;
			current_attributes->clefs.clear();
			current_attributes->staves = 1;
			c_clef clef;
			clef.line = 1;
			clef.sign = "percussion";
			current_attributes->clefs.push_back(clef);
		}
		// deleteBarLabel(newMeasure);
		part_expresseur.measures.push_back(newMeasure);
	}
	part_expresseur.compile(score->part_list->score_parts.size() - 1);

	compiled_score->part_list->score_parts.push_back(c_score_part(ExpresseurId, PART_EXPRESSEUR_LONG, PART_EXPRESSEUR_SHORT));
	compiled_score->parts.push_back(c_part(ExpresseurId));
}
void musicxmlcompile::deleteBarLabel(c_measure *mmeasure)
{

	// delete double-bars,and label
	for (auto & current_measure_sequence : mmeasure->measure_sequences)
	{
		//c_measure_sequence *current_measure_sequence = *iter_measure_sequence;
		current_measure_sequence.tobedeleted = false;
		switch (current_measure_sequence.type)
		{
		case t_barline:
		{
			current_measure_sequence.tobedeleted = true;
			break;
		}
		case t_direction:
		{
			c_direction *direction = (c_direction*)(current_measure_sequence.pt);
			for (auto & direction_type : direction->direction_types)
			{
				direction_type.tobedeleted = false;
				switch (direction_type.type)
				{
				case t_segno:
				case t_rehearsal:
					direction_type.tobedeleted = true;
					break;
				case t_words:
				{
					c_words *words = (c_words *)(direction_type.pt);
					wxString s = words->value.Lower();
					if ((s == "fine") || (s.Contains("coda")) || (s.Contains("segno")))
						direction_type.tobedeleted = true;
					break;
				}
				default:
					direction_type.tobedeleted = false;
					break;
				}
			}
			direction->direction_types.erase(
				std::remove_if(direction->direction_types.begin(), direction->direction_types.end(),
				[](c_direction_type x) { return (x.tobedeleted) ; }),
				direction->direction_types.end());
			break;
		}
		}
	}
	mmeasure->measure_sequences.erase(
		std::remove_if(mmeasure->measure_sequences.begin(), mmeasure->measure_sequences.end(),
			[](c_measure_sequence x) { return (x.tobedeleted); }),
		mmeasure->measure_sequences.end());

}
void musicxmlcompile::buildMeasures()
{
	// build the sequence of measures in the compiled parts

	int nrFirstPartVisible = -1;
	for (auto & current_part_list : compiled_score->part_list->score_parts)
	{
		nrFirstPartVisible++;
		if (current_part_list.view)
			break;
	}

	int nrPart = -1;
	for (auto & current_part : compiled_score->parts) 
	{
		nrPart++;
		int nbMeasure_current_part = score->parts[nrPart].measures.size();
		int newMeasureNr = 0;
		int nbmeasureList = measureList.size();
		/*
		for (int nrmeasureList = 0; nrmeasureList < nbmeasureList; nrmeasureList++)
		{
			int nrMeasure = measureList[nrmeasureList] - 1;
			int h = nrMeasure;
		}
		*/

		for (int nrmeasureList = 0; nrmeasureList < nbmeasureList; nrmeasureList++)
		{
			int nrMeasure = measureList[nrmeasureList] - 1;
			if ((nrMeasure < 0) || (nrMeasure >= nbMeasure_current_part))
			{
				wxASSERT(false);
				break;
			}
			c_measure & measure = score->parts[nrPart].measures[nrMeasure];
			bool barlineTodo = false;
			wxString label;
			if (newMeasureNr == ( nbmeasureList - 1 ))
			{
				barlineTodo = true;
			}
			else
			{
				int nrNextMeasure = measureList[nrmeasureList + 1] - 1;
				if ((nrNextMeasure < 0) || (nrNextMeasure >= nbMeasure_current_part))
				{
					wxASSERT(false);
					break;
				}
				for (auto & measureMark : lMeasureMarks ) 
				{
					if (!measureMark.name.IsSameAs(END_OF_THE_SCORE))
					{
						if ((measureMark.number - 1) == nrMeasure)
						{
							label = measureMark.name;
						}
						if ((measureMark.number - 1) == nrNextMeasure)
						{
							barlineTodo = true;
						}
					}
				}
			}

			

			c_measure newMeasure (measure);
			(measure.repeat)++;

			deleteBarLabel(&newMeasure);
			
			//if ((label.empty() == false) && (nrPart == nrFirstPartVisible))
			//{
			//	// write a label
			//	if (newMeasure->repeat > 0)
			//		label.Printf("%s*%d", label, newMeasure->repeat + 1);
			//	c_words *words = new c_words();
			//	words->value = label;
			//	c_direction_type *direction_type = new c_direction_type();
			//	direction_type->type = t_words;
			//	direction_type->pt = (void*)(words);
			//	c_direction *direction = new c_direction();
			//	direction->placement = "above";
			//	direction->direction_types.Append(direction_type);
			//	c_measure_sequence *measure_sequence = new c_measure_sequence();
			//	measure_sequence->type = t_direction;
			//	measure_sequence->pt = (void *)(direction);
			//	newMeasure->measure_sequences.Insert(size_t(0), measure_sequence);
			//}

			if (barlineTodo)
			{
				// add a barline
				bool barlineFound = false;
				for (auto & current_measure_sequence : newMeasure.measure_sequences )
				{
					if (current_measure_sequence.type == t_barline)
					{
						c_barline *current_barline = (c_barline *)(current_measure_sequence.pt);
						current_barline->bar_style = (newMeasureNr == (nbmeasureList - 1)) ? "light-heavy" : "light-light";
						barlineFound = true;
					}
				}
				if (!barlineFound)
				{
					c_measure_sequence measure_sequence;
					c_barline *barline = new c_barline();
					barline->bar_style = (newMeasureNr == (nbmeasureList - 1)) ? "light-heavy" : "light-light";
					barline->location = "right";
					measure_sequence.pt = (void *)barline;
					measure_sequence.type = t_barline;
					newMeasure.measure_sequences.push_back(measure_sequence);
				}
			}
			newMeasure.number = newMeasureNr + 1;
			newMeasureNr++;
			current_part.measures.push_back(newMeasure);
		}
	}
}
void musicxmlcompile::compilePlayedScore()
{

	// compile the score and build the notes to play in lMusicxmlevents

	grace.Empty();
	int key_fifths = 0;
	int nrPart = -1;
	for (auto & current_compiled_part : compiled_score->parts )
	{
		nrPart++;
		// pour chaque partie de la partition
		current_compiled_part.idNr = getTrackNr(current_compiled_part.id);
		// c_score_part *current_score_part = *iter_score_part;
		for (auto & current_measure : current_compiled_part.measures ) 
		{
			// pour chaque mesure de la partie
			int current_t = 0;
			if (current_measure.key_fifths != NULL_INT)
				key_fifths = current_measure.key_fifths;
			for (auto & current_measure_sequence : current_measure.measure_sequences )
			{
				// pour chacun des elements de la mesure
				switch (current_measure_sequence.type)
				{
				case t_note:
				{
					c_note *current_note = (c_note *)(current_measure_sequence.pt);
					// process the note 
					if (compiled_score->part_list->score_parts[nrPart].play)
						current_t = compileNote(&current_compiled_part, current_note, current_measure.number, current_measure.original_number, current_t, current_measure.division_measure, current_measure.division_beat, current_measure.division_quarter, current_measure.repeat, key_fifths);
				}
					break;
					/*
					case t_harmony:
					{
					c_harmony *current_harmony = (c_harmony *)(current_measure_sequence.pt);
					}
					break;
					*/
				case t_backup:
				{
					c_backup *current_backup = (c_backup *)(current_measure_sequence.pt);
					current_t -= current_backup->duration;
				}
					break;
				case t_forward:
				{
					c_forward *current_forward = (c_forward *)(current_measure_sequence.pt);
					current_t += current_forward->duration;
				}
					break;
				case t_barline: 
					break;
				case t_direction:
					break;
				default:
					break;
				}
			}
		}
	}
}
void musicxmlcompile::compileExpresseurPart()
{
	// add the part for Expresseur, according to lMusicxmlevents

	std::sort(lMusicxmlevents.begin() , lMusicxmlevents.end() , musicXmlEventsCompareStart);

	auto part_expresseur = compiled_score->parts.end() - 1;	
	auto currentMeasure = part_expresseur->measures.begin();

	size_t currentMeasure_i = 0;

	wxString text;
	bool breath_mark = false;
	bool fermata = false;
	bool staccato = false;
	bool cross = false;
	int currentT = 0;
	int nrExpresseurNote = 0;
	bool tie_back = false;
	bool firstNote = true;
	bool ternaire = false;
	int ituplet = 0;
	for (size_t musicxmlevent_from_i = 0 ; musicxmlevent_from_i < lMusicxmlevents.size() ; musicxmlevent_from_i++)
	{
		auto musicxmlevent_from = lMusicxmlevents.begin() + musicxmlevent_from_i;
		ternaire = false ;
		if (musicxmlevent_from->division_beat == ( (3 * musicxmlevent_from->division_quarter) / 2))
			ternaire = true;
		if (musicxmlevent_from->text.empty() == false)
			text = musicxmlevent_from->text;
		if (musicxmlevent_from->staccato)
			staccato = true;
		if (musicxmlevent_from->breath_mark)
			breath_mark = true;
		if (musicxmlevent_from->fermata)
			fermata = true;
		if (musicxmlevent_from->visible == false)
			continue;
		if (musicxmlevent_from->starts.empty())
			continue;
		wxASSERT(musicxmlevent_from->duration >= 0);
		musicxmlevent_from->nrExpresseurNote = nrExpresseurNote; // to have a sequential marker for the list of Expresseur notes
		int from_start_measureNr = musicxmlevent_from->start_measureNr;
		int from_stop_measureNr = musicxmlevent_from->stop_measureNr;
		int from_startT = musicxmlevent_from->start_t;
		int from_stopT = musicxmlevent_from->stop_t;
		if (from_stopT == 0)
		{
			from_stop_measureNr--;
			from_stopT = musicxmlevent_from->division_measure;
		}
		int to_start_measureNr = from_stop_measureNr + 1;
		int to_stop_measureNr = from_stop_measureNr + 1;
		int to_startT = 0;
		int to_stopT = 0;
		for (size_t musicxmlevent_to_i = musicxmlevent_from_i + 1; musicxmlevent_to_i < lMusicxmlevents.size(); musicxmlevent_to_i++)
		{
			auto musicxmlevent_to = lMusicxmlevents.begin() + musicxmlevent_to_i;
			if (musicxmlevent_to->visible == false) 
				continue ;
			if (musicxmlevent_to->starts.empty())
				continue ;
			if ((musicxmlevent_to->start_measureNr == from_start_measureNr) && ((musicxmlevent_to->start_t ) == from_startT))
				continue;;
			to_start_measureNr = musicxmlevent_to->start_measureNr;
			to_stop_measureNr = musicxmlevent_to->stop_measureNr;
			to_startT = musicxmlevent_to->start_t;
			to_stopT = musicxmlevent_to->stop_t;
			if (to_stopT == 0)
			{
				to_stop_measureNr--;
				to_stopT = musicxmlevent_to->division_measure ;
			}
			break;
		}
		int start_measureNr = from_start_measureNr;
		int stop_measureNr = from_stop_measureNr;
		int startT = from_startT ;
		int stopT = from_stopT;
		if ((to_start_measureNr < from_stop_measureNr) || ((to_start_measureNr == from_stop_measureNr) && (to_startT <= from_stopT)))
		{
			stop_measureNr = to_start_measureNr;
			stopT = to_startT;
		}
		bool unused_first_note;

		// add rests
		currentMeasure = part_expresseur->measures.begin() + currentMeasure_i;
		while (start_measureNr > currentMeasure->number)
		{
			// finish the current measure with rests
			addNote(currentMeasure, false, currentT, currentMeasure->division_measure, true, false, false, &unused_first_note, &nrExpresseurNote, 0, &text, &staccato, &fermata, &breath_mark, ternaire, false, &ituplet);
			// go to next measure
			currentT = 0;
			currentMeasure_i++;
			if (currentMeasure_i == part_expresseur->measures.size())
			{
				wxASSERT(false);
				break;
			}
			currentMeasure = part_expresseur->measures.begin() + currentMeasure_i;
		}

		// add rests to fill the gap up to this note
		addNote(currentMeasure, false , currentT, startT, true, false, false, &unused_first_note, &nrExpresseurNote, 0, &text, &staccato, &fermata, &breath_mark, ternaire , false, &ituplet);

		// calculate total_duration 
		currentT = startT;
		tie_back = false;
		firstNote = true;
		cross = musicxmlevent_from->cross;
		// add the figure-note per measure
		for (int n = start_measureNr; n < stop_measureNr; n++)
		{
			addNote(currentMeasure, (stopT > 0 ) , startT, currentMeasure->division_measure, false, tie_back, true, &firstNote, &nrExpresseurNote, musicxmlevent_from->nb_ornaments, &text , &staccato, &fermata, &breath_mark, ternaire , cross , &ituplet);
			firstNote = false;
			currentMeasure_i++;
			if (currentMeasure_i == part_expresseur->measures.size())
			{
				wxASSERT(false);
				break;
			}
			currentMeasure = part_expresseur->measures.begin() + currentMeasure_i;
			currentT = 0;
			start_measureNr++;
			startT = 0;
			tie_back = true;
		}
		addNote(currentMeasure, (stopT > 0), currentT, stopT, false, tie_back, false, &firstNote, &nrExpresseurNote, musicxmlevent_from->nb_ornaments, &text, &staccato, &fermata, &breath_mark, ternaire , cross , &ituplet);
		firstNote = false;
		currentT = stopT;
	}
	addNote(currentMeasure, false, currentT, currentMeasure->division_measure, true, false, false, &firstNote, &nrExpresseurNote, 0, &text, &staccato, &fermata, &breath_mark, ternaire , cross , &ituplet);

}
void musicxmlcompile::addNote(std::vector<c_measure>::iterator measure, bool after_measure, int from_t, int to_t, bool rest, bool tie_back, bool tie_next,
	                          bool *first_note, int * nrExpresseurNote, int nbOrnaments, wxString *text , bool *staccato, bool *fermata,
	                          bool *breath_mark, bool ternaire , bool cross, int *ituplet)
{
	// add symbols for one note [from_t,to_t], to_t <= divisions, inside one measure

	if (from_t == to_t)
		return;

	/*
	if (rest && (from_t == 0) && (to_t == measure->division_measure))
	{
		// full measure rest
		c_rest *rest = new c_rest();
		rest->measure = "yes";
		c_note *note = new c_note();
		note->rest = rest;
		c_measure_sequence *measure_sequence = new c_measure_sequence();
		measure_sequence->pt = (void *)note;
		measure_sequence->type = t_note;
		measure->measure_sequences.Append(measure_sequence);
		return;
	}
	*/

	int ffrom_t = from_t;
	bool ttie_back = tie_back;
	wxASSERT(measure->division_beat != NULL_INT);
	if ((from_t % measure->division_beat) != 0)
	{
		// insert figures up to next beat
		int t0 = 0;
		while (t0 < from_t)
			t0 += measure->division_beat;
		t0 += measure->division_beat;
		if (t0 < to_t)
		{
			addSymbolNote(measure, after_measure, t0 - from_t, rest, tie_back, true, first_note, nrExpresseurNote, nbOrnaments, text, staccato, fermata, breath_mark, ternaire , cross , ituplet);
			*first_note = false;
			ttie_back = true;
			ffrom_t = t0;
		}
	}
	// insert figures starting on the beat
	addSymbolNote(measure, after_measure, to_t - ffrom_t, rest, ttie_back, tie_next, first_note, nrExpresseurNote, nbOrnaments, text, staccato, fermata, breath_mark, ternaire , cross, ituplet);
}
void musicxmlcompile::addSymbolNote(std::vector<c_measure>::iterator measure, bool after_measure, int duration, bool rest, bool tie_back, bool tie_next, bool *first_note, int * nrExpresseurNote, int nbOrnaments, wxString *text , bool *staccato, bool *fermata, bool *breath_mark, bool ternaire , bool cross, int *ituplet)
{

	c_note *note = NULL ;
	int duration_todo = duration;
	int antiLoop = 0;
	while (duration_todo > 0)
	{
		if (antiLoop++ > 10)
			break;

		wxString typeNote;
		int duration_done;
		int dot;
		int tuplet;
		calculateDuration(duration_todo, measure->division_quarter, ternaire , &duration_done, & typeNote , &dot , &tuplet);
		duration_todo -= duration_done;
		if (typeNote.empty() == false)
		{
			note = new c_note();

			note->duration = duration_done;
			note->mtype = typeNote;
			note->dots = dot;
			if (*ituplet > 0)
			{
				(*ituplet)--;
				if (*ituplet == 1)
				{
					if (!note->notations)
						note->notations = new c_notations();
					if (!note->notations->tuplet)
						note->notations->tuplet = new c_tuplet();
					note->notations->tuplet->type = "stop";
				}

			}
			if (tuplet > 1)
			{
				if (*ituplet == 0)
				{
					if (!note->notations)
						note->notations = new c_notations();
					if (!note->notations->tuplet)
						note->notations->tuplet = new c_tuplet();
					if (strcmp(note->notations->tuplet->type, "stop") != 0)
					{
						note->notations->tuplet->type = "start";
						*ituplet = tuplet;
					}
				}
				note->time_modification = new c_time_modification();
				switch (tuplet)
				{
				case 5:
					note->time_modification->actual_notes = 5;
					note->time_modification->normal_notes = 4;
					break;
				case 3:
				default:
					note->time_modification->actual_notes = 3;
					note->time_modification->normal_notes = 2;
					break;
				}
			}

			if (rest)
			{
				c_rest *mrest = new c_rest();
				note->rest = mrest;
			}
			else
			{
				(*nrExpresseurNote)++; // note suivante dans la partition (pour la coorelation avec lilypond )
				c_pitch *pitch = new c_pitch();
				//pitch->unpitched = true;
				pitch->step = "F";
				pitch->octave = 4;
				note->stem = "up";
				note->pitch = pitch;
				if ((*first_note && tie_back) || (! *first_note))
				{
					c_tied *tied = new c_tied();
					tied->stop = true;
					if (note->notations == NULL)
						note->notations = new c_notations();
					note->notations->tied = tied;
					*first_note = false;
				}
				if (duration_todo > 0)
				{
					c_tied *tied = new c_tied();
					tied->start = true;
					if (note->notations == NULL)
						note->notations = new c_notations();
					note->notations->tied = tied;
				}
			}

			if (cross)
			{
				note->notehead = "x";
			}
			if (*first_note)
			{
				if (nbOrnaments > 0)
				{
					c_lyric lyric;
					lyric.text = wxString::Format("*%d", nbOrnaments + 1);
					//lyric->placement = "below";
					note->lyrics.push_back(lyric);

				}
				if (text->IsEmpty() == false)
				{
					c_words *words = new c_words();
					words->value = *text;
					c_direction_type direction_type;
					direction_type.type = t_words;
					direction_type.pt = (void*)(words);
					c_direction direction;
					direction.placement = "above";
					direction.direction_types.push_back(direction_type);
					c_measure_sequence measure_sequence;
					measure_sequence.type = t_direction;
					measure_sequence.pt = (void *)(& direction);
					measure->measure_sequences.push_back(measure_sequence);
					text->Empty();
				}
				if (*staccato)
				{
					if (note->notations == NULL)
						note->notations = new c_notations();
					if (note->notations->articulations == NULL)
						note->notations->articulations = new c_articulations();
					note->notations->articulations->articulations.push_back("staccato");
					note->notations->articulations->placements.push_back(NULL_STRING);
					note->notations->articulations->default_xs.push_back(NULL_INT);
					note->notations->articulations->default_ys.push_back(NULL_INT);
					*staccato = false;
				}
				if (*breath_mark)
				{
					if (note->notations == NULL)
						note->notations = new c_notations();
					if (note->notations->articulations == NULL)
						note->notations->articulations = new c_articulations();
					note->notations->articulations->articulations.push_back("breath-mark");
					note->notations->articulations->placements.push_back(NULL_STRING);
					note->notations->articulations->default_xs.push_back(NULL_INT);
					note->notations->articulations->default_ys.push_back(NULL_INT);
					*breath_mark = false;
				}
				if (*fermata)
				{
					if (note->notations == NULL)
						note->notations = new c_notations();
					if (note->notations->fermata == NULL)
						note->notations->fermata = new c_fermata();
					note->notations->fermata->type = "upright" ;
					*fermata = false;
				}
			}
			*first_note = false;
			c_measure_sequence measure_sequence;
			measure_sequence.pt = (void *)note;
			measure_sequence.type = t_note;
			measure->measure_sequences.push_back(measure_sequence);
		}
	}

	if ((! rest) && (tie_next) && ( note != NULL ) && (after_measure))
	{
		c_tied *tied = new c_tied();
		tied->start = true;
		if (note->notations == NULL)
			note->notations = new c_notations();
		note->notations->tied = tied;
	}
}
void musicxmlcompile::calculateDuration(int duration, int division_quarter, bool ternaire , int *durationDone, wxString *typeNote, int *dot, int *tuplet)
{

	*dot = 0;
	*tuplet = 1;

	*durationDone = 4 * division_quarter ;
	if (((*durationDone) <= duration) && (! ternaire))
	{
		*typeNote = "whole";
		return;
	}
	if (((((2 * division_quarter) * 7) % 4) == 0) && (! ternaire))
	{
		*durationDone = ((2 * division_quarter) * 7) / 4;
		if ((*durationDone) <= duration)
		{
			*typeNote = "half";
			*dot = 2;
			return;
		}
	}
	if ((((2 * division_quarter) * 3) % 2) == 0)
	{
		*durationDone = ((2 * division_quarter) * 3) / 2;
		if ((*durationDone) <= duration)
		{
			*typeNote = "half";
			*dot = 1;
			return;
		}
	}
	if ((((4 * division_quarter) * 2) % 3) == 0)
	{
		*durationDone = ((4 * division_quarter) * 2) / 3;
		if (((*durationDone) == duration) || ((*durationDone) <= (duration * 2 / 3)))
		{
			*typeNote = "whole";
			*tuplet = 3;
			return;
		}
	}
	*durationDone = 2 * division_quarter;
	if (((*durationDone) <= duration) && (! ternaire))
	{
		*typeNote = "half";
		return;
	}
	if (((((1 * division_quarter) * 7) % 4) == 0) && (! ternaire ))
	{
		*durationDone = ((1 * division_quarter) * 7) / 4;
		if ((*durationDone) <= duration)
		{
			*typeNote = "quarter";
			*dot = 2;
			return;
		}
	}
	if ((((4 * division_quarter) * 2) % 5) == 0)
	{
		*durationDone = ((4 * division_quarter) * 2) / 5;
		if (((*durationDone) == duration) || ((*durationDone) <= (duration * 2 / 5)))
		{
			*typeNote = "half";
			*tuplet = 5;
			return;
		}
	}
	if ((((1 * division_quarter) * 3) % 2) == 0)
	{
		*durationDone = ((1 * division_quarter) * 3) / 2;
		if ((*durationDone) <= duration)
		{
			*typeNote = "quarter";
			*dot = 1;
			return;
		}
	}
	if ((((2 * division_quarter) * 2) % 3) == 0)
	{
		*durationDone = ((2 * division_quarter) * 2) / 3;
		if (((*durationDone) == duration) || ((*durationDone) <=( duration*2/3)))
		{
			*typeNote = "half";
			*tuplet = 3;
			return;
		}
	}
	*durationDone = 1 * division_quarter;
	if ((*durationDone) <= duration)
	{
		*typeNote = "quarter";
		return;
	}
	if (((division_quarter * 7) % (2 * 4)) == 0)
	{
		*durationDone = (division_quarter * 7) / (2 * 4);
		if ((*durationDone) <= duration)
		{
			*typeNote = "eighth";
			*dot = 2;
			return;
		}
	}
	if ((((2 * division_quarter) * 2) % 5) == 0)
	{
		*durationDone = ((2 * division_quarter) * 2) / 5;
		if (((*durationDone) == duration) || ((*durationDone) <= (duration * 2 / 5)))
		{
			*typeNote = "quarter";
			*tuplet = 5;
			return;
		}
	}
	if (((division_quarter * 3) % (2 * 2)) == 0)
	{
		*durationDone = (division_quarter * 3) / (2 * 2);
		if ((*durationDone) <= duration)
		{
			*typeNote = "eighth";
			*dot = 1;
			return;
		}
	}
	if ((((1 * division_quarter) * 2) % 3) == 0)
	{
		*durationDone = ((1 * division_quarter) * 2) / 3;
		if (((*durationDone) == duration) || ((*durationDone) <= (duration * 2 / 3)))
		{
			*typeNote = "quarter";
			*tuplet = 3;
			return;
		}
	}
	if ((division_quarter % 2) == 0)
	{
		*durationDone = division_quarter / 2;
		if ((*durationDone) <= duration)
		{
			*typeNote = "eighth";
			return;
		}
	}
	if (((division_quarter * 7) % (4 * 4)) == 0)
	{
		*durationDone = (division_quarter * 7) / (4 * 4);
		if ((*durationDone) <= duration)
		{
			*typeNote = "16th";
			*dot = 2;
			return;
		}
	}
	if ((((1 * division_quarter) * 2) % 5) == 0)
	{
		*durationDone = ((1 * division_quarter) * 2) / 5;
		if (((*durationDone) == duration) || ((*durationDone) <= (duration * 2 / 5)))
		{
			*typeNote = "eighth";
			*tuplet = 5;
			return;
		}
	}
	if (((division_quarter * 3) % (4 * 2)) == 0)
	{
		*durationDone = (division_quarter * 3) / (4 * 2);
		if ((*durationDone) <= duration)
		{
			*typeNote = "16th";
			*dot = 1;
			return;
		}
	}
	if (((division_quarter * 2) % (3 * 2)) == 0)
	{
		*durationDone = (division_quarter * 2) / (3 * 2);
		if (((*durationDone) == duration) || ((*durationDone) <= (duration * 2 / 3)))
		{
			*typeNote = "eighth";
			*tuplet = 3;
			return;
		}
	}
	if ((division_quarter % 4) == 0)
	{
		*durationDone = division_quarter / 4;
		if ((*durationDone) <= duration)
		{
			*typeNote = "16th";
			return;
		}
	}
	if (((division_quarter * 8) % (8 * 4)) == 0)
	{
		*durationDone = (division_quarter * 7) / (8 * 4);
		if ((*durationDone) <= duration)
		{
			*typeNote = "32nd";
			*dot = 2;
			return;
		}
	}
	if (((division_quarter * 2) % (2 * 5)) == 0)
	{
		*durationDone = (division_quarter * 2) / (2 * 5);
		if (((*durationDone) == duration) || ((*durationDone) <= (duration * 2 / 5)))
		{
			*typeNote = "16th";
			*tuplet = 5;
			return;
		}
	}
	if (((division_quarter * 3) % (8 * 2)) == 0)
	{
		*durationDone = (division_quarter * 3) / (8 * 2);
		if ((*durationDone) <= duration)
		{
			*typeNote = "32nd";
			*dot = 1;
			return;
		}
	}
	if (((division_quarter * 2) % (4 * 3)) == 0)
	{
		*durationDone = (division_quarter * 2) / (4 * 3);
		if (((*durationDone) == duration) || ((*durationDone) <= (duration * 2 / 3)))
		{
			*typeNote = "16th";
			*tuplet = 3;
			return;
		}
	}
	if ((division_quarter % 8) == 0)
	{
		*durationDone = division_quarter / 8;
		if ((*durationDone) <= duration)
		{
			*typeNote = "32nd";
			return;
		}
	}
	if (((division_quarter * 7) % (16 * 4)) == 0)
	{
		*durationDone = (division_quarter * 7) / (16 * 4);
		if ((*durationDone) <= duration)
		{
			*typeNote = "64th";
			*dot = 2;
			return;
		}
	}
	if (((division_quarter * 2) % (4 * 5)) == 0)
	{
		*durationDone = (division_quarter * 2) / (4 * 5);
		if (((*durationDone) == duration) || ((*durationDone) <= (duration * 2 / 5)))
		{
			*typeNote = "32nd";
			*tuplet = 5;
			return;
		}
	}
	if (((division_quarter * 3) % (16 * 2)) == 0)
	{
		*durationDone = (division_quarter * 3) / (16 * 2);
		if ((*durationDone) <= duration)
		{
			*typeNote = "64th";
			*dot = 1;
			return;
		}
	}
	if (((division_quarter * 2) % (8 * 3)) == 0)
	{
		*durationDone = (division_quarter * 2) / (8 * 3);
		if (((*durationDone) == duration) || ((*durationDone) <= (duration * 2 / 3)))
		{
			*typeNote = "32nd";
			*tuplet = 3;
			return;
		}
	}
	if ((division_quarter % 16) == 0)
	{
		*durationDone = division_quarter / 16;
		if ((*durationDone) <= duration)
		{
			*typeNote = "64th";
			return;
		}
	}
	if (((division_quarter * 2) % (8 * 5)) == 0)
	{
		*durationDone = (division_quarter * 2) / (8 * 5);
		if (((*durationDone) == duration) || ((*durationDone) <= (duration * 2 / 5)))
		{
			*typeNote = "64th";
			*tuplet = 5;
			return;
		}
	}
	if (((division_quarter * 2) % (16 * 3)) == 0)
	{
		*durationDone = (division_quarter * 2) / (16 * 3);
		if (((*durationDone) == duration) || ((*durationDone) <= (duration * 2 / 3)))
		{
			*typeNote = "64th";
			*tuplet = 3;
			return;
		}
	}
	if (((division_quarter * 2) % (16 * 5)) == 0)
	{
		*durationDone = (division_quarter * 2) / (16 * 5);
		if (((*durationDone) == duration) || ((*durationDone) <= (duration * 2 / 5)))
		{
			*typeNote = "128th";
			*tuplet = 5;
			return;
		}
	}
	if ((division_quarter % 32) == 0)
	{
		*durationDone = duration;
		*typeNote = "128th";
	}
	*durationDone = duration;
	*typeNote = "";
}
bool musicxmlcompile::getPosEvent(int nrEvent, int *pageNr, wxRect *rect , bool *turn , int *nr_ornament )
{

	*nr_ornament = -1;
	if ((nrEvent < 0) || (nrEvent >= nbEvents))
	{
		*pageNr = -1 ;
		rect->x = 0;
		rect->y = 0;
		rect->width = 0;
		rect->height = 0;
		*turn = false;
		return false;
	}
	c_musicxmlevent *m = & (lMusicxmlevents[nrEvent] );
	*pageNr = m->pageNr;
	rect->x = m->rect.x;
	rect->y = m->rect.y;
	rect->width = m->rect.width;
	rect->height = m->rect.height;
	*turn = m->turnPage;
	if (m->nb_ornaments > 0)
	{
		*nr_ornament = m->nb_ornaments - m->nr_ornament;
	}
	return (m->pageNr >= 0);
}
int musicxmlcompile::setPosEvent(int nrExpresseurNote, int pageNr, wxRect rect)
{
	int measure = -1;
	int t = -1;

	for (auto & musicxmlevent : lMusicxmlevents )
	{
		if (musicxmlevent.nrExpresseurNote == nrExpresseurNote)
		{
			musicxmlevent.pageNr = pageNr;
			musicxmlevent.rect = rect;
			measure = musicxmlevent.start_measureNr;
			t = musicxmlevent.start_t;
			break;
		}
	}
	if (measure == -1)
		return true;
	for (auto & musicxmlevent : lMusicxmlevents)
	{
		if ((musicxmlevent.start_measureNr == measure) && (musicxmlevent.start_t == t))
		{
			musicxmlevent.pageNr = pageNr;
			musicxmlevent.rect = rect;
		}
	}
	return measure;
}
int musicxmlcompile::pageToEventNr(int pageNr)
{

	for ( auto & current_musicxmlevent : lMusicxmlevents )
	{
		if (current_musicxmlevent.pageNr == pageNr)
			return current_musicxmlevent.nr;
	}
	return true;
}
void musicxmlcompile::setMeasureTurnEvent(int nrMeasure, bool clean)
{
	for (auto & musicxmlevent : lMusicxmlevents)
	{
		if (clean)
		{
			musicxmlevent.turnPage = false;
		}
		else
		{
			if (musicxmlevent.start_measureNr == nrMeasure)
				musicxmlevent.turnPage = true;
			if (musicxmlevent.start_measureNr > nrMeasure)
				return;
		}
	}
}
int musicxmlcompile::stringToEventNr(wxString s)
{
	int measureNr = -1;
	int repeat = -1;
	bool absolute = false;
	wxString label =  s;
	wxString srepeat ;
	if (s.Contains("*"))
		label = s.BeforeFirst('*', &srepeat);
	long l;
	if (srepeat.ToLong(&l))
		repeat = l;
	if (s.StartsWith("!"))
	{
		absolute = true;
		label = label.Mid(1);
	}
	if (label.ToLong(&l))
		measureNr = l;
	else
	{
		for (auto & measureMark : lMeasureMarks )
		{
			if (measureMark.name.IsSameAs(label))
			{
				measureNr = measureMark.number;
				break;
			}
		}
	}
	if (measureNr == -1)
		return true;
	for (auto & current_musicxmlevent : lMusicxmlevents )
	{
		if (absolute)
		{
			if (current_musicxmlevent.start_measureNr == measureNr)
				return current_musicxmlevent.nr;
		}
		else
		{
			if (repeat == -1)
			{	
				if (current_musicxmlevent.original_measureNr == measureNr)
				{
					int m = current_musicxmlevent.nr;
					return m;
				}
			}
			else
			{
				if ((current_musicxmlevent.original_measureNr == measureNr) && (current_musicxmlevent.repeat == (repeat - 1)))
					return current_musicxmlevent.nr;
			}
		}
	}
	return true;
}
bool musicxmlcompile::getScorePosition(int nrEvent , int *absolute_measure_nr, int *measure_nr, int *repeat , int *beat, int *t , int *uid)
{
	if ((nrEvent < 0) || (nrEvent >= nbEvents))
	{
		*absolute_measure_nr = 0;
		*measure_nr = 0;
		*repeat = 0;
		*beat = 0;
		*t = 0;
		*uid = -1;
		return true;
	}
	c_musicxmlevent *m = & (lMusicxmlevents[nrEvent]);
	*absolute_measure_nr = m->start_measureNr;
	*measure_nr = m->original_measureNr;
	*repeat = m->repeat ;
	*beat = m->start_t / m->division_beat;
	*t = m->start_t % m->division_beat  ;
	if (m->end_score)
		return true;
	return false ;
}
std::vector <wxString>  musicxmlcompile::getListOrnament()
{
	std::vector <wxString> a;
	for (int i = o_dynamic; i < o_flagend; i++)
	{
		wxString s = ornamentName[i];
		switch (i)
		{
		case o_dynamic: s = s + "=127 -- (=0 .. =127)"; break;
		case o_random_delay: s = s + "=50 -- in ms"; break;
		case o_pedal_bar: s = s + "=64 -- add a pedal on each bar (=0 .. =127)"; break;
		case o_pedal: s = s + "=64 -- add a pedal (=0 .. =127)"; break;
		case o_lua: s = s + "=trackvolume 1 80 -- =mainvolume 65 =chord G7 =instrument piano(P1) =tune 415 =bendrange 2 =gm 2 =scale just G =scale arabian ..."; break;
		case o_text: s = s + "=remark -- text added in Expresseur part"; break;
		case o_pianissimo: s = s + " -- pianissimo on part(s)"; break;
		case o_piano:  s = s + " -- pianio on part(s)"; break ;
		case o_mesopiano: s = s + " -- mesopiano on part(s)";  break ;
		case o_mesoforte: s = s + " -- mesoforte on part(s)";  break ;
		case o_forte: s = s + " -- forte on part(s)";  break ;
		case o_fortissimo: s = s + " -- fortissimo on part(s)";  break ;
		case o_crescendo: s = s + " -- creschendo between two nuances";  break ;
		case o_diminuendo: s = s + " -- diminuendo between two nuances";  break ;
		case o_tenuto: s = s + " -- tenuto up to the next note";  break ;
		case o_staccato: s = s + "=1/2 -- (=1/2, =3/4, =1/3, =2/3, =display)"; break;
		case o_accent: s = s + "=20 --  (=-127 .. =127)"; break;
		case o_grace: s = s + "=up -- (=up, =inverted, list of pitches like =C4/E4,G4)"; break;
		case o_mordent: s = s + "=normal -- (=normal, =inverted)";  break;
		case o_turn: s = s + "=normal -- (=normal, =inverted)";  break;
		case o_btrill: s = s + "=2 -- =nb bemol trills";  break;
		case o_trill: s = s + " =2 -- =nb trills";  break;
		case o_arpeggiate: s = s + "=up -- (=up , =down)";  break;
		case o_transpose: s = s + "=0 -- (=-24 .. =24 half-tone)"; break;
		case o_delay: s = s + "=100 -- in ms"; break;
		case o_before:  s.Empty() ;break ;
		case o_after:  s.Empty() ;break ;
		case o_flagend : s.Empty() ;break ;
		default:
			break;
		}
		if ( ! s.empty() )
			a.push_back(s);
	}
	return a;
}
int musicxmlcompile::pointToEventNr(int pageNr , wxPoint p)
{

	// return the closest note-square , on pageNr, closed to point p 
	int dmin = 100 * 100 + 100 * 100;
	int nmin = -1;
	for (auto & current_musicxmlevent: lMusicxmlevents )
	{
		if (current_musicxmlevent.pageNr == pageNr)
		{
			if (current_musicxmlevent.rect.Contains(p))
				return current_musicxmlevent.nr;
			wxRect r = current_musicxmlevent.rect;
			int dx = p.x - (r.x + r.width / 2);
			int dy = p.y - (r.y + r.height / 2);
			int d2 = dx * dx + dy * dy;
			if (d2 < (40 * 40 + 40 * 40))
			{
				if (d2 < dmin)
				{
					dmin = d2;
					nmin = current_musicxmlevent.nr;
				}
			}
		}
		if (current_musicxmlevent.pageNr > pageNr)
			return nmin;
	}
	return nmin;
}
wxString musicxmlcompile::getTitle()
{
	if (isOk() && score->work != NULL)
		return score->work->work_title;
	return wxEmptyString;
}
int musicxmlcompile::getTracksCount()
{
	// return the number of tracks in the original musicXML file 
	// does not count Expresseur part which will be calculated in score_compiled )
	if (!isOk())
		return 0;
	int nb_tracks = 0;
	for (auto & current : score->part_list->score_parts ) 
	{
		if (current.id != ExpresseurId)
			nb_tracks++;
	}
	return (nb_tracks);
}
std::vector <wxString> musicxmlcompile::getTracksName()
{

	std::vector <wxString> l;

	if (!isOk())
		return l;

	int nb_track = getTracksCount();
	for ( int nrTrack = 0; nrTrack < nb_track; nrTrack++)
	{
		l.push_back(getTrackName( nrTrack) );
	}

	return l;
}
wxString musicxmlcompile::getTrackName(int nrTrack)
{
	if (!isOk(true))
		return wxEmptyString;
	wxString s;
	int nbTrack = getTracksCount();
	if ((nrTrack >= 0) && (nrTrack < nbTrack))
	{
		c_score_part *mpart = &(compiled_score->part_list->score_parts[nrTrack]);
		if (mpart->part_alias == NULL_STRING)
		{
			return wxEmptyString;
		}
		return(mpart->part_alias);
	}
	return wxEmptyString;
}
wxString musicxmlcompile::getTrackId(int nrTrack)
{
	if (!isOk())
		return "";

	if ((nrTrack != wxNOT_FOUND) && (nrTrack >= 0) && (nrTrack < getTracksCount()))
		return score->part_list->score_parts[nrTrack].id;

	return wxEmptyString;
}
bool musicxmlcompile::getTrackPlay(int nrTrack)
{
	if (!isOk())
		return false;
	if ((nrTrack != wxNOT_FOUND) && (nrTrack >= 0) && (nrTrack < getTracksCount()))
		return(score->part_list->score_parts[nrTrack].play);
	return false;
}
std::vector <int> musicxmlcompile::getTracksPlay()
{
	std::vector <int> a;

	if (!isOk())
		return a;
	for (auto & current : score->part_list->score_parts ) 
	{
		a.push_back(current.play);
	}
	return a;
}
bool musicxmlcompile::getTrackDisplay(int nrTrack)
{
	if (!isOk())
		return false;
	if ((nrTrack != wxNOT_FOUND) && (nrTrack >= 0) && (nrTrack < getTracksCount()))
		return(score->part_list->score_parts[nrTrack].view);
	return false;
}
std::vector <int> musicxmlcompile::getTracksDisplay()
{
	std::vector <int> a;

	if (!isOk())
		return a;
	for (auto & current : score->part_list->score_parts ) 
	{
		a.push_back(current.view);
	}
	return a;
}
int musicxmlcompile::getTrackNr(wxString idTrack)
{
	// return the index of the idTrack ( or wxNOT_FOUND if not found )

	if (!isOk())
		return wxNOT_FOUND;

	int nr = -1 ;
	for (auto & current_score_part : score->part_list->score_parts )
	{
		nr++;
		if (current_score_part.id == idTrack)
			return nr;
	}
	return wxNOT_FOUND;
}
void musicxmlcompile::initRecordPlayback()
{
	lEventPlaybacks.clear() ;
}
void musicxmlcompile::initPlayback()
{
	nrEventPlayback = 0 ;
	t0Playback =  wxGetUTCTimeMillis(); 	
}
void musicxmlcompile::recordPlayback(wxLongLong time, int nr_device, int type_msg, int channel, int value1, int value2)
{
	if (lEventPlaybacks.size() == 0 )
		t0RecordPlayback = time ;
	lEventPlaybacks.push_back(c_eventPlayback(	( time - t0RecordPlayback ), nr_device, type_msg, channel, value1, value2));
}
bool musicxmlcompile::playback()
{
	wxLongLong tPlayback =  wxGetUTCTimeMillis() - t0Playback ;
	while(true)
	{
		if (nrEventPlayback == lEventPlaybacks.size())
			return false ;
		c_eventPlayback *m_eventPlayback= &(lEventPlaybacks[nrEventPlayback]) ;
		if ( m_eventPlayback->time > tPlayback )
			return true ;
		basslua_playbackmsg(m_eventPlayback->nr_device,m_eventPlayback->type_msg , m_eventPlayback->channel , m_eventPlayback->value1, m_eventPlayback->value2 );
		nrEventPlayback++ ;
	}
	return false;
}
wxString musicxmlcompile::getPlayback()
{
	if (lEventPlaybacks.size() == 0)
		return wxEmptyString;

	// estimatin of the string size :
	wxString s, f;
	size_t lf ;
	size_t ln ;
	wxString sn = "1222333:1,10,10,127,127\n" ; // typical line
	ln = sn.length() ;

	s.Printf("%s :\n", SET_PLAYBACK);

	// allocation of the string size
	lf = lEventPlaybacks.size() * ln + s.length() ;
	f.Alloc(lf);

	// make the string
	f += (s);
	s.Printf("%s100\n", SET_RATIO);
	f += (s);
	wxLongLong t0;
	bool firstPlayback = true;
	for (auto & meventPlayback : lEventPlaybacks )
	{
		if (firstPlayback)
			t0 = meventPlayback.time;
		firstPlayback = false;
		wxString s4;
		wxLongLong ldt = meventPlayback.time - t0;
		t0 = meventPlayback.time;
		wxString sdt = ldt.ToString();
		s4.Printf("%s:%d,%d,%d,%d,%d\n", sdt , meventPlayback.nr_device, meventPlayback.type_msg, meventPlayback.channel, meventPlayback.value1, meventPlayback.value2);
		f += s4;
	}
	return f;
}
