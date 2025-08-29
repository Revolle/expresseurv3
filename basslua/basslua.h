// Midi-IN Event LUA processor
//
// The events are processed through LUA script
//
// process events come from 
//    - MIDI-IN real-time devices
//    - internal timer
//    - external User-interface
// update : 31 / 10 / 2016 18 : 00
//////////////////////////////////////////////



#define moduleGlobal "_G"

// LUA-script-module loaded by basslua

// LUA tables in the LUA-module loaded by basslua, to be driven by the GUI
#define tableActions "actions"
#define tableCurves "curves"
#define tablePoints "points"
#define tableTracks "tracks"
#define tableValues "values"
#define tableInfo "info"
// LUA fields of the LUA tables, in the LUA-module loaded by basslua, to be driven by the GUI
#define fieldName "name"
#define fieldNrTrack "nrTrack"
#define fieldCallFunction "callFunction"
#define fieldCallScore "callScore"
#define fieldCallChord "callChord"
#define fieldCallMode "call"
#define fieldValue "value"
#define fielDefaultValue "defaultValue"
#define fieldIcone "icone"
#define fieldHelp "help"
#define fieldShortcut "shortcut"
#define fieldStatus "status"
#define fieldAction "action"
#define modeNil 0
#define modeScore 1
#define modeChord 2
// action flags in basslua_table
#define tableGetKeyValue ( 0x1 << 0 )
#define tableSetKeyValue ( 0x1 << 1 )
#define tableCallKeyFunction ( 0x1 << 2 )
#define tableCallTableFunction ( 0x1 << 3 )
#define tableNilKeyValue ( 0x1 << 4 )

// LUA-C-module "luabass" ( for Midi-out ), to be driven by the GUI. This module is loaded by defaut by basslua.
#define moduleLuabass "luabass"

// LUA-script-module "luascore.lua" ( for score structured ), to be driven by the GUI. This module is loade by defaut by basslua.
#define moduleScore "luascore"
#define functionScoreInitScore "initScore"
#define functionScoreFinishScore "finishScore"
#define functionScoreSetNrEvent "setNrEvent"
#define functionScoreAddEvent "addEvent"
#define functionScoreAddEventStarts "addEventStarts"
#define functionScoreAddEventStops "addEventStops"
#define functionScoreAddTrack "addTrack"
#define functionScoreGetPosition "getPosition"
#define functionScoreGotoNrEvent "gotoNrEvent"

// LUA-script-module "luachord.lua" ( for text with chords ), to be driven by the GUI. This module is loade by defaut by basslua.
#define moduleChord "luachord"
#define functionChordGetRecognizedScore "getRecognizedScore"
#define functionChordGetPosition "getPosition"
#define functionChordSetPosition "setPosition"
#define functionChordSetNrEvent "setNrEvent"
#define functionChordSetScore "setScore"
#define functionScorePlay "play"

// LUA constants in LUA midi modules
#define NOTEONOFF		7  // non MIDI message : for selectors only
#define NOTEOFF			8  // 0x8
#define NOTEON			9  // 0x9
#define KEYPRESSURE		10 // 0xA
#define CONTROL			11 // 0xB
#define PROGRAM			12 // 0xC
#define CHANNELPRESSURE 13 // 0xD
#define PITCHBEND		14 // 0xE
#define SYSTEMCOMMON	15 // 0xF
#define SYSEX 0xF0
#define ACTIVESENSING 0xFE
#define CLOCK 0xF8

// label for the event which are logged for the shortcut GUI
#define snoteonoff  "note-on&off"
#define snoteononly "note-on"
#define scontrol    "control"
#define sprogram    "program"


// start and initialize the LUA thread, using the LUA script fname 
typedef void(*voidcallback) (double time , int nr_device , int type_msg , int channel , int value1 , int value2 , bool isProcessed );
bool basslua_open(const char* fluaname, const char* luaparam, bool reset, long datefluaname, voidcallback fcallback , const char *logpath, const char *luapath, bool external_timer , int timerDt);
// stop and close the LUA thread
void basslua_close();
void basslua_external_timer();

int basslua_table(const char *module, const char *table, const int index, const char* field, char*svalue, int *ivalue, int action);
bool basslua_call(const char *module, const char *function, const char *sig, ...);

bool basslua_openMidiIn(int *nrDevices, int nbDevices);
bool basslua_getLog(char *buf);

void basslua_setSelector(int i, int luaNrAction, char op, int nrDevice, int nrChannel, const char *type_msg, const int *pitch, int nbPitch, bool stopOnMatch , const char* param );
bool basslua_selectorSearch(int nrDevice, int nrChannel, int type_msg, int p, int v);
bool basslua_selectorTrigger(int nr_selector,int nrDevice, int nrChannel, int type_msg, int d1, int d2);
void basslua_playbackmsg(int midinr, int type_msg, int channel, int value1, int value2);
void basslua_setMode(int mode);

void mlog_in(const char * format, ...);
