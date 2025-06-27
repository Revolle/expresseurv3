
#ifndef DEF_MUSICXML

#define DEF_MUSICXML

#define MAX_SCORE_PART 64

class c_default_xy
{
public:
	c_default_xy();
	~c_default_xy() {};
	c_default_xy(const c_default_xy &default_xy);
	c_default_xy(wxXmlNode *xmlnode);
	void write_xy(wxFFile *f);
	int default_x = NULL_INT;
	int default_y = NULL_INT;
	int relative_x = NULL_INT;
	int relative_y = NULL_INT;
};
class c_score_part
{
public:
	c_score_part();
	~c_score_part() {};
	c_score_part(wxString id , wxString part_name, wxString part_abbreviation);
	c_score_part(const c_score_part & score_part);
	c_score_part(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString id;
	wxString part_name;
	wxString part_abbreviation;
	wxString part_alias;
	wxString part_alias_abbreviation;
	bool play = true;
	bool view = true;
};
WX_DECLARE_LIST(c_score_part, l_score_part);

class c_time
{
public:
	c_time();
	~c_time() {};
	c_time(const c_time & time);
	c_time(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	int beats = NULL_INT;
	int beat_type = NULL_INT;
	wxString symbol = NULL_STRING;
};

class c_transpose
{
public:
	c_transpose();
	~c_transpose() {};
	c_transpose(const c_transpose & transpose);
	c_transpose(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	int diatonic = NULL_INT;
	int chromatic = NULL_INT;
	int octave_change = NULL_INT;
};

class c_key
{
public:
	c_key();
	~c_key() {};
	c_key(const c_key & key);
	c_key(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	int fifths = NULL_INT;
	wxString mode = NULL_STRING ;
};

class c_root
{
public:
	c_root();
	~c_root() {};
	c_root(const c_root & root);
	c_root(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString root_step = NULL_STRING;
	int root_alter = NULL_INT;
};
class c_bass
{
public:
	c_bass();
	~c_bass() {};
	c_bass(const c_bass &bass);
	c_bass(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString bass_step = NULL_STRING;
	int bass_alter = NULL_INT;
};
class c_kind
{
public:
	c_kind();
	~c_kind() {};
	c_kind(const c_kind &kind);
	c_kind(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString text = NULL_STRING;
	wxString use_symbols = NULL_STRING;
	wxString value = NULL_STRING;
};
class c_harmony : c_default_xy
{
public:
	c_harmony();
	c_harmony(const c_harmony &harmony);
	c_harmony(wxXmlNode *xmlnode);
	~c_harmony() {
		delete root;
		delete bass;
		delete kind;
	};
	void write(wxFFile *f);
	wxString function = NULL_STRING ;
	int inversion = NULL_INT;
	c_root *root = NULL;
	c_bass *bass = NULL ;
	c_kind *kind = NULL ;
};

class c_backup
{
public:
	c_backup();
	~c_backup() {};
	c_backup(const c_backup &backup);
	c_backup(wxXmlNode *xmlnode);
	void compile(bool twelved);
	void divisionsAlign(int ratio);
	void write(wxFFile *f);
	int duration = NULL_INT;
};
class c_forward
{
public:
	c_forward();
	~c_forward() {};
	c_forward(const c_forward &forward);
	c_forward(wxXmlNode *xmlnode);
	void compile(bool twelved);
	void divisionsAlign(int ratio);
	void write(wxFFile *f);
	int duration = NULL_INT;
};
class c_repeat
{
public:
	c_repeat();
	~c_repeat() {};
	c_repeat(const c_repeat & repeat);
	c_repeat(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString direction = NULL_STRING;
	wxString times = NULL_STRING;
};
class c_ending : c_default_xy
{
public:
	c_ending();
	~c_ending() {};
	c_ending(const c_ending &ending);
	c_ending(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	int end_length = NULL_INT;
	wxString number = NULL_STRING;
	wxString type = NULL_STRING;
	wxString value = NULL_STRING;
};
class c_pedal :c_default_xy
{
public:
	c_pedal();
	~c_pedal() {};
	c_pedal(const c_pedal &pedal);
	c_pedal(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString type = NULL_STRING;
	wxString line = NULL_STRING;
	wxString sign = NULL_STRING;
};
class c_octave_shift :c_default_xy
{
public:
	c_octave_shift();
	~c_octave_shift() {};
	c_octave_shift(const c_octave_shift &octave_shift);
	c_octave_shift(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString type = NULL_STRING;
	int number = NULL_INT;
	int size = NULL_INT;
};
class c_rehearsal :c_default_xy
{
public:
	c_rehearsal();
	~c_rehearsal() {};
	c_rehearsal(const c_rehearsal &rehearsal);
	c_rehearsal(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString value;
};
class c_words :c_default_xy
{
public:
	c_words();
	~c_words() {};
	c_words(const c_words &words);
	c_words(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString value;
};
class c_wedge :c_default_xy
{
public:
	c_wedge();
	~c_wedge() {};
	c_wedge(const c_wedge &wedge);
	c_wedge(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString type = NULL_STRING;
	int spread = NULL_INT ;
};
class c_coda :c_default_xy
{
public:
	c_coda();
	~c_coda() {};
	c_coda(c_coda const &coda);
	c_coda(wxXmlNode *xmlnode);
	void write(wxFFile *f);
};
class c_segno :c_default_xy
{
public:
	c_segno();
	~c_segno() {};
	c_segno(c_segno const &segno);
	c_segno(wxXmlNode *xmlnode);
	void write(wxFFile *f);
};
class c_dynamics : c_default_xy
{
public:
	c_dynamics();
	~c_dynamics() {};
	c_dynamics(const c_dynamics &dynamics);
	c_dynamics(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString dynamic;
	wxString placement;
};
enum { t_pedal, t_octave_shift, t_rehearsal, t_wedge, t_coda, t_dynamics, t_segno , t_words };
class c_direction_type
{
public:
	c_direction_type();
	~c_direction_type() {
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
			default:  wxASSERT(false);  break;
			}
		}
	}
	c_direction_type(c_direction_type const &direction_type);
	c_direction_type(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	void *pt = NULL;
	int type = NULL_INT;
};
WX_DECLARE_LIST(c_direction_type, l_direction_type);
class c_sound
{
public:
	c_sound();
	~c_sound() {};
	c_sound(c_sound const &sound);
	c_sound(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString name;
	wxString value;
};
WX_DECLARE_LIST(c_sound, l_sound);

class c_direction
{
public:
	c_direction();
	~c_direction() {
		direction_types.DeleteContents(true);
		direction_types.Clear();
		sounds.DeleteContents(true);
		sounds.Clear();
	};
	c_direction(const c_direction &direction);
	c_direction(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString placement = NULL_STRING;
	wxString directive = NULL_STRING;
	l_direction_type direction_types;
	l_sound sounds;
};

class c_clef
{
public:
	c_clef();
	~c_clef() {};
	c_clef(const c_clef & clef);
	c_clef(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	int number = NULL_INT;
	wxString sign = NULL_STRING;
	int line = NULL_INT;
	int clef_octave_change = NULL_INT;
};
WX_DECLARE_LIST(c_clef, l_clef);

class c_barline
{
public:
	c_barline();
	c_barline(const c_barline & barline);
	c_barline(wxXmlNode *xmlnode);
	~c_barline() {
		delete repeat;
		delete ending;
	}
	void write(wxFFile *f);
	c_repeat *repeat = NULL;
	c_ending *ending = NULL;
	wxString location = NULL_STRING;
	wxString bar_style = NULL_STRING ;
	bool segno = false;
	bool coda = false ;
	bool fermata = false;
};

class c_staff_details
{
public: 
	c_staff_details();
	~c_staff_details() {};
	c_staff_details(const c_staff_details & staff_details);
	c_staff_details(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	int staff_lines = NULL_INT;
	int staff_size = NULL_INT;
};
class c_measure;
class c_attributes
{
public:
	c_attributes();
	c_attributes(const c_attributes & attributes , bool withContent = true);
	c_attributes(wxXmlNode *xmlnode);
	c_attributes::~c_attributes()
	{
		clefs.DeleteContents(true);
		clefs.Clear();
		delete key;
		delete mtime;
		delete staff_details;
		delete transpose;
	}
	void compile(bool twelved, c_measure *measure);
	void divisionsAlign();
	void write(wxFFile *f);
	l_clef clefs;
	c_key* key = NULL;
	c_time *mtime = NULL;
	c_staff_details *staff_details = NULL;
	c_transpose *transpose = NULL;
	int divisions = NULL_INT;
	int staves = NULL_INT;
};

class c_pitch
{
public:
	c_pitch();
	~c_pitch() {};
	c_pitch(const c_pitch & pitch);
	c_pitch(wxXmlNode *xmlnode , bool unpitched = false);
	void compile();
	void write(wxFFile *f);
	int toMidiPitch();
	static int shiftPitch(int p, int up, int fifths);
	bool isEqual(const c_pitch & pitch);
	wxString step;
	int octave = NULL_INT;
	int alter = NULL_INT;
	bool unpitched = false;
	int transpose = 0;
};
class c_rest
{
public:
	c_rest();
	~c_rest() {};
	c_rest(const c_rest & rest);
	c_rest(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString measure = NULL_STRING;
	wxString display_step = NULL_STRING;
	int display_octave = NULL_INT;
};
class c_time_modification
{
public:
	c_time_modification();
	~c_time_modification() {};
	c_time_modification(const c_time_modification & time_modification);
	c_time_modification(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	int actual_notes = NULL_INT;
	int normal_notes = NULL_INT;
};
class c_lyric : c_default_xy
{
public:
	c_lyric();
	~c_lyric() {};
	c_lyric(const c_lyric & lyric);
	c_lyric(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString placement = NULL_STRING;
	int number = 1 ;
	wxString name = NULL_STRING;
	wxString text = NULL_STRING;
	wxString syllabic = NULL_STRING;
	wxString extend_type = NULL_STRING;
};
WX_DECLARE_LIST(c_lyric, l_lyric);

class c_beam
{
public:
	c_beam();
	~c_beam() {};
	c_beam(const c_beam &beam);
	c_beam(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	int number = NULL_INT ;
	wxString value;
};
WX_DECLARE_LIST(c_beam, l_beam);

class c_articulations
{
public:
	c_articulations();
	~c_articulations() {};
	c_articulations(const c_articulations & articulations);
	c_articulations(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxArrayString articulations;
	wxArrayString placements;
	wxArrayInt default_xs;
	wxArrayInt default_ys;
};
class c_ornaments
{
public:
	c_ornaments();
	~c_ornaments() {};
	c_ornaments(const c_ornaments & lOrnaments);
	c_ornaments(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxArrayString lOrnaments;
	wxArrayString placements;
	wxArrayInt default_xs;
	wxArrayInt default_ys;
};
class c_arpeggiate
{
public:
	c_arpeggiate();
	~c_arpeggiate() {};
	c_arpeggiate(const c_arpeggiate & arpeggiate);
	c_arpeggiate(wxXmlNode *xmlNode);
	void write(wxFFile *f);
	int number = NULL_INT;
	wxString direction = NULL_STRING ;
};
class c_fermata : c_default_xy
{
public:
	c_fermata();
	~c_fermata() {};
	c_fermata(const c_fermata &fermata);
	c_fermata(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString type = NULL_STRING;
	wxString placement = NULL_STRING;
};
class c_glissando : c_default_xy
{
public:
	c_glissando();
	~c_glissando() {};
	c_glissando(const c_glissando &glissando);
	c_glissando(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString type = NULL_STRING;
	int number = NULL_INT;
};
class c_slide : c_default_xy
{
public:
	c_slide();
	~c_slide() {};
	c_slide(const c_slide &slide);
	c_slide(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString type = NULL_STRING;
	int number = NULL_INT;
};
class c_slur : c_default_xy
{
public:
	c_slur();
	~c_slur() {};
	c_slur(const c_slur &slur);
	c_slur(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString type;
	int number = NULL_INT;
	wxString placement = NULL_STRING;
};
WX_DECLARE_LIST(c_slur, l_slur);

class c_tied
{
public:
	c_tied();
	~c_tied() {};
	c_tied(const c_tied &tied);
	c_tied(wxXmlNode *xmlnode);
	void complete(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	bool start = false;
	bool stop = false;
};
class c_tie
{
public:
	c_tie();
	~c_tie() {};
	c_tie(const c_tie &tie);
	c_tie(wxXmlNode *xmlnode);
	void complete(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	bool start = false;
	bool stop = false ;
	bool compiled = false;
};
class c_tuplet : c_default_xy
{
public:
	c_tuplet();
	~c_tuplet() {};
	c_tuplet(const c_tuplet & tuplet);
	c_tuplet(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString type;
	int number = NULL_INT;
	wxString placement = NULL_STRING;
};
class c_notations
{
public:
	c_notations();
	c_notations(const c_notations & notations);
	c_notations(wxXmlNode *xmlnode);
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
		slurs.DeleteContents(true);
		slurs.Clear();
	};
	void write(wxFFile *f);
	c_arpeggiate *arpeggiate = NULL;
	c_articulations *articulations = NULL;
	c_ornaments *lOrnaments = NULL;
	c_dynamics *dynamics = NULL;
	c_fermata *fermata = NULL;
	c_glissando *glissando = NULL;
	c_slide *slide = NULL;
	c_tied *tied = NULL;
	c_tuplet *tuplet = NULL;
	l_slur slurs;
};
class c_note : c_default_xy
{
public:
	c_note();
	c_note(const c_note & note);
	c_note(wxXmlNode *xmlnode );
	c_note::~c_note()
	{
		delete pitch;
		delete rest;
		delete tie;
		delete time_modification;
		delete notations;
		lyrics.DeleteContents(true);
		lyrics.Clear();
		beams.DeleteContents(true);
		beams.Clear();
	};
	void compile(int partNr , bool twelved = false);
	void divisionsAlign(int ratio);
	void write(wxFFile *f);
	c_pitch *pitch = NULL;
	c_rest *rest = NULL;
	c_tie *tie = NULL;
	c_time_modification *time_modification = NULL;
	c_notations *notations = NULL;
	l_lyric lyrics;
	l_beam beams;
	int duration = NULL_INT;
	int voice = NULL_INT;
	wxString mtype = NULL_STRING  ;
	wxString stem = NULL_STRING ;
	wxString accidental = NULL_STRING;
	wxString notehead = NULL_STRING;
	wxString notecolor = NULL_STRING;
	int staff = NULL_INT ;
	int partNr = 0;
	bool chord = false;
	int chord_order = 0;
	int dots = 0;
	bool grace = false;
	bool cue = false;
};

enum { t_attributes , t_note, t_harmony, t_backup, t_forward, t_barline, t_direction };
class c_measure_sequence
{
public:
	c_measure_sequence();
	c_measure_sequence(const c_measure_sequence & measure_sequence, bool withContent = true);
	c_measure_sequence(void *pt, int type);
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
	};
	void compile(int partNr , bool twelved , c_measure *measure);
	void divisionsAlign(int ratio);
	void write(wxFFile *f);
	int type = NULL_INT ;
	void *pt = NULL ;
};
WX_DECLARE_LIST(c_measure_sequence, l_measure_sequence);
class c_measure
{
public:
	c_measure();
	c_measure::~c_measure()
	{
		measure_sequences.DeleteContents(true);
		measure_sequences.Clear();
	};
	c_measure(int number, int width);
	c_measure(const c_measure &measure, bool withContent = true );
	c_measure(wxXmlNode *xmlnode);
	void write(wxFFile *f, bool layout);
	void compile(c_measure *previous_measure,int partNr, bool twelved = false);
	void divisionsAlign();
	int divisions = NULL_INT;
	int division_quarter = NULL_INT;
	int division_beat = NULL_INT;
	int division_measure = NULL_INT;
	int beat_type = NULL_INT;
	int beats = NULL_INT;
	int key_fifths = NULL_INT;
	int number = NULL_INT;
	int width = NULL_INT;
	int original_number; // original number of the measure
	int repeat = 0; // internal sequence number of the repetition of this measure
	l_measure_sequence measure_sequences;
};
WX_DECLARE_LIST(c_measure, l_measure);

class c_part
{
public:
	c_part();
	c_part::~c_part()
	{
		measures.DeleteContents(true);
		measures.Clear();
	};
	c_part(wxString id);
	c_part(const c_part &part, bool withMeasures = true);
	c_part(wxXmlNode *xmlnode);
	void compile(int nr, bool twelved = false);
	void divisionsAlign();
	void write(wxFFile *f, bool layout);
	wxString id = NULL_STRING;
	int idNr = 0;
	int partNr = 0;
	l_measure measures;
};
WX_DECLARE_LIST(c_part, l_part);

class c_part_list
{
public:
	c_part_list();
	c_part_list(const c_part_list &parlist);
	c_part_list(wxXmlNode *xmlnode);
	c_part_list::~c_part_list()
	{
		score_parts.DeleteContents(true);
		score_parts.Clear();
	};
	void write(wxFFile *f);
	l_score_part score_parts;
};
class c_work
{
public:
	c_work();
	~c_work() {};
	c_work(const c_work &work);
	c_work(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString work_title;
};
class c_scaling
{
public:
	c_scaling();
	~c_scaling() {};
	c_scaling(const c_scaling &scaling);
	void read(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	float millimeters = 7.0 ;
	float tenths = 40.0;
};
class c_page_layout
{
public:
	c_page_layout();
	~c_page_layout() {};
	c_page_layout(const c_page_layout &page_layout);
	void read(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	float page_height = 1000.0;
	float page_width = 1000.0;
	float margin = 5.0;
};
class c_defaults
{
public:
	c_defaults();
	~c_defaults() {};
	c_defaults(const c_defaults &work);
	c_defaults(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	c_scaling scaling;
	c_page_layout page_layout;
};
class c_score_partwise
{
public:
	c_score_partwise();
	c_score_partwise(const c_score_partwise &score_partwise, bool withMeasures = true);
	c_score_partwise(wxXmlNode *xmlnode);
	c_score_partwise::~c_score_partwise()
	{
		parts.DeleteContents(true);
		parts.Clear();
		delete work;
		delete part_list;
	};
	void write(wxString filename, bool layout);
	l_part parts;
	c_work *work = NULL;
	c_part_list* part_list = NULL;
	c_defaults defaults;
	void compile( bool twelved = false);
	bool already_twelved = false;
};

#endif
