// Entries of the configuration parameters :

#ifdef _MSC_VER
#pragma warning(disable: 4800) //  'int' : forcing value to bool 'true' or 'false' (performance warning)
#pragma warning(disable: 4305) //  trunncation from double to float
#endif // _MSC_VER

#define APP_NAME wxString("ExpresseurV3")

// CONFIG_keys to store the parameters :

#define CONFIG_FILE "Configuration ExpresseurV3"
#define LIST_FILE "list of files ExpresseurV3"
#define LIST_RECT "list of rectangles ExpresseurV3"

#define CONFIG_TIMERDT "/timerdtms"
#define CONFIG_DAYCACHE "/daycache"

#define CONFIG_VERSION_CHECKED "/versionchecked"
#define CONFIG_BEGINNER "/beginner"
#define CONFIG_INITIALIZED "/initialized"
#define CONFIG_HARDWARE "/hardware"
#define CONFIG_DEFAULT_AUDIO "/default_audio"
#define CONFIG_MIDI_SETTING "/midi_setting"
#define CONFIG_AUDIO_UPDATE "/audio/update"
#define CONFIG_AUDIO_BUFFER "/audio/buffer"
#define CONFIG_EXPRESSIONVALUE "/expression/value_"
#define CONFIG_EXPRESSIONX "/expression/x"
#define CONFIG_EXPRESSIONY "/expression/y"
#define CONFIG_EXPRESSIONWIDTH "/expression/width"
#define CONFIG_EXPRESSIONHEIGHT "/expression/height"
#define CONFIG_EXPRESSIONVISIBLE "/expression/visible"

#define CONFIG_MIDIOUT "/mididevice/midiout"
#define CONFIG_MIDIIN "/mididevice/midiin"

#define CONFIG_MAINMAXIMIZED "/main/maximized"
#define CONFIG_MAINWIDTH "/main/width"
#define CONFIG_MAINDELTAHEIGHT "/main/deltaheight"
#define CONFIG_MAINDELTAWIDTH "/main/deltawidth"
#define CONFIG_MAINHEIGHT "/main/height"
#define CONFIG_MAIN_SCROLLHORIZONTAL "/main/sashhorizontal"
#define CONFIG_MAIN_SCROLLVERTICAL "/main/sashvertical"

#define CONFIG_ZOOM_MUSICXML "/edit/zoommusicxml"
#define CONFIG_CORRECTINCH "/edit/correctinch"
#define CONFIG_ZOOM_TEXT "/edit/zoomtext"
#define CONFIG_FILENAME "/edit/fileName"
#define CONFIG_LISTNAME "/edit/listName"
#define CONFIG_LOCALOFF "/localoff"

#define CONFIG_MIXERX "/mixer/x"
#define CONFIG_MIXERY "/mixer/y"
#define CONFIG_MIXERWIDTH "/mixer/width"
#define CONFIG_MIXERHEIGHT "/mixer/height"
#define CONFIG_MIXERVISIBLE "/mixer/visible"
#define CONFIG_MIXERMAIN "/mixer/main"
#define CONFIG_MIXERVOLUME "/mixer/volume"
#define CONFIG_MIXERDEVICENAME "/mixer/devicename"
#define CONFIG_MIXERDEVICEDEFAULT "mixer/devicedefault"
#define CONFIG_MIXERCHANNEL "/mixer/channel"
#define CONFIG_MIXERCURVENAME "/mixer/curvename"
#define CONFIG_MIXERINSTRUMENT "/mixer/instrument"
#define CONFIG_MIXER_EXTENSION "mixer/extendedchannel"

#define CONFIG_SHORTCUTNB "/shortcut/selectornb"
#define CONFIG_SHORTCUTCHANNEL "/shortcut/channel"
#define CONFIG_SHORTCUTDEVICENAME "/shortcut/devicename"
#define CONFIG_SHORTCUTVOLUME "/shortcut/volume"
#define CONFIG_SHORTCUTNAME "/shortcut/name"
#define CONFIG_SHORTCUTACTION "/shortcut/action"
#define CONFIG_SHORTCUTKEY "/shortcut/key"
#define CONFIG_SHORTCUTEVENT "/shortcut/event"
#define CONFIG_SHORTCUTMIN "/shortcut/min"
#define CONFIG_SHORTCUTMAX "/shortcut/max"
#define CONFIG_STOPONMATCH "/shortcut/stoponmatch"
#define CONFIG_SHORTCUTPARAM "/shortcut/param"
#define CONFIG_SHORTCUTX "/shortcut/x"
#define CONFIG_SHORTCUTY "/shortcut/y"
#define CONFIG_SHORTCUTWIDTH "/shortcut/width"
#define CONFIG_SHORTCUTHEIGHT "/shortcut/height"
#define CONFIG_SHORTCUTWIDTHLIST "/shortcut/widthlist"

#define CONFIG_KEYBOARDCONFIG "/shortcut/keyboardconfig"

#define CONFIG_EMPTYSCOREWARNING "/warningscore"

#define CONFIG_BITMAPSCOREX "/bitmapscore/x"
#define CONFIG_BITMAPSCOREY "/bitmapscore/y"
#define CONFIG_BITMAPSCOREWIDTH "/bitmapscore/width"
#define CONFIG_BITMAPSCOREHEIGHT "/bitmapscore/height"
#define CONFIG_BITMAPSCOREWARNINGTAGIMAGE "/bitmapscore/warningtagimage"

#define CONFIG_LUA_USER_SCRIPT "/script/luascript"
#define CONFIG_LUA_PARAMETER "/script/luaparameter"
#define CONFIG_MUSESCORE "/script/musescore"
#define CONFIG_MUSESCOREV3 "/script/musescorev3"

#define MIDIOUT_MAX 16
#define MIDIIN_MAX 16
#define MAXPITCH 128
#define MAXCHANNEL 16
#define VI_ZERO MIDIOUT_MAX
#define VI_MAX 16
#define OUT_MAX_DEVICE (VI_ZERO + VI_MAX)
#define MAX_TRACK 32
#define MAX_KEYS 2048
#define MAX_RECTCHORD 200
#define MAX_COLUMN_SHORTCUT 10
#define MAX_EXPRESSION 32

#define ID_MAIN 1000
#define ID_EDITSHORTCUT 2000
#define ID_EXPRESSION 3000
#define ID_LUAFILE 4000
#define ID_MIDISHORTCUT 5000
#define ID_MIXER 6000
#define ID_MUSICXML 7000
#define ID_LOGERROR 8000
#define ID_AUDIO 9000

#define SUFFIXE_MUSICXML "xml"
#define SUFFIXE_MUSICMXL "mxl"
#define SUFFIXE_BITMAPCHORD "png"
#define SUFFIXE_TEXT "txt"
#define SUFFIXE_PRECONFIG "dua"

#define SLINEAR "linear"
#define SALLMIDIIN "all Midi-In"
#define SALLCHANNEL "All channels"
#define SSTOP "stop"
#define SMIDI "MIDI"
#define SVI "VI"
#define NSF2 "SF2"
#define NVST "VST"
#define SSF2 "sf2"
#define SVST "dll"

#define SET_MUSICXML_FILE "MUSICXML"
#define SET_TITLE "TITLE"
#define SET_MARKS "SET MARKS"
#define SET_PLAY_MARKS "PLAY MARKS"
#define SET_ORNAMENTS "ORNAMENTS"
#define SET_PLAYBACK "~PLAYBACK"
#define SET_RATIO "speed="
#define SET_PARTS "PARTS"
#define COMMENT_EXPRESSEUR "--"


#define DIR_RESOURCES "ressources"
#define DIR_EXAMPLE "example"

#define ExpresseurId "ExpresseurMusicXMLPartId"
#define MINSCALEBAR 0.2
#define MAXSCALEBAR 4.0

#define MAXBUFCHAR 2048
#define NULL_INT -9999
#define NULL_STRING "-9999"

enum{ EMPTYVIEWER, BITMAPVIEWER, MUSICXMLVIEWER };

#if defined(_WIN32) || defined(WIN32)
    #define RUN_WIN 1
#elif defined(__APPLE__)
    #define RUN_MAC 1
#elif defined(__linux)
    #define RUN_LINUX 1
#else
    #error Unsupported platform
#endif
