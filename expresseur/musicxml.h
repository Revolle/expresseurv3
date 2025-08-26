
#ifndef DEF_MUSICXML

#define DEF_MUSICXML

#define MAX_SCORE_PART 64

class c_default_xy
{
public:
	c_default_xy() = default;
	c_default_xy(const c_default_xy &default_xy) = default;
	c_default_xy(wxXmlNode *xmlnode);
	void write_xy(wxFFile *f);
	int default_x = NULL_INT;
	int default_y = NULL_INT;
	int relative_x = NULL_INT;
	int relative_y = NULL_INT;
	bool usedxy = false;
};
class c_score_part
{
public:
	c_score_part() = default;
	c_score_part(wxString id , wxString part_name, wxString part_abbreviation);
	c_score_part(const c_score_part & score_part) = default;
	c_score_part(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString id;
	wxString part_name;
	wxString part_abbreviation;
	wxString part_alias;
	wxString part_alias_abbreviation;
	bool play = true;
	bool view = true;
	bool used = true;
};


class c_time
{
public:
	bool used = false;
	c_time() = default;
	c_time(const c_time& time) = default;
	c_time(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	int beats = NULL_INT;
	int beat_type = NULL_INT;
	wxString symbol = NULL_STRING;
};

class c_transpose
{
public:
	c_transpose() = default;
	c_transpose(const c_transpose& transpose) = default;
	c_transpose(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	int diatonic = NULL_INT;
	int chromatic = NULL_INT;
	int octave_change = NULL_INT;
	bool used = false;
};

class c_key
{
public:
	c_key() = default;
	c_key(const c_key& key) = default;
	c_key(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	int fifths = NULL_INT;
	wxString mode = NULL_STRING ;
	bool used = false;
};

class c_root
{
public:
	c_root() = default;
	c_root(const c_root& root) = default;
	c_root(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString root_step = NULL_STRING;
	int root_alter = NULL_INT;
	bool used = false;
};
class c_bass
{
public:
	c_bass() = default;
	c_bass(const c_bass& bass) = default;
	c_bass(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString bass_step = NULL_STRING;
	int bass_alter = NULL_INT;
	bool used = false;
};
class c_kind
{
public:
	c_kind() = default;
	c_kind(const c_kind& kind) = default;
	c_kind(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString text = NULL_STRING;
	wxString use_symbols = NULL_STRING;
	wxString value = NULL_STRING;
	bool used = false;
};
class c_harmony : c_default_xy
{
public:
	c_harmony() = default;
	c_harmony(const c_harmony& harmony) = default;
	c_harmony(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString function = NULL_STRING ;
	int inversion = NULL_INT;
	c_root root;
	c_bass bass  ;
	c_kind kind  ;
	bool used = false;
};

class c_backup
{
public:
	c_backup() = default;
	c_backup(const c_backup& backup) = default;
	c_backup(wxXmlNode *xmlnode);
	void compile(bool twelved);
	void divisionsAlign(int ratio);
	void write(wxFFile *f);
	int duration = NULL_INT;
	bool used = false;
};
class c_forward
{
public:
	c_forward() = default;
	c_forward(const c_forward& forward) = default;
	c_forward(wxXmlNode *xmlnode);
	void compile(bool twelved);
	void divisionsAlign(int ratio);
	void write(wxFFile *f);
	int duration = NULL_INT;
	bool used = false;
};
class c_repeat
{
public:
	c_repeat() = default;
	c_repeat(const c_repeat& repeat) = default;
	c_repeat(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString direction = NULL_STRING;
	wxString times = NULL_STRING;
	bool used = false;
};
class c_ending : c_default_xy
{
public:
	c_ending() = default;
	c_ending(const c_ending& ending) = default;
	c_ending(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	int end_length = NULL_INT;
	wxString number = NULL_STRING;
	wxString type = NULL_STRING;
	wxString value = NULL_STRING;
	bool used = false;
};
class c_pedal :c_default_xy
{
public:
	c_pedal() = default;
	c_pedal(const c_pedal &pedal) = default;
	c_pedal(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString type = NULL_STRING;
	wxString line = NULL_STRING;
	wxString sign = NULL_STRING;
	bool used = false;
};
class c_octave_shift :c_default_xy
{
public:
	c_octave_shift() = default;
	c_octave_shift(const c_octave_shift &octave_shift) = default;
	c_octave_shift(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString type = NULL_STRING;
	int number = NULL_INT;
	int size = NULL_INT;
	bool used = false;
};
class c_rehearsal :c_default_xy
{
public:
	c_rehearsal() = default;
	c_rehearsal(const c_rehearsal &rehearsal) = default;
	c_rehearsal(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString value;
	bool used = false;
};
class c_words :c_default_xy
{
public:
	c_words() = default ;
	c_words(const c_words &words) = default;
	c_words(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString value;
	bool used = false;
};
class c_wedge :c_default_xy
{
public:
	c_wedge() = default ;
	c_wedge(const c_wedge &wedge) = default ;
	c_wedge(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString type = NULL_STRING;
	int spread = NULL_INT ;
	bool used = false;
};
class c_coda :c_default_xy
{
public:
	c_coda() = default ;
	c_coda(c_coda const &coda) = default ;
	c_coda(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	bool used = false;
};
class c_segno :c_default_xy
{
public:
	c_segno() = default;
	c_segno(c_segno const &segno) = default ;
	c_segno(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	bool used = false;
};
class c_dynamics : c_default_xy
{
public:
	c_dynamics() = default;
	c_dynamics(c_dynamics const& dynamics) = default;
	c_dynamics(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString dynamic;
	wxString placement;
	bool used = false;
};
enum { t_pedal, t_octave_shift, t_rehearsal, t_wedge, t_coda, t_dynamics, t_segno , t_words };
class c_direction_type
{
public:
	c_direction_type() = default;
	c_direction_type(c_direction_type const &direction_type) = default;
	c_direction_type(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	c_pedal pedal;
	c_octave_shift octave_shift;
	c_rehearsal rehearsal;
	c_wedge wedge;
	c_coda coda;
	c_segno segno;
	c_words words;
	c_dynamics dynamics;
	bool tobedeleted = false;
	bool used = false;
};

class c_sound
{
public:
	c_sound() = default;
	c_sound(c_sound const& sound) = default;
	c_sound(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString name;
	wxString value;
	bool used = false;
};

class c_direction
{
public:
	c_direction() = default ;
	c_direction(c_direction const& direction) = default;
	c_direction(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString placement = NULL_STRING;
	wxString directive = NULL_STRING;
	std::vector<c_direction_type> direction_types;
	std::vector<c_sound> sounds;
	bool used = false;
};

class c_clef
{
public:
	c_clef() = default;
	c_clef(const c_clef& clef) = default;
	c_clef(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	int number = NULL_INT;
	wxString sign = NULL_STRING;
	int line = NULL_INT;
	int clef_octave_change = NULL_INT;
	bool used = false;
};

class c_barline
{
public:
	c_barline() = default;
	c_barline(const c_barline& barline) = default;
	c_barline(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	c_repeat repeat ;
	c_ending ending ;
	wxString location = NULL_STRING;
	wxString bar_style = NULL_STRING ;
	bool segno = false;
	bool coda = false ;
	bool fermata = false;
	bool used = false;
};

class c_staff_details
{
public: 
	c_staff_details() = default;
	c_staff_details(const c_staff_details& staff_details) = default;
	c_staff_details(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	int staff_lines = NULL_INT;
	int staff_size = NULL_INT;
	bool used = false;
};
class c_measure;
class c_attributes
{
public:
	c_attributes() = default;
	c_attributes(const c_attributes& attributes) = default;
	c_attributes(const c_attributes & attributes , bool withContent);
	c_attributes(wxXmlNode *xmlnode);
	void compile(bool twelved, c_measure *measure);
	void divisionsAlign();
	void write(wxFFile *f);
	c_key key;
	c_time mtime;
	c_staff_details staff_details;
	std::vector<c_clef> clefs;
	c_transpose transpose;
	int divisions = NULL_INT;
	int staves = NULL_INT;
	bool used = false;
};

class c_pitch
{
public:
	c_pitch() = default;
	c_pitch(const c_pitch& pitch) = default;
	c_pitch(wxXmlNode *xmlnode , bool unpitched = false);
	void compile();
	void write(wxFFile *f);
	int toMidiPitch() const;
	static int shiftPitch(int p, int up, int fifths);
	bool isEqual(const c_pitch & pitch) const;
	wxString step;
	int octave = NULL_INT;
	int alter = NULL_INT;
	bool unpitched = false;
	int transpose = 0;
	bool used = false;
};
class c_rest
{
public:
	c_rest() = default;
	c_rest(const c_rest& rest) = default;
	c_rest(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString measure = NULL_STRING;
	wxString display_step = NULL_STRING;
	int display_octave = NULL_INT;
	bool used = false;
};
class c_time_modification
{
public:
	c_time_modification() = default;
	c_time_modification(const c_time_modification& time_modification) = default;
	c_time_modification(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	int actual_notes = NULL_INT;
	int normal_notes = NULL_INT;
	bool used = false;
};
class c_lyric : c_default_xy
{
public:
	c_lyric() = default;
	c_lyric(const c_lyric& lyric) = default;
	c_lyric(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString placement = NULL_STRING;
	int number = 1 ;
	wxString name = NULL_STRING;
	wxString text = NULL_STRING;
	wxString syllabic = NULL_STRING;
	wxString extend_type = NULL_STRING;
	bool used = false;
};

class c_beam
{
public:
	c_beam() = default;
	c_beam(const c_beam& beam) = default;
	c_beam(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	int number = NULL_INT ;
	wxString value;
	bool used = true;
};

class c_articulations
{
public:
	c_articulations() = default;
	c_articulations(const c_articulations& articulations) = default;
	c_articulations(wxXmlNode* xmlnode);
	void write(wxFFile *f);
	std::vector<wxString> articulations;
	std::vector<wxString> placements;
	std::vector<int> default_xs;
	std::vector<int> default_ys;
	bool used = false;
};
class c_ornaments
{
public:
	c_ornaments(wxXmlNode *xmlnode);
	c_ornaments(const c_ornaments& ornaments) = default;
	c_ornaments() = default;
	void write(wxFFile *f);
	std::vector<wxString> lOrnaments;
	std::vector<wxString> placements;
	std::vector<int> default_xs;
	std::vector<int> default_ys;
	bool used = false;
};
class c_arpeggiate
{
public:
	c_arpeggiate() = default;
	c_arpeggiate(const c_arpeggiate& arpeggiate) = default;
	c_arpeggiate(wxXmlNode *xmlNode);
	void write (wxFFile *f);
	int number = NULL_INT;
	wxString direction = NULL_STRING ;
	bool used = false;
};
class c_fermata : c_default_xy
{
public:
	c_fermata() = default;
	c_fermata(const c_fermata& fermata) = default;
	c_fermata(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString type = NULL_STRING;
	wxString placement = NULL_STRING;
	bool used = false;
};
class c_glissando : c_default_xy
{
public:
	c_glissando() = default;
	c_glissando(const c_glissando& glissando) = default;
	c_glissando(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString type = NULL_STRING;
	int number = NULL_INT;
	bool used = false;
};
class c_slide : c_default_xy
{
public:
	c_slide() = default;
	c_slide(const c_slide& slide) = default;
	c_slide(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString type = NULL_STRING;
	int number = NULL_INT;
	bool used = false;
};
class c_slur : c_default_xy
{
public:
	c_slur() = default;
	c_slur(const c_slur& slur) = default;
	c_slur(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString type;
	int number = NULL_INT;
	wxString placement = NULL_STRING;
	bool used = false;
};

class c_tied
{
public:
	c_tied() = default;
	c_tied(const c_tied& tied) = default;
	c_tied(wxXmlNode *xmlnode);
	void complete(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	bool start = false;
	bool stop = false;
	bool used = false;
};
class c_tie
{
public:
	c_tie() = default;
	c_tie(const c_tie& tie) = default;
	c_tie(wxXmlNode *xmlnode);
	void complete(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	bool start = false;
	bool stop = false ;
	bool compiled = false;
	bool used = false;
};
class c_tuplet : c_default_xy
{
public:
	c_tuplet() = default;
	c_tuplet(const c_tuplet& tuplet) = default;
	c_tuplet(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString type;
	int number = NULL_INT;
	wxString placement = NULL_STRING;
	bool used = false;
};
class c_notations
{
public:
	c_notations() = default ;
	c_notations(const c_notations& notations) = default;
	c_notations(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	c_arpeggiate arpeggiate;
	c_articulations articulations;
	c_ornaments lOrnaments ;
	c_dynamics dynamics ;
	c_fermata fermata ;
	c_glissando glissando ;
	c_slide slide ;
	std::vector<c_slur> slurs ;
	c_tied tied ;
	c_tuplet tuplet ;
	bool used = false;
};
class c_note : c_default_xy
{
public:
	c_note() = default;
	c_note(const c_note& note) = default;
	c_note(wxXmlNode *xmlnode );
	void compile(int partNr , bool twelved = false);
	void divisionsAlign(int ratio);
	void write(wxFFile *f);
	c_pitch pitch;
	c_rest rest ;
	c_tie tie;
	c_time_modification time_modification;
	c_notations notations ;
	std::vector<c_lyric> lyrics;
	std::vector<c_beam> beams;
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
	bool used = false;
};

class c_measure_sequence
{
public:
	c_measure_sequence() = default;
	c_measure_sequence(const c_measure_sequence& measure_sequence) = default;
	c_measure_sequence(c_attributes const& attributes);
	c_measure_sequence(c_note const& note);
	c_measure_sequence(c_harmony const& harmony);
	c_measure_sequence(c_backup const& backup);
	c_measure_sequence(c_forward const& forward);
	c_measure_sequence(c_barline const& barline);
	c_measure_sequence(c_direction const& direction);
	c_measure_sequence(const c_measure_sequence & measure_sequence, bool withContent);
	void compile(int partNr , bool twelved , c_measure *measure);
	void divisionsAlign(int ratio);
	void write(wxFFile *f);
	c_attributes attributes;
	c_note note;
	c_harmony harmony;
	c_backup backup;
	c_forward forward;
	c_barline barline;
	c_direction direction;
	bool tobedeleted = false; 
	bool used = false;
};

class c_measure
{
public:
	c_measure() = default;
	c_measure(const c_measure& measure) = default;
	c_measure(int number, int width);
	c_measure(const c_measure &measure, bool withContent );
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
	std::vector<c_measure_sequence> measure_sequences;
	bool used = false;
};

class c_part
{
public:
	c_part() = default;
	c_part(const c_part& part) = default;
	c_part(wxString id);
	c_part(const c_part &part, bool withMeasures);
	c_part(wxXmlNode *xmlnode);
	void compile(int nr, bool twelved = false);
	void divisionsAlign();
	void write(wxFFile *f, bool layout);
	wxString id = NULL_STRING;
	int idNr = 0;
	int partNr = 0;
	std::vector<c_measure> measures;
	bool used = false;
};

class c_part_list
{
public:
	c_part_list() = default;
	c_part_list(const c_part_list& part_list) = default;
	c_part_list(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	std::vector<c_score_part> score_parts;
	bool used = false;
};
class c_work
{
public:
	c_work() = default;
	c_work(const c_work& work) = default;
	c_work(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	wxString work_title;
	bool used = false;
};
class c_scaling
{
public:
	c_scaling() = default;
	c_scaling(const c_scaling& scaling) = default;
	void read(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	float millimeters = 7.0 ;
	float tenths = 40.0;
	bool used = false;
};
class c_page_layout
{
public:
	c_page_layout() = default;
	c_page_layout(const c_page_layout& page_layout) = default;
	void read(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	float page_height = 1000.0;
	float page_width = 1000.0;
	float margin = 5.0;
	bool used = false;
};
class c_defaults
{
public:
	c_defaults() = default;
	c_defaults(const c_defaults& defaults) = default;
	c_defaults(wxXmlNode *xmlnode);
	void write(wxFFile *f);
	c_scaling scaling;
	c_page_layout page_layout;
	bool used = false;
};
class c_score_partwise
{
public:
	c_score_partwise() = default;
	c_score_partwise(const c_score_partwise&) = default;
	c_score_partwise(const c_score_partwise& score_partwise, bool withMeasures);
	c_score_partwise(wxXmlNode *xmlnode);
	void write(wxString filename, bool layout);
	c_work work;
	c_defaults defaults;
	c_part_list part_list;
	std::vector<c_part> parts;
	void compile( bool twelved = false);
	bool already_twelved = false;
	bool used = false;
};

#endif
