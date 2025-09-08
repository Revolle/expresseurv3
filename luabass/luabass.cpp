/**
* \file luabass.c
* \brief LUA extension to drive Midi-out
* \author Franck Revolle
* \version 1.1
* \date 05/10/2015
* Objective :
*  LUA script language can use these functions to create easily any logic over MIDI-out.
*  List of function is available at the end of this source code file.

* platform :  MAC PC LINUX
*/


#ifdef _WIN32
#define V_PC 1
#endif
#ifdef _WIN64
#define V_PC 1
#endif

#ifdef __APPLE__
#include "TargetConditionals.h"
#ifdef TARGET_IPHONE_SIMULATOR
// iOS Simulator
#endif
#ifdef TARGET_OS_IPHONE
// iOS device
#endif
#ifdef TARGET_OS_MAC
#define V_MAC 1
#endif
#endif

#ifdef __linux
// linux
#define V_LINUX 1
#endif
#ifdef __unix // all unices not caught above
// Unix
#endif
#ifdef __posix
// POSIX
#endif

#define V_DMX 1

#include <stdio.h>
#include <stdlib.h>
//#include <iostream.h>
//#include <cstdlib.h>

#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>

#include <lua.hpp>
#include <lauxlib.h>
#include <lualib.h>

#ifdef V_PC
#include <ctgmath>
#include <windows.h>   /* required before including mmsystem.h */
#include <mmsystem.h>  /* multimedia functions (such as MIDI) for Windows */
#include <winbase.h>
#define V_ASIO 1
#endif
#include <assert.h>

#ifdef V_MAC
#include <math.h>
#include <CoreFoundation/CoreFoundation.h>
//#include <CoreServices/CoreServices.h>
//#include <CoreMIDI/CoreMIDI.h>
//#include <CoreMIDI/MIDIServices.h>
//#include <CoreMIDI/MIDISetup.h>
//#include <CoreMIDI/MIDIThruConnection.h>
#include <pthread.h>
#define byte Byte
#endif

#ifdef V_LINUX
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#endif

// bass audio to manage audio and midi-in
#include <bass.h>
#include <bassmidi.h>
#include <bassmix.h>

#ifdef V_ASIO
#include <bassasio.h>
#endif

// RTMidi library to manage midi-out
#include "RtMidi.h"
// libserialport library to manage serial DMX
#include "libserialport.h"

// VST libraries
#ifdef V_VST
#include "aeffect.h"
#include "aeffectx.h"
#include "vstfxstore.h"
#endif

#include "luabass.h"
#include "global.h"

#ifdef V_PC
// disable warning for mismatch lngth between LUA int and C int
#pragma warning( disable : 4244 )
// disable warning for deprecated ( e.g. sprintf )
#pragma warning( disable : 4996 )
#endif

#if defined(V_LINUX) || defined(V_MAC)
#define strtok_s strtok_r
#define strnlen_s strnlen
#endif

#define MAX_AUDIO_DEVICE 32
int g_transposition = 0;
int g_nb_audio_bass_device = 0; // number of non asio audio device
int g_nb_asio_device = 0; // number of asio audio device
int g_default_audio_device = 1; // default audio device ( 0 == nosound )
int g_audio_updateperiod[MAX_AUDIO_DEVICE] = { 0 };
int g_audio_buffer[MAX_AUDIO_DEVICE] = { 0 };
bool g_audio_first_list = true;
int g_audio_buffer_length = 0; // length if audio buffer ( large by default, latency, but no crack .. )

/**
* \union T_midimsg
* \brief store a short Midi Messages ( noteOn, ...)
*
* The data manipulated is a 3 bytes structure.
* It is used for short MIDI messages , like Note-On, Note-Off, Control ...
*/
typedef union t_midimsg
{
	DWORD dwData; /*!< The block of data. */
	BYTE bData[4]; /*!< The data, byte per byte. */
} T_midimsg;

#define MAX_VSTI_PENDING_MIDIMSG DMX_V_MAX

/**
* \struct T_vi_opened
* \brief Queue of VI already opened.
*
*/
typedef struct t_vi_opened
{
	char filename[512];// file used by this Virtual Instrument
	int nr_device_audio; // audio-device used by this Virtual Instrument
	HSTREAM mstream; // BASS stream connected to the mixer of the audio-device

	HSOUNDFONT sf2_midifont; // handler Soundfont SF2

#ifdef V_VST
	AEffect *vsti_plugins; // handler of the VSTi
	int vsti_nb_outputs; // number of audio outputs of the vsti
	bool vsti_midi_prog; // vst-prog will be sent 
	int vsti_last_prog; // last vst-prog sent to the VSTi on next update
	bool vsti_todo_prog;// pending vst-prog to send to the VSTi on next update
	T_midimsg vsti_pending_midimsg[MAX_VSTI_PENDING_MIDIMSG]; // pending midi msg to send to the VSTi on next update
	int vsti_nb_pending_midimsg; // nb pending midi msg to send to the VS on next update
	float **vsti_outputs; // buffer for vsti output
#ifdef V_PC
	HMODULE vsti_modulePtr; // library dll loaded for the VSTi
#endif
#ifdef V_MAC
  CFBundleRef vsti_modulePtr;// library dll loaded for the VSTi
#endif
#endif

} T_vi_opened;

/**
* \struct T_midioutmsg
* \brief store a short Midi Messages, with additional information for the output
*
* The information is used to track the output of the short MIDI message.
*/
typedef struct t_midioutmsg
{
	long id; /*!< unique id of the msg used for noteon noteoff control */
	int track; /*!< midiout track for this message. */
	long dt; /*!< delay in ms before to send this message */
	BYTE nbbyte; /*!< number of bytes of the midi message*/
	T_midimsg midimsg; /*!<  short midi msg itself */
} T_midioutmsg;

/**
* \struct T_channel
* \brief properties of a MIDI Channel on a device : extention
*
* The information is used to produce the MIDI out message, just before to send it physically on the phisical MIDI channel.
*/
typedef struct t_channel
{
	int extended; /*!< specify which logical MIDI channel drives this MIDI physical channel */
} T_channel;

#define MAXCURVE 10
#define MAXPOINT 10
typedef struct t_curve
{
	int x[MAXPOINT], y[MAXPOINT]; /*!< list of points for the volume curve of the channel */
} T_curve;
#define MAXTRACK 32
/**
* \struct T_track
* \brief properties of a track : volume and curve
*
* A track is logical group of MIDI flow, which is attaced to a device/channel, through T_channel
* The information is used to control volume on a track..
*/
typedef struct t_track
{
	int device; /*!< MIDI device used to send the MIDI messages */
	int channel; /*!< logical MIDI channel used to send the MIDI messages */
	int volume; /*!< MIDI volume of the channel ( not the CTRL-7 MIDI Volume ) 0..127 */
	bool mute;
	int nrCurve;
} T_track;
/**
* \struct T_chord
* \brief track information to play a chord
*
* The information is used to start ans stop a chord.
* The chord-on can be used many times on the same chord-id.
* The chord-off is used one time on the same chord-id. It off all the previous chord-on.
*/
#define CHORDMAXPITCH 24
#define CHORDMAX 40
typedef struct t_chord
{
	long id; /*!< unique id of the chord */
	int dt; /*!< delay between notes in the chord, in ms */
	int dv; /*!< ratio, in % , between velocity notes in the chord */
	int pitch[CHORDMAXPITCH]; /*!< pitches of the chord */
	int nbPitch; /*!< number of pitches in the chord */

	int nbOff; // number of noteoff when chord-off
	T_midioutmsg msg_off[CHORDMAXPITCH]; // noteon to off
} T_chord;
/**
* \struct T_queue_msg
* \brief Queue of pend MIDI-out messages.
*
* MIDI messages which must be sent later are stored in this queue.
*/
typedef struct t_queue_msg
{
	T_midioutmsg midioutmsg; /*!< The midi message delayed */
	long t; /*!< the time to send the message */
	bool free; /*!< slot free */
} T_queue_msg;
#define OUT_QUEUE_MAX_MSG 1024

#define MAXBUFERROR 64


#define OUT_QUEUE_FLUSH 0
#define OUT_QUEUE_NOTEOFF 1

#define smidiToOpen "midiinOpen" // global LUA table which contains the MIDI-in to open

// LUA-functions called by this DLL 
#define LUAFunctionNoteOn "onNoteon" // LUA funtion to call on midi msg noteon
#define LUAFunctionNoteOff "onNoteoff" // LUA funtion to call on midi msg noteoff
#define LUAFunctionKeyPressure "onKeypressure" // LUA funtion to call on midi msg keypressure
#define LUAFunctionControl "onControl" // LUA funtion to call on midi msg control
#define LUAFunctionProgram "onProgram" // LUA funtion to call on midi msg program
#define LUAFunctionChannelPressure "onChannelpressure" // LUA funtion to call on midi msg channelpressure
#define LUAFunctionPitchBend "onPitchbend" // LUA funtion to call on midi msg pitchbend
#define LUAFunctionSystemCommon "onSystemecommon" // LUA funtion to call on midi msg systemcommon
#define LUAFunctionSysex "onSysex" // LUA funtion to call on midi msg sysex
#define LUAFunctionActive "onActive" // LUA funtion to call on midi msg active sense
#define LUAFunctionClock "onClock" // LUA funtion to call when midiin clock
#define LUAFunctionTimer "onTimer" // LUA funtion to call when cascaded timer is triggered

#define onTimer "onTimer" // LUA funtion to call when timer is triggered 
#define onSelector "onSelector" // LUA functions called with noteon noteoff event,a dn add info 

#define L_MIDI_NOTEONOFF		7  // non MIDI message : for selectors only
#define L_MIDI_NOTEOFF			8  // 0x8
#define L_MIDI_NOTEON			9  // 0x9
#define L_MIDI_KEYPRESSURE		10 // 0xA
#define L_MIDI_CONTROL			11 // 0xB
#define L_MIDI_PROGRAM			12 // 0xC
#define L_MIDI_CHANNELPRESSURE 13 // 0xD
#define L_MIDI_PITCHBEND		14 // 0xE
#define L_MIDI_SYSTEMCOMMON	15 // 0xF

#define L_MIDI_SYSEX 0xF0
#define L_MIDI_ACTIVESENSING 0xFE
#define L_MIDI_CLOCK 0xF8

//////////////////////////////
//  static statefull variables
//////////////////////////////
static RtMidiOut *g_midiout_rt[MIDIOUT_MAX] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };// list of midiout
static RtMidiOut g_g_midiout_rt ;
static unsigned int g_nb_midi_out = 0 ; // nb of midi_out
static unsigned int g_nb_midi_in = 0 ; // nb of midi_in
static char g_name_midi_out[MIDIOUT_MAX][512] = { "" , "" ,  "" , "" , "" , "" , "" , "" , "" , "" , "" , "" , "" , "" , "" , "" }; // names of midi_out
static char g_name_midi_in[MIDIIN_MAX][512] = { "" , "" ,  "" , "" , "" , "" , "" , "" , "" , "" , "" , "" , "" , "" , "" , "" } ; // names of midi_in

static bool g_audio_open[MAX_AUDIO_DEVICE] = { false, false,false, false,false, false,false, false,false, false,false, false,false, false,false, false,false, false,false, false,false, false,false, false,false, false,false, false,false, false,false, false };
static T_vi_opened g_vi_opened[VI_MAX];
static int g_vi_opened_nb = 0;
static HSTREAM g_mixer_stream[MAX_AUDIO_DEVICE];

#ifdef V_VST
#define VSTI_BUFSIZE 4096
VstEvents *g_vsti_events = NULL;
int g_vsti_bufsize = 1024;
#endif

static int g_midiout_max_nr_device;

static T_channel g_channels[OUT_MAX_DEVICE][MAXCHANNEL];

static long g_midistatuspitch[OUT_MAX_DEVICE][MAXCHANNEL][MAXPITCH]; // the status of a output pitch
static long g_midistatuscontrol[OUT_MAX_DEVICE][MAXCHANNEL][MAXPITCH]; // the status of a output control
static long g_miditimepitch[OUT_MAX_DEVICE][MAXCHANNEL][MAXPITCH]; // the time of a output pitch
static long g_miditimecontrol[OUT_MAX_DEVICE][MAXCHANNEL][MAXPITCH]; // the time of a output control

static int g_chordCompensation = 0; // compensation of velocity for each note in a chord
static int g_randomDelay = 0; // random delay in seconds for each note of a chord
static int g_randomVelocity = 0; // random velocity for each note of a chord

static bool g_debug = false;
static bool g_collectLog = false;
static int nrOutBufLog = 0;
static int nrInBufLog = 0;
#define MAXBUFLOGOUT 512
#define MAXNBLOGOUT 64
static char bufLog[MAXNBLOGOUT][MAXBUFLOGOUT];

static long g_unique_id = 128;

static T_track g_tracks[MAXTRACK]; 
static int g_volume = 64;
static T_curve g_curves[MAXCURVE];

static T_chord g_chords[CHORDMAX];

static long g_current_out_t = 0 ; // relative time in ms for output
static long g_timer_out_dt = 50 ; // ms between two timer interrupts on output

static bool g_timer_out_ok = false ;
static bool g_mutex_out_ok = false ;
#ifdef V_PC
// timer to flush the midiout queud messages
static HANDLE g_timer_out = NULL;
// mutex to protect the output ( from LUA and timer, to outputs )
static HANDLE g_mutex_out = NULL;
#endif

#ifdef V_MAC
// mutex to protect the access od the midiout queud messages
static pthread_mutex_t g_mutex_out ;
static pthread_t g_loop_out_run_thread ;
#endif
#ifdef V_LINUX
static pthread_t g_loop_out_run_thread ;
static timer_t g_timer_out_id;
#define MTIMERSIGNALOUT (SIGRTMIN+0)
static pthread_mutex_t g_mutex_out;
#endif

static T_queue_msg g_queue_msg[OUT_QUEUE_MAX_MSG];
static int g_end_queue_msg = 0; // end of potential waitin slot
static int g_max_queue_msg = 0; // max of waiting slot

static 	lua_State *g_LUAoutState = 0 ; // LUA state for the process of midiout messages
static bool g_process_NoteOn, g_process_NoteOff;
static bool g_process_Control, g_process_Program;
static bool g_process_PitchBend, g_process_KeyPressure, g_process_ChannelPressure;
static bool g_process_SystemCommon, g_process_Clock , g_process_Timer ;

static char g_path_out_error_txt[MAXBUFCHAR];


#define DMX_CH_MAX 256
#define DMX_V_MAX 256
#ifdef V_DMX
// dmx management
struct sp_port** g_dmx_port_list;
struct sp_port *g_dmx_port = NULL;	// dmx comport handle
int g_portdmx_nr = -1; // dmx comport number
float g_dmx_float_value[DMX_CH_MAX]; // // dmx values actual
float g_dmx_float_target[DMX_CH_MAX]; // // dmx values target
byte g_dmx_byte_value[DMX_CH_MAX]; // // dmx values to send
unsigned int g_dmx_byte_nb = 0; // number of dmx values to send
float g_dmx_ramping = 1.0; // attack value in the time (0..256). 256==direct attack 0==slow ramping, default 256
int g_dmx_track[MAXTRACK] ; // track to hook for DMX output
int g_dmx_nb_track = 0;
int g_dmx_midi_map[MAXPITCH] ; // map of the MIDI to dmx channel
int g_dmx_nb_midi_map = 0;
float g_dmx_tenuto = 1.0; // tenuto value in the time (0..256). 256==no decrease 0==quick decrease, default 256
#endif



static int cap(int vin, int min, int max, int offset)
{
	// -offset is applied to vin
	// return vin inside range [min..max[ . 
	int v = vin - offset;
	if (v < min) return min;
	if (v >= max) return ( max - 1);
	return v;
}
static int pitchbend_value(T_midimsg u)
{
	return((int)(u.bData[2]) * (int)(0x80) + (int)(u.bData[1]) - (int)(0x2000));
}
static void log_out_init(const char *fname)
{
	if ((fname != NULL) && (strlen(fname) > 0))
	{
		strcpy(g_path_out_error_txt, fname);
		strcat(g_path_out_error_txt, "_out.txt");
	}
	else
	{
		strcpy(g_path_out_error_txt, "luabass_log_out.txt");
	}
	FILE * pFile = fopen(g_path_out_error_txt, "w");;
	if (pFile == NULL) return;
	fprintf(pFile, "log luabass out\n");
	fclose(pFile);
}
static int mlog_out(const char * format, ...)
{
	char msg[MAXBUFLOGOUT];
	va_list args;
	va_start(args, format);
	vsprintf(msg, format, args);
	va_end(args);
	FILE * pFile = fopen(g_path_out_error_txt, "a");;
	if (pFile != NULL)
	{
		fprintf(pFile, "%s\n",msg);
		fclose(pFile);
	}
	else
	{
		fprintf(stderr,"error mlog_out file <%s> : %s\n",g_path_out_error_txt, msg);
	}
	if (g_collectLog)
	{
		strcpy(bufLog[nrInBufLog], msg);
		nrInBufLog++;
		if (nrInBufLog >= MAXNBLOGOUT)
			nrInBufLog = 0;
	}
	return(-1);
}
static int mlog_xml(const char * format, ...)
{
	char g_path_out_xml[MAXBUFCHAR];
	char msg[MAXBUFLOGOUT];
	va_list args;
	va_start(args, format);
	vsprintf(msg, format, args);
	va_end(args);
	strcpy(g_path_out_xml,g_path_out_error_txt);
	strcpy(g_path_out_xml+strlen(g_path_out_xml)-4,".xml");
	FILE * pFile ;
	if ( strncmp(msg,"<?xml",5) == 0)
		pFile = fopen(g_path_out_xml, "w");
	else
		pFile = fopen(g_path_out_xml, "a");
	if (pFile != NULL)
	{
		fprintf(pFile, "%s",msg);
		fclose(pFile);
	}
	return(0);
}
static void lock_mutex_out()
{
	if ( ! g_mutex_out_ok )
		return ;
#ifdef V_PC
	WaitForSingleObject(g_mutex_out, INFINITE ) ;
#endif
#ifdef V_MAC
  pthread_mutex_lock(&g_mutex_out);
#endif
#ifdef V_LINUX
	pthread_mutex_lock(&g_mutex_out);
#endif
}
static bool try_lock_mutex_out()
{
	if ( ! g_mutex_out_ok )
		return true;
#ifdef V_PC
	return(WaitForSingleObject(g_mutex_out, 0 ) == WAIT_OBJECT_0);
#endif
#ifdef V_MAC
  return (pthread_mutex_trylock(&g_mutex_out) == 0 );
#endif
#ifdef V_LINUX
  return ( pthread_mutex_trylock(&g_mutex_out) == 0 );
#endif
}
static void unlock_mutex_out()
{
	if ( ! g_mutex_out_ok )
		return ;
#ifdef V_PC
	ReleaseMutex(g_mutex_out);
#endif
#ifdef V_MAC
	pthread_mutex_unlock(&g_mutex_out);
#endif
#ifdef V_LINUX
	pthread_mutex_unlock(&g_mutex_out);
#endif
}

#ifdef V_DMX
void dmxClose()
{
	if (g_dmx_port)
	{
		sp_close(g_dmx_port);
		sp_free_port_list(g_dmx_port_list);
	}
	g_dmx_port = NULL;
	g_portdmx_nr = -1; // comport closed
}
bool dmxOpen(int comport)
{
	if ((comport == g_portdmx_nr) && g_dmx_port)
	{
		// comport already opened
		return true;
	}

	if (g_dmx_port)
		dmxClose();

	if (sp_list_ports(&g_dmx_port_list) != SP_OK)
	{
		return false;
	}
	g_dmx_port = NULL;
	for (int i = 0; g_dmx_port_list[i] != NULL; i++) {
		if (i == comport)
		{
			g_dmx_port = g_dmx_port_list[i];
			break;
		}
	}
	if (g_dmx_port == NULL)
	{
		sp_free_port_list(g_dmx_port_list);
		return false;
	}
	if ( sp_open(g_dmx_port, SP_MODE_WRITE) != SP_OK)
	{
		sp_free_port_list(g_dmx_port_list);
		g_dmx_port = NULL;
		g_portdmx_nr = -1; // comport closed	
		return false;
	}
	g_portdmx_nr = comport;
	sp_set_baudrate(g_dmx_port, 256000);
	sp_set_bits(g_dmx_port, 8);
	sp_set_parity(g_dmx_port, SP_PARITY_NONE);
	sp_set_stopbits(g_dmx_port, 2);
	sp_set_flowcontrol(g_dmx_port, SP_FLOWCONTROL_NONE);
	mlog_out("dmx Open OK : #%d  %s", g_portdmx_nr , sp_get_port_name(g_dmx_port));
	return true;
}
void dmxStart()
{
	if ( !g_dmx_port)
		return;	
	sp_set_baudrate(g_dmx_port, 115200);
	sp_start_break(g_dmx_port);
	sp_set_baudrate(g_dmx_port, 256000);
	byte hDmx = (byte)(255);
	sp_blocking_write(g_dmx_port, &hDmx, 1, 200);
}
void dmxSend()
{
	if (!g_dmx_port)
		return;
	sp_blocking_write(g_dmx_port, g_dmx_byte_value, g_dmx_byte_nb, 200);
}
void dmx_init()
{
	// initialize dmx values
	for (unsigned int i = 0; i < DMX_CH_MAX; i++)
	{
		g_dmx_float_value[i] = 0.0;
		g_dmx_float_target[i] = 0.0;
		g_dmx_byte_value[i] = 0;
	}
	g_dmx_byte_nb = 0;
	g_dmx_ramping = 1.0;
	g_dmx_tenuto = 1.0;
	g_portdmx_nr = -1; // comport closed
}
void dmxRefresh()
{
	if (g_dmx_port)
	{
		dmxStart();
		dmxSend();
		if (g_dmx_ramping != 1.0)
		{
			// ramping dmx values
			for (unsigned int i = 0; i < g_dmx_byte_nb; i++)
			{
				g_dmx_float_value[i] += (g_dmx_float_target[i] - g_dmx_float_value[i]) * g_dmx_ramping;
				g_dmx_byte_value[i] = (byte)(cap((int)(g_dmx_float_value[i] * (float)(DMX_V_MAX)), 0, DMX_V_MAX, 0));
			}
		}
		if (g_dmx_tenuto != 0.0)
		{
			// decrease dmx values
			for (unsigned int i = 0; i < g_dmx_byte_nb; i++)
			{
				if (abs(g_dmx_float_value[i] - g_dmx_float_target[i]) < 0.01)
				{
					g_dmx_float_value[i] += (0.0 - g_dmx_float_value[i]) * g_dmx_tenuto;
					g_dmx_float_target[i] = g_dmx_float_value[i];
					g_dmx_byte_value[i] = (byte)(cap((int)(g_dmx_float_value[i] * (float)(DMX_V_MAX)), 0, DMX_V_MAX, 0));
				}
			}
		}
	}
}
void dmx_hook(int pitch, int v)
{
	int c = g_dmx_midi_map[pitch % g_dmx_nb_midi_map];
	if (c < 0)
		return;
	g_dmx_float_target[c] = ((float)(v * 2 )) / ((float)DMX_V_MAX);
	if (g_dmx_ramping == 1.0)
	{
		// no ramping , direct assignment
		g_dmx_float_value[c] = g_dmx_float_target[c];
		g_dmx_byte_value[c] = (byte)v;
	}
}
static int LdmxOpen(lua_State* L)
{
	// open dmx com port
	// param 1 : comport ( 0..DMX_V_MAX )
	// param 2 : nb channel ( 1..DMX_CH_MAX )
	// retun true if open, else return false

	if (lua_gettop(L) < 2)
	{
		lua_pushboolean(L, false);
		return 1;
	}
	lock_mutex_out();

	int comport = cap((int)lua_tointeger(L, 1), 0, 128, 0);
	g_dmx_byte_nb = cap((int)lua_tointeger(L, 2), 1, DMX_CH_MAX, 0);
	if (!dmxOpen(comport))
		lua_pushboolean(L, false);

	lua_pushboolean(L, true);

	unlock_mutex_out();
	return 1;
}
static int LdmxSet(lua_State* L)
{
	// set dmx values
	// optional param 1 : tenuto value in the time (0..256). 256==direct 0,  0==no decrease, default 256
	// optional param 2 : ramping value in the time (0..256). 256==direct value,  0==slow ramping, default 256
	// optional param 3 : string of hook track nr, comma separated , default : no change
	// optional param 4 : string of mapping MIDI-pitch (comma separated) to DMX-channel, default : no change

	lock_mutex_out();
	int tenuto = luaL_optinteger(L, 1, DMX_V_MAX);
	int ramping = luaL_optinteger(L, 2, DMX_V_MAX);
	char dmx_track[4 * MAXTRACK];
	strcpy(dmx_track, luaL_optstring(L, 3, ""));
	char dmx_midi_map[4 * MAXPITCH];
	strcpy(dmx_midi_map, luaL_optstring(L, 4, ""));
	if (*dmx_track != '\0')
	{
		char* pt = strtok(dmx_track, ",;:/ ");
		g_dmx_nb_track = 0;
		while (pt != NULL)
		{
			g_dmx_track[g_dmx_nb_track] = atoi(pt);
			g_dmx_nb_track++;
			pt = strtok(NULL, ",");
		}
	}
	if (*dmx_midi_map != '\0')
	{
		char* pt = strtok(dmx_midi_map, ",;:/ ");
		g_dmx_nb_midi_map = 0;
		while (pt != NULL)
		{
			g_dmx_midi_map[g_dmx_nb_midi_map] = atoi(pt);
			g_dmx_nb_midi_map++;
			pt = strtok(NULL, ",");
		}
	}
	if (tenuto <= 1)
	{
		// tenuto
		g_dmx_tenuto = 0.0;
	}
	else
	{
		g_dmx_tenuto = 0.2 * (float)(cap(tenuto, 1, DMX_V_MAX, 0)) / ((float)DMX_V_MAX);
	}
	if (ramping >= 255)
	{
		// no ramping
		g_dmx_ramping = 1.0;
	}
	else
	{
		g_dmx_ramping = 0.2* (float)(cap(ramping, 1, DMX_V_MAX, 0)) / ((float)DMX_V_MAX);
	}

	unlock_mutex_out();

	return 0;
}
static int LdmxOutAll(lua_State* L)
{
	// send dmx values
	// param 1..512 : dmx values (0..255)

	lock_mutex_out();

	int nbArg = cap(lua_gettop(L),0,DMX_CH_MAX,0);
	float* ptDmx = g_dmx_float_target;
	float* pvDmx = g_dmx_float_value;
	byte* pDmxByte = g_dmx_byte_value;
	int v; 
	for (unsigned int nrArg = 1 ; nrArg <= nbArg; nrArg ++)
	{
		v = (byte)(cap(lua_tointeger(L, nrArg), 0, DMX_V_MAX, 0));
		*ptDmx = (float)(*pDmxByte) / (float)(DMX_V_MAX);
		if (g_dmx_ramping == 1.0)
		{
			// no ramping , direct assignment
			*pvDmx = *ptDmx;
			*pDmxByte = v;
		}
		nrArg++;
		pDmxByte++;
		ptDmx++;
		pvDmx++;
	}

	unlock_mutex_out();
	return 0;
}
static int LdmxOut(lua_State* L)
{
	// send dmx value
	// param 1 : channel (0..255)
	// param 2 : dmx value (0..255)
	if (lua_gettop(L) < 2)
	{
		return 0;
	}

	lock_mutex_out();

	int c = cap((int)luaL_optinteger(L, 1, 1), 0, DMX_CH_MAX, 0);
	int v = cap((int)luaL_optinteger(L, 2, 1), 0, DMX_V_MAX, 0);
	g_dmx_float_target[c] = ((float)(v))/((float)DMX_V_MAX);
	if (g_dmx_ramping == 1.0)
	{
		// no ramping , direct assignment
		g_dmx_float_value[c] = g_dmx_float_target[c];
		g_dmx_byte_value[c] = (byte)v;
	}

	unlock_mutex_out();
	return 0;
}
static int LdmxList(lua_State* L)
{
	// return the midi In device in an array
	// no parameter
	lock_mutex_out();

	lua_newtable(L);
	struct sp_port** port_list;
	enum sp_return result = sp_list_ports(&port_list);
	if (result == SP_OK)
	{
		for (DWORD i = 0; port_list[i] != NULL; i++)
		{
			struct sp_port* port = port_list[i];
			lua_pushinteger(L, i + 1);
			lua_pushstring(L, sp_get_port_name(port));
			lua_settable(L, -3);
		}

	}
	sp_free_port_list(port_list);
	unlock_mutex_out();
	return(1);
}
static int LdmxCount(lua_State* L)
{
	// return the number of dmx serial device
	// no parameter
	lock_mutex_out();
	struct sp_port** port_list;
	enum sp_return result = sp_list_ports(&port_list);
	int i = 0;
	if (result == SP_OK) 
	{
		for (i = 0; port_list[i] != NULL; i++) 
		{
			struct sp_port* port = port_list[i];
		}

	}
	lua_pushinteger(L, i);
	sp_free_port_list(port_list);
	unlock_mutex_out();
	return(1);
}
static int LdmxName(lua_State* L)
{
	// return the name of the midi In device 
	// parameter #1 : device nr
	lock_mutex_out();

	int nrDevice = cap((int)lua_tointeger(L, 1), 0, OUT_MAX_DEVICE, 0);
	char name_device[MAXBUFCHAR] = "";
	struct sp_port** port_list;
	enum sp_return result = sp_list_ports(&port_list);
	int i = 0;
	if (result == SP_OK)
	{
		for (i = 0; port_list[i] != NULL; i++)
		{
			struct sp_port* port = port_list[i];
			if (i == nrDevice)
			{
				char* port_name = sp_get_port_name(port);
				strcpy(name_device , port_name);
				break;
			}
		}
	}
	lua_pushstring(L, name_device);

	unlock_mutex_out();
	sp_free_port_list(port_list);
	return(1);
}
#endif // V_DMX

static int apply_volume(int nrTrack, int v)
{
	if ((nrTrack < 0) || (nrTrack >= MAXTRACK))
		return v;

	T_track *t;
	int mainvolume;
	t = &(g_tracks[nrTrack]);
	mainvolume = g_volume;

	if ((t->volume == 64) && (mainvolume == 64) && ((t->nrCurve < 0) || (g_curves[t->nrCurve].x[0] == -1)))
		return cap(v, 0, 128, 0);
	if (t->mute)
		return 0;

	int vout = v;
	int x0, y0, x1, y1;
	if (t->nrCurve > 0)
	{
		T_curve *curve = &(g_curves[t->nrCurve]);
		int n = 0;
		while ((n < MAXPOINT) && (curve->x[n] >= 0) && (vout > curve->x[n]))
			n++;
		if ((n >= (MAXPOINT)) || (curve->x[n] < 0))
		{
			x0 = curve->x[n - 1];
			y0 = curve->y[n - 1];
			x1 = 127;
			y1 = 127;
		}
		else
		{
			if (n == 0)
			{
				x0 = 1;
				y0 = 1;
				x1 = curve->x[0];
				y1 = curve->y[0];
			}
			else
			{
				x0 = curve->x[n - 1];
				y0 = curve->y[n - 1];
				x1 = curve->x[n];
				y1 = curve->y[n];
			}
		}
		if (x1 == x0)
			x1 = x0 + 1;
		vout = y0 + ((vout - x0) * (y1 - y0)) / (x1 - x0);
	}

	if (t->volume != 64)
	{
		if (t->volume < 64)
		{
			x0 = 1; y0 = 1;
			x1 = 127; y1 = 2 * t->volume;
		}
		else
		{
			x0 = 1; y0 = (t->volume - 64) * 2;
			x1 = 127; y1 = 127;
		}
		vout = cap(y0 + ((vout - x0) * (y1 - y0)) / (x1 - x0), 1, 128, 0);
	}

	if (mainvolume != 64)
	{
		if (mainvolume < 64)
		{
			x0 = 1; y0 = 1;
			x1 = 127; y1 = 2 * mainvolume;
		}
		else
		{
			x0 = 1; y0 = (mainvolume - 64) * 2;
			x1 = 127; y1 = 127;
		}
		vout = cap(y0 + ((vout - x0) * (y1 - y0)) / (x1 - x0), 1, 128, 0);
	}

	return cap(vout, 1, 128, 0);
}
static bool asio_name(int nr_device, char *name)
{
	*name = '\0';
#ifdef V_ASIO
	BASS_ASIO_DEVICEINFO info;
	if (BASS_ASIO_GetDeviceInfo(nr_device, &info) == TRUE)
	{
		strcpy(name, "asio_");
		strcat(name,info.name);
		return true;
	}
#endif
	return false;
}
static bool audio_bass_name(int nr_device, char *name)
{
	*name = '\0';
	BASS_DEVICEINFO info;
	if (BASS_GetDeviceInfo(nr_device, &info) == TRUE)
	{
		if (info.flags & BASS_DEVICE_ENABLED)
		{
			strcpy(name, info.name);
			return true;
		}
	}
	return false;

}
static bool audio_name(int nr_device, char *name)
{
	*name = '\0';
	if (nr_device < g_nb_audio_bass_device)
		return (audio_bass_name(nr_device, name));
	int nr_asio_device = nr_device - g_nb_audio_bass_device;
	if (nr_asio_device < g_nb_asio_device)
		return (asio_name(nr_asio_device, name));
	return false;
}
static int midi_out_count()
{
	return g_nb_midi_out ;
}

static int midi_out_open(int nr_device)
{
	if (( nr_device < 0) || (nr_device >= midi_out_count()))
		return -1;

	assert((nr_device>=0)&&(nr_device<MIDIOUT_MAX));

	if ( g_midiout_rt[nr_device] == 0 )
	{
		g_midiout_rt[nr_device] =  new RtMidiOut();
		if ( g_midiout_rt[nr_device] == 0 )
		{
			mlog_out("Error : midi_out_open : not allocated");
			return -1 ; // not allocated
		}
	}
	if ( g_midiout_rt[nr_device]->isPortOpen() == false )
	{
		g_midiout_rt[nr_device]->openPort(nr_device);
		if ( g_midiout_rt[nr_device]->isPortOpen() == false )
		{
			mlog_out("Error : midi_out_open : not opened");
			return -1 ; // not opened
		}
		mlog_out("Information : midiOut open device#%d : OK", nr_device + 1);
	}

	if (nr_device >= g_midiout_max_nr_device)
		g_midiout_max_nr_device = nr_device + 1;

  	std::vector<unsigned char> message;
  	message.push_back(L_MIDI_CONTROL << 4);
  	message.push_back(123);// all note off
  	message.push_back(0);
	g_midiout_rt[nr_device]->sendMessage( &message );

	message[1]=120;// all sound off
	g_midiout_rt[nr_device]->sendMessage( &message );

	message[1]=121;// // reset al controller
	g_midiout_rt[nr_device]->sendMessage( &message );

	return(nr_device);
}
static bool midi_in_name(int nr_device, char *name_device)
{
	name_device[0] = '\0';
	if ((nr_device < 0 ) || (nr_device >= g_nb_midi_in))
	{
		//mlog_out("luabass midi_in_name nrdevice%d not exist",nr_device);
		return false ;
	}
	strcpy(name_device,g_name_midi_in[nr_device]);
	//mlog_out("luabass midi_in_name nrdevice%d = %s",nr_device,name_device);
	return true;
}
static bool midiout_name(int nr_device, char *name_device)
{
	assert((nr_device>=0)&&(nr_device<MIDIOUT_MAX));
	name_device[0] = '\0';
	if (( nr_device < 0 ) || ( nr_device >= midi_out_count()))
	{
		return false ;
	}
	strcpy(name_device, g_name_midi_out[nr_device]);
	return true ;
}
/*
static void inspect_channel()
{
	mlog_out("=============================================== outInspect extended channel");
	char buf[1024];
	int nb = 0;
	for (int d = 0; d < MIDIOUT_MAX; d++)
	{
		if (g_midiout_opened[d])
		{
			for (int c = 0; c < MAXCHANNEL; c++)
			{
				if ((g_channels[d][c].extended != c) && (g_channels[d][c].extended != -1))
				{
					sprintf(buf, "device#%d : channel#%d is extension of channel %d", d + 1, c + 1, g_channels[d][c].extended + 1);
					mlog_out(buf);
					nb++;
				}
			}
		}
	}
	mlog_out("=============================================== end extended channel");
}
static void inspect_queue()
{
	char buf[1024];
	int nbFree;
	mlog_out("=============================================== outInspect queue ");
	nbFree = 0;
	for (int n = 0; n < OUT_QUEUE_MAX_MSG; n++)
	{
		if (g_queue_msg[n].free)
			nbFree++;
		else
		{
			sprintf(buf, "waiting %d : t=%ld msg=%02X %02X %02X", n,
				g_queue_msg[n].t,
				g_queue_msg[n].midioutmsg.midimsg.bData[0],
				g_queue_msg[n].midioutmsg.midimsg.bData[1],
				g_queue_msg[n].midioutmsg.midimsg.bData[2]);
			mlog_out(buf);
		}
	}
	mlog_out("");
	mlog_out("nb queue free : %d / max used : %d", nbFree, g_max_queue_msg);

	mlog_out("=============================================== end queue");
}
static void inspect_note()
{
	mlog_out("=============================================== outInspect note");
	char buf[1024];
	int nb = 0;
	for (int d = 0; d < OUT_MAX_DEVICE; d++)
	{
		for (int c = 0; c < MAXCHANNEL; c++)
		{
			for (int p = 0; p < MAXPITCH; p++)
			{
				if (g_midistatuspitch[d][c][p] != -1)
				{
					sprintf(buf, "note-ON : device #%d , channel #%d , pitch #%d = %lu", d + 1, c + 1, p, g_midistatuspitch[d][c][p]);
					mlog_out(buf);
					nb++;
				}
			}
		}
	}
	mlog_out("total of note-ON : %d", nb);
	mlog_out("===============================================  end note");
}
static void inspect_device()
{
	mlog_out("=============================================== outInspect device");
	char buf[MAXBUFCHAR];
	for (int m = 0; m < MIDIOUT_MAX; m++)
	{
		midiout_name(m, buf);
		if (g_midiout_opened[m])
			mlog_out("Midiout open : %s (#%d)", buf, m + 1);
	}
	for (int d = 0; d < MAX_AUDIO_DEVICE; d++)
	{
		if (g_mixer_stream[d])
		{
			sprintf(buf, "vi : mixer_stream[device #%d] %d", d + 1, g_mixer_stream[d]);
			mlog_out(buf);
		}
}
	for (int s = 0; s < VI_MAX; s++)
	{
		if (g_vi_opened[s].mstream)
			mlog_out("vi : midi[stream #%d] , %d", s, g_vi_opened[s].mstream + 1);
		if (g_vi_opened[s].sf2_midifont)
			mlog_out("vi : midifont[stream #%d] %d", s, g_vi_opened[s].sf2_midifont);
	}
	mlog_out("=============================================== end device");
}
static void inspect()
{
	inspect_device();
	inspect_channel();
	inspect_note();
	inspect_queue();
}
*/
#ifdef V_ASIO
DWORD CALLBACK asioProc(BOOL input, DWORD channel, void *buffer, DWORD length, void *user)
{
	DWORD c = BASS_ChannelGetData((DWORD)user, buffer, length);
	if (c == -1) c = 0; // an error, no data
	return c;
}
#endif

#ifdef V_VST
// VSTi : C callbacks  Main host callback
extern "C" { VstIntPtr VSTCALLBACK hostCallback(AEffect *effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void *ptr, float opt);}
typedef AEffect *(*vstPluginFuncPtr)(audioMasterCallback host);// Plugin's entry point
typedef VstIntPtr(*dispatcherFuncPtr)(AEffect *effect, VstInt32 opCode,	VstInt32 index, VstInt32 value, void *ptr, float opt);// Plugin's dispatcher function
typedef float(*getParameterFuncPtr)(AEffect *effect, VstInt32 index);// Plugin's getParameter() method
typedef void(*setParameterFuncPtr)(AEffect *effect, VstInt32 index, float value);// Plugin's setParameter() method
typedef VstInt32(*processEventsFuncPtr)(VstEvents *events);// Plugin's processEvents() method
typedef void(*processFuncPtr)(AEffect *effect, float **inputs,	float **outputs, VstInt32 sampleFrames);// Plugin's process() method
extern "C"
{
	VstIntPtr VSTCALLBACK hostCallback(AEffect *effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void *ptr, float opt)
	{
		switch (opcode)
		{
		case audioMasterVersion:				// VST Version supported (for example 2200 for VST 2.2) --
			return kVstVersion;					// 2 for VST 2.00, 2100 for VST 2.1, 2200 for VST 2.2 etc.
		case audioMasterGetSampleRate:
			return M_SAMPLE_RATE;
		case audioMasterGetVendorString:		// fills <ptr> with a string identifying the vendor (max 64 char)
			strcpy((char*)ptr, "Expresseur" /*max 64 char!*/);
			return(true);
		case audioMasterGetProductString:		// fills <ptr> with a string with product name (max 64 char)
			strcpy((char*)ptr, "ExpresseurV3");
			return(true);
		case audioMasterGetVendorVersion:		// returns vendor-specific version
			return(3);
			break;
		case audioMasterCanDo:					// string in ptr, see below
			if ((strcmp((char*)ptr, "supplyidle") == 0
				|| strcmp((char*)ptr, "sendvstmidievent") == 0	// ... esp. MIDI event for VSTi
				|| strcmp((char*)ptr, "startstopprocess") == 0))	// we calls effStartProcess  and effStopProcess
				return 1;
			else
				return 0;
			break;
		case audioMasterCurrentId:				// Returns the unique id of a plug that's currently loading
		case audioMasterIdle:
		case audioMasterUpdateDisplay: // the plug-in reported an update (e.g. after a program load/rename or any other param change)
		case audioMasterGetTime:
		case audioMasterSizeWindow:				// index: width, value: height
		case audioMasterGetLanguage:			// see enum
		case audioMasterOpenFileSelector:		// open a fileselector window with VstFileSelect* in <ptr>
		case audioMasterCloseFileSelector:
			return 0;
		}
		return 0;
	}
}
static void vsti_send_shortmsg(int vsti_nr, T_midimsg midimsg)
{
	T_vi_opened *vi = &(g_vi_opened[vsti_nr]);
	if (((midimsg.bData[0] >> 4) == L_MIDI_CONTROL) &&  (midimsg.bData[1] != 0) && (midimsg.bData[2] != 99)) // bank 99
		vi->vsti_midi_prog = false ;
	if (((midimsg.bData[0] >> 4) == L_MIDI_PROGRAM) && (vi->vsti_midi_prog == false) && (midimsg.bData[1] != vi->vsti_last_prog))
	{
		// send the VST program
		vi->vsti_last_prog = midimsg.bData[1];
		vi->vsti_todo_prog = true;
		vi->vsti_midi_prog = true ;
		return;
	}
	if (vi->vsti_nb_pending_midimsg >= MAX_VSTI_PENDING_MIDIMSG)
		return;
	vi->vsti_pending_midimsg[vi->vsti_nb_pending_midimsg].dwData = midimsg.dwData;
	(g_vi_opened[vsti_nr].vsti_nb_pending_midimsg)++;
}
static void vsti_init()
{
	// malloc for an data-structure in VST-DSK, to send block of midi-msg :-/
	g_vsti_events = (VstEvents *)malloc(sizeof(VstEvents)+MAX_VSTI_PENDING_MIDIMSG*(sizeof(VstEvent *)));;
	g_vsti_events->numEvents = 0;
	g_vsti_events->reserved = 0;
	for (int nrEvent = 0; nrEvent < MAX_VSTI_PENDING_MIDIMSG; nrEvent++)
	{
		VstEvent *mvstevent = (VstEvent *)malloc(sizeof(VstMidiEvent));;
		g_vsti_events->events[nrEvent] = mvstevent;
		VstMidiEvent *midiEvent = (VstMidiEvent *)mvstevent;
		midiEvent->type = kVstMidiType;
		midiEvent->byteSize = sizeof(VstMidiEvent);
		midiEvent->midiData[0] = 0;
		midiEvent->midiData[1] = 0;
		midiEvent->midiData[2] = 0;
		midiEvent->midiData[3] = 0;
		midiEvent->deltaFrames = 0;	///< sample frames related to the current block start sample position
		midiEvent->flags = kVstMidiEventIsRealtime;			///< @see VstMidiEventFlags
		midiEvent->noteLength = 0;	 ///< (in sample frames) of entire note, if available, else 0
		midiEvent->noteOffset = 0;	 ///< offset (in sample frames) into note from note start if available, else 0
		midiEvent->detune = 0;			///< -64 to +63 cents; for scales other than 'well-tempered' ('microtuning')
		midiEvent->noteOffVelocity = 0;	///< Note Off Velocity [0, 127]
		midiEvent->reserved1 = 0;			///< zero (Reserved for future use)
		midiEvent->reserved2 = 0;			///< zero (Reserved for future use)
	}
}
static void vsti_free()
{
	if (g_vsti_events == NULL)
		return;
	for (int nrEvent = 0; nrEvent < MAX_VSTI_PENDING_MIDIMSG; nrEvent++)
	{
		VstEvent *mvstevent = g_vsti_events->events[nrEvent];
		free(mvstevent);
	}
	free(g_vsti_events);
	g_vsti_events = NULL;
}
#ifdef V_PC
static bool closeVSTi(T_vi_opened *vi)
{
    if ( vi->vsti_modulePtr == NULL)
        return false ;
    if (vi->vsti_plugins != NULL)
        vi->vsti_plugins->dispatcher(vi->vsti_plugins, effClose, 0, 0, NULL, 0.0f);
    vi->vsti_plugins = NULL;
    FreeLibrary(vi->vsti_modulePtr);
    vi->vsti_modulePtr = NULL ;
    return true ;
}
static bool openVSTi(const char *fname , T_vi_opened *vi)
{
    vi->vsti_plugins = NULL ;
    vi->vsti_modulePtr = NULL ;
    
    wchar_t wtext[MAXBUFCHAR];
    mbstowcs(wtext, fname, strlen(fname) + 1);//Plus null
    LPWSTR sdll = wtext;
    vi->vsti_modulePtr = LoadLibrary(sdll);
    if (vi->vsti_modulePtr == NULL)
    {
        mlog_out("Failed trying to load VST from <%s>, error %d",	fname, GetLastError());
        return false;
    }
    
    LPCSTR spoint = "VSTPluginMain";
    vstPluginFuncPtr mainEntryPoint = (vstPluginFuncPtr)GetProcAddress(vi->vsti_modulePtr, spoint);
    if (mainEntryPoint == NULL)
    {
        spoint = "main";
        mainEntryPoint = (vstPluginFuncPtr)GetProcAddress(vi->vsti_modulePtr, spoint);
        if ( mainEntryPoint == NULL )
        {
            spoint = "main_macho";
            mainEntryPoint = (vstPluginFuncPtr)GetProcAddress(vi->vsti_modulePtr, spoint);
            if ( mainEntryPoint == NULL )
            {
                mlog_out("Failed VSTPluginMain VST from <%s>, error %d", fname, GetLastError());
                closeVSTi(vi) ;
                return false;
            }
        }
    }
    // Instantiate the plugin
    vi->vsti_plugins = mainEntryPoint(hostCallback);
    if(vi->vsti_plugins == NULL)
    {
        mlog_out("Plugin's main() returns null for VSTi %s",fname);
        closeVSTi(vi);
        return false;
    }

    if (vi->vsti_plugins->magic != kEffectMagic) {
        mlog_out("Plugin magic number is bad <%s>",fname );
        closeVSTi(vi);
        return FALSE;
    }
    
    //dispatcher = (dispatcherFuncPtr)(plugin->dispatcher);
    //plugin->getParameter = (getParameterFuncPtr)plugin->getParameter;
    //plugin->processReplacing = (processFuncPtr)plugin->processReplacing;
    //plugin->setParameter = (setParameterFuncPtr)plugin->setParameter;
    
    
    int numOutputs = vi->vsti_plugins->numOutputs;
    if (numOutputs < 1)
    {
        mlog_out("Error : VST does not have stereo output <%s>", fname);
        closeVSTi(vi);
        return FALSE;
    }
    vi->vsti_nb_outputs = numOutputs;
    
    
    return true;
}
#endif // V_PC

#ifdef V_MAC
static bool closeVSTi(T_vi_opened *vi)
{
    if ( vi->vsti_modulePtr == NULL)
        return false ;
    if (vi->vsti_plugins != NULL)
        vi->vsti_plugins->dispatcher(vi->vsti_plugins, effClose, 0, 0, NULL, 0.0f);
    vi->vsti_plugins = NULL;
    CFBundleUnloadExecutable(vi->vsti_modulePtr);
    CFRelease(vi->vsti_modulePtr);
    vi->vsti_modulePtr = NULL ;
    return true ;
}
static bool openVSTi(const char *fname , T_vi_opened *vi)
{
    vi->vsti_plugins = NULL ;
    vi->vsti_modulePtr = NULL ;
    
    // Create a path to the bundle
    CFStringRef pluginPathStringRef = CFStringCreateWithCString(NULL,fname, kCFStringEncodingASCII);
    CFURLRef bundleUrl = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,pluginPathStringRef, kCFURLPOSIXPathStyle, true);
    if(bundleUrl == NULL)
    {
        mlog_out("Failed trying to load VST from <%s>",	fname);
        return false;
    }
    
    // Open the bundle
    vi->vsti_modulePtr = CFBundleCreate(kCFAllocatorDefault, bundleUrl);
    if(vi->vsti_modulePtr == NULL)
    {
        mlog_out("Couldn't create bundle reference for VSTi %s" , fname);
        CFRelease(pluginPathStringRef);
        vi->vsti_modulePtr = NULL ;
        return false;
    }
    
    // Clean up
    CFRelease(pluginPathStringRef);
    CFRelease(bundleUrl);
    
    vstPluginFuncPtr mainEntryPoint = NULL;
    mainEntryPoint = (vstPluginFuncPtr)CFBundleGetFunctionPointerForName(vi->vsti_modulePtr,CFSTR("VSTPluginMain"));
    // VST plugins previous to the 2.4 SDK used main_macho for the entry point name
    if(mainEntryPoint == NULL)
    {
        mainEntryPoint = (vstPluginFuncPtr)CFBundleGetFunctionPointerForName(vi->vsti_modulePtr, CFSTR("main"));
        if(mainEntryPoint == NULL)
        {
            mainEntryPoint = (vstPluginFuncPtr)CFBundleGetFunctionPointerForName(vi->vsti_modulePtr, CFSTR("main_macho"));
        }
    }
    
    if(mainEntryPoint == NULL)
    {
        mlog_out("Couldn't get a pointer to plugin's main() for VSTi %s",fname);
        closeVSTi(vi);
       return false;
    }
    
    audioMasterCallback hostCallbackFuncPtr ;
    hostCallbackFuncPtr = (audioMasterCallback)hostCallback;
    vi->vsti_plugins = mainEntryPoint(hostCallbackFuncPtr);
    if(vi->vsti_plugins == NULL)
    {
        mlog_out("Plugin's main() returns null for VSTi %s",fname);
        closeVSTi(vi);
        return false;
    }

    if (vi->vsti_plugins->magic != kEffectMagic) {
        mlog_out("Plugin magic number is bad <%s>",fname );
        closeVSTi(vi);
        return FALSE;
    }

    //dispatcher = (dispatcherFuncPtr)(plugin->dispatcher);
    //plugin->getParameter = (getParameterFuncPtr)plugin->getParameter;
    //plugin->processReplacing = (processFuncPtr)plugin->processReplacing;
    //plugin->setParameter = (setParameterFuncPtr)plugin->setParameter;
    
    
    int numOutputs = vi->vsti_plugins->numOutputs;
    if (numOutputs < 1)
    {
        mlog_out("Error : VST does not have stereo output <%s>", fname);
        closeVSTi(vi);
       return FALSE;
    }
    vi->vsti_nb_outputs = numOutputs;
    
    
    return true;
}
#endif // V_MAC

static bool vsti_start(const char *fname, int vsti_nr)
{
	T_vi_opened *vi = &(g_vi_opened[vsti_nr]);
    bool ret_code ;
    ret_code = openVSTi(fname , vi);
    if ( ! ret_code )
        return false ;

	vi->vsti_outputs = (float**)malloc(sizeof(float*)* vi->vsti_nb_outputs);
	for (int channel = 0; channel < vi->vsti_nb_outputs; channel++)
		vi->vsti_outputs[channel] = (float*)malloc(sizeof(float)* VSTI_BUFSIZE);
	for (int channel = 0; channel < vi->vsti_nb_outputs; ++channel)
	for (long frame = 0; frame < VSTI_BUFSIZE; ++frame)
		vi->vsti_outputs[channel][frame] = 0.0f;


	MidiProgramName mProgram;
	mProgram.thisProgramIndex = 0;
    VstIntPtr nbProgram ;
    nbProgram = vi->vsti_plugins->dispatcher(vi->vsti_plugins, effGetMidiProgramName, 0, 0, &mProgram, 0.0f);

	vi->vsti_plugins->dispatcher(vi->vsti_plugins, effOpen, 0, 0, NULL, 0.0f);

	// Set some default properties
	float sampleRate = (float)(M_SAMPLE_RATE);
	vi->vsti_plugins->dispatcher(vi->vsti_plugins, effSetSampleRate, 0, 0, NULL, sampleRate);
	vi->vsti_plugins->dispatcher(vi->vsti_plugins, effSetBlockSize, 0, VSTI_BUFSIZE, NULL, 0.0f);

	vi->vsti_plugins->dispatcher(vi->vsti_plugins, effMainsChanged, 0, 1, NULL, 0.0f);

	return true;
}
static void vsti_stop(int vsti_nr)
{
	T_vi_opened *vi = &(g_vi_opened[vsti_nr]);
	if (vi->mstream != 0)
		BASS_StreamFree(vi->mstream);
	vi->mstream = 0;
    closeVSTi(vi);
	if (vi->vsti_outputs != NULL)
	{
		for (int channel = 0; channel < vi->vsti_nb_outputs; channel++)
		{
			if (vi->vsti_outputs[channel] != NULL)
				free(vi->vsti_outputs[channel]);
			vi->vsti_outputs[channel] = NULL;
		}
		free(vi->vsti_outputs);
		vi->vsti_outputs = NULL;
	}
}
DWORD CALLBACK vsti_streamProc(HSTREAM handle, void *buffer, DWORD length, void * pvsti_nr)
{
    VstIntPtr intptr = (VstIntPtr)pvsti_nr ;
    int vsti_nr = (int)intptr;
	T_vi_opened *vi = &(g_vi_opened[vsti_nr]);

	lock_mutex_out();
	// send pending vst-program
	if (vi->vsti_todo_prog)
	{
		g_vi_opened[vsti_nr].vsti_plugins->dispatcher(g_vi_opened[vsti_nr].vsti_plugins, effSetProgram, 0, vi->vsti_last_prog, NULL, 0.0f);
		vi->vsti_todo_prog = false;
	}
	// send pending midi messages
	if (vi->vsti_nb_pending_midimsg > 0)
	{
		g_vsti_events->numEvents = vi->vsti_nb_pending_midimsg;
		for (int nrEvent = 0; nrEvent < vi->vsti_nb_pending_midimsg; nrEvent++)
		{
			VstEvent *mvstevent = g_vsti_events->events[nrEvent] ;
			VstMidiEvent *midiEvent = (VstMidiEvent *)mvstevent;
			midiEvent->midiData[0] = vi->vsti_pending_midimsg[nrEvent].bData[0];
			midiEvent->midiData[1] = vi->vsti_pending_midimsg[nrEvent].bData[1];
			midiEvent->midiData[2] = vi->vsti_pending_midimsg[nrEvent].bData[2];
		}
		vi->vsti_plugins->dispatcher(vi->vsti_plugins, effProcessEvents, 0, 0, g_vsti_events, 0.0f);
		vi->vsti_nb_pending_midimsg = 0;
	}
	unlock_mutex_out();


	float *fbuf = (float *)buffer;
	int nbfloat = length / (sizeof(float) * vi->vsti_nb_outputs);
	float ** ouput = vi->vsti_outputs;
	vi->vsti_plugins->processReplacing(vi->vsti_plugins, NULL, ouput, nbfloat);
	float *pt[10];
	for (int nr_channel = 0; nr_channel < vi->vsti_nb_outputs; nr_channel++)
		pt[nr_channel] = ouput[0];
	for (long frame = 0; frame < nbfloat; ++frame)
	{
		for (int nr_channel = 0; nr_channel < vi->vsti_nb_outputs; nr_channel++)
		{
			*fbuf = *(pt[nr_channel]);
			(pt[nr_channel]) ++;
			fbuf++;
		}
	}
	return length;
}
#endif // V_VST

static void sf2_send_shortmsg(int nr_device, T_midimsg msg)
{
	BYTE channel = msg.bData[0] & 0x0F;
	HSTREAM mstream = g_vi_opened[nr_device].mstream;
	// mlog_out("vi_send nr_device = %d , channel =%d", nr_device, channel);
	static int vi_rpn_msb = 0;
	static int vi_rpn_lsb = 0;
	switch (msg.bData[0] >> 4)
	{
	case L_MIDI_NOTEON: BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_NOTE, MAKEWORD(msg.bData[1], msg.bData[2])); break;
	case L_MIDI_NOTEOFF: BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_NOTE, MAKEWORD(msg.bData[1], 0)); break;
	case L_MIDI_PROGRAM: BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_PROGRAM, msg.bData[1]); break;
	case L_MIDI_PITCHBEND:	BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_PITCH, pitchbend_value(msg) + 0x2000); break;
	case L_MIDI_CHANNELPRESSURE: BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_CHANPRES, msg.bData[1]); break;
	case L_MIDI_CONTROL:
		switch (msg.bData[1])
		{
		case 0: BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_BANK, msg.bData[2]); break;
		case 1: BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_MODULATION, msg.bData[2]); break;
		case 5: BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_PORTATIME, msg.bData[2]); break;
		case 7:	BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_VOLUME, msg.bData[2]); break;
		case 10: BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_PAN, msg.bData[2]); break;
		case 11: BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_EXPRESSION, msg.bData[2]); break;
		case 64: BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_SUSTAIN, msg.bData[2]); break;
		case 65: BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_PORTAMENTO, msg.bData[2]); break;
		case 71: BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_RESONANCE, msg.bData[2]); break;
		case 72: BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_RELEASE, msg.bData[2]); break;
		case 73: BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_ATTACK, msg.bData[2]); break;
		case 74: BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_CUTOFF, msg.bData[2]); break;
		case 84: BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_PORTANOTE, msg.bData[2]); break;
		case 91: BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_REVERB, msg.bData[2]); break;
		case 93: BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_CHORUS, msg.bData[2]); break;
		case 120: BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_SOUNDOFF, 0); break;
		case 121: BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_RESET, 0); break;
		case 123: BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_NOTESOFF, 0); break;
		case 126: BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_MODE, msg.bData[2]); break;
		case 127: BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_MODE, msg.bData[2]); break;

		case 100: vi_rpn_msb = msg.bData[2]; break;
		case 101: vi_rpn_lsb = msg.bData[2]; break;
		case 6:
			if (vi_rpn_msb == 0)
			{
				switch (vi_rpn_lsb)
				{
				case 0:BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_PITCHRANGE, msg.bData[2]); break;
				case 1:BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_FINETUNE, msg.bData[2]); break;
				case 2:BASS_MIDI_StreamEvent(mstream, channel, MIDI_EVENT_COARSETUNE, msg.bData[2]); break;
				default: break;
				}
			}
			break;

		default: break; // other CTRL CHG ignored
		}
	default: break;
	}
}
static bool sf2_create_list_prog(const char *fname)
{
	HSOUNDFONT hvi = BASS_MIDI_FontInit((void*)fname, 0);
	if (hvi == 0)
	{
		mlog_out("Error BASS_MIDI_FontInit %s, err#%d",fname, BASS_ErrorGetCode());
		return(false);
	}
	const char *name_preset;
	FILE *ftxt;
	char fnameext[MAXBUFCHAR];
	if ((strlen(fname) > 5) && ((strncmp(fname + strlen(fname) - 4, ".sf2", 4) == 0) || (strncmp(fname + strlen(fname) - 4, ".SF2", 4) == 0)))
	{
		char fnamessext[MAXBUFCHAR];
		strcpy(fnamessext, fname);
		fnamessext[strlen(fname) - 4] = '\0';
		sprintf(fnameext, "%s.txt", fnamessext);
	}
	else
		sprintf(fnameext, "%s.txt", fname);
	if ((ftxt = fopen(fnameext, "r")) != NULL)
	{
		fclose(ftxt);
		return(true);
	}
	if ((ftxt = fopen(fnameext, "w")) == NULL)
	{
		mlog_out("mlog_out opening vi text list %s err=%d\n", fnameext, errno);
		return false;
	}
	for (int bank = 0; bank < 127; bank++)
	{
		for (int program = 0; program < 127; program++)
		{
			name_preset = BASS_MIDI_FontGetPreset(hvi, program, bank);
			if (name_preset != 0)
				fprintf(ftxt, "%s(P%d/%d)\n", name_preset, bank, program);
		}
	}
	fclose(ftxt);
	return true;
}
#ifdef V_VST
static bool vst_create_list_prog(const char *fname)
{
    T_vi_opened vi ;
    bool ret_code ;
    ret_code = openVSTi(fname , &vi);
    if ( ! ret_code )
        return false ;
    
	FILE *ftxt;
	char fnameext[MAXBUFCHAR];
	if ((strlen(fname) > 5) && ((strncmp(fname + strlen(fname) - 4, ".dll", 4) == 0) || (strncmp(fname + strlen(fname) - 4, ".DLL", 4) == 0)))
	{
		char fnamessext[MAXBUFCHAR];
		strcpy(fnamessext, fname);
		fnamessext[strlen(fname) - 4] = '\0';
		sprintf(fnameext, "%s.txt", fnamessext);
	}
	else
		sprintf(fnameext, "%s.txt", fname);
	if ((ftxt = fopen(fnameext, "r")) != NULL)
	{
		fclose(ftxt);
		return(true);
	}
	if ((ftxt = fopen(fnameext, "w")) == NULL)
	{
		mlog_out("mlog_out opening vi text list %s err=%d\n", fnameext, errno);
		closeVSTi(&vi);
		return false;
	}

	MidiProgramName mProgram;
	mProgram.thisProgramIndex = 0;
    VstIntPtr nbProgram ;
    nbProgram = vi.vsti_plugins->dispatcher(vi.vsti_plugins, effGetMidiProgramName, 0, 0, &mProgram, 0.0f);
	if (nbProgram > 0)
	{
		for (int nrProgram = 0; nrProgram < nbProgram; nrProgram++)
		{
			mProgram.thisProgramIndex = nrProgram;
			vi.vsti_plugins->dispatcher(vi.vsti_plugins, effGetMidiProgramName, 0, 0, &mProgram, 0.0f);
			if ((mProgram.midiProgram >= 0) && (mProgram.midiBankLsb < 0) && (mProgram.midiBankMsb < 0))
				fprintf(ftxt, "%s(P%d)\n", mProgram.name, mProgram.midiProgram);
			if ((mProgram.midiProgram >= 0) && (mProgram.midiBankLsb >= 0) && (mProgram.midiBankMsb < 0))
				fprintf(ftxt, "%s(P%d/%d)\n", mProgram.name, mProgram.midiBankLsb, mProgram.midiProgram);
			if ((mProgram.midiProgram >= 0) && (mProgram.midiBankLsb >= 0) && (mProgram.midiBankMsb >= 0))
				fprintf(ftxt, "%s(P%d/%d/%d)\n", mProgram.name, mProgram.midiBankMsb, mProgram.midiBankLsb, mProgram.midiProgram);

		}
	}
	else
	{
		int numPrograms = vi.vsti_plugins->numPrograms;
		char nameProgram[kVstMaxProgNameLen];
		for (int nrProgram = 0; nrProgram < numPrograms; nrProgram++)
		{
			bool retCode = vi.vsti_plugins->dispatcher(vi.vsti_plugins, effGetProgramNameIndexed, nrProgram, 0, nameProgram, 0.0f);
			if (retCode)
			{
				fprintf(ftxt, "%s_vst(P99/%d)\n", nameProgram, nrProgram);
			}
		}
	}

	fclose(ftxt);
    closeVSTi(&vi);
	return true;
}
#endif

static void sf2_stop(int vsti_nr)
{
	T_vi_opened *vi = &(g_vi_opened[vsti_nr]);
	BASS_MIDI_FontFree(vi->sf2_midifont);
	vi->sf2_midifont = 0;
	BASS_StreamFree(vi->mstream);
	vi->mstream = 0;
}
static int mixer_create(int nr_deviceaudio)
{
	if ( nr_deviceaudio < 0 )
		return -1 ;

	if (nr_deviceaudio < g_nb_audio_bass_device)
	{
		// non asio device
		if (!g_mixer_stream[nr_deviceaudio])
		{
			// audio device not yet set
			if (BASS_Init(nr_deviceaudio, M_SAMPLE_RATE, BASS_DEVICE_STEREO | BASS_DEVICE_LATENCY , 0, NULL) == FALSE)
			{
				mlog_out("Error BASS_Init device#%d, err=%d\n", nr_deviceaudio + 1, BASS_ErrorGetCode());
				return(-1);
			}
			if (BASS_SetDevice(nr_deviceaudio) == FALSE)
			{
				mlog_out("Error BASS_SetDevice device#%d, err=%d\n", nr_deviceaudio + 1, BASS_ErrorGetCode());
				return(-1);
			}
			if (g_audio_updateperiod[nr_deviceaudio] > 0 )
				BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, g_audio_updateperiod[nr_deviceaudio]);
			DWORD updateperiod = BASS_GetConfig(BASS_CONFIG_UPDATEPERIOD); // get update period
			BASS_INFO info;
			BASS_GetInfo(&info); // retrieve device info
			DWORD len = updateperiod + info.minbuf + 1 + g_audio_buffer[nr_deviceaudio] ; // add the 'minbuf' plus 1ms margin plus additional margin
			BASS_SetConfig(BASS_CONFIG_BUFFER, len); // set the buffer length
			
			g_mixer_stream[nr_deviceaudio] = BASS_Mixer_StreamCreate(M_SAMPLE_RATE , 2, BASS_SAMPLE_FLOAT );
			if (g_mixer_stream[nr_deviceaudio] == 0)
			{
				mlog_out("Error BASS_Mixer_StreamCreate , err=%d\n", BASS_ErrorGetCode());
				return(-1);
			}
			if (BASS_ChannelPlay(g_mixer_stream[nr_deviceaudio], FALSE) == FALSE)
			{
				mlog_out("Error BASS_ChannelPlay , err=%d\n", BASS_ErrorGetCode());
				return(-1);
			}

			mlog_out("Information : audio mixer device#%d . updateperiod=%dms ; minbuf=%d ms ; len=%d ; latency=%d ms : OK", nr_deviceaudio + 1, updateperiod, info.minbuf, len, info.latency);
			//mlog_out("Information : audio mixer device#%d : OK", nr_deviceaudio + 1);
		}
	}

#ifdef V_ASIO
	if (nr_deviceaudio >= g_nb_audio_bass_device)
	{
		// asio
		if (g_audio_open[nr_deviceaudio])
			return nr_deviceaudio;
		// not playing anything via BASS, so don't need an update thread
		BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, 0);
		// setup BASS - "no sound" device
		BASS_Init(0, 48000, 0, 0, NULL);
		g_audio_open[nr_deviceaudio] = true;

		int nr_deviceasio = nr_deviceaudio - g_nb_audio_bass_device;
		if (BASS_ASIO_Init(nr_deviceasio, 0) == FALSE)
		{
			mlog_out("Error BASS_ASIO_Init device#%d , err=%d\n", nr_deviceaudio + 1, BASS_ASIO_ErrorGetCode());
			return(-1);
		}
		BASS_ASIO_SetDevice(nr_deviceasio);
		BASS_ASIO_SetRate(M_SAMPLE_RATE);
		g_mixer_stream[nr_deviceaudio] = BASS_Mixer_StreamCreate(M_SAMPLE_RATE, 2, BASS_STREAM_DECODE | BASS_SAMPLE_FLOAT);
		if (!g_mixer_stream[nr_deviceaudio])
		{
			mlog_out("Error asio BASS_Mixer_StreamCreate, err=%d\n", BASS_ErrorGetCode());
			return(-1);
		}
		// setup ASIO stuff
		BASS_CHANNELINFO i;
		BASS_ChannelGetInfo(g_mixer_stream[nr_deviceaudio], &i);
		BASS_ASIO_ChannelEnable(0, 0, &asioProc, (void*)g_mixer_stream[nr_deviceaudio]); // enable 1st output channel...
		for (unsigned int a = 1; a < i.chans; a++)
			BASS_ASIO_ChannelJoin(0, a, 0); // and join the next channels to it
		if (i.chans == 1) BASS_ASIO_ChannelEnableMirror(1, 0, 0); // mirror mono channel to form stereo output
		BASS_ASIO_ChannelSetFormat(0, 0, BASS_ASIO_FORMAT_FLOAT); // set the source format (float)
		BASS_ASIO_ChannelSetRate(0, 0, i.freq); // set the source rate
		BASS_ASIO_SetRate(i.freq); // try to set the device rate too (saves resampling)
		if (!BASS_ASIO_Start(0, 0)) // start output using default buffer/latency
		{
			mlog_out("Error BASS_ASIO_start device#%d , err=%d\n", nr_deviceaudio + 1, BASS_ASIO_ErrorGetCode());
			return -1;
		}
		else
			mlog_out("Information : ASIO start #device %d OK", nr_deviceaudio + 1);
	}
#endif

	return(nr_deviceaudio);
}
static void mixer_init()
{
	for (int n = 0; n < MAX_AUDIO_DEVICE; n++)
	{
		g_mixer_stream[n] = 0;
		g_audio_open[n] = false;
	}

	// list of standard audio device
	g_nb_audio_bass_device = 0;
	char name_audio[MAXBUFCHAR];
	while (audio_bass_name(g_nb_audio_bass_device, name_audio))
	{
		if (g_audio_first_list)
			mlog_out("Information : Audio interface #%d <%s>", g_nb_audio_bass_device + 1, name_audio);
		g_nb_audio_bass_device++;
		if ( g_nb_audio_bass_device >= MAX_AUDIO_DEVICE)
		{
			mlog_out("mixer_init : only first %d audio are managed", MAX_AUDIO_DEVICE);
			g_nb_audio_bass_device = MAX_AUDIO_DEVICE - 1 ;
			break ;
		}
	}

	// list of asio device
	g_nb_asio_device = 0;
	while (asio_name(g_nb_asio_device, name_audio))
	{
		if (g_audio_first_list)
			mlog_out("Information : Audio interface #%d <%s>", g_nb_audio_bass_device + g_nb_asio_device + 1, name_audio);
		g_nb_asio_device++;
	}
	g_audio_first_list = false;
}
static void mixer_free()
{
	for (int nr_audio_device = 0; nr_audio_device < MAX_AUDIO_DEVICE; nr_audio_device++)
	{
		if (g_mixer_stream[nr_audio_device] > 0)
		{
			if (BASS_StreamFree(g_mixer_stream[nr_audio_device]) == FALSE)
				mlog_out("Error free mixer on device audio #%d Err=%d", nr_audio_device + 1, BASS_ErrorGetCode());
			else
				mlog_out("Information : free mixer on device audio #%d OK", nr_audio_device + 1);
			if (nr_audio_device < g_nb_audio_bass_device)
			{
				//non asio
				if (BASS_SetDevice(nr_audio_device) == FALSE)
				{
					mlog_out("Error BASS_SetDevice device#%d, err=%d\n", nr_audio_device + 1, BASS_ErrorGetCode());
				}
				BASS_Free();
			}
#ifdef V_ASIO
			if (nr_audio_device >= g_nb_audio_bass_device)
			{
				// asio 
				BASS_ASIO_SetDevice(nr_audio_device);
				if (BASS_ASIO_Stop() == FALSE)
					mlog_out("Error stop ASIO device audio #%d Err=%d", nr_audio_device + 1, BASS_ErrorGetCode());
				else
					mlog_out("Information : stop ASIO device#%d OK", nr_audio_device + 1);
				if (BASS_ASIO_Free() == FALSE)
					mlog_out("Error free ASIO device#%d Err=%d", nr_audio_device + 1, BASS_ErrorGetCode());
				else
					mlog_out("Information : free ASIO device#%d  OK", nr_audio_device + 1);
			}
#endif
		}
		g_mixer_stream[nr_audio_device] = 0;
	}
}
static int vi_open(const char *fname, int nr_deviceaudio, bool sf2)
{
	T_vi_opened *vi;
	// fname is the full name ( *.dll for a vsti or *.sf2 for an sf2 )
	for (int nr_vi = 0; nr_vi < VI_MAX; nr_vi++)
	{
		vi = &(g_vi_opened[nr_vi]);
		if ((strcmp(vi->filename, fname) == 0) && (vi->nr_device_audio == nr_deviceaudio))
		{
			// VI allready opened in the same audio channel
			mlog_out("Information : open vi<%s> audio-device#%d : already open", fname, nr_deviceaudio + 1);
			return (nr_vi);
		}
	}
	int nr_vi = g_vi_opened_nb;
	g_vi_opened_nb++;
	vi = &(g_vi_opened[nr_vi]);
	// new VI stream to create open vi
	strcpy(vi->filename, fname);
	vi->nr_device_audio = nr_deviceaudio;

	if (mixer_create(nr_deviceaudio) == -1) return(-1);
	if (sf2)
	{
		// connect a midi-channel on the mixer-device
		vi->mstream = BASS_MIDI_StreamCreate(MAXCHANNEL, BASS_STREAM_DECODE | BASS_SAMPLE_FLOAT, M_SAMPLE_RATE);
		if (vi->mstream == 0)
		{
			mlog_out("Error BASS_MIDI_StreamCreate VI, err=%d", BASS_ErrorGetCode());
			return(-1);
		}
		if (BASS_Mixer_StreamAddChannel(g_mixer_stream[nr_deviceaudio], vi->mstream, 0) == FALSE)
		{
			mlog_out("Error BASS_Mixer_StreamAddChannel VI , err=%d", BASS_ErrorGetCode());
			return -1;
		}
		vi->sf2_midifont = BASS_MIDI_FontInit((void*)fname, 0);
		if (vi->sf2_midifont == 0)
		{
			mlog_out("Error BASS_MIDI_FontInit <%s> , err=%d", fname, BASS_ErrorGetCode());
			return -1;
		}
		// connect a font in the midi-channel
		if (BASS_MIDI_FontLoad(vi->sf2_midifont, -1, -1) == FALSE)
		{
			mlog_out("Error BASS_MIDI_FontLoad <%s>, err=%d", fname, BASS_ErrorGetCode());
			return -1;
		}
		BASS_MIDI_FONT mfont;
		mfont.font = vi->sf2_midifont;
		mfont.preset = -1;
		mfont.bank = 0;
		if (BASS_MIDI_StreamSetFonts(vi->mstream, &mfont, 1) == FALSE)
		{
			mlog_out("Error BASS_MIDI_StreamSetFonts <%s> , err=%d", fname, BASS_ErrorGetCode());
			return -1;
		}
	}

#ifdef V_VST
	if (! sf2)
	{
		// load the vsti
		if (vsti_start(fname, nr_vi) == false)
		{
			mlog_out("Error vsti_start vi <%s>", fname);
			return(-1);
		}
		// connect the vsti on a new stream, via a callback vsti_streamProc
        VstIntPtr intptr = nr_vi ;
        void *voidptr = (void*)intptr ;
		vi->mstream = BASS_StreamCreate(M_SAMPLE_RATE, vi->vsti_nb_outputs, BASS_STREAM_DECODE | BASS_SAMPLE_FLOAT, &vsti_streamProc, voidptr);
		if (vi->mstream == 0)
		{
			mlog_out("Error BASS_MIDI_StreamCreate vi<%s>, err=%d", fname , BASS_ErrorGetCode());
			return(-1);
		}
		// connect the stream-channel on the mixer-device
		if (BASS_Mixer_StreamAddChannel(g_mixer_stream[nr_deviceaudio], vi->mstream, BASS_MIXER_DOWNMIX | BASS_MIXER_NORAMPIN ) == FALSE)
		{
			mlog_out("Error BASS_Mixer_StreamAddChannel vi<%s> , err=%d", fname, BASS_ErrorGetCode());
			return(-1);
		}
	}
#endif

	mlog_out("Information : open vi<%s> audio-device#%d : OK", fname, nr_deviceaudio + 1);
	return nr_vi;
}
static void vi_init()
{
	mixer_init();
	g_vi_opened_nb = 0;
	for (int n = 0; n < VI_MAX; n++)
	{
		g_vi_opened[n].mstream = 0;
		g_vi_opened[n].sf2_midifont = 0;
		g_vi_opened[n].filename[0] = '\0';
		g_vi_opened[n].nr_device_audio = -1;
#ifdef V_VST
		g_vi_opened[n].vsti_plugins = NULL;
		g_vi_opened[n].vsti_modulePtr = NULL;
		g_vi_opened[n].vsti_outputs = NULL;
		g_vi_opened[n].vsti_last_prog = -1;
		g_vi_opened[n].vsti_todo_prog = false;
		g_vi_opened[n].vsti_nb_pending_midimsg = 0;
		g_vi_opened[n].vsti_midi_prog = true;
		g_vi_opened[n].vsti_nb_outputs = 2;
#endif
	}
#ifdef V_VST
	vsti_init();
#endif
}
static void vi_free()
{
	for (int nr_vi = 0; nr_vi < g_vi_opened_nb; nr_vi++)
	{
		if (g_vi_opened[nr_vi].sf2_midifont != 0)
			sf2_stop(nr_vi);
#ifdef V_VST
		if (g_vi_opened[nr_vi].vsti_plugins != NULL)
			vsti_stop(nr_vi);
#endif
	}
	g_vi_opened_nb = 0;
#ifdef V_VST
	vsti_free();
#endif
}
static int sound_play(const char*fname, int volume, int pan, int nr_deviceaudio)
{
	/*
	bool retcode = BASS_Init(1, M_SAMPLE_RATE, 0 , 0, NULL);
	HSTREAM hmixer = BASS_Mixer_StreamCreate(M_SAMPLE_RATE , 2, BASS_SAMPLE_FLOAT );
	HSTREAM hsound = BASS_StreamCreateFile(FALSE, fname, 0, 0, BASS_STREAM_DECODE | BASS_SAMPLE_FLOAT);
	BASS_Mixer_StreamAddChannel(hmixer, hsound, BASS_STREAM_AUTOFREE);
	BASS_ChannelPlay(hmixer, FALSE);
	return hsound;
	*/

	if (mixer_create(nr_deviceaudio) == -1) 
		return(-1);

	HSTREAM hsound = BASS_StreamCreateFile(FALSE, fname, 0, 0, BASS_STREAM_DECODE | BASS_SAMPLE_FLOAT);
	if (!hsound)
		return(mlog_out("Error BASS_StreamCreateFile mixer %s, err=%d\n", fname, BASS_ErrorGetCode()));
	BASS_ChannelSetAttribute(hsound, BASS_ATTRIB_VOL, (float)(volume) / 64.0);
	BASS_ChannelSetAttribute(hsound, BASS_ATTRIB_PAN, (float)(pan - 64) / 64.0);
	if (BASS_Mixer_StreamAddChannel(g_mixer_stream[nr_deviceaudio], hsound, BASS_STREAM_AUTOFREE) == FALSE)
		return(mlog_out("Error BASS_Mixer_StreamAddChannel, err=%d\n", BASS_ErrorGetCode()));

	return(hsound);

}
static int sound_control(HSTREAM hsound, int volume, int pan, int ctrl)
{
	int return_code = BASS_ChannelSetAttribute(hsound, BASS_ATTRIB_VOL, (float)(volume) / 64.0);
	if (return_code)
	{
		BASS_ChannelSetAttribute(hsound, BASS_ATTRIB_PAN, (float)(pan - 64) / 64.0);
		switch (ctrl)
		{
		case 0: BASS_ChannelPause(hsound); break;
		case 1: BASS_ChannelPlay(hsound, FALSE); break;
		case 2:BASS_ChannelStop(hsound); break;
		default: break;
		}
	}
	return(return_code);
}
static void picth_init()
{
	for (int n = 0; n < OUT_MAX_DEVICE; n++)
	{
		for (int c = 0; c < MAXCHANNEL; c++)
		{
			for (int p = 0; p < MAXPITCH; p++)
			{
				g_midistatuspitch[n][c][p] = -1;
				g_miditimepitch[n][c][p] = 0;
				g_midistatuscontrol[n][c][p] = -1;
				g_miditimecontrol[n][c][p] = 0;
			}
		}
	}
}
static bool midiout_sysex(int nrTrack, const char *sysex)
{
	if ((nrTrack < 0) || (nrTrack >= MAXTRACK))
		return false;
	int nr_device = g_tracks[nrTrack].device;
	if ((nr_device < 0) || (nr_device >= MIDIOUT_MAX) || (g_midiout_rt[nr_device] == 0))
		return false;

	// translate ASCII Hexa format to binary byffer
	// e.g. GM-Modesysex = "F0 7E 7F 09 01 F7";

	std::vector<unsigned char> message;
	char *bufstr;
	bufstr = (char*)malloc(strlen(sysex) + 2);
	strcpy(bufstr, sysex);
	bufstr[strlen(sysex)] = '\0';
  char *pch = strtok(bufstr, " ,;.-");
	while (pch != NULL)
	{
		message.push_back( (BYTE)strtol(pch, NULL, 16) );
		pch = strtok(NULL, " ,;.-");
	}
	free(bufstr);
	g_midiout_rt[nr_device]->sendMessage( &message );
	return true ;
}
static void queue_insert(const T_midioutmsg midioutmsg)
{
	T_queue_msg* pt = g_queue_msg;
	bool found = false;
	for (int n = 0; n < OUT_QUEUE_MAX_MSG; n++ , pt ++)
	{
		if (pt->free)
		{
			if (n >= g_end_queue_msg)
				g_end_queue_msg = n + 1;
			found = true;
			break;
		}
	}
	if (! found)
	{
		return;
	}
	pt->free = false;
	pt->midioutmsg = midioutmsg;
	pt->t = g_current_out_t + midioutmsg.dt;
	if (g_end_queue_msg > g_max_queue_msg)
		g_max_queue_msg = g_end_queue_msg;
}
static bool sendmidimsg(T_midioutmsg midioutmsg, bool first);
static bool processPostMidiOut(T_midioutmsg midioutmsg)
{
	int type_msg = (midioutmsg.midimsg.bData[0] & 0xF0) >> 4;
	switch (type_msg)
	{
	case L_MIDI_NOTEON:
		if (midioutmsg.midimsg.bData[2] > 0)
		{
			if (!g_process_NoteOn) return(false);
			lua_getglobal(g_LUAoutState, LUAFunctionNoteOn);
		}
		else
		{
			if (!g_process_NoteOff) return(false);
			lua_getglobal(g_LUAoutState, LUAFunctionNoteOff);
		}
		break;
	case L_MIDI_NOTEOFF:
		if (!g_process_NoteOff) return(false);
		lua_getglobal(g_LUAoutState, LUAFunctionNoteOff);
		break;
	case L_MIDI_PROGRAM:
		if (!g_process_Program) return(false);
		lua_getglobal(g_LUAoutState, LUAFunctionProgram);
		break;
	case L_MIDI_CONTROL:
		if (!g_process_Control) return(false);
		lua_getglobal(g_LUAoutState, LUAFunctionControl);
		break;
	case L_MIDI_KEYPRESSURE:
		if (!g_process_KeyPressure) return(false);
		lua_getglobal(g_LUAoutState, LUAFunctionKeyPressure);
		break;
	case L_MIDI_CHANNELPRESSURE:
		if (!g_process_ChannelPressure) return(false);
		lua_getglobal(g_LUAoutState, LUAFunctionChannelPressure);
		break;
	case L_MIDI_CLOCK:
		if (!g_process_Clock) return(false);
		lua_getglobal(g_LUAoutState, LUAFunctionClock);
		break;
	case L_MIDI_SYSTEMCOMMON:
		if (!g_process_SystemCommon) return(false);
		lua_getglobal(g_LUAoutState, LUAFunctionSystemCommon);
		break;
	default:
		return(false);
	}
	lua_pushinteger(g_LUAoutState, midioutmsg.track + 1);
	int nbParam = 1;
	switch (type_msg)
	{
	case L_MIDI_CLOCK:
		break;
	case L_MIDI_CHANNELPRESSURE:
	case L_MIDI_PROGRAM:
		lua_pushinteger(g_LUAoutState, midioutmsg.midimsg.bData[1]); // program#
		nbParam += 1;
		break;
	case L_MIDI_PITCHBEND:
		lua_pushinteger(g_LUAoutState, pitchbend_value(midioutmsg.midimsg)); // pitchend value
		nbParam += 1;
		break;
	default:
		lua_pushinteger(g_LUAoutState, midioutmsg.midimsg.bData[1]); // pitch#
		lua_pushinteger(g_LUAoutState, midioutmsg.midimsg.bData[2]); // velocity for note, value for control , or msb pitchbend
		nbParam += 2;
		break;
	}
	if (lua_pcall(g_LUAoutState, nbParam, LUA_MULTRET, 0) != LUA_OK)
	{
		mlog_out("erreur onMidiOut calling LUA , err: %s", lua_tostring(g_LUAoutState, -1));
		lua_pop(g_LUAoutState, 1);
		return(false);
	}
	else
	{
		int dtIdMidioutPost = 10000;
		// pop midi-msgs from the returned values, to send it imediatly
		// syntax of retrunerd values :
		// list of MIDI : track 1..MAXTRACK , integer byte 1 ( e.g. pitch ), integer byte 2 ( e.g. velocity ), string type msg ( Noteon, Noteoff, Control, Program, Pressure, Keypressuer, Pichbend )
		// DMX values : "1/2/3/4/5/6/7/8" DMXall
		// DMX value : channel value DMX

		while (lua_gettop(g_LUAoutState) >= 1)
		{
			const char* stype = lua_tostring(g_LUAoutState, -1);
			lua_pop(g_LUAoutState, 1);
#ifdef V_MAC
			Byte data0;
			Byte nbbyte;
#else
			byte data0;
			byte nbbyte;
#endif
			data0 = 0;
			nbbyte = 3;

			int min = 0;
			bool midiok = false;
			switch (*stype)
			{
#ifdef V_DMX
			case 'D': // DMX or DMXall
			case 'd':
				if (strlen(stype) < 4)
				{
					if (lua_gettop(g_LUAoutState) >= 2)
					{
						// set oneDMX channel
						int v = cap(lua_tonumber(g_LUAoutState, -1),0,DMX_V_MAX,0);
						lua_pop(g_LUAoutState, 1);
						int c = cap(lua_tonumber(g_LUAoutState, -1),0,DMX_CH_MAX,0);
						lua_pop(g_LUAoutState, 1);
						g_dmx_float_target[c] = ((float)(v)) / ((float)DMX_V_MAX);
						if (g_dmx_ramping == 1.0)
						{
							// no ramping
							g_dmx_byte_value[c] = v;
							g_dmx_float_value[c] = g_dmx_float_target[c];
						}
					}
				}
				else
				{
					// reload all DMX values
					if (lua_gettop(g_LUAoutState) >= 1)
					{
						const char* dmxstr = lua_tostring(g_LUAoutState, -1);
						lua_pop(g_LUAoutState, 1);
						int dmx_byte_nb = 0;
						int v;
						char* pch = strtok((char*)dmxstr, "/");
						while (pch != NULL)
						{
							if (dmx_byte_nb < DMX_CH_MAX)
							{
								v = cap(atoi(pch), 0, DMX_V_MAX, 0);
								g_dmx_float_target[dmx_byte_nb] = (float)(v) / ((float)DMX_V_MAX);
								if (g_dmx_ramping == 1.0)
								{
									// no ramping
									g_dmx_byte_value[dmx_byte_nb] = v;
									g_dmx_float_value[dmx_byte_nb] = g_dmx_float_target[dmx_byte_nb];
								}
								dmx_byte_nb++;
							}
							pch = strtok(NULL, "/");
						}
					}
				}
				break;
#endif // V_DMX
			case 'P': // Program or Pitchbend
			case 'p':
				if (strlen(stype) > 7)
					data0 = (L_MIDI_PITCHBEND << 4);
				else
				{
					data0 = (L_MIDI_PROGRAM << 4);
					nbbyte = 2;
				}
				midiok = (lua_gettop(g_LUAoutState) >= 3);
				break;
			case 'N': // NoteOn or NoteOff
			case 'n':
				if (strlen(stype) > 6)
				{
					data0 = (L_MIDI_NOTEOFF << 4);
				}
				else
				{
					data0 = (L_MIDI_NOTEON << 4);
					min = 1;
				}
				midiok = (lua_gettop(g_LUAoutState) >= 3);
				break;
			case 'C': // Control or ChannelPressure
			case 'c':
				if (strlen(stype) > 7)
				{
					data0 = (L_MIDI_CHANNELPRESSURE << 4);
				}
				else
					data0 = (L_MIDI_CONTROL << 4);
				midiok = (lua_gettop(g_LUAoutState) >= 3);
				break;
			case 'K': // keyPressure
			case 'k':
				data0 = (L_MIDI_KEYPRESSURE << 4);
				midiok = (lua_gettop(g_LUAoutState) >= 3);
				break;
			default:
				break;
			}
			if (midiok)
			{
				T_midioutmsg midioutpostmsg;
				midioutpostmsg.id = midioutmsg.id + (dtIdMidioutPost++);
				midioutpostmsg.dt = 0;
				midioutpostmsg.midimsg.bData[0] = data0;
				midioutpostmsg.nbbyte = nbbyte;
				midioutpostmsg.track = cap(lua_tonumber(g_LUAoutState, -3), 0, MAXTRACK, 1);
				midioutpostmsg.midimsg.bData[1] = cap(lua_tonumber(g_LUAoutState, -2), 0, 128, 0);
				midioutpostmsg.midimsg.bData[2] = cap(lua_tonumber(g_LUAoutState, -1), min, 128, 0);
				lua_pop(g_LUAoutState, 3);
				sendmidimsg(midioutpostmsg, false);
			}
		}
		lua_pop(g_LUAoutState, lua_gettop(g_LUAoutState));
	}
	return(true);
}
static bool midiout_short_msg(T_midioutmsg midioutmsg, bool first)
{
	if (g_debug)
		mlog_out("midiout_short_msg device=%d msg=%d ch=%d p=%d v=%d", 
			g_tracks[midioutmsg.track].device, midioutmsg.midimsg.bData[0] >> 4, midioutmsg.midimsg.bData[0] & 0xF, midioutmsg.midimsg.bData[1], midioutmsg.midimsg.bData[2]);
	int nr_device = g_tracks[midioutmsg.track].device;
	if (nr_device >= VI_ZERO) 
    {
		int nrvi = nr_device - VI_ZERO;
		if (g_vi_opened[nrvi].sf2_midifont != 0)
			sf2_send_shortmsg(nrvi, midioutmsg.midimsg);
#ifdef V_VST
		if (g_vi_opened[nrvi].vsti_plugins != 0)
			vsti_send_shortmsg(nrvi, midioutmsg.midimsg);
#endif
		return true;
    }
	else
	{
		if ( g_midiout_rt[nr_device] != 0 )
		{
			// send to RT_midiout
			std::vector<unsigned char> message;
			for(int i = 0 ; i < midioutmsg.nbbyte ; i ++ )
				message.push_back( midioutmsg.midimsg.bData[i] );
			g_midiout_rt[nr_device]->sendMessage( &message );
		}
	}
	return true;
}
static bool sendmidimsg(T_midioutmsg midioutmsg , bool first)
{
	// process midiout messages
	if (first && (g_LUAoutState) && processPostMidiOut(midioutmsg))
		return true;

	// check the non replication of Note-on on a same channel, and repair it if neccesary

	int type_msg = (midioutmsg.midimsg.bData[0] & 0xF0) >> 4;
	int nr_device = g_tracks[midioutmsg.track].device;
	int nr_channel = g_tracks[midioutmsg.track].channel;
	int pitch = midioutmsg.midimsg.bData[1];
	bool retCode = true;
	switch (type_msg)
    {
    case L_MIDI_NOTEON:
		if (g_transposition != 0)
		{
			int p = midioutmsg.midimsg.bData[1] + g_transposition;
			while (p < 0)
				p += 12;
			while (p > 127)
				p -= 12;
			midioutmsg.midimsg.bData[1] = p;
		}
		midioutmsg.midimsg.bData[2] = apply_volume(midioutmsg.track, midioutmsg.midimsg.bData[2]);
		for (int c = 0; c < MAXCHANNEL; c++)
		{
			// search a noteon slot free in the extended channels
			if (g_channels[nr_device][c].extended == nr_channel)
			{
				if (g_midistatuspitch[nr_device][c][pitch] == -1 )
				{
					midioutmsg.midimsg.bData[0] = (L_MIDI_NOTEON << 4) + (BYTE)c;
					midiout_short_msg(midioutmsg,first);
					g_midistatuspitch[nr_device][c][pitch] = midioutmsg.id;
					g_miditimepitch[nr_device][nr_channel][pitch] = g_current_out_t;
					// mlog_out("note p=%d sent on device#%d channel#%d", pitch, nr_device,c);
					return(true);
				}
			}
		}
		// filter flooding of same [pitch] in short period
		if ((g_midistatuspitch[nr_device][nr_channel][pitch] != midioutmsg.id) && (g_miditimepitch[nr_device][nr_channel][pitch] < (g_current_out_t - 200)))
		{
			// no slot for this new note : note-off and note-on
			midioutmsg.midimsg.bData[0] = (L_MIDI_NOTEOFF << 4) + (BYTE)nr_channel;
			if (midiout_short_msg(midioutmsg, first) == false)
				return(false);
			midioutmsg.midimsg.bData[0] = (L_MIDI_NOTEON << 4) + (BYTE)nr_channel;
			midiout_short_msg(midioutmsg, first);
			g_midistatuspitch[nr_device][nr_channel][pitch] = midioutmsg.id;
			g_miditimepitch[nr_device][nr_channel][pitch] = g_current_out_t;
		}
		return(true);
    case L_MIDI_NOTEOFF:
		if (g_transposition != 0)
		{
			int p = midioutmsg.midimsg.bData[1] + g_transposition;
			while (p < 0)
				p += 12;
			while (p > 127)
				p -= 12;
			midioutmsg.midimsg.bData[1] = p;
		}
		for (int c = 0; c < MAXCHANNEL; c++)
		{
			// search a noteon slot occupied in the extended channels
			if (g_channels[nr_device][c].extended == nr_channel)
			{
				if (g_midistatuspitch[nr_device][c][pitch] == midioutmsg.id)
				{
					g_midistatuspitch[nr_device][c][pitch] = -1 ;
					midioutmsg.midimsg.bData[0] = (L_MIDI_NOTEOFF << 4) + (BYTE)c;
					midiout_short_msg(midioutmsg, first);
					return(true);
				}
			}
		}
		return(false); // no slot for this note : mlog_out !?
	case L_MIDI_SYSTEMCOMMON:
		return(midiout_short_msg(midioutmsg, first));
    default:
    	//mlog_out("          sendmidimsg default channel=%d",nr_channel+1);
		for (int c = 0; c < MAXCHANNEL; c++)
		{
			// replication of the messages on all extended channels
			if (g_channels[nr_device][c].extended == nr_channel)
			{
				midioutmsg.midimsg.bData[0] = (midioutmsg.midimsg.bData[0] & 0xF0) + (BYTE)c;
    			//mlog_out("          sendmidimsg default extended channel=%d",c+1);
				// filter flooding of same [L_MIDI_CONTROL/value] in short period
				if ((type_msg != L_MIDI_CONTROL ) || (g_miditimecontrol[nr_device][c][midioutmsg.midimsg.bData[1]] < (g_current_out_t - 200)) || (g_midistatuscontrol[nr_device][c][midioutmsg.midimsg.bData[1]] != midioutmsg.midimsg.bData[2]))
				{
					if (midiout_short_msg(midioutmsg, first) == false)
						retCode = false;
				}
				g_miditimecontrol[nr_device][c][midioutmsg.midimsg.bData[1]] = g_current_out_t;
				g_midistatuscontrol[nr_device][c][midioutmsg.midimsg.bData[1]] = midioutmsg.midimsg.bData[2];
			}
		}
        return(retCode);
    }
}
static bool sendmsg(T_midioutmsg midioutmsg)
{
  // send immediatly the short midioutmsg on the nr_device
	bool return_code = false;
	int nr_device = g_tracks[midioutmsg.track].device;
	if ((nr_device >= 0) && (nr_device < OUT_MAX_DEVICE))
    {
 	  //mlog_out("sendmsg track=%d nrdevice=%d nbByte=%d byte0=%02X byte1=%02X byte2=%02X", midioutmsg.track, nr_device , midioutmsg.nbbyte , midioutmsg.midimsg.bData[0] ,midioutmsg.midimsg.bData[1] ,midioutmsg.midimsg.bData[2] );
      // send to one device
		if (nr_device < MIDIOUT_MAX)
		{
				if (g_midiout_rt[nr_device])
				{
					return_code = sendmidimsg(midioutmsg, true);
				}
		}
		else
		{
				if (nr_device < (VI_ZERO + g_vi_opened_nb))
			 return_code = sendmidimsg(midioutmsg,true);
		}
	}
#ifdef V_DMX
	bool hookfound = false;
	for (int i = 0; i < g_dmx_nb_track; i++)
	{
		if (g_dmx_track[i] == midioutmsg.track)
		{
			hookfound = true;
			break;
		}
	}
	if (hookfound)
	{
		int type_msg = (midioutmsg.midimsg.bData[0] & 0xF0) >> 4;
		switch (type_msg )
		{
		case L_MIDI_NOTEON:
			dmx_hook(midioutmsg.midimsg.bData[1], midioutmsg.midimsg.bData[2]);
			break;
		case L_MIDI_NOTEOFF:
			dmx_hook(midioutmsg.midimsg.bData[1], 0);
			break;
		default : 
			break;
		}
	}
#endif
  return(return_code);
}
static int unqueue(int critere, T_midioutmsg midioutmsg)
{
	if (critere == OUT_QUEUE_FLUSH)
		g_current_out_t += g_timer_out_dt;
	long tmsg = g_current_out_t + midioutmsg.dt;
	int nb_waiting = 0;
	int retCode = 0;
	T_queue_msg *pt = g_queue_msg;
	for (int n = 0; n < g_end_queue_msg; n++, pt++)
	{
		if (!(pt->free))
		{
			nb_waiting++;
			switch (critere)
			{
			case OUT_QUEUE_FLUSH:
			{	// flush all messages
									if (pt->t <= g_current_out_t) // which are in the past
									{
										// remove the message from the waiting queue
										pt->free = true;
										sendmsg(pt->midioutmsg);
									}
									break;
			}
			case OUT_QUEUE_NOTEOFF:
			{
									  if ((((pt->midioutmsg.midimsg.bData[0]) & 0xF0) == (L_MIDI_NOTEON << 4)) // search a note-on 
										  && (((pt->midioutmsg.midimsg.bData[0]) & 0xF) == ((midioutmsg.midimsg.bData[0]) & 0xF)) // on same channel
										  && ((pt->midioutmsg.midimsg.bData[1] == midioutmsg.midimsg.bData[1]) || (midioutmsg.midimsg.bData[1] == 0)) //  and same pitch
										  && (pt->midioutmsg.track == midioutmsg.track) // on same track
										  && (pt->midioutmsg.id == midioutmsg.id) // with same id
										  && (pt->t >= tmsg) // after the required note-off
										  )
									  {
										  pt->free = true;
										  retCode = 1;
									  }
									  break;
			}
			default: break;
			}
		}
	}
	if (nb_waiting == 0)
		g_end_queue_msg = 0; // no more waintig slot

	return retCode;
}
static bool sendmsgdt(const T_midioutmsg midioutmsg)
{
    // send a midid_msg on nr_device immedialtly, or queud it for later process

	bool retCode = true;
    if (midioutmsg.dt == 0)
    {
		retCode = sendmsg(midioutmsg);
    }
    else
    {
        queue_insert(midioutmsg);
    }
	return(retCode);
}
static int mvi_open(const char *fname, int nr_deviceaudio, int volume,bool sf2)
{
	// fname is the full name ( *.dll for a vsti or *.sf2 for an sf2 )
	int nr_vi = vi_open(fname, nr_deviceaudio, sf2);
	if (nr_vi == -1)
		return -1;
	if (sf2)
	{
		if (BASS_MIDI_FontSetVolume(g_vi_opened[nr_vi].sf2_midifont, (float)(volume) / 64.0) == FALSE)
		{
			mlog_out("Error setting volume SF2<%s> , err=%d", fname, BASS_ErrorGetCode());
		}
		// send a dummy note to load vi
		T_midioutmsg u;
		u.midimsg.bData[0] = (L_MIDI_NOTEON << 4);
		u.midimsg.bData[1] = 30;
		u.midimsg.bData[2] = 1;
		u.midimsg.bData[3] = 0;
		u.track = VI_ZERO + nr_vi;
		u.dt = 50;
		u.nbbyte = 3;
		sendmsgdt(u);
		u.dt = 200;
		u.midimsg.bData[0] = (L_MIDI_NOTEOFF << 4);
		sendmsgdt(u);
	}
	else
	{
		if (BASS_ChannelSetAttribute(g_vi_opened[nr_vi].mstream, BASS_ATTRIB_VOL, (float)(volume) / 64.0) == FALSE)
		{
			mlog_out("Error setting volume VST<%s> , err=%d", fname, BASS_ErrorGetCode());
		}
	}
	return(VI_ZERO + nr_vi);
}
static void send_control(int nrTrack, int nrControl, int v, unsigned int dt)
{
	T_midioutmsg m;
	m.midimsg.bData[1] = nrControl;
	m.midimsg.bData[2] = v;
	m.track = nrTrack;
	m.dt = dt;
	m.nbbyte = 3;
	m.id = 0;
	m.midimsg.bData[0] = (L_MIDI_CONTROL << 4 ) ;
	sendmsgdt(m);
}
static void send_program(int nrTrack, int nrProgram, unsigned int dt)
{
	T_midioutmsg m;
	m.midimsg.bData[1] = nrProgram;
	m.midimsg.bData[2] = 0;
	m.track = nrTrack;
	m.dt = dt;
	m.nbbyte = 2;
	m.id = 0;
	m.midimsg.bData[0] = ( L_MIDI_PROGRAM << 4 ) ;
	sendmsgdt(m);
}
static void send_tune(int nrTrack, float freq)
{
    int coarse, coarsemsb , finemsb ;
    float fine;
    float cents = 1200.0 * log2f(freq / 440.0);
    if (cents >= 0)
    {
        coarse = (int)((cents + 50.0)/100.0);
        fine = cents - 100.0 * (float)(coarse);
    }
    else
    {
        cents = (-1.0)*cents;
        coarse = (int)((cents + 50.0)/100.0);
        fine = cents - 100.0 * (float)(coarse);
        coarse *= -1;
        fine *= -1.0;
        cents = (-1.0)*cents;
    }
    finemsb = (int)((float)(0x20) * fine / 50.0 ) + 0x40;
    
    coarsemsb = coarse + 0x40;
    
	send_control(nrTrack, 101, 0, 0);
	send_control(nrTrack, 100, 2, 0);
	send_control(nrTrack, 6, coarsemsb, 0);
    
	send_control(nrTrack, 101, 0, 0);
	send_control(nrTrack, 100, 1, 0);
	send_control(nrTrack, 6, finemsb, 0);
    // 	send_control(nrTrack,38, finelsb, 0);
}
static void send_bendrange(int nrTrack, int semitone)
{
	send_control(nrTrack, 101, 0, 0);
	send_control(nrTrack, 100, 0, 0);
	send_control(nrTrack, 6, 0, semitone);
}
static void chord_init()
{
	for (int n = 0; n < CHORDMAX; n++)
	{
		g_chords[n].id = -1;
		g_chords[n].nbPitch = 0;
		g_chords[n].nbOff = 0;
	}
}
static T_chord* chord_new(long id)
{
	for (int n = 0; n < CHORDMAX; n++)
	{
		if ((g_chords[n].id == -1) || (g_chords[n].id == id))
		{
			if (id == -1)
				g_chords[n].id = n;
			else
				g_chords[n].id = id;
			return(&(g_chords[n]));
		}
	}
	return (NULL);
}
static T_chord* chord_get(int id)
{
	if (id == -1)
		return NULL;
	for (int n = 0; n < CHORDMAX; n++)
	{
		if (g_chords[n].id == id)
			return(&(g_chords[n]));
	}
	return (NULL);
}
static void channel_extended_init()
{
	// all channels are unused
	for (int nr_device = 0; nr_device < OUT_MAX_DEVICE; nr_device++)
	{
		for (int channel = 0; channel < MAXCHANNEL; channel++)
		{
			g_channels[nr_device][channel].extended = -1 ; // not used
		}
	}
}
static int channel_extended_set(int nr_device , int nr_channel, int nb_additional_channel, bool except_channel10)
{
	if (g_channels[nr_device][nr_channel].extended == nr_channel)
	{
		// already set
		int nb = 0;
		for (int n = 0; n < MAXCHANNEL; n++)
		{
			if (g_channels[nr_device][n].extended == nr_channel)
				nb++;
		}
		if (nb == nb_additional_channel)
		{
			// already done 
			return(1);
		}
		else
		{
			// free the previous extension to redo it
			for (int n = 0; n < MAXCHANNEL; n++)
			{
				if (g_channels[nr_device][n].extended == nr_channel)
					g_channels[nr_device][n].extended = -1;
			}

		}
	}
	g_channels[nr_device][nr_channel].extended = nr_channel;
	// search a channel not yet "used", starting from 16, 15, 14...
	int m = MAXCHANNEL - 1;
	while (g_channels[nr_device][m].extended != -1)
	{
		m--;
		if ((except_channel10) && (m == 9))
			m--;
		if (m <= nr_channel)
			return(-1);
	}
	// m contains a channel not yet used
	// extend the channel on the last ones : m, m-1, m-2, ...
	for (int n = 0; n < nb_additional_channel; n++)
	{
		if ((except_channel10) && (m == 9))
			m--;
		if (m <= nr_channel)
			return(-1);
		if (g_channels[nr_device][m].extended == -1) // channel#v is not free
			g_channels[nr_device][m].extended = nr_channel; //  channel#v receives order from <channel>
		m--;
	}
	return(0);
}
static void string_to_control(int nrTrack, const char *param)
{
	// read the string and send control on the track
	// syntaxe of the string  :
	// name(P[[Bank_MSB/]Bank_LSB/]<Program>],[C<Control-nr>=<Value>,]*)
	// example :
	//    change progam to value 30 : myProgram(P30)
	//    change volume to value 40 : myVolume(C7/40)
	//    change MSB=10/LSB=11/program=30, volume=40 and pan=5 : myControl(P10/11/30,C7/40,C10/5)

	char bufstr[1024];
	strcpy(bufstr, param);
	bufstr[strlen(param)] = '\0';

	char *next_token_name = NULL;
	strtok_s(bufstr, "()", &next_token_name); // name
	char *progcontrol;
	progcontrol = strtok_s(NULL, "()", &next_token_name); // prog,control
	if (progcontrol == NULL)
		return;

	char *next_token_param = NULL;
	char * ptprogram;
	char *control[65];
	int nbControl = 0;
	ptprogram = strtok_s(progcontrol, ",", &next_token_param); // program ?
	if ((ptprogram == NULL) || (strlen(ptprogram) < 2))
		return;
	if (ptprogram[0] == 'P')
	{
		//good program
		control[0] = strtok_s(NULL, ",", &next_token_param); // next is a control
	}
	else
	{
		control[0] = ptprogram; // not a good program. Give it to the control
		ptprogram = NULL;
	}
	while ((control[nbControl] != NULL) && (nbControl < 64))
	{
		nbControl++;
		control[nbControl] = strtok_s(NULL, ",", &next_token_param);
	}

	// send program 
	if (ptprogram != NULL)
	{
		char *next_tokenProgram = NULL;
		char *bankprogram[5];
 		int nb_bankprogram = 0;
        bankprogram[nb_bankprogram] = strtok_s(ptprogram + 1 , "/" , &next_tokenProgram);
        while ((bankprogram[nb_bankprogram] != NULL) && (nb_bankprogram < 4))
		{
			nb_bankprogram++;
            bankprogram[nb_bankprogram] = strtok_s(NULL, "/" , &next_tokenProgram);
        }
		switch (nb_bankprogram)
		{
		case 1:
			send_program(nrTrack, cap(atoi(bankprogram[0]), 0, 128, 0), 0);
			break;
		case 2:
			send_control(nrTrack, 0, cap(atoi(bankprogram[0]), 0, 128, 0), 0);
			send_program(nrTrack, cap(atoi(bankprogram[1]), 0, 128, 0), 0);
			break;
		case 3:
			send_control(nrTrack, 0, cap(atoi(bankprogram[0]), 0, 128, 0), 0);
			send_control(nrTrack, 0x20, cap(atoi(bankprogram[1]), 0, 128, 0), 0);
			send_program(nrTrack, cap(atoi(bankprogram[2]), 0, 128, 0), 0);
			break;
		default:
			break;
		}
	}

	// send controls
	for (int nrControl = 0; nrControl < nbControl; nrControl++)
	{
		char *next_tokenControl = NULL;
		char *controlnumber, *controlvalue;
		controlnumber = strtok_s(control[nrControl] + 1, "/", &next_tokenControl);
		if (controlnumber != NULL)
		{
			controlvalue = strtok_s(NULL, "/", &next_tokenControl);
			if (controlvalue != NULL)
			{
				send_control(nrTrack, cap(atoi(controlnumber), 0, 127, 0), cap(atoi(controlvalue), 0, 128, 0), 0);
			}
		}
	}
}
static void midiout_close_device(int nr_device)
{
	if (g_midiout_rt[nr_device] == 0 )
		return ;
/*        
        T_midimsg midimsg[3];

        midimsg[0].bData[0] = (L_MIDI_CONTROL << 4);
        midimsg[0].bData[1] = 123;// all note off
        midimsg[0].bData[2] = 0;
        
				midimsg[1].bData[0] = (L_MIDI_CONTROL << 4);
        midimsg[1].bData[1] = 120;// all sound off
        midimsg[1].bData[2] = 0;

        midimsg[2].bData[0] = (L_MIDI_CONTROL << 4);
        midimsg[2].bData[1] = 121;// reset al controller
        midimsg[2].bData[2] = 0;
#ifdef V_PC
				for(int i = 0 ; i < 3 ; i ++ )
	        midiOutShortMsg(g_midiout_opened[nr_device], midimsg[i].dwData);
#endif
#ifdef V_MAC
        OSStatus err;
        // envoi du message Midi version Mac dans un packet
        Byte buffer[1024];
        MIDIPacketList *pktlist = (MIDIPacketList *)buffer;
        MIDIPacket *curPacket = MIDIPacketListInit(pktlist);
				for(int i = 0 ; i < 3 ; i ++ )
	        curPacket = MIDIPacketListAdd(pktlist, sizeof(buffer), curPacket, 0, 3, & (midimsg[i].bData[0] ));
        err = MIDISend(g_midiOutPortRef, g_midiout_opened[nr_device], pktlist);
#endif
    BASS_MIDI_OutFree(nr_device);
*/
  std::vector<unsigned char> message;
  message.push_back(L_MIDI_CONTROL << 4);
  message.push_back(123);// all note off
  message.push_back(0);
	g_midiout_rt[nr_device]->sendMessage( &message );

	message[1]=120;// all sound off
	g_midiout_rt[nr_device]->sendMessage( &message );

	message[1]=121;// // reset al controller
	g_midiout_rt[nr_device]->sendMessage( &message );
}
static void all_note_off(const char *soption, int nrTrack)
{
	int track_min = nrTrack;
	int track_max = nrTrack + 1;
	if (nrTrack < 0)
	{
		track_min = 0;
		track_max = MAXTRACK;
	}
	for (int nrTrack = track_min; nrTrack < track_max; nrTrack++)
	{
		for (unsigned int p = 0; p < strnlen_s(soption, 5); p++)
		{
			switch (soption[p])
			{
			case 's': 
				send_control(nrTrack, 120, 0, 0); // all sound off
				break; 
			case 'c': 
				send_control(nrTrack, 121, 0, 0); // reset controller
				break; 
			case 'n':
				send_control(nrTrack, 64, 0, 0); // pedal off
				send_control(nrTrack, 123, 0, 0);  // all note off
				break; 
			case 'a': // all
				send_control(nrTrack, 64, 0, 0); // pedal off
				send_control(nrTrack, 120, 0, 0);// all sound off
				send_control(nrTrack, 121, 0, 0);// reset controller
				send_control(nrTrack, 123, 0, 0); // all note off
				break;
			default: break;
			}
		}
	}
	for (unsigned int p = 0; p < strnlen_s(soption, 5); p++)
	{
		switch (soption[p])
		{
		case 'n' :
		case 'a': // all
			picth_init();
			break;
		default: break;
		}
	}
#ifdef V_DMX
	for (int i = 0; i < DMX_CH_MAX; i++)
	{
		g_dmx_byte_value[i] = 0;
		g_dmx_float_value[i] = 0.0 ;
	}
#endif
}
static void onMidiOut_filter_set()
{
	// validate the LUA functions available to process the MIDI messages
	//g_process_Sysex = (lua_getglobal(g_LUAoutState, LUAFunctionSysex) == LUA_TFUNCTION);
	//lua_pop(g_LUAoutState, 1);
	//if (g_process_Sysex) mlog_out("Information : onMidiOut function %s registered", LUAFunctionSysex);

	//g_process_Activesensing = (lua_getglobal(g_LUAoutState, LUAFunctionActive) == LUA_TFUNCTION);
	//lua_pop(g_LUAoutState, 1);
	//if (g_process_Activesensing) mlog_out("Information : onMidiOut function %s registered", LUAFunctionActive);

	g_process_Timer = (lua_getglobal(g_LUAoutState, LUAFunctionTimer) == LUA_TFUNCTION);
	lua_pop(g_LUAoutState, 1);
	if (g_process_Timer) mlog_out("Information : onMidiOut function %s registered", LUAFunctionTimer);

	g_process_Clock = (lua_getglobal(g_LUAoutState, LUAFunctionClock) == LUA_TFUNCTION);
	lua_pop(g_LUAoutState, 1);
	if (g_process_Clock) mlog_out("Information : onMidiOut function %s registered", LUAFunctionClock);

	g_process_ChannelPressure = (lua_getglobal(g_LUAoutState, LUAFunctionChannelPressure) == LUA_TFUNCTION);
	lua_pop(g_LUAoutState, 1);
	if (g_process_ChannelPressure) mlog_out("Information : onMidiOut function %s registered", LUAFunctionChannelPressure);

	g_process_KeyPressure = (lua_getglobal(g_LUAoutState, LUAFunctionKeyPressure) == LUA_TFUNCTION);
	lua_pop(g_LUAoutState, 1);
	if (g_process_KeyPressure) mlog_out("Information : onMidiOut function %s registered", LUAFunctionKeyPressure);

	g_process_Control = (lua_getglobal(g_LUAoutState, LUAFunctionControl) == LUA_TFUNCTION);
	lua_pop(g_LUAoutState, 1);
	if (g_process_Control) mlog_out("Information : onMidiOut function %s registered", LUAFunctionControl);

	g_process_SystemCommon = (lua_getglobal(g_LUAoutState, LUAFunctionSystemCommon) == LUA_TFUNCTION);
	lua_pop(g_LUAoutState, 1);
	if (g_process_SystemCommon) mlog_out("Information : onMidiOut function %s registered", LUAFunctionSystemCommon);

	g_process_Program = (lua_getglobal(g_LUAoutState, LUAFunctionProgram) == LUA_TFUNCTION);
	lua_pop(g_LUAoutState, 1);
	if (g_process_Program) mlog_out("Information : onMidiOut function %s registered", LUAFunctionProgram);

	g_process_NoteOff = (lua_getglobal(g_LUAoutState, LUAFunctionNoteOff) == LUA_TFUNCTION);
	lua_pop(g_LUAoutState, 1);
	if (g_process_NoteOff) mlog_out("Information : onMidiOut function %s registered", LUAFunctionNoteOff);

	g_process_NoteOn = (lua_getglobal(g_LUAoutState, LUAFunctionNoteOn) == LUA_TFUNCTION);
	lua_pop(g_LUAoutState, 1);
	if (g_process_NoteOn) mlog_out("Information : onMidiOut function %s registered", LUAFunctionNoteOn);

	g_process_PitchBend = (lua_getglobal(g_LUAoutState, LUAFunctionPitchBend) == LUA_TFUNCTION);
	lua_pop(g_LUAoutState, 1);
	if (g_process_PitchBend) mlog_out("Information : onMidiOut function %s registered", LUAFunctionPitchBend);
}
static bool onMidiout_open(const char* fname)
{
	if (g_LUAoutState)
		lua_close(g_LUAoutState);
	g_LUAoutState = 0;

	// open the dedicated midiin-LUA-thread to process midiout msg
	g_LUAoutState = luaL_newstate(); // newthread 
	luaL_openlibs(g_LUAoutState);

	if (luaL_loadfile(g_LUAoutState, fname) != LUA_OK)
	{
		mlog_out("onMIdiOut mlog_out lua_loadfile <%s>", lua_tostring(g_LUAoutState, -1));
		lua_close(g_LUAoutState);
		g_LUAoutState = NULL;
		return false;
	}
	// run the script
	if (lua_pcall(g_LUAoutState, 0, 0, 0) != LUA_OK)
	{
		mlog_out("onMIdiOut mlog_out lua_pcall <%s>", lua_tostring(g_LUAoutState, -1));
		lua_pop(g_LUAoutState, 1);
		lua_close(g_LUAoutState);
		g_LUAoutState = NULL;
		return false;
	}
	mlog_out("Information : onMidiOutOpen(%s) OK", fname);
	onMidiOut_filter_set();
	return true;
}
static bool getTypeFile(char *vinamedevice, int *nr_deviceaudio, char *viname , char *extension)
{
	// return true if extension(vinamedevice) is sf2 or dll or wav
	// extension is completed
	// nr_deviceaudio is set to the device audio if suffixe @name_audio or @nr_audio
	// viname contains the name , with extension and without the section @audio
	*extension = '\0';
	bool extension_ok = false;
	*nr_deviceaudio = -1;
	if ((strlen(vinamedevice) < 5) || (vinamedevice[strlen(vinamedevice) - 4] != '.'))
		return false;
	if ((strncmp(vinamedevice + strlen(vinamedevice) - 4, ".sf2", 4) == 0) || (strncmp(vinamedevice + strlen(vinamedevice) - 4, ".SF2", 4) == 0))
	{
		extension_ok = true;
		strcpy(extension,"sf2");
	}
	if ((strncmp(vinamedevice + strlen(vinamedevice) - 4, ".dll", 4) == 0) || (strncmp(vinamedevice + strlen(vinamedevice) - 4, ".DLL", 4) == 0))
	{
		extension_ok = true;
		strcpy(extension, "dll");
	}
	if ((strncmp(vinamedevice + strlen(vinamedevice) - 4, ".wav", 4) == 0) || (strncmp(vinamedevice + strlen(vinamedevice) - 4, ".WAV", 4) == 0))
	{
		extension_ok = true;
		strcpy(extension, "wav");
	}
	if (!extension_ok)
		return false;
	vinamedevice[strlen(vinamedevice) - 4] = '\0';
	char *ptdevice = strchr(vinamedevice, '@');
	if (ptdevice)
	{
		// audio-device is specified after the @
		*ptdevice = '\0';
		*nr_deviceaudio = 0;
		char name_device[MAXBUFCHAR];
		bool found = false;
		while (true)
		{
			// search an audio_device with the same name
			audio_name(*nr_deviceaudio, name_device);
			if (*name_device == '\0')
				break;
			if (strcmp(name_device, ptdevice + 1) == 0)
			{
				found = true;
				break;
			}
			(*nr_deviceaudio)++;
		}
		if (!found)
			// select the audio device #
			*nr_deviceaudio = atoi(ptdevice + 1);
	}
	strcpy(viname, vinamedevice); 
	if (strcmp(extension, "sf2") == 0)
		strcat(viname, ".sf2");
	if (strcmp(extension, "dll") == 0)
		strcat(viname, ".dll");
	if (strcmp(extension, "wav") == 0)
		strcat(viname, ".wav");
	return true;
}
static void curve_init()
{
	for (int c = 0; c < MAXCURVE; c++)
	{
		for (int n = 0; n < MAXPOINT; n++)
		{
			g_curves[c].x[n] = -1;
			g_curves[c].y[n] = -1;
			g_curves[c].x[n] = -1;
			g_curves[c].y[n] = -1;
		}
	}
}
static void track_init()
{
	bool channelUsed[OUT_MAX_DEVICE][MAXCHANNEL];
	for (int nr_device = 0; nr_device < OUT_MAX_DEVICE; nr_device++)
	{
		for (int nr_channel = 0; nr_channel < MAXCHANNEL; nr_channel++)
			channelUsed[nr_device][nr_channel] = false;
	}
	for (int nrTrack = 0; nrTrack < MAXTRACK; nrTrack++)
	{
		if ((g_tracks[nrTrack].device >= 0) && (g_tracks[nrTrack].device < OUT_MAX_DEVICE) && (g_tracks[nrTrack].channel> 0) && (g_tracks[nrTrack].channel < MAXCHANNEL))
		{
			if (channelUsed[g_tracks[nrTrack].device][g_tracks[nrTrack].channel] == false)
			{
				channelUsed[g_tracks[nrTrack].device][g_tracks[nrTrack].channel] = true;
				send_control(nrTrack, 123, 0, 0); // all note off
				send_control(nrTrack, 120, 0, 0); // all sound off
				send_control(nrTrack, 121, 0, 0); // reset controller
			}
		}
		g_tracks[nrTrack].volume = 64; // neutral volume
		g_tracks[nrTrack].mute = false;
		g_tracks[nrTrack].device = -2; // no device  attached to this track
		g_tracks[nrTrack].channel = -2; // no channel  attached to this track
		g_tracks[nrTrack].nrCurve = 0; // no curve
		channel_extended_init();
		g_volume = 64;
	}
}
static void midi_out_free()
{
	for (int n = 0; n < MIDIOUT_MAX; n++)
		midiout_close_device(n);
	for (int n = 0; n < MIDIOUT_MAX; n++)
	{
		if (g_midiout_rt[n] != 0 )
			g_midiout_rt[n]->closePort();
	}
	for (int n = 0; n < MIDIOUT_MAX; n++)
	{
		if (g_midiout_rt[n] != 0 )
			delete g_midiout_rt[n] ;
		g_midiout_rt[n] = 0;
	}
	g_midiout_max_nr_device = 0;
}
static void midi_out_init()
{
	midi_out_free();
	for (int n = 0; n < MIDIOUT_MAX; n++)
	{
		g_midiout_rt[n] = 0;
		g_name_midi_out[n][0] = '\0';
	}
	g_midiout_max_nr_device =  0;

	g_nb_midi_out = g_g_midiout_rt.getPortCount();
	if ( g_nb_midi_out >= MIDIOUT_MAX)
	{
		g_nb_midi_out = MIDIOUT_MAX - 1 ;
		mlog_out("midi_out_init : only first %d Midi-out are managed",MIDIOUT_MAX);
	}
	for(unsigned int nr_device = 0 ; nr_device < g_nb_midi_out ; nr_device ++)
	{
		std::string portName;
		portName = g_g_midiout_rt.getPortName(nr_device) ;
		strcpy(g_name_midi_out[nr_device], portName.c_str());
		// delete the index nr added at the end of the name by rtmidiout
		char *pt = g_name_midi_out[nr_device] + strlen(g_name_midi_out[nr_device]);
		bool cont = true;
		while ((pt != g_name_midi_out[nr_device]) && (cont))
		{
			switch (*pt)
			{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				*pt = '\0';
				break;
			case ' ':
				*pt = '\0';
				cont = false;
				break;
			default:
				break;
			}
			pt--;
		}

		mlog_out("Information : midiout#%d <%s> ", nr_device + 1, g_name_midi_out[nr_device]);
	}

	for (int n = 0; n < MIDIIN_MAX; n++)
	{
		g_name_midi_in[n][0] = '\0';
	}
	g_nb_midi_in = 0;
	BASS_MIDI_DEVICEINFO info;
	while (BASS_MIDI_InGetDeviceInfo(g_nb_midi_in, &info))
	{
		strcpy(g_name_midi_in[g_nb_midi_in],info.name);
		mlog_out("Information : midiin#%d <%s>", g_nb_midi_in + 1, g_name_midi_in[g_nb_midi_in]);
		g_nb_midi_in++;
		if ( g_nb_midi_in >= MIDIIN_MAX )
		{
			g_nb_midi_in = MIDIIN_MAX - 1 ;
			mlog_out("midi_out_init : only first %d Midi-in are managed",MIDIIN_MAX);
		}
	}
}
static void fifo_out_init()
{
	for (int n = 0; n < OUT_QUEUE_MAX_MSG; n++)
		g_queue_msg[n].free = true;
	g_end_queue_msg = 0;
}
static void mutex_out_init()
{
	if ( g_mutex_out_ok )
		return ;
	g_mutex_out_ok = true ;
#ifdef V_PC
	// create a mutex to manipulate safely the outputs ( timer , lua ) up to ( queus, midiout )
	g_mutex_out = CreateMutex(NULL, FALSE, NULL);
	if (g_mutex_out == NULL)
	{
		mlog_out("error mutex_out_init");
		g_mutex_out_ok = false ;
	}
#endif
#ifdef V_MAC
	// create a mutex to manipulae safely the queued midiout msg
	if (pthread_mutex_init(&g_mutex_out, NULL) != 0)
	{
		mlog_out("error mutex_out_init");
		g_mutex_out_ok = false ;
	}
#endif
#ifdef V_LINUX
	// create a mutex to manipulae safely the queued midiout msg
	if (pthread_mutex_init(&g_mutex_out, NULL) != 0)
	{
		mlog_out("error mutex_out_init");
		g_mutex_out_ok = false ;
	}
#endif
}
static void mutex_out_free()
{
	if ( ! g_mutex_out_ok )
		return;
	g_mutex_out_ok = false ;
#ifdef V_PC
	CloseHandle(g_mutex_out);
#endif
#ifdef V_MAC
	pthread_mutex_destroy(&g_mutex_out);
#endif
#ifdef V_LINUX
	pthread_mutex_destroy(&g_mutex_out);
#endif
}

static void process_timer_out()
{
	T_midioutmsg msg;
	msg.midimsg.dwData = 0;
	// flush des messages en attente de sortie ( pour un dt , .. )
	unqueue(OUT_QUEUE_FLUSH, msg);
	// appel du onTimer du module LUA de pst-traitement de midi-out
	if ( g_process_Timer )
	{
		lua_getglobal(g_LUAoutState, LUAFunctionTimer);
		if (lua_pcall(g_LUAoutState, 0, 0, 0) != LUA_OK)
		{
			mlog_out("erreur onTimer calling LUA , err: %s", lua_tostring(g_LUAoutState, -1));
			lua_pop(g_LUAoutState, 1);
		}
	}
#ifdef V_DMX
	dmxRefresh();
#endif
}
#ifdef V_PC
VOID CALLBACK timer_out_callback(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	if ( try_lock_mutex_out())
	{
		process_timer_out();
		unlock_mutex_out();
	}
}
#endif
#ifdef V_MAC
void timer_out_callback(CFRunLoopTimerRef timer, void *info)
{
	if ( try_lock_mutex_out() )
	{
		process_timer_out();
		unlock_mutex_out();
	}
}
void *loop_out_run(void *void_ptr)
{
	int n;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,&n) ;
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,&n);
	__CFRunLoopTimer *mLoopTimer = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent() , (float)(g_timer_out_dt) / 1000.0, 0, 0, timer_out_callback, NULL);
	if ( mLoopTimer != 0 )
	{
		__CFRunLoop *mRunLoop = CFRunLoopGetCurrent();
		CFRunLoopAddTimer(mRunLoop , mLoopTimer, kCFRunLoopCommonModes);
		CFRunLoopRun() ;
	}
	return NULL;
}
#endif
#ifdef V_LINUX
static void timer_out_callback(int sig, siginfo_t *si, void *uc)
{
	if ( try_lock_mutex_out() )
	{
		process_timer_out();
		unlock_mutex_out();
	}
}
void *loop_out_run(void *void_ptr)
{
	struct sigaction msigaction;
    msigaction.sa_flags = SA_SIGINFO | SA_RESTART ;
    msigaction.sa_sigaction = timer_out_callback;
    sigemptyset(&(msigaction.sa_mask));
    if (sigaction(MTIMERSIGNALOUT, &msigaction, NULL) == -1)
	{
		mlog_out("timer_out_init : sigaction error=%d",errno);
		g_timer_out_ok = false ;
	}
	else
	{
	 	struct sigevent msigevent;
		msigevent.sigev_notify = SIGEV_SIGNAL;
		msigevent.sigev_signo = MTIMERSIGNALOUT;
		msigevent.sigev_value.sival_ptr = &g_timer_out_id;
		if (timer_create(CLOCK_REALTIME, &msigevent, &g_timer_out_id) == -1)
		{
			mlog_out("timer_out_init : timer_create error=%d",errno);
		    g_timer_out_ok = false ;
		}
		else
		{
			 //mlog_out("debug timer_in_init : timer_create OK");
	 		 struct itimerspec mitimerspec;
			 mitimerspec.it_value.tv_sec =  g_timer_out_dt / 1000;
			 mitimerspec.it_value.tv_nsec =  (g_timer_out_dt % 1000 )* 1000000;
			 mitimerspec.it_interval.tv_sec = mitimerspec.it_value.tv_sec;
			 mitimerspec.it_interval.tv_nsec = mitimerspec.it_value.tv_nsec;
			 if (timer_settime(g_timer_out_id, 0, &mitimerspec, NULL) == -1)
			 {
				mlog_out("timer_out_init : timer_settime error=%d",errno);
				g_timer_out_ok = false ;
			 }
			 else
			 	mlog_out("Information : timer_out_init.timer_settime OK");
	   }
	}
	return NULL ;
}
#endif
static void timer_out_init(bool externalTimer , int timerDt)
{
	g_timer_out_dt = timerDt ;

	if ( externalTimer )
	{
		g_timer_out_ok = false ;
		return ;
	}

	// create a periodic timer to flush-out the queud midiout msg
	if ( g_timer_out_ok )
		return ;
	g_timer_out_ok = true ;
#ifdef V_PC
	if (!CreateTimerQueueTimer(&g_timer_out, NULL, timer_out_callback, 0, g_timer_out_dt, g_timer_out_dt, 0))
	{
		mlog_out("timer_out_init : error");
		g_timer_out_ok = false ;
	}
	else
		mlog_out("debug timer_out_init : CreateTimerQueueTimer OK");
#endif
#ifdef V_MAC
	if ( pthread_create(&g_loop_out_run_thread, NULL, loop_out_run, NULL)) 
	{
		g_timer_out_ok = false ;
		mlog_out("timer_out_init : pthread_create error");
	}
	else
		mlog_out("debug timer_out_init : pthread_create OK");
#endif
#ifdef V_LINUX
	if ( pthread_create(&g_loop_out_run_thread, NULL, loop_out_run, NULL)) 
	{
		mlog_out("timer_out_init : pthread_create error");
		g_timer_out_ok = false ;
	}
#endif
}
static void timer_out_free()
{
	if ( ! g_timer_out_ok )
		return ;
	g_timer_out_ok = false ;
#ifdef V_PC
	DeleteTimerQueueTimer(NULL, g_timer_out, NULL);
#endif
#ifdef V_MAC
	pthread_cancel(g_loop_out_run_thread);
#endif
#ifdef V_LINUX
	timer_delete(g_timer_out_id);
	pthread_cancel(g_loop_out_run_thread);
#endif
}
static void init_out(const char *fname, bool externalTimer , int timerDt)
{
	log_out_init(fname);
	midi_out_init();
	picth_init();
	fifo_out_init();
	vi_init();
	chord_init();
	channel_extended_init();
	track_init();
	curve_init();
#ifdef V_DMX
	dmx_init();
#endif
	//if (!externalTimer) // external process protects conflicts with its own mutex
	mutex_out_init();
	timer_out_init(externalTimer, timerDt);
	mixer_init();
	g_LUAoutState = NULL;
}
static void free()
{
	timer_out_free();
	mutex_out_free();
	vi_free();
	midi_out_free();
	mixer_free();
#ifdef V_DMX
	dmxClose();
#endif
	if (g_LUAoutState)
	{
		lua_close(g_LUAoutState);
	}
	g_LUAoutState = 0;
}

static int LoutTrackMute(lua_State *L)
{
	// set MIDI noteon volume for a channel 
	// It's different than an audio volume on the channel using note ctrl7 !!
	// parameter #1 : mute = 0, unmute = 1, toggle = 2
	// parameter #2 : optional nrTrack ( default 1 ) 
	lock_mutex_out();

	int mute = (int)lua_tointeger(L, 1);
	int nrTrack = cap((int)luaL_optinteger(L, 2, 1), 0, MAXTRACK, 1);
	switch (mute)
	{
	case 0: g_tracks[nrTrack].mute = true; break;
	case 1: g_tracks[nrTrack].mute = false; break;
	case 2: g_tracks[nrTrack].mute = !(g_tracks[nrTrack].mute ) ; break;
	}
	unlock_mutex_out();
	return (0);
}
static int LoutSetTrackVolume(lua_State *L)
{
	// set MIDI noteon volume for a channel 
	// It's different than an audio volume on the channel using note ctrl-7 !!
	// parameter #1 : volume 0..64(neutral)..127
	// parameter #2 : optional nrTrack ( default 1 ) 
	lock_mutex_out();

	int volume = (int)lua_tointeger(L, 1);
	int nrTrack = cap((int)luaL_optinteger(L, 2, 1), 0, MAXTRACK, 1);
	g_tracks[nrTrack].volume = volume;
	unlock_mutex_out();
	return (0);
}
static int LoutGetTrackVolume(lua_State *L)
{
	// get MIDI noteon volume for a track 
	// It's different than an audio volume on the channel using note ctrl7 !!
	// parameter #1 : optional nrTrack ( default 1 ) 
	// return : volume 0..64(neutral)..127
	lock_mutex_out();

	int nrTrack = cap((int)luaL_optinteger(L, 1, 1), 0, MAXTRACK, 1);

	int v = g_tracks[nrTrack].volume * ((g_tracks[nrTrack].mute) ? 0 : 1);
	lua_pushinteger(L, v );
	
	unlock_mutex_out();
	return (1);
}
static int LoutSetCurve(lua_State *L)
{
	// set MIDI noteon curve for a channel 
	// parameter #1 : nrCurve 
	// parameetr #2 : table of points x1,y1 ; x2,y2, ...
	// e.g to reverse the velocity : 0,1 1,0
	// e.g. to have more p and mp : 0,0 0.8,0.5 1,0.8
	lock_mutex_out();

	int nrCurve = cap((int)lua_tointeger(L, 1), 0, MAXCURVE, 0);
	int nrArg = 2;
	int x, y, nbp;
	nbp = 0;
	while (nrArg <= lua_gettop(L))
	{
		x = cap((int)lua_tointeger(L, nrArg - 1), 0, 128, 0);
		y = cap((int)lua_tointeger(L, nrArg), 0, 128, 0);
		g_curves[nrCurve].x[nbp] = x;
		g_curves[nrCurve].y[nbp] = y;
		if ((nbp + 1) < MAXPOINT)
		{
			g_curves[nrCurve].x[nbp + 1] = -1;
		}
		nrArg += 2;
		nbp++;
		if (nbp >= MAXPOINT)
			break;
	}

	unlock_mutex_out();
	return (0);
}
static int LoutTranspose(lua_State *L)
{
	// transpose midiout ( make also a allnoteoff, else : mismatch between previous noteon and netx noteoff on midiout )
	// parameter #1 : g_transposition value 
	lock_mutex_out();
	all_note_off("n", -1);
	g_transposition = cap((int)lua_tointeger(L, 1), -24, 24, 0);
	unlock_mutex_out();
	return (0);
}

static int LoutSetTrackCurve(lua_State *L)
{
	// set MIDI noteon curve for a track 
	// parameter #1 : curve to use
	// parameter #2 : optional nrTrack ( default 1 ) 
	lock_mutex_out();

	int nrCurve = cap((int)lua_tointeger(L, 1), 0, MAXCURVE, 0);
	int nrTrack = cap((int)luaL_optinteger(L, 2, 1), 0, MAXTRACK, 1);
	g_tracks[nrTrack].nrCurve = nrCurve;
	
	unlock_mutex_out();
	return(0);
}
static int LoutSetTrackInstrument(lua_State *L)
{
	// set instrument for a track 
	// parameter #1 : string which describe the instrument required
	// parameter #2 : optional nrTrack ( default 1 ) 
	lock_mutex_out();

	const char *tuning = lua_tostring(L, 1);
	int nrTrack = cap((int)luaL_optinteger(L, 2, 1), 0, MAXTRACK , 1);
	string_to_control(nrTrack, tuning);

	unlock_mutex_out();
	return(0);
}
static int LoutSetVolume(lua_State *L)
{
	// set MIDI noteon volume for all outputs 
	// parameter #1 : volume 0..64(neutral)..127
	lock_mutex_out();

	g_volume = (int)lua_tointeger(L, 1);
	
	unlock_mutex_out();
	return (0);
}
static int LoutGetVolume(lua_State *L)
{
	// get MIDI-out noteon volume for all outputs 
	// return : volume 0..64(neutral)..127
	lock_mutex_out();

	lua_pushinteger(L, g_volume);

	unlock_mutex_out();
	return (1);
}
/**
* \fn void LinMidiList()
* \brief List the Midi in devices.
* LUA function inMidiList().
* \return table of midi in devices, with device number, and name.
**/
static int LinGetMidiList(lua_State *L)
{
    // return the midi In device in an array
    // no parameter
	lock_mutex_out();

	DWORD nrDevice;
	char name_device[MAXBUFCHAR];
	lua_newtable(L);
	nrDevice = 0;
	while (midi_in_name(nrDevice, name_device))
	{
		lua_pushinteger(L, nrDevice + 1);
		lua_pushstring(L, name_device);
		lua_settable(L, -3);
		nrDevice++;
	}
	
	unlock_mutex_out();
	return(1);
}
static int LinCountMidi(lua_State *L)
{
    // return the number of midi In device
    // no parameter
	lock_mutex_out();

	lua_pushinteger(L, g_nb_midi_in);

	unlock_mutex_out();
	return(1);
}
static int LinGetMidiName(lua_State *L)
{
	// return the name of the midi In device 
	// parameter #1 : device nr
	lock_mutex_out();

	int nrDevice = cap((int)lua_tointeger(L, 1), 0, OUT_MAX_DEVICE, 1);
	char name_device[MAXBUFCHAR] = "";
	midi_in_name(nrDevice, name_device);
	lua_pushstring(L, name_device);

	unlock_mutex_out();
	return(1);
}
/**
* \fn void LoutMidiList()
* \brief List the Midi out devices.
* LUA function outMidiList().
* \return table of midi out devices, with device number, and name.
**/
static int LoutGetMidiList(lua_State *L)
{
	// return the midi Out devices in an array
	// no parameter
	lock_mutex_out();

	DWORD nrDevice;
	char name_device[MAXBUFCHAR];
	lua_newtable(L);
	nrDevice = 0;
	while (midiout_name(nrDevice, name_device))
	{
		lua_pushinteger(L, nrDevice + 1);
		lua_pushstring(L, name_device);
		lua_settable(L, -3);
		nrDevice++;
	}

	unlock_mutex_out();
	return(1);
}
static int LoutGetMidiName(lua_State *L)
{
	// return the midi Out device name
	// parameter #1 : device nr
	lock_mutex_out();

	int nrDevice = cap((int)lua_tointeger(L, 1), 0, OUT_MAX_DEVICE, 1);
	char name_device[MAXBUFCHAR] = "";
	midiout_name(nrDevice, name_device);
	lua_pushstring(L, name_device);

	unlock_mutex_out();
	return(1);
}
static int LoutOpenMidi(lua_State *L)
{
	// open the midi Out device nr
	// parameter #1 : device nr
	lock_mutex_out();
	int nr_devicemidi = cap((int)lua_tointeger(L, 1), 0, OUT_MAX_DEVICE, 1);
	midi_out_open(nr_devicemidi);
	unlock_mutex_out();
	return(0);
}
static int LoutPreOpenMidi(lua_State *L)
{
	// deprecated
	return(0);
}
static int LoutSetChordCompensation(lua_State *L)
{
	// set the Chord Compensation for the chords
	// parameter #1 : [0..32]  Compensation
	lock_mutex_out();
	g_chordCompensation = (int)lua_tointeger(L, 1);
	unlock_mutex_out();
	return(0);
}
static int LoutSetRandomDelay(lua_State *L)
{
	// set the  random delay for chords
	// parameter #1 : [0..127] random delay in ms
	lock_mutex_out();
	g_randomDelay = (int)lua_tointeger(L, 1);
	unlock_mutex_out();
	return(0);
}
static int LoutSetRandomVelocity(lua_State *L)
{
	// set the random velocity for the g_chords
	// parameter #1 : [0..100] random velocity in %
	lock_mutex_out();
	g_randomVelocity = (int)lua_tointeger(L, 1);
	unlock_mutex_out();
	return(0);
}
static int LoutChordSet(lua_State *L)
{
	// set the chord to play with chordon
	// parameter #1 : unique_id
	//     with -1 : return a unique-id
	// parameter #2 : transpose
	// parameter #3 : [0..] delay in ms between notes
	// parameter #4 : ([0..50..100]) % dvelocity between notes ( 50 all equal, 25 divide by two for next note )
	// parameter #5 & #6 : start# and end# of pitches ( 1 & -1 : all pitches , -1 & 1 : all in reverse order )
	// parameter #7.. : list of pitch
	// return the unique_id
	lock_mutex_out();

	long retCode = 0;
	int erropt;
	long id = (int)lua_tointegerx(L, 1, &erropt);
	if (! erropt)
	{
		mlog_out("Error chordset,  id is not valid");
		retCode = -3;
	}
	else
	{
		if (id == -1)
			id = g_unique_id++;
		T_chord* chord = chord_new(id);
		if (chord == NULL)
		{
			mlog_out("mlog_out outChordSet %d. No more chord-slot available\n", id);
			retCode = -1;
		}
		else
		{
			int transpose = (int)lua_tointeger(L, 2);
			chord->dt = (int)lua_tointeger(L, 3);
			chord->dv = (int)lua_tointeger(L, 4);
			int start = (int)lua_tointeger(L, 5);
			int end = (int)lua_tointeger(L, 6);
			int endArg, startArg;
			if (start < 0)
				startArg = lua_gettop(L) - (start * (-1) - 1);
			else
				startArg = start - 1 + 7;
			if (end < 0)
				endArg = lua_gettop(L) - (end * (-1) - 1);
			else
				endArg = end - 1 + 7;
			chord->nbPitch = 0;
			for (int nrArg = startArg; (startArg <= endArg) ? (nrArg <= endArg) : (nrArg >= endArg); nrArg += (startArg <= endArg) ? 1 : -1)
			{
				int p = (int)lua_tointeger(L, nrArg);
				p %= 128 ;
				p += transpose;
				while (p < 0)
					p += 12;
				while (p > 127)
					p -= 12;
				chord->pitch[chord->nbPitch] = p;
				chord->nbPitch++;
				if (chord->nbPitch >= CHORDMAXPITCH)
					break;
			}
			retCode = chord->id;
		}
	}
	lua_pushinteger(L, retCode);
	unlock_mutex_out();
	return (1);
}
static int LoutChordOn(lua_State *L)
{
	// play a chord prepared by chordset
	// parameter #1 : unique_id ( set in outChordSet )
	// parameter #2 : velocity
	// parameter #3 : optional integer delay dt in ms before to send the msg
	// parameter #4 : optional nrTrack ( default 0 )
	lock_mutex_out();

	T_chord* chord = NULL ;
	long retCode = -3;
	int erropt;
	int id = (int)lua_tointegerx(L, 1 , &erropt);
	if (! erropt)
	{
		mlog_out("Error chordon,  id is not valid");
		retCode = -3;
	}
	else
	{
		chord = chord_get(id);
		if (chord == NULL)
		{
			mlog_out("Error chordon,  chord %d does not exist", id);
			retCode = -2;
		}
		else
		{
			retCode = chord->id;
			T_midioutmsg u;
			//mlog_out("chordon,  chord %d \n", id);
			int v = (int)lua_tointeger(L, 2);
			if (g_chordCompensation != 0)
				v = ((200 - (g_chordCompensation * (chord->nbPitch - 1))) * v) / 200;
			int initialdt = (int)luaL_optinteger(L, 3, 0); ;
			u.dt = initialdt ;
			u.track = cap((int)luaL_optinteger(L, 4, 1), 0, MAXTRACK, 1);
			//mlog_out("chordon,  u.track %d \n", u.track);
			if (u.track >= 0)
			{
				retCode = 0;
				u.midimsg.bData[0] = (L_MIDI_NOTEON << 4);
				u.midimsg.bData[3] = 0;
				u.nbbyte = 3;

				for (int c = 0; c < chord->nbPitch; c++)
				{
					int p = chord->pitch[c];
					while (p < 0)
						p += 12;
					while (p>127)
						p -= 12;
					u.midimsg.bData[1] = p;
					int rv = v;
					if (g_randomVelocity != 0)
						rv += (g_randomVelocity * rand()) / RAND_MAX - g_randomVelocity / 2;
					u.midimsg.bData[2] = cap(rv, 1, 128, 0);
					// mlog_out("chordon p=%d,v=%d", u.midimsg.bData[1], u.midimsg.bData[2]);
					u.id = g_unique_id++;
					if (sendmsgdt(u) == false)
						retCode = -1;
					else
					{
						chord->msg_off[chord->nbOff] = u; // to remember that this msg is played
						(chord->nbOff)++;
					}
					if (chord->dv == 0)
						break;
					u.dt = initialdt + c * chord->dt;
					if (g_randomDelay != 0)
						u.dt += (g_randomDelay * rand()) / RAND_MAX;
					if (chord->dv != 64)
						v = ((127 + (chord->dv - 64)) * v) / 127;
					if (v < 1)
						break;
				}
			}
		}
	}
	lua_pushinteger(L, retCode);
	unlock_mutex_out();
	return (1);
}
static int LoutChordOff(lua_State *L)
{
	// stop a chord played with chordon
	// parameter #1 : unique_id ( set in outChordSet )
	// parameter #2 : optional velocity, default 0
	// parameter #3 : optional integer delay dt in ms before to send the msg
	lock_mutex_out();
	int erropt;
	T_chord* chord = NULL;
	int retCode = -1;
	int id = (int)lua_tointegerx(L, 1, &erropt);
	if (! erropt)
	{
		mlog_out("Error chordoff,  id is not valid");
		retCode = -3;
	}
	else
	{
		chord = chord_get(id);
		if (chord == NULL)
		{
			mlog_out("Error chordoff,  chord %d does not exist", id);
			retCode = -2;
		}
		else
		{
			T_midioutmsg u;
			int velo = cap((int)luaL_optinteger(L, 2, 0), 0, 128, 0);
			int dt = (int)luaL_optinteger(L, 3, 0);
			retCode = 0;
			for (int c = 0; c < chord->nbOff; c++)
			{
				u = chord->msg_off[c];
				u.midimsg.bData[2] = velo;
				if (dt != -1000)
					u.dt = dt;
				u.midimsg.bData[0] = (L_MIDI_NOTEOFF << 4) + ((u.midimsg.bData[0]) & 0xF);
				if (unqueue(OUT_QUEUE_NOTEOFF, u) == 0)
				{
					if (sendmsgdt(u) == false)
						retCode = -1;
				}
				else
					retCode = -1;
			}
			chord->id = -1;
			chord->nbOff = 0;
		}
	}
	lua_pushinteger(L, retCode);
	
	unlock_mutex_out();
	return (1);
}

static int LoutNoteOn(lua_State *L)
{
	// parameter #1 : pitch 1..127
	// parameter #2 : optional velocity ( default 64 )
	// parameter #3 : optional unique id.  ( to manage concurrential note-on note-off )
			// default 0
			// with -1 : a unique_id is returned
	// parameter #4 : optional integer delay dt in ms before to send the msg
	// parameter #5 : optional nrTrack ( default 1 )
	// return the unique id
	lock_mutex_out();

	unsigned long retCode = -1;
	T_midioutmsg u;
	int p = (int)lua_tointeger(L, 1);
	p %= 128;
	u.midimsg.bData[1] = p;
	u.midimsg.bData[2] = cap((int)luaL_optinteger(L, 2, 64), 1, 128, 0);
	u.id = (int)luaL_optinteger(L, 3, 0);
	if (u.id == -1)
		u.id = g_unique_id++;
	u.dt = (int)luaL_optinteger(L, 4, 0);
	u.track = cap((int)luaL_optinteger(L, 5, 1), 0, MAXTRACK, 1);
	if (u.track >= 0)
	{
		u.midimsg.bData[0] = (L_MIDI_NOTEON << 4);
		u.midimsg.bData[3] = 0;
		u.nbbyte = 3;
		//mlog_out("noteon %X %d %d", u.midimsg.bData[0], u.midimsg.bData[1], u.midimsg.bData[2]);
		if (sendmsgdt(u) == false)
			retCode = -1;
		else
			retCode = u.id;
	}
	lua_pushinteger(L, retCode);	
	unlock_mutex_out();
	return (1);
}
static int LoutNoteOff(lua_State *L)
{
	// parameter #1 : pitch  1..127 ( if unique id is defined : optional = 0 )
	// parameter #2 : optional velocity, default 0
	// parameter #3 : optional unique id ( to manage concurrential note-on note-off )
	// parameter #4 : optional integer delay dt in ms before to send the msg
	// parameter #5 : optional nrTrack ( default 1 )
	lock_mutex_out();

	int retCode = -1;
	T_midioutmsg u;
	int p = (int)lua_tointeger(L, 1);
	p %= 128;
	u.midimsg.bData[1] = p;
	u.midimsg.bData[2] = cap((int)luaL_optinteger(L, 2, 0), 0, 128, 0);
	u.id = (int)luaL_optinteger(L, 3, 0);
	u.dt = (int)luaL_optinteger(L, 4, 0);
	u.track = cap((int)luaL_optinteger(L, 5, 1), 0, MAXTRACK, 1);
	if (u.track >= 0)
	{
		u.midimsg.bData[0] = (L_MIDI_NOTEOFF << 4);
		u.midimsg.bData[3] = 0;
		u.nbbyte = 3;
		if (unqueue(OUT_QUEUE_NOTEOFF, u) == 0)
			retCode = sendmsgdt(u);
		else
			retCode =  0;
	}
	else
		retCode = -1;

	lua_pushinteger(L, retCode);
	unlock_mutex_out();
	return (1);
}
static int LoutPressure(lua_State *L)
{
    // parameter #1 : pitch
    // parameter #2 : pressure
    // parameter #3 : optional integer delay dt in ms before to send the msg
 	// parameter #4 : optional nrTrack ( default 1 )
	lock_mutex_out();

	int retCode = -1;
	T_midioutmsg u;
	int p = (int)lua_tointeger(L, 1);
	while (p > 127)
		p -= 12;
	while (p < 0)
		p += 12;
	u.midimsg.bData[1] = p;
	u.midimsg.bData[2] = cap((int)lua_tointeger(L, 2), 0, 128, 0);
	u.dt = (int)luaL_optinteger(L, 3, 0);
	u.track = cap((int)luaL_optinteger(L, 4, 1), 0, MAXTRACK, 1);
	if (u.track >= 0)
	{
		u.nbbyte = 3;
		u.id = 0;
		u.midimsg.bData[0] = (L_MIDI_KEYPRESSURE << 4);
		u.midimsg.bData[3] = 0;
		retCode = sendmsgdt(u);
	}
	else
		retCode = -1;

	lua_pushinteger(L, retCode);
	unlock_mutex_out();
	return (1);
}
static int LoutControl(lua_State *L)
{
    // parameter #1 : nr control
    // parameter #2 : data
    // parameter #3 : optional integer delay dt in ms before to send the msg
	// parameter #4 : optional nrTrack ( default 1 )
	lock_mutex_out();

	int retCode = -1;
	T_midioutmsg u;
	u.midimsg.bData[1] = cap((int)lua_tointeger(L, 1), 0, 128, 0);
	u.midimsg.bData[2] = cap((int)lua_tointeger(L, 2), 0, 128, 0);
	u.dt = (int)luaL_optinteger(L, 3, 0);
	u.track = cap((int)luaL_optinteger(L, 4, 1), 0, MAXTRACK, 1);
	if (u.track >= 0)
	{
		u.midimsg.bData[0] = (L_MIDI_CONTROL << 4);
		u.midimsg.bData[3] = 0;
		u.nbbyte = 3;
		u.id = 0;
		retCode =sendmsgdt(u);
	}
	else
		retCode = -1;

	lua_pushinteger(L, retCode);
	unlock_mutex_out();
	return (1);
}
static int LoutProgram(lua_State *L)
{
    // parameter #1 : nr program
    // parameter #2 : optional integer delay dt in ms before to send the msg
	// parameter #3 : optional nrTrack ( default 1 )
    // parameter #4 : optional bank msb
    // parameter #5 : optional bank lsb
	lock_mutex_out();
	
	T_midioutmsg u;
	int retCode = -1;
	u.midimsg.bData[1] = cap((int)lua_tointeger(L, 1), 0, 128, 0);
	u.midimsg.bData[2] = 0;
	u.dt = (int)luaL_optinteger(L, 2, 0);
	u.track = cap((int)luaL_optinteger(L, 3, 1), 0, MAXTRACK, 1);
	if (u.track >= 0)
	{
		u.midimsg.bData[0] = (L_MIDI_PROGRAM << 4);
		u.midimsg.bData[3] = 0;
		u.nbbyte = 2;
		u.id = 0;
		int bank_msb = cap((int)luaL_optinteger(L, 4, -1), -1, 128, 0);
		int bank_lsb = cap((int)luaL_optinteger(L, 5, -1), -1, 128, 0);
		if (bank_msb != -1)
		{
			T_midioutmsg u1;
			u1.midimsg.bData[0] = (L_MIDI_CONTROL << 4);
			u1.midimsg.bData[1] = 0;
			u1.midimsg.bData[2] = bank_msb;
			u1.midimsg.bData[3] = 0;
			u1.track = u.track;
			u1.dt = u.dt;
			u1.id = 0;
			u1.nbbyte = 3;
			sendmsgdt(u1);
		}
		if (bank_lsb != -1)
		{
			T_midioutmsg u1;
			u1.midimsg.bData[0] = (L_MIDI_CONTROL << 4);
			u1.midimsg.bData[1] = 0x20;
			u1.midimsg.bData[2] = bank_lsb;
			u1.midimsg.bData[3] = 0;
			u1.track = u.track;
			u1.dt = u.dt;
			u1.id = 0;
			u1.nbbyte = 3;
			sendmsgdt(u1);
		}
		retCode = sendmsgdt(u);
	}
	else
		retCode = -1;

	lua_pushinteger(L, retCode);
	unlock_mutex_out();
	return (1);
}
static int LoutPitchbend(lua_State *L)
{
    // parameter #1 : value -8192 ..0..8192
    // parameter #2 : optional integer delay dt in ms before to send the msg
	// parameter #3 : optional nrTrack ( default 1 )
	lock_mutex_out();

	int retCode = -1;
	T_midioutmsg u;
	int v = cap((int)lua_tointeger(L, 1), -8192, 8192, 0) + (int)(0x40)*(int)(0x80);
	u.dt = (int)luaL_optinteger(L, 2, 0);
	u.track = cap((int)luaL_optinteger(L, 3, 1), 0, MAXTRACK, 1);
	if (u.track >= 0)
	{
		u.midimsg.bData[0] = (L_MIDI_PITCHBEND << 4) ;
		u.midimsg.bData[2] = (int)(v / (int)(0x80));
		u.midimsg.bData[1] = (int)(v - (int)(u.midimsg.bData[2]) * (int)(0x80));
		u.midimsg.bData[3] = 0;
		u.nbbyte = 3;
		u.id = 0;
		retCode =  sendmsgdt(u);
	}
	else
		retCode = -1;

	lua_pushinteger(L, retCode);
	unlock_mutex_out();
	return (1);
}
static int LoutChannelPressure(lua_State *L)
{
    // parameter #1 : value
    // parameter #2 : optional integer delay dt in ms before to send the msg
    // parameter #3 : optional nrTrack ( default 1 )
	lock_mutex_out();
	
	int retCode = -1;
	T_midioutmsg u;
	u.midimsg.bData[1] = cap((int)lua_tointeger(L, 1), 0, 128, 0);
	u.midimsg.bData[2] = 0;
	u.dt = (int)luaL_optinteger(L, 2, 0);
	u.track = cap((int)luaL_optinteger(L, 3, 1), 0, MAXTRACK, 1);
	if (u.track >= 0)
	{
		u.midimsg.bData[0] = (L_MIDI_CHANNELPRESSURE << 4);
		u.midimsg.bData[3] = 0;
		u.nbbyte = 2;
		u.id = 0;
		retCode = sendmsgdt(u);
	}
	else
		retCode = -1;

	lua_pushinteger(L, retCode);
	unlock_mutex_out();
	return (1);
}
static int LoutTune(lua_State *L)
{
    // parameter #1 : optional, float, A4 Frequency Hz ( default 440 Hz )
	// parameter #2 : optional nrTrack ( default 1 )
	lock_mutex_out();
	
	float freq = luaL_optnumber(L, 1, 440.0);
	int nrTrack = cap((int)luaL_optinteger(L, 2, 1), 0, MAXTRACK, 1);
	send_tune(nrTrack, freq);
	unlock_mutex_out();
	return (0);
}
static int LoutBendRange(lua_State *L)
{
    // parameter #1 : semitone , integer , ( default 1 )
	// parameter #2 : optional nrTrack ( default 1 )
	lock_mutex_out();
	
	int semitone = (int)luaL_optinteger(L, 1, 1);
	int nrTrack = cap((int)luaL_optinteger(L, 2, 1), 0, MAXTRACK, 1);
	send_bendrange(nrTrack, semitone);
	unlock_mutex_out();
	return (0);
}
static int LoutAllNoteOff(lua_State *L)
{
	// parameter #1 : optional, string options with a n s or c ( a:all n:noteoff s:soundoff , c:controler )
	// parameter #2 : optional integer track ( default all )
	lock_mutex_out();
	const char* soption = luaL_optstring(L, 1, "a");
	int nrTrack = cap((int)luaL_optinteger(L, 2, 0), -1, MAXTRACK, 1);
	all_note_off(soption, nrTrack);
	unlock_mutex_out();
	return (0);
}

static int LoutClock(lua_State *L)
{
    // parameter #1 : optional integer track ( default 1  )
	lock_mutex_out();
	
	T_midioutmsg u;
	u.track = cap((int)luaL_optinteger(L, 2, 1), 0, MAXTRACK, 1);
	u.midimsg.bData[0] = L_MIDI_CLOCK;
    u.midimsg.bData[1] = 0;
    u.midimsg.bData[2] = 0;
    u.midimsg.bData[3] = 0;
	u.nbbyte = 1;
    lua_pushinteger(L, sendmsgdt(u));  
	unlock_mutex_out();
	return (1);
}
static int LoutSystem(lua_State *L)
{
    // parameter #1 : byte 1
    // parameter #2 : byte 2
    // parameter #3 : byte 3
   // parameter #4 : optional integer track ( default 1  )
	lock_mutex_out();
	
	T_midioutmsg u;
	u.midimsg.bData[0] = cap((int)lua_tointeger(L, 1), 0, 128, 0);
	u.midimsg.bData[1] = cap((int)lua_tointeger(L, 2), 0, 128, 0);
	u.midimsg.bData[2] = cap((int)lua_tointeger(L, 3), 0, 128, 0);
	u.nbbyte = 3;
	u.dt = 0;
	u.id = 145341;
	u.track = cap((int)luaL_optinteger(L, 4, 1), 0, MAXTRACK, 1);
	lua_pushinteger(L, sendmsgdt(u));
    
	unlock_mutex_out();
	return (1);
}
static int LoutSysex(lua_State *L)
{
    // parameter #1 : message sysex
	// parameter #2 : optional integer track ( default 0  )
	lock_mutex_out();

	const char *sysex = lua_tostring(L, 1);
	int nrTrack = cap((int)luaL_optinteger(L, 2, 1), 0, MAXTRACK, 1);
	bool retCode = midiout_sysex(nrTrack, sysex);
	lua_pushboolean(L, retCode);

	unlock_mutex_out();
	return (1);
}
static int LaudioClose(lua_State *L)
{
	// close audio
	lock_mutex_out();
	mixer_free();
	vi_free();
	vi_init();
	mixer_init();
	unlock_mutex_out();
	return(0);
}
static int LgetAudioList(lua_State *L)
{
	// return list of audio devices available
    // no parameter
	// return : LUA table strings
	lock_mutex_out();
	
	DWORD nr_device;
    lua_newtable(L);
	char name_audio[MAXBUFCHAR];
    nr_device = 0;
	while (audio_name(nr_device, name_audio))
	{
		lua_pushinteger(L, nr_device + 1);
		lua_pushstring(L, name_audio);
		lua_settable(L, -3);
		nr_device++;
	}
	unlock_mutex_out();
	return(1);
}
static int LgetAudioName(lua_State *L)
{
	// return name of audio device
	// parameter #1 : audio_device 1...
	// return : string ( empty if no  device )
	lock_mutex_out();

	char name_audio[MAXBUFCHAR];
	*name_audio = '\0';
	int nr_deviceaudio = cap((int)lua_tointeger(L, 1), 0, MAX_AUDIO_DEVICE,1);
	audio_name(nr_deviceaudio, name_audio);
	lua_pushstring(L, name_audio);
	unlock_mutex_out();
	return(1);
}
static int LaudioSet(lua_State *L)
{
	// set update and buffer delay
	// parameter #1 : audio_device 1...
	// parameter #2 : bass_config_update_period in ms ( 0 = no change )
	// parameter #3 : additional ms for bass_config_buffer ( 0 = no change )
	lock_mutex_out();
	int nr_deviceaudio = cap((int)lua_tointeger(L, 1), 0, MAX_AUDIO_DEVICE, 1);
	g_audio_updateperiod[nr_deviceaudio] = (int)lua_tointeger(L, 2);
	g_audio_buffer[nr_deviceaudio] = (int)lua_tointeger(L, 3);
	unlock_mutex_out();
	return(0);
}

static int LaudioDefaultDevice(lua_State *L)
{
	// set audio default device ( to be called before any use of audio ( vi, wav ... )
	// parameter #1 : audio_device 1...
	lock_mutex_out();
	g_default_audio_device = cap((int)lua_tointeger(L, 1), 0, MAX_AUDIO_DEVICE, 1);
	unlock_mutex_out();
	return (0);
}
static int LasioSet(lua_State *L)
{
	// set audio settings ( to be called after audio opening )
	// parameter #1 : audio device 1...
	// return : buffer_length
	lock_mutex_out();
	int buflen = 0;
#ifdef V_PC
	buflen = 1024;
	int nr_deviceasio = lua_tointeger(L, 1) - 1 - g_nb_audio_bass_device ;
	bool toBeFree = true ;
	bool err = false;
	if ((nr_deviceasio < 0) || (nr_deviceasio >= g_nb_asio_device))
	{
		err = true;
	}
	else
	{
		if (BASS_ASIO_Init(nr_deviceasio, 0) == FALSE)
		{
			if (BASS_ASIO_ErrorGetCode() == BASS_ERROR_ALREADY)
				toBeFree = false;
			else
			{
				err = true;
				mlog_out("audioSet : Error BASS_ASIO_Init err:%d", BASS_ASIO_ErrorGetCode());
			}
		}
		if ((!err) && (BASS_ASIO_SetDevice(nr_deviceasio) == FALSE))
		{
			mlog_out("audioSet :  Error BASS_ASIO_SetDevice err:%d", BASS_ASIO_ErrorGetCode());
			err = true;
		}
		if ((!err) && (BASS_ASIO_ControlPanel() == FALSE))
		{
			mlog_out("audioSet :  Error BASS_ASIO_ControlPanel err:%d", BASS_ASIO_ErrorGetCode());
		}
		BASS_ASIO_INFO info;
		if (!err)
		{
			if (BASS_ASIO_GetInfo(&info) == FALSE)
			{
				mlog_out("audioSet :  Error BASS_ASIO_GetInfo err:%d", BASS_ASIO_ErrorGetCode());
			}
			else
			{
				buflen = info.bufpref;
			}
		}
		if ((!err) && (toBeFree))
		{
			BASS_ASIO_Free();
		}
	}
#endif
	lua_pushinteger(L, buflen);
	unlock_mutex_out();
	return (1);
}
static int LviVolume(lua_State *L)
{
	// set the volume of a midi VI
	// parameter #1 : VI track ( returned by vi_open )
	// parameter #2 : volume 0..64..127
	lock_mutex_out();

	int return_code = 0;
	int vi_nr = (int)lua_tointeger(L, 1) - VI_ZERO;
	int volume = (int)lua_tointeger(L, 2);
	if ((vi_nr >= 0) && (vi_nr < g_vi_opened_nb))
	{
		if (g_vi_opened[vi_nr].sf2_midifont != 0)
		{
			if (BASS_MIDI_FontSetVolume(g_vi_opened[vi_nr].sf2_midifont, (float)(volume) / 64.0) == FALSE)
			{
				mlog_out("Error setting volume VI , err=%d\n", BASS_ErrorGetCode());
				return_code = -1;
			}
		}
		else
		{
			if (BASS_ChannelSetAttribute(g_vi_opened[vi_nr].mstream, BASS_ATTRIB_VOL, (float)(volume) / 64.0) == FALSE)
			{
				mlog_out("Error setting volume VI , err=%d\n", BASS_ErrorGetCode());
				return_code = -1;
			}
		}
	}
	else
	{
		mlog_out("Error volume VI, incorrect nrVI %d\n", vi_nr);
		return_code = -1;
	}
	lua_pushinteger(L, return_code);

	unlock_mutex_out();
	return (1);
}

static int LsoundPlay(lua_State *L)
{
    // parameter #1 : audio file name, with its extension
    // parameter #2 : optional  volume 0..64..127
    // parameter #3 : optional  pan 0..64..127
    // parameter #4 : optional  integer, audio ( ASIO for PC ) device nr, default g_default_audio_device 
    // returned : ref for further manipulation
	lock_mutex_out();
	
	int return_code = 0;
	const char *fname = lua_tostring(L, 1);
    int volume = (int)luaL_optinteger(L, 2, 64);
    int pan = (int)luaL_optinteger(L, 3, 64);
	int nr_deviceaudio = cap((int)luaL_optinteger(L, 4, g_default_audio_device + 1), 0, MAX_AUDIO_DEVICE, 1);
	char vinamedevice[MAXBUFCHAR];
	char viname[MAXBUFCHAR];
	char extension[6];
	strcpy(vinamedevice, fname);
	int forced_device_audio = -1;
	if (getTypeFile(vinamedevice, &forced_device_audio, viname, extension) && ( strcmp(extension,"wav") == 0))
	{
		if (forced_device_audio != -1)
			nr_deviceaudio = forced_device_audio;
		return_code = sound_play(viname, volume, pan, nr_deviceaudio);
	}
    lua_pushinteger(L, return_code);

	unlock_mutex_out();
	return (1);
}
static int LsoundControl(lua_State *L)
{
	// parameter #1 : reference to audio ( returned by sound_play )
	// parameter #2 : optional  volume 0..64..127
	// parameter #3 : optional  pan 0..64..127
	// parameter #4 : control the sound : 0=pause, 1=restart, 2=stop
	lock_mutex_out();

	int return_code = 0;
	int hsound = (int)lua_tointeger(L, 1);
	int volume = luaL_optnumber(L, 2, 64);
	int pan = luaL_optnumber(L, 3, 64);
	int ctrl = (int)luaL_optinteger(L, 4, -1);
	return_code = sound_control(hsound, volume, pan , ctrl);
	lua_pushinteger(L, return_code);

	unlock_mutex_out();
	return (1);
}

static int LoutListProgramVi(lua_State *L)
{
	// create files with the programs in a VI
	// parameter #1 : VI file name, with its .sf2 or .dll extension
	lock_mutex_out();

	const char *fname = lua_tostring(L, 1);
	char vinamedevice[MAXBUFCHAR];
	int nr_deviceaudio ;
	char viname[MAXBUFCHAR];
	strcpy(vinamedevice, fname);
	char extension[6];
	int retCode = true;;
	if (getTypeFile(vinamedevice, &nr_deviceaudio, viname, extension))
	{
		if ( strcmp(extension,"sf2") == 0 )
			retCode = sf2_create_list_prog(viname);
#ifdef V_VST
		if (strcmp(extension, "dll") == 0)
			retCode = vst_create_list_prog(viname);
#endif
	}
	lua_pushinteger(L, retCode);

	unlock_mutex_out();
	return (1);
}
static int LoutTrackOpenVi(lua_State *L)
{
	// open the track on a Virtual-Instrument ( midi-SF2 or VSTI )
	// parameter #1 : integer track nr
	// parameter #2 : integer midi channel
	// parameter #3 : string initial
	// parameter #4 : SF2/VSTI filename, with its .sf2/.dll extension. If file-name ends with @x : x is the parameter#7
	// parameter #5 : optional , integer, number of additional physical MIDI channels attached to this MIDI channel ( default 0 )
	// parameter #6 : optional  integer, Vi font volume 0..64..127
	// parameter #7 : optional  integer, audio ( ASIO for PC ) device nr. Default g_default_audio_device, or as defined in parameter #4
	lock_mutex_out();
	bool retCode = false;
	int nrTrack = cap((int)lua_tointeger(L, 1), 0, MAXTRACK, 1);
	int nr_channelmidi = cap((int)lua_tointeger(L, 2), 0, MAXTRACK, 1);
	const char *tuning = lua_tostring(L, 3);
	const char *fname = lua_tostring(L, 4);
	int nb_extended_midichannel = cap((int)luaL_optinteger(L, 5, 0), 0, 10, 0);
	int volume = (int)luaL_optinteger(L, 6, 64);
	int nr_deviceaudio = cap((int)luaL_optinteger(L, 7, g_default_audio_device + 1), 0, MAX_AUDIO_DEVICE, 1);
	char vinamedevice[MAXBUFCHAR];
	char viname[MAXBUFCHAR];
	char extension[6];
	strcpy(vinamedevice, fname);
	int forced_device_audio = -1;
	if (getTypeFile(vinamedevice, &forced_device_audio, viname, extension) && ((strcmp(extension, "dll") == 0) || (strcmp(extension, "sf2") == 0)))
	{
		int nr_device;
		if (forced_device_audio != -1)
			nr_deviceaudio = forced_device_audio;
		nr_device = mvi_open(viname, nr_deviceaudio, volume, (strcmp(extension, "sf2") == 0));
		if (nr_device != -1)
		{
			g_tracks[nrTrack].device = nr_device;
			g_tracks[nrTrack].channel = nr_channelmidi;
			channel_extended_set(nr_device, nr_channelmidi, nb_extended_midichannel, true);
			g_tracks[nrTrack].volume = 64;
			string_to_control(nrTrack, tuning);
			retCode = true;
			mlog_out("Information : vi open file %s for track#%d : OK", fname, nrTrack + 1);
		}
		else
			mlog_out("Error : midi vi open file %s for track#%d", fname, nrTrack + 1);
	}
	lua_pushinteger(L, retCode);
	unlock_mutex_out();
	return (1);
}
static int LoutTrackOpenMidi(lua_State *L)
{
	// open the track on a midi out
	// parameter #1 : integer track nr
	// parameter #2 : integer midi channel
	// parameter #3 : string initial
	// parameter #4 : midi out device
	// parameter #5 : optional number of additional physical MIDI channels attached to this MIDI channel ( default 0 )
	// parameter #6 : optional track name for information
	// parameter #7 : optional localoff. Default true
	lock_mutex_out();

	int nrTrack = cap((int)lua_tointeger(L, 1), 0, MAXTRACK, 1);
	int nr_channelmidi = cap((int)lua_tointeger(L, 2), 0, MAXCHANNEL, 1);
	const char *tuning = lua_tostring(L, 3);
	int nr_devicemidi = cap((int)lua_tointeger(L, 4), 0, MIDIOUT_MAX, 1);
	int nb_extended_midichannel = cap((int)luaL_optinteger(L, 5, 0), 0, 10, 0);
	const char *trackName = luaL_optstring(L, 6, "");
	int localoff = (int)luaL_optinteger(L, 7, (int)true);
	
	int nr_device = midi_out_open(nr_devicemidi);
	if (nr_device != -1)
	{
		g_tracks[nrTrack].device = nr_device;
		g_tracks[nrTrack].channel = nr_channelmidi;
		channel_extended_set(nr_device, nr_channelmidi, nb_extended_midichannel, true);
		g_tracks[nrTrack].volume = 64;
		if ( localoff ) 
			string_to_control(nrTrack, "localoff(C122/0)");
		string_to_control(nrTrack, tuning);
		char name_device[MAXBUFCHAR] = "";
		midiout_name(nr_device, name_device);
		char buf_ext[MAXCHANNEL+1];
		buf_ext[MAXCHANNEL] = '\0';
		for (int n = 0; n < MAXCHANNEL; n++)
		{
			if (g_channels[nr_device][n].extended == nr_channelmidi)
			{
				buf_ext[n] = '0' + ((n+1) % 10);
			}
			else
				buf_ext[n] = '_';
		}
		mlog_out("Information : midiOut open channel#%d (extended %s) device#%d<%s> for track#%d<%s>  %s : OK", nr_channelmidi + 1, buf_ext, nr_device + 1, name_device, nrTrack + 1, trackName,  localoff ? "with localoff" : "");
	}
	else
		mlog_out("Error : midiOut open device#%d for track#%d<%s>", nr_device + 1, nrTrack + 1, trackName);
	lua_pushinteger(L, (nr_device + 1));
	
	unlock_mutex_out();
	return (1);
}
static int LoutTracksClose(lua_State *L)
{
//	mlog_out("tracksclose");
//	inspect_device();
//	inspect_channel();
	// close all output tracks
	lock_mutex_out();
	track_init();
	unlock_mutex_out();

	return(0);
}

static int Linit(lua_State *L)
{
	// init the luabass module.
	// parameter #1 : fullpath of log file
	// parameter #2 : external timer 
	// parameter #3 : dt in ms
	lock_mutex_out();
	const char *fname = lua_tostring(L, 1);
	bool externalTimer = lua_toboolean(L,2);
	int timerDt = lua_tointeger(L,3);
	init_out(fname,externalTimer, timerDt);
	unlock_mutex_out();
	return (0);
}
static int LexternalTimer(lua_State *L)
{
	// external timer.
	if ( try_lock_mutex_out())
	{
		process_timer_out();
		unlock_mutex_out();
	}
	return (0);
}
static int Lfree(lua_State *L)
{
	lock_mutex_out();
	free();
	unlock_mutex_out();
	return (0);
}

static int LonMidiOut(lua_State *L)
{
	// set a LUA script for each midiout msg
	// this script contains optional functions :
	//	onNoteOn(track,pitch,velocity) : process midiout noteon
	//	onNoteOff(track,pitch,velocity) : process midiout noteoff
	//	onProgram(track,program) : process midiout program
	//	onControl(track,control,value) : process midiout control
	//	onPitchbend(track,value) : process midiout pitchbend
	//	onChannelPressure(track,value) : process midiout channel pressure
	//	onKeyPressure(track,pitch, value) : process midiout key pressure
	//	onClock(track) : process midiout clock
	// The LUA function returns midi-messages to play. 
	// These midi-messages are played immediatly.
	// Each midi-messages is define by these 5 parameters :
	//	track-nr : integer ( must already exists and opened )
	//	type message : string ( L_MIDI_NOTEON L_MIDI_NOTEOFF L_MIDI_CONTROL L_MIDI_PROGRAM L_MIDI_CHANNELPRESSURE L_MIDI_KEYPRESSURE L_MIDI_PITCHBEND )
	//  value 1 : integer 0..127 ( e.g pitch for noteon, program, ... )
	//  value 2 : integer 0..127 ( e.g. velocity for noteon, no meaning fro program , .. )

	// parameter #1 : LUA file name, with its extension
	lock_mutex_out();
	const char *fname = lua_tostring(L, 1);
	bool retCode = onMidiout_open(fname);
	lua_pushboolean(L, retCode);
	unlock_mutex_out();
	return (1);
}
static int LsetVarMidiOut(lua_State *L)
{
	// set a global variable in LUA script which processes the midiout  
	// parameter #1 : variable name
	// parameter #2 : its value
	if (g_LUAoutState)
	{
		lock_mutex_out();
		const char *name = lua_tostring(L, 1);
		lua_xmove(L, g_LUAoutState,1);
		lua_setglobal(g_LUAoutState, name);
		unlock_mutex_out();
	}
	return (0);
}

static int Llogmsg(lua_State *L)
{
	const char* s = lua_tostring(L, 1);
	if (s == NULL) return (0);
	lock_mutex_out();
	mlog_out(s);
	unlock_mutex_out();
	return(0);
}
static int Llogxml(lua_State *L)
{
	const char* s = lua_tostring(L, 1);
	lock_mutex_out();
	mlog_xml(s);
	unlock_mutex_out();
	return(0);
}
static int LoutGetLog(lua_State *L)
{
	lock_mutex_out();
	g_collectLog = lua_tointeger(L, 1) ? true : false;
	if ((g_collectLog) && (nrOutBufLog != nrInBufLog))
	{
		lua_pushboolean(L, true);
		lua_pushstring(L, bufLog[nrOutBufLog]);
		nrOutBufLog++;
		if (nrOutBufLog >= MAXNBLOGOUT)
			nrOutBufLog = 0;
	}
	else
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, "");
	}
	unlock_mutex_out();

	return(2);
}
static int Llogmidimsg(lua_State *L)
{
	lock_mutex_out();
	g_debug = lua_tointeger(L, 1) ? true : false;
	unlock_mutex_out();

	return(2);
}


// publication of functions visible from LUA script
//////////////////////////////////////////////////

static const struct luaL_Reg luabass[] =
{
	// list of functions available in the module { "name_in_LUA", C_name } 

	{ sinit, Linit }, // init MIDI and audio
	{ sexternalTimer, LexternalTimer }, // externalTimer
	{ sfree , Lfree }, // free MIDI and audio ressources

	{ "onMidiOut", LonMidiOut }, // set a LUA script for each MIDI-out
	{ "setVarMidiOut", LsetVarMidiOut }, // set a global variable in the LUA script for MIDI-out

	{ "logmsg", Llogmsg }, // log a string in mlog_out txt file
	{ "logxml", Llogxml }, // log a string in mlog_out xml file
	{ "logmidimsg", Llogmidimsg }, // log midi_msg in mlog_out txt file
	{ soutGetLog, LoutGetLog }, // get log

#ifdef V_DMX
	////// Dmx //////
	{ soutOpenDmx , LdmxOpen },
	{ soutSetDmx , LdmxSet },
	{ soutDmx , LdmxOut },
	{ "outDmxall" , LdmxOutAll },
	{ "outGetDmxList" , LdmxList },
	{ "outGetDmxCount" , LdmxCount },
	{ soutGetDmxName , LdmxName },
	
#endif

	////// in ///////

	{ "inGetMidiList", LinGetMidiList }, // list the midiin ports 
	{ "inCountMidi", LinCountMidi }, // count the midiin ports 
	{ sinGetMidiName, LinGetMidiName }, // name of midiin port 

	/////// out ////////
	{ "outSetCurve", LoutSetCurve }, // set the curves
	{ "outTranspose", LoutTranspose }, // transpose

	{ "outGetMidiList", LoutGetMidiList }, // list the midiout ports 
	{ soutGetMidiName, LoutGetMidiName }, // return name of midiout port 
	{ soutOpenMidi, LoutOpenMidi }, // open an MIDI-out
	{ soutPreOpenMidi, LoutPreOpenMidi }, // deprecated : NIL

	{ soutListProgramVi, LoutListProgramVi }, // create list of programs available in a Virtual Instrument ( SF2 or VST )
	{ soutTrackOpenVi, LoutTrackOpenVi }, // open a track on Virtual Instrument ( SF2 or VST )
	{ soutTrackOpenMidi, LoutTrackOpenMidi }, // open a track on an MIDI-out/channel

	{ soutTracksClose, LoutTracksClose }, // close all outputs tracks
	{ soutTrackMute, LoutTrackMute }, // mute this track
	{ soutSetTrackVolume, LoutSetTrackVolume }, // set the volume of the noteOn on this track
	{ soutGetTrackVolume, LoutGetTrackVolume }, // get the volume of the noteOn on this track
	{ soutSetTrackCurve, LoutSetTrackCurve }, // set the curve of the noteOn on this track
	{ soutSetTrackInstrument, LoutSetTrackInstrument }, // set the instrument on this track
	{ soutSetVolume, LoutSetVolume }, // set the volume of the outputs
	{ soutGetVolume, LoutGetVolume }, // get the volume of the outputs

	{ soutSetChordCompensation, LoutSetChordCompensation }, // set chord compensation 
	{ soutSetRandomDelay, LoutSetRandomDelay }, // set random delay 
	{ soutSetRandomVelocity, LoutSetRandomVelocity }, // set random velocity 

	{ "outChordSet", LoutChordSet }, // set a chord 
	{ "outChordOn", LoutChordOn }, // send a chord-on a track
	{ "outChordOff", LoutChordOff }, // send a chord-off on a track

	{ "outNoteOn", LoutNoteOn }, // send a note-on a track
	{ "outNoteOff", LoutNoteOff }, // send a note-off on a track
	{ "outProgram", LoutProgram }, // send a program on a track
	{ "outControl", LoutControl }, // send a control on a track
	{ "outPitchbend", LoutPitchbend }, // send a pitchbend on a track
	{ "outChannelPressure", LoutChannelPressure }, // send a channel pressure on a track
	{ "outPressure", LoutPressure }, // send a key pressure on a track
	{ "outTune", LoutTune }, // tune the A4 frequency on a track
	{ "outBendrange", LoutBendRange }, // set the bend range in semiton on a track

	{ soutAllNoteOff, LoutAllNoteOff }, // switch off all current notes on a track
	
	{ "outSysex", LoutSysex }, // send a sysex message on a track ( midi only )
	{ "outClock", LoutClock }, // send a timing-clock message on a track ( midi only )
	{ "outSystem", LoutSystem }, // send a free-format short midi message on a track ( midi only )

	{ "getAudioList", LgetAudioList }, // list audio device
	{ "getAudioName", LgetAudioName }, // name audio device
	{ "audioClose", LaudioClose }, // close audio device
	{ "asioSet", LasioSet }, // open asio device settings
	{ "audioSet", LaudioSet }, // set update and buffer delay 
	{ "audioDefaultDevice", LaudioDefaultDevice }, // set audio default device 
	{ "viVolume", LviVolume }, // volume of the vi

	{ "outSoundPlay", LsoundPlay }, // play a sound file
	{ "outSoundControl", LsoundControl }, // control a sound playing

	{ NULL, NULL }
};
extern "C" int luaopen_luabass(lua_State *L)
{
    // used by LUA funtion : luabass = require "luabass"
    // luabass.dll must be in a path "visible" from the LUA interpreter
    // after "require", function can be called from LUA, using luabass.open, ...
    // this function is the only one which must be visible in the DLL from LUA script
    // it is declared in the luabass.def
    // the def file is linked with the optopn set in the properties/link
    luaL_newlib(L, luabass);
    return(1);
}

