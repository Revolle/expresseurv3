#ifndef DEF_MUSICXMLCOMPILE

#define DEF_MUSICXMLCOMPILE

#define END_OF_THE_SCORE "END_OF_THE_SCORE"
#define PART_PLAYED "played"
#define PART_NOT_PLAYED "not played"
#define PART_VISIBLE "visible"
#define PART_NOT_VISIBLE "not visible"
#define PART_EXPRESSEUR_LONG "_X_"
#define PART_EXPRESSEUR_SHORT "_X_"

// class to have a list of lMeasureMarks
///////////////////////////////////////
class c_measureMark
{
public:
	c_measureMark(int inr) { number = inr; name.sprintf("M%d", inr); }
	void changeMeasure(int inr) { number = inr; name.sprintf("M%d", inr); }
	void merge(const c_measureMark &m) 
	{ 
		rehearsal = rehearsal || m.rehearsal;
		restart = restart || m.restart;
		segno = segno || m.segno;
		repeatForward = repeatForward || m.repeatForward;
		repeatBackward = repeatBackward || m.repeatBackward;
		jumpnext = jumpnext || m.jumpnext;
		coda = coda || m.coda;
		tocoda = tocoda || m.tocoda;
		dacapo = dacapo || m.dacapo;
		fine = fine || m.fine;
		dalsegno = dalsegno || m.dalsegno;
	}
	wxString name;
	int number;
	bool rehearsal = false;
	bool restart = false;
	bool segno = false;
	bool repeatForward = false;
	bool repeatBackward = false;
	bool jumpnext = false;
	bool coda = false;
	bool tocoda = false;
	bool dacapo = false;
	bool fine = false;
	bool dalsegno = false;
	int repeatNumber = 0;
	bool toBeDeleted = false;
};

// class to have a list of ornements
////////////////////////////////////
class c_ornament
{
public:
	c_ornament(int itype, int imeasureNumber, int it, int ipartNr, int istaffNr, int irepeat , bool ibefore , wxString ivalue )
	{
		type = itype;
		measureNumber = imeasureNumber;
		t = it;
		partNr = ipartNr;
		staffNr = istaffNr;
		repeat = irepeat;
		before = ibefore;
		value = ivalue;
	};
	int type; // type of lOrnaments : enum lOrnaments o_xx
	int partNr = -1; // track to apply the ornament ( -1 == all )
	int staffNr = -1; // staff to apply the ornament ( -1 == all )
	int measureNumber; // measure Number
	int t; // time in the meausre, in divisions of the measure
	int chord_order = -1; // order in the chord 
	int repeat = -1; // restrict the ornament for the 1st time ( 0 ), 2nd time, ... always (-1)
	bool before = false; // to make the ornament before the time specified
	wxString value = wxEmptyString; // generic value
	bool absolute_measureNr = false; // specify an absolute position ( after compilation of the measures )
	int mark_prefix = -1; // if there is a marker to specify the relative measure nr 
	int tInBeat = NULL_INT;
	int beat = NULL_INT;
	bool calculated=false;
	bool processed = false;
	bool tobedeleted = false;
};

// class to have a sorted list of musicXml events
/////////////////////////////////////////////////
class c_musicxmlevent
{
public:
	c_musicxmlevent()
	{
	}
	/*
	c_musicxmlevent(const c_musicxmlevent &musicxmlevent)
	{
		nr = musicxmlevent.nr; 
		starts = musicxmlevent.starts;
		stops = musicxmlevent.stops;
		nextNr = musicxmlevent.nextNr;
		partNr = musicxmlevent.partNr;
		staffNr = musicxmlevent.staffNr;
		voice = musicxmlevent.voice;
		original_measureNr = musicxmlevent.original_measureNr;
		start_measureNr = musicxmlevent.start_measureNr;
		start_t = musicxmlevent.start_t;
		start_order = musicxmlevent.start_order;
		stop_measureNr = musicxmlevent.stop_measureNr;
		stop_t = musicxmlevent.stop_t;
		stop_order = musicxmlevent.stop_order;
		pitch = musicxmlevent.pitch;
		duration = musicxmlevent.duration;
		repeat = musicxmlevent.repeat;
		division_measure = musicxmlevent.division_measure;
		division_beat = musicxmlevent.division_beat;
		division_quarter = musicxmlevent.division_quarter;
		fifths = musicxmlevent.fifths;
		visible = musicxmlevent.visible;
		played = musicxmlevent.played;
		//nrnote = musicxmlevent.nrnote;
		//pageNr = musicxmlevent.pageNr;
		//rect = musicxmlevent.rect;
		//turnPage = musicxmlevent.turnPage;
		end_score = musicxmlevent.end_score;

	}
	*/
	c_musicxmlevent(int ipartNr, int istaffNr, int ivoice, 
		int istart_measureNr, int ioriginal_measureNr, int istart_t, int istop_measureNr, int istop_t, 
		int ipitch, int idivision_measure, int idivision_beat, int idivision_quarter, 
		int irepeat, int iorder, int ififths)
	{
		partNr = ipartNr;
		staffNr = istaffNr;
		voice = ivoice;
		original_measureNr = ioriginal_measureNr;
		start_measureNr = istart_measureNr;
		start_t = istart_t;
		stop_measureNr = istop_measureNr;
		stop_t = istop_t;
		pitch = ipitch;
		duration = idivision_measure * (stop_measureNr - start_measureNr) + ( stop_t - start_t ) ;
		repeat = irepeat;
		division_measure = idivision_measure;
		division_beat = idivision_beat;
		division_quarter = idivision_quarter;
		start_order = iorder;
		fifths = ififths;
		nrExpresseurNote = -1;
	}
	int nr = 0; // index sorted with start_* as key
	int nextNr = 0; // next element just which starts after the stop ;

	int partNr = 0;
	int staffNr = 0;
	int original_measureNr = 1;
	int voice = 0;
	bool visible = true;
	bool played = true;

	int division_measure = 4*4; // nb division per measure
	int division_beat = 4; // nb division per beat
	int division_quarter = 1; // nb division per quarter
	int repeat = 0; // repeat of this event in the score 

	int start_measureNr = 1;
	int start_t = 0; // start of the note in divisions of the quarter
	int start_order = 0; // start_order of the lOrnaments for the same start_t
	int chord_order = 0;

	int stop_measureNr = 1;
	int stop_t = 0; // stop of the note in divisions of the quarter
	int stop_order = 0; 

	std::vector <int> starts; // index of musicxmlevent to start synchronously at the trigger-on
	std::vector <int> stops; // index of musicxmlevent to stop synchronously 
	int will_stop_index = -1 ; // in a starts musicxmlevent, link with the index of musicxmlevent to stop at the trigger-off
	int stop_index = -1; // in a starts musicxmlevent, link with the index of musicxmlevent to stop synchronously with the trigger-on 
	bool stop_orpheline = true; // true when the stop is not linked to a musicxmlevent

	int pitch = -1 ;
	int nuance = NULL_INT;
	int velocity = 64 ;
	int transpose = NULL_INT;
	int delay = 0;
	int duration = 0; // duration of the note in 2*2*3*divisions of the quarter
	bool tenuto = false;
	int accent = 0;
	bool crescendo = false;
	int dynamic = -1; // influence on the dynamic of velocity-input
	int random_delay = -1;
	int pedal = -1 ;
	int o_arpeggiate = 0 ;
	wxString text;
	wxString lua;
	int fifths = 0;
	int nb_ornaments = 0;
	int nr_ornament = 0;
	bool cross = false;
	bool staccato = false; // for display usage
	bool fermata = false; // for display usage
	bool breath_mark = false; // for display usage

	// graphical positions
	int pageNr = 0;
	wxRect rect;
	bool turnPage = false;
	bool end_score = false;
	int nrExpresseurNote = -1; // indice o the note in the Expresseur rythm-line
};

// class to have a list of arpeggiate
////////////////////////////////////
class c_arpeggiate_toapply
{
public:
	c_arpeggiate_toapply(int inr, bool idown, bool ibefore, c_musicxmlevent *imusicxmlevent)
	{
		nr = inr;
		down = idown;
		before = ibefore;
		musicxmlevent = imusicxmlevent;
	}
	int nr;
	bool down;
	bool before;
	c_musicxmlevent *musicxmlevent;
};

// class to have a list of pedal_bar
////////////////////////////////////
class c_pedal_bar_toapply
{
public:
	c_pedal_bar_toapply(int ivalue, c_musicxmlevent *imusicxmlevent)
	{
		value = ivalue;
		measureNr = imusicxmlevent->start_measureNr;
	}
	int value , measureNr;
};

// class for playback
/////////////////////
class c_eventPlayback
{
public:
	c_eventPlayback(	wxLongLong itime , int inr_device, int itype_msg, int ichannel, int ivalue1, int ivalue2)
	 { 
		time=itime ; nr_device=inr_device;  type_msg=itype_msg; channel=ichannel; value1=ivalue1;  value2=ivalue2;
	 }
	wxLongLong time ; int nr_device; int type_msg; int channel; int value1; int value2 ; 
};



// class to compile a musicXML score
////////////////////////////////////
class musicxmlcompile
{
public:
	musicxmlcompile();
	~musicxmlcompile();
	bool isModified = true;
	wxFileName loadTxtFile(wxFileName txtfile);
	void setNameFile(wxFileName txtfile, wxFileName xmlfile);
	wxFileName getNameTxtFile();
	wxFileName getNameXmlFile();
	bool loadXmlFile(wxString xmlfilein,bool useMarkFile = true);
	bool isOk(bool compiled_score = false);
	bool getPosEvent(int nrEvent, int *pageNr, wxRect *rect, bool *turn, int *nr_ornament); // , wxBitmap **mbitmap);
	int setPosEvent(int nrExpresseurNote, int pageNr, wxRect rect); //  , wxBitmap *mbitmap);
	void setMeasureTurnEvent(int nrMeasure, bool clean = false);
	int getPartNr(wxString spart, int *partNb = NULL);
	int pageToEventNr(int pageNr);
	int pointToEventNr(int pageNr , wxPoint p);
	int stringToEventNr(wxString s);
	bool getScorePosition( int nrEvent , int *absolute_measure_nr , int *measure_nr, int *repeat , int *beat, int *t , int *uid);
	bool getTrackDisplay(int nrTrack);
	bool getTrackPlay(int nrTrack);
	std::vector <int> getTracksPlay();
	std::vector <int> getTracksDisplay();
	int getTrackNr(wxString idTrack);
	wxString getTitle();
	std::vector <wxString> getTracksName();
	wxString getTrackName(int nrTrack);
	wxString getTrackId(int nrTrack);
	int getTracksCount();
	static void clearLuaScore();
	wxString pitchToString(int p);
	wxString pitchToString(std::vector <int> p);
	std::vector <int> stringToPitch(wxString s, int *nbChord);
	static std::vector <wxString> getListOrnament();
	wxString music_xml_complete_file;
	wxString music_xml_displayed_file;
	c_score_partwise *compiled_score = NULL; // score compiled, refer to score, measureList, partidToPlay, partidToPlay

	void initRecordPlayback();
	void initPlayback();
	void recordPlayback(wxLongLong time, int nr_device, int type_msg, int channel, int value1, int value2);
	bool playback();
	wxString getPlayback();

private:
	void dump_musicxmlevents();
	void fillStartStopNext();
	void compileScore(bool reanalyse);
	void writeMarks();
	void readMarks(bool full = true);
	bool readMarkLine(wxString line, wxString sectionName);
	void xmlLoad(wxString xmlfilein);
	void analyseMeasure(); // analyse the default repeat-sequence from "score" to "measureMark" and "markList"
	void analyseMeasureMarks();
	int getMarkNr(int measureNr);
	int getMeasureNr(int measureNr);
	void analyseList();
	void analyseNoteOrnaments(const c_note & note, int measureNumber, int t);
	void sortMeasureMarks();
	int getDivision(int measure_nr, int *division_quarter, int *division_measure);
	int compileNote(const c_part & part,const c_note & note, int measureNr, int originalMeasureNr, int t, int division_measure, int division_beat, int division_quarter, int repeat, int key_fifths);
	void compileTie(const c_part & part, const c_note & note, int *measureNr, int *t, int nbDivision);
	void compileMusicxmlevents();
	void pushLuaMusicxmlevents();
	void addOrnaments();
	void clearOrnaments();
	void singleOrnaments();
	//void createOrnament(c_ornament *ornament);
	void createImperativeOrnament(c_ornament *ornament);
	void createImperativePartOrnament(c_ornament *ornament,int nrPart,int nrStaff, int end_measure,int end_start_t, int division_measure, int division_beat , int division_quarter );
	void addGraces(wxString gracePitches, bool before, c_musicxmlevent *musicxmlevent);
	void addGraces(std::vector <int> gracePitches, bool before, c_musicxmlevent *musicxmlevent);
	void addOrnament(c_ornament *ornament, c_musicxmlevent *musicxmlevent,int nr_ornament);
	void compileCrescendo();
	void compileTransposition();
	void compileArppegio();
	void compilePedalBar();
	void removeExpresseurPart();
	void createListMeasures();
	void buildMeasures();
	void buildBars();
	void compilePlayedScore();
	void deleteBarLabel(c_measure *newMeasure);
	void addExpresseurPart();
	void compileExpresseurPart();
	void addNote(std::vector<c_measure>::iterator measure, bool after_measure , int from_t, int to_t, bool rest, bool tie_back, bool tie_next, bool *first_note , int * nrExpresseurNote, int nbOrnaments , wxString *text, bool *staccato, bool *fermata , bool *breath_mark , bool ternaire, bool cross, int* ituplet);
	void addSymbolNote(std::vector<c_measure>::iterator measure, bool after_measure,  int duration, bool rest, bool tie_back, bool tie_next, bool *first_note, int* nrExpresseurNote, int nbOrnaments, wxString *text , bool *staccato, bool *fermata, bool *breath_mark, bool ternaire,  bool cross, int *ituplet);
	void calculateDuration(int duration, int divisions, bool ternaire , int *duration_done , wxString *typeNote , int *dot , int *tuplet);
	wxFileName txtFile;
	wxFileName musicxmlFile;
	c_score_partwise *score; // original score
	std::vector <c_measureMark> lMeasureMarks; // list of markers : a marker is linked to a measure in the original score . e.g. marker[1]=measure(10), marker[2] =measure(14) , marker[3]=18
	std::vector <int> markList; // list of markers to play succesfully, refer to measureMark . e.g. 1,2,2,1 means play marker[1], marker[2] twice, and marker[1] to finish
	std::vector <int>  measureList; // list of measures to play, refer to markList and score. e.g. 10,11,12,13,14,15,16,17,14,15,16,17,10,11,12,13
	std::vector < c_ornament> lOrnaments; // list of lOrnaments to compile and add
	std::vector <c_musicxmlevent> lMusicxmlevents;
	std::vector <c_musicxmlevent> lOrnamentsMusicxmlevents;
	wxString grace;
	std::vector <c_arpeggiate_toapply> lArpeggiate_toapply;
	std::vector <c_pedal_bar_toapply> lPedal_bar_toapply;
	int nbEvents = 0;
	int nb_measure = 0;
	std::vector <int>  indexStop; // inex des lMusicxmlevents classes par stop


	std::vector <c_eventPlayback> lEventPlaybacks;
	wxLongLong t0Playback ;
	wxLongLong t0RecordPlayback ;
	wxLongLong timeReadPlayback;
	long nrEventPlayback = 0; // number of the event in the playback;
	int ratioPlayback = 100;


};


#endif
