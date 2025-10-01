/**
* \file basslua.c
* \brief Midi-IN Event LUA processor 
* \author Franck Revolle
* Objective :
*   This module :
*     - manages hardware :
*         - MIDI-IN real-time devices ( using un4seen bass-midi )
*         - internal timer
*         - external User-interface
*     - manages an LUA script file to process the hardware events

* External dependency :
*   bass-DLL for midi-in management ( www.un4seen.com )
*
* platform :  MAC PC LINUX
*
*/

//////////////////////////////////////////////


#ifdef _WIN32
#define V_PC 1
#endif

#ifdef _WIN64
#define V_PC 1
#endif

#ifdef __APPLE__
#include "TargetConditionals.h"
#if TARGET_IPHONE_SIMULATOR
// iOS Simulator
#elif TARGET_OS_IPHONE
// iOS device
#elif TARGET_OS_MAC
#define V_MAC 1
#else
// Unsupported platform
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

/*
#if defined(V_PC)
#define V_VST 1
#endif
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#ifdef V_PC
#include <ctgmath>
#include <windows.h>
#endif
#ifdef V_MAC
#include <math.h>
#endif
#include <assert.h>

#include <lua.hpp>
#include <lauxlib.h>
#include <lualib.h>
#include <bass.h>
#include <bassmidi.h>
#include <luabass.h>

#ifdef  V_LINUX
#define V_MLOG 1
#endif

#ifdef V_PC
#include <mmsystem.h>
#endif

#ifdef V_MAC
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <CoreMIDI/CoreMIDI.h>
#include <CoreMIDI/MIDIServices.h>
#include <CoreMIDI/MIDISetup.h>
#include <CoreMIDI/MIDIThruConnection.h>
#include <pthread.h>
#endif

#ifdef V_LINUX
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/soundcard.h>
#include <unistd.h>
#include <math.h>
#endif

#ifdef V_VST
#include <aeffect.h>
#include <aeffectx.h>
#include <vstfxstore.h>
#endif

#ifdef V_MLOG
#include <mlog.h>
#endif


#include "global.h"
#include "basslua.h"

#ifdef V_PC
// disable warning for mismatch lngth between LUA int and C int
#pragma warning( disable : 4244 )
// disable warning for deprecated ( e.g. sprintf )
#pragma warning( disable : 4996 )
#endif


static char pluaparam[1024] = "";
static char pfluaname[1024] = "";
static long pluadatefname = 0;

#define MAXPITCH 128
#define MAXCHANNEL 16
#define MIDIIN_MAX 16

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

/**
* \struct T_selector
* \brief Group of criteria to trigger an event
*
* It groups a set of notes in a single object, to trigger event in teh LUA script.
*/
#define SELECTORMAXPITCH 5
#define SELECTORMAX 50
typedef struct t_selector
{
	int luaNrAction;
	char op; /*!< operator between the pitch ( o for or , b for between ) */
	int nrDevice; /*!< on this device  */
	int nrChannel; /*!< on this channel  */
	int type_msg; /*!< for this type of msg */
	int pitch[SELECTORMAXPITCH]; /*!< pitches of the selector */
	int nbPitch; /*!< number of pitches */
	bool stopOnMatch;
	char param[128];
} T_selector;

static char g_path_in_error_txt[MAXBUFCHAR] = "basslua_log_in.txt";
#define MAXBUFERROR 64
#define MAXLENBUFERROR 1024
#define BUFFER_SIZE 1000 // for wchar translation


#define smidiToOpen "midiinOpen" // global LUA table which contains the MIDI-in to open
#define smidiSelector "midiinSelector" // global LUA boolean to validate or not selectors

#define sinit "init" // luabass function called to init the bass midi out 
#define sonStart "onStart" // LUA function called just after the init ( e.g. to initialise the MIDI settings)
#define sonStop "onStop" // LUA function called before to close the LUA stack g_LUAstate
#define sfree "free" // luabass function called to free the bass midi out 

// LUA-functions called by this DLL 
#define LUAFunctionMidi "onMidi" // LUA funtion to call when new midi short msg ( 3 bytes like or program )
#define LUAFunctionNoteOn "onNoteOn" // LUA funtion to call when new midiin noteon
#define LUAFunctionNoteOff "onNoteOff" // LUA funtion to call when new midiin noteoff
#define LUAFunctionKeyPressure "onKeypressure" // LUA funtion to call when new midiin keypressure
#define LUAFunctionControl "onControl" // LUA funtion to call when new midiin control
#define LUAFunctionProgram "onProgram" // LUA funtion to call when new midiin program
#define LUAFunctionChannelPressure "onChannelPressure" // LUA funtion to call when new midiin channelpressure
#define LUAFunctionPitchBend "onPitchBend" // LUA funtion to call when new midiin pitchbend
#define LUAFunctionSystemCommon "onSystemeCommon" // LUA funtion to call when new midiin systemcommon
#define LUAFunctionSysex "onSysex" // LUA funtion to call when new midiin sysex
#define LUAFunctionActive "onActive" // LUA funtion to call when new midiin active sense
#define LUAFunctionClock "onClock" // LUA funtion to call when midiin clock

#define LUAFunctionTimer "onTimer" // LUA funtion to call when timer is triggered 
#define onSelector "onSelector" // LUA functions called with noteon noteoff event, add info 

//////////////////////////////
//  static statefull variables
//////////////////////////////

// list of midiin status. true if midin is open.
static bool g_midiopened[MIDIIN_MAX] = { false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false } ;

static bool g_collectLog = false;
static int nrOutBufLog = 0;
static int nrInBufLog = 0;
#define MAXNBLOGOUT 64
static char bufLog[MAXNBLOGOUT][MAXBUFCHAR];


// static bool g_statuspitch[MIDIIN_MAX][MAXCHANNEL][MAXPITCH]; // status of input  pitch

static lua_State *g_LUAstate = 0; // LUA state, loaded with the script which manage midiIn messages

static bool g_process_Midi = false ;
static bool g_process_NoteOn = false;
static bool g_process_NoteOff = false;
static bool g_process_Control = false;
static bool g_process_Program = false;
static bool g_process_PitchBend = false;
static bool g_process_KeyPressure = false;
static bool g_process_ChannelPressure = false ;
static bool g_process_Sysex = false;
static bool g_process_SystemCommon = false;
static bool g_process_Clock = false;
static bool g_process_Activesensing = false;
static bool g_process_Timer = false;
static int g_countmidiin = 0;

static T_selector g_selectors[SELECTORMAX];
static int g_selectormax = 0;
static bool g_selector_active = true ;

static int g_timer_in_dt = 50 ; // ms between two internal timer interrupts on LUA input
static double g_current_t = 0.0; // time in s for the timer

static int actionMode = modeChord;

static bool g_timer_in_ok = false ;
static bool g_mutex_in_ok = false ;
#ifdef V_PC
// timer to trigger LUA regularly
static HANDLE g_timer_in = NULL;
// mutex to protect the input ( from GUI, timer, and Mid-in , to LUA )
static HANDLE g_mutex_in = NULL;
#endif
#ifdef V_MAC
// timer to trigger LUA regularly
static pthread_t g_loop_in_run_thread ;
// mutex to protect the access of the midiin process
static pthread_mutex_t g_mutex_in;
#endif
#ifdef V_LINUX
static pthread_t g_loop_in_run_thread ;
static timer_t g_timer_in_id;
#define MTIMERSIGNALIN (SIGRTMIN+0)
static pthread_mutex_t g_mutex_in;
#endif


voidcallback g_fcallback;
static void null_fcallback(double time , int nr_device , int type_msg , int channel , int value1 , int value2 , bool isSelected )
{

}
static int pitchbend_value(T_midimsg u)
{
	return((int)(u.bData[2]) * (int)(0x80) + (int)(u.bData[1]) - (int)(0x2000));
}
static int pitch_to_white_key(int pitchin, int pitch0, int *sharp)
{
	// return the white key index of pitchin, Relatively to pitch0
	// sharp is set to one if pitch0 is a black key

	int o0 = pitch0 / 12;
	int oi = pitchin / 12;

	int m0 = pitch0 % 12;
	int mi = pitchin % 12;

	int ri, r0;
	*sharp = 0;

	switch (m0)
	{
	case 0: r0 = 0; break;
	case 1: case 2: r0 = 1; break;
	case 3: case 4: r0 = 2; break;
	case 5: r0 = 3; break;
	case 6: case 7: r0 = 4; break;
	case 8: case 9: r0 = 5; break;
	case 10: case 11: r0 = 6; break;
	default: r0 = 0; break;
	}
	switch (mi)
	{
	case 0: ri = 0; break;
	case 1: ri = 0; *sharp = 1; break;
	case 2: ri = 1; break;
	case 3: ri = 1; *sharp = 1; break;
	case 4: ri = 2; break;
	case 5: ri = 3; break;
	case 6: ri = 3; *sharp = 1; break;
	case 7: ri = 4; break;
	case 8: ri = 4; *sharp = 1; break;
	case 9: ri = 5; break;
	case 10: ri = 5; *sharp = 1; break;
	case 11: ri = 6; break;
	default: ri = 0; break;
	}
	int r = ri - r0 + 7 * (oi - o0);
	return r;
}

/////////////////////////////////////////////////////
// FOR INPUT ( GUI, timer, Midin ===> LUA )
/////////////////////////////////////////////////////


static void log_in_init(const char *fname)
{
	if ((fname != NULL) && (strlen(fname) > 0))
	{
		strcpy(g_path_in_error_txt, fname);
		strcat(g_path_in_error_txt, "_in.txt");
	}
	FILE * pFile = fopen(g_path_in_error_txt, "w");;
	if (pFile == NULL) return;
	fclose(pFile);
}
void mlog_in(const char * format, ...)
{
	char msg[MAXBUFCHAR];
	va_list args;
	va_start(args, format);
	vsprintf(msg, format, args);
	va_end(args);
	FILE * pFile = fopen(g_path_in_error_txt, "a");;
	if (pFile != NULL)
	{
		fprintf(pFile, "%s", msg);
		fprintf(pFile, "\n");
		fclose(pFile);
	}
	if (g_collectLog)
	{
		assert((nrInBufLog>=0)&&(nrInBufLog<MAXNBLOGOUT));
		assert(strlen(msg)<MAXBUFCHAR);
		strcpy(bufLog[nrInBufLog], msg);
		nrInBufLog++;
		if (nrInBufLog >= MAXNBLOGOUT)
			nrInBufLog = 0;
	}
	return;
}
static void lock_mutex_in()
{
#ifdef V_PC
	WaitForSingleObject(g_mutex_in, INFINITE );
#endif
#ifdef V_MAC
  pthread_mutex_lock(&g_mutex_in);
#endif
#ifdef V_LINUX
  pthread_mutex_lock(&g_mutex_in);
#endif
}
static bool try_lock_mutex_in()
{
#ifdef V_PC
	return(WaitForSingleObject(g_mutex_in, 0 ) == WAIT_OBJECT_0);
#endif
#ifdef V_MAC
  return (pthread_mutex_trylock(&g_mutex_in) == 0 );
#endif
#ifdef V_LINUX
  return ( pthread_mutex_trylock(&g_mutex_in) == 0 );
#endif
}
static void unlock_mutex_in()
{
#ifdef V_PC
	ReleaseMutex(g_mutex_in);
#endif
#ifdef V_MAC
	pthread_mutex_unlock(&g_mutex_in);
#endif
#ifdef V_LINUX
	pthread_mutex_unlock(&g_mutex_in);
#endif
}
static int action_table(const char *module, const char *table, const int index, const char* field, char*svalue, int *ivalue, int action)
{
	// if index >= 0 : works on module.table[index+1].field
	// if index < 0 , works on module.table.field
	// action is made of :
	//		0x1<<0 : get field value in svalue or *ivalue
	//		0x1<<1 : set svalue|*ivalue in field
	//		0x1<<2 : field(index[,svalue][,*ivalue]) at this index
	//		0x1<<3 : field(index[,svalue][,*ivalue]) of the table
	//		0x1<<4 : set nil in field
	// return action done
	if (g_LUAstate == NULL)
		return 0;
	int retCode = 0;
	if (lua_getglobal(g_LUAstate, module) != LUA_TTABLE)
	{
		mlog_in("basslua_table : module %s is not available in LUA script", module);
		lua_pop(g_LUAstate, lua_gettop(g_LUAstate)); // pop all
		return 0;
	}
	if (lua_getfield(g_LUAstate, -1, table) != LUA_TTABLE)
	{
		if (strcmp(table, tableInfo) != 0)
			mlog_in("basslua_table : table %s is not available in module %s ", table, module);
		lua_pop(g_LUAstate, lua_gettop(g_LUAstate)); // pop all
		return 0;
	}
	if ((index >= 0) && (lua_geti(g_LUAstate, -1, index + 1) != LUA_TTABLE))
	{
		lua_pop(g_LUAstate, lua_gettop(g_LUAstate)); // pop all
		return 0;
	}
	if ((action & tableSetKeyValue) && (field != NULL) && ((svalue) || (ivalue)))
	{
		// set values
		if (svalue)
		{
			lua_pushstring(g_LUAstate, svalue);
			lua_setfield(g_LUAstate, -2, field);
		}
		if (ivalue)
		{
			lua_pushinteger(g_LUAstate, *ivalue);
			lua_setfield(g_LUAstate, -2, field);
		}
		retCode |= tableSetKeyValue;
	}
	if ((action & tableGetKeyValue) && (field != NULL) && ((svalue) || (ivalue)))
	{
		// get values
		lua_getfield(g_LUAstate, -1, field);
		if (svalue)
			*svalue = '\0';
		if (lua_isstring(g_LUAstate, -1) && svalue)
		{
			strcpy(svalue, lua_tostring(g_LUAstate, -1));
			retCode |= tableGetKeyValue;
		}
		if (ivalue)
			*ivalue = 0;
		if (lua_isinteger(g_LUAstate, -1) && ivalue)
		{
			*ivalue = (int)lua_tointeger(g_LUAstate, -1);
			retCode |= tableGetKeyValue;
		}
		lua_pop(g_LUAstate, 1); // pop field
	}
	if ((action & tableNilKeyValue) && (field != NULL) && ((svalue) || (ivalue)))
	{
		// nil values
		lua_pushnil(g_LUAstate);
		lua_setfield(g_LUAstate, -2, field);
		retCode |= tableNilKeyValue;
	}
	if ((action & tableCallKeyFunction) && (field != NULL))
	{
		// call lua_function([svalue][,ivalue]) of this item
		if (lua_getfield(g_LUAstate, -1, field) == LUA_TFUNCTION)
		{
			int nbArg = 0;
			if (svalue)
			{
				lua_pushstring(g_LUAstate, svalue);
				nbArg++;
			}
			if (ivalue)
			{
				lua_pushinteger(g_LUAstate, *ivalue);
				nbArg++;
			}
			if (lua_pcall(g_LUAstate, nbArg, 0, 0) != LUA_OK)
			{
				mlog_in("basslua_table mlog_in calling item callfunction in %s %s : %s", module, table, lua_tostring(g_LUAstate, -1));
				lua_pop(g_LUAstate, 1);
			}
			else
			{
				retCode |= tableCallKeyFunction;
			}
		}
	}
	if ((action & tableCallTableFunction) && (field != NULL))
	{
		// call lua_function(index[,svalue][,ivalue]) of this table
		if (lua_getfield(g_LUAstate, -2, field) == LUA_TFUNCTION)
		{
			lua_pushinteger(g_LUAstate, index + 1);
			int nbArg = 1;
			if (svalue)
			{
				lua_pushstring(g_LUAstate, svalue);
				nbArg++;
			}
			if (ivalue)
			{
				lua_pushinteger(g_LUAstate, *ivalue);
				nbArg++;
			}
			if (lua_pcall(g_LUAstate, nbArg, 0, 0) != LUA_OK)
			{
				mlog_in("basslua_table : mlog_in calling table callfunction in %s %s : %s", module, table, lua_tostring(g_LUAstate, -1));
				lua_pop(g_LUAstate, 1);
			}
			else
			{
				retCode |= tableCallTableFunction;
			}
		}
	}
	lua_pop(g_LUAstate, lua_gettop(g_LUAstate)); // pop all
	return retCode;
}
bool basslua_call(const char *module, const char *function, const char *sig, ...)
{
	// call module.function with parameters as described in first parameter, with syntax [arguments]*>[results]*:
	// d : double
	// i : integer
	// b : boolean
	// s : char*
	// > : end of descriptor of input argument, followed by descriptor of results
	// example : 
	//    basslua_call("_G","func","dd>dd",x,y,&w,&z)
	//       call the LUA funtion in module global
	//       dd>dd : the function has two integers as input ( x , y ) , and returns two integers ( &w, &z )
	lock_mutex_in();
	bool retCode = true;
	if (g_LUAstate == NULL)
		retCode = false;
	else
	{
		if (lua_getglobal(g_LUAstate, module) == LUA_TTABLE)
		{
			if (lua_getfield(g_LUAstate, -1, function) == LUA_TFUNCTION)
			{
				va_list vl;
				int narg, nres; // number of arguments ans results
				va_start(vl, sig);
				for (narg = 0; *sig; narg++) // for each argument
				{
					luaL_checkstack(g_LUAstate, 1, "lua_call : too many arguments");
					switch (*sig++)
					{
					case 'd': // double argument
						lua_pushnumber(g_LUAstate, va_arg(vl, double));
						break;
					case 'i':
						lua_pushinteger(g_LUAstate, va_arg(vl, int));
						break;
					case 'b':
						lua_pushboolean(g_LUAstate, (bool)va_arg(vl, int));
						break;
					case 's':
					{
								char buf[5000];
								strcpy(buf, va_arg(vl, char*));
								lua_pushstring(g_LUAstate, buf);
								break;
					}
					case '>':
						goto endargs;
					default:
						mlog_in("basslua_call : Invalid ergument descriptor , calling %s", function);
					}
				}
			endargs:
				nres = (int)strlen(sig);
				if (lua_pcall(g_LUAstate, narg, nres, 0) != LUA_OK)
				{
					mlog_in("basslua_call : error calling LUA function %s :err=%s", function, lua_tostring(g_LUAstate, -1));
					lua_pop(g_LUAstate, 1);
				}
				else
				{
					int nrReturnedParam = 1;
					nres = -nres;
					while (*sig) // repeat for each result
					{
						switch (*sig++)
						{
						case 'd':
						{
									int isnum;
									double n = lua_tonumberx(g_LUAstate, nres, &isnum);
									if (!isnum)
										mlog_in("basslua_call : result#%d should be number in %s, returned type : %s", nrReturnedParam, function, lua_typename(g_LUAstate, lua_type(g_LUAstate, nres)));
									*va_arg(vl, double *) = n;
									break;
						}
						case 'i':
						{
									int isnum;
									int n = (int)lua_tointegerx(g_LUAstate, nres, &isnum);
									if (!isnum)
										mlog_in("basslua_call : result#%d should be integer in %s, returned type : %s", nrReturnedParam, function, lua_typename(g_LUAstate, lua_type(g_LUAstate, nres)));
									*va_arg(vl, int *) = n;
									break;
						}
						case 'b':
						{
									if (lua_isboolean(g_LUAstate, nres))
									{
										bool n = (lua_toboolean(g_LUAstate, nres)) ? true : false;
										*va_arg(vl, bool *) = n;
									}
									else
										mlog_in("basslua_call : result#%d should be boolean in %s, returned type : %s", nrReturnedParam, function, lua_typename(g_LUAstate, lua_type(g_LUAstate, nres)));
									break;
						}
						case 's':
						{
									const char* s = lua_tostring(g_LUAstate, nres);
									if (s == NULL)
										mlog_in("basslua_call : result#%d should be string in %s, returned type : %s", nrReturnedParam, function, lua_typename(g_LUAstate, lua_type(g_LUAstate, nres)));
									else
										strcpy(va_arg(vl, char *), s);
									break;
						}
						default:
							mlog_in("basslua_call : result#%d not recognized in %s", nrReturnedParam, function);
						}
						nres++;
						nrReturnedParam++;
					}
				}
				va_end(vl);
			}
			else
			{
				mlog_in("basslua_call : function %s is not available in module %s ", function, module);
				retCode = false;
				lua_pop(g_LUAstate, 1); // popup non function
			}
		}
		else
		{
			mlog_in("module %s is not available in LUA script", module);
			retCode = false;
		}
		lua_pop(g_LUAstate, 1); // pop the module
		lua_pop(g_LUAstate, lua_gettop(g_LUAstate)); // pop all
	}
	unlock_mutex_in();
	return retCode;
}
static void runAction(int nrAction,double time, int nr_selector, int nrChannel, int type_msg, int d1, int d2, const char *param, int index, int mediane, int whiteIndex, int whiteMediane, int sharp)
{
	// mlog_in("runaction #%d (%d==0!)", nrAction , lua_gettop(g_LUAstate));
	if (lua_getglobal(g_LUAstate, tableActions) == LUA_TTABLE)
	{
		// mlog_in("lua_getglobal tableActions OK");
		int ret = lua_geti(g_LUAstate, 1, nrAction + 1);
		if ( ret == LUA_TTABLE)
		{
			//mlog_in("g_LUAstate nrAction=%d OK", nrAction);
			bool functionOK = false;
			if (lua_getfield(g_LUAstate, 2, fieldCallFunction) == LUA_TFUNCTION)
			{
				functionOK = true;
				//mlog_in("fieldCallFunction OK");
			}
			else
			{
				lua_pop(g_LUAstate, 1); // pop the bad value
				// try with the Function<mode>
				if (actionMode == modeScore)
				{
					functionOK = (lua_getfield(g_LUAstate, 2, fieldCallScore) == LUA_TFUNCTION);
					//mlog_in("fieldCallScore %s", functionOK?"OK":"ko");
				}
				if (actionMode == modeChord)
				{
					functionOK = (lua_getfield(g_LUAstate, 2, fieldCallChord) == LUA_TFUNCTION);
					//mlog_in("fieldCallChord %s", functionOK ? "OK" : "ko");
				}
			}
			if ( functionOK )
			{
				lua_pushnumber(g_LUAstate, time);
				int bid = nr_selector * 127 * 17 + (nrChannel+1) * 17 + d1;
				lua_pushinteger(g_LUAstate, bid);
				lua_pushinteger(g_LUAstate, nrChannel + 1);
				lua_pushinteger(g_LUAstate, type_msg);
				lua_pushinteger(g_LUAstate, d1);
				lua_pushinteger(g_LUAstate, d2);
				//mlog_in("call parameters t=%f bid=%d nr_chanem=%d d1=%d d2=%d", time, bid, nrChannel, d1, d2);
				if (param)
					lua_pushstring(g_LUAstate, param);
				else
					lua_pushstring(g_LUAstate, "");
				lua_pushinteger(g_LUAstate, index);
				lua_pushinteger(g_LUAstate, mediane);
				lua_pushinteger(g_LUAstate, whiteIndex);
				lua_pushinteger(g_LUAstate, whiteMediane);
				lua_pushinteger(g_LUAstate, sharp);
				if (lua_pcall(g_LUAstate, 12, 0, 0) != LUA_OK) // call and pop function & parameters
				{
					mlog_in("error call LUA %s, action= %d ", lua_tostring(g_LUAstate, -1), nrAction);
					lua_pop(g_LUAstate, 1);
				}
			}
			else
			{
				lua_pop(g_LUAstate, 1); // pop fcallfunction
				mlog_in("error : no LUA function for action= %d ",  nrAction);
			}
		}
		else
		{
			mlog_in("g_LUAstate nrAction=%d KO", nrAction);
		}
		lua_pop(g_LUAstate, 1); // pop luaAction table
	}
	else
	{
		mlog_in("lua_getglobal tableActions KO");
	}
	lua_pop(g_LUAstate, 1); // pop table actions
}
bool selectorTrigger(int nr_selector, double time, int nrDevice, int nrChannel, int type_msg, int d1, int d2)
{
	// if this note/program/control matches a selector, it calls the LUA functions
	//     midibetween() : for between selector
	//			parameters :
	//				sid : unique id of the selector ( specified in the creation )
	//				double time
	//              bid : unique id of the note which trigger this selector
	//              type_msg : MIDI type of msg
	//              channel : channel of the note
	//				pitch : pitch of the note
	//              velocity : velocity of the note ( 0 for noteff )
	//              param : string of parameter in the selector
	//              index : index of the note within the selector ( 1=first , 2=second, ... )
	//              mediane : index of the note from the middle of the selector ( 0 = middle )
	//              white-key index : index of the white_key within the selector
	//              white-key mediane : index of the white_key from the middle of the selector
	//              sharp : 1 for black key , else 0 
	//     midior() : for or selector
	//			parameters :
	//				sid : unique id of the selector ( specified in the creation )
	//				double time
	//              bid : unique id of the note which trigger ths selector
	//              type_msg : MIDI type of msg
	//              channel : channel of the note
	//				pitch : pitch of the note
	//              velocity : velocity of the note ( 0 for noteff )
	//              param : string of parameter in the selector
	//              index : index of the note within the selector ( 1=first , 2=second, ... )
	//              0
	//              0
	//              0
	//              0
	T_selector *s = &(g_selectors[nr_selector]);
	int sharp;
	switch (s->op)
	{
	case 'b': //between
		if ((s->nbPitch == 2) &&
			(d1 >= s->pitch[0]) && (d1 <= s->pitch[1]))
		{
			int v = pitch_to_white_key(d1, s->pitch[0], &sharp);
			int w = pitch_to_white_key(d1, (s->pitch[1] + s->pitch[0]) / 2, &sharp);
			runAction(s->luaNrAction, time, nr_selector, nrChannel, type_msg, d1, (type_msg == NOTEOFF) ? 0 : d2, s->param,
				d1 - s->pitch[0] + 1, d1 - (s->pitch[1] + s->pitch[0]) / 2 + 1,
				v, w, sharp);
			//mlog_in("selectorSearch #%d found between", nr_selector);
			return true;
		}
		break;
	case 'o': //or
		for (int m = 0; m < s->nbPitch; m++)
		{
			if (d1 == s->pitch[m])
			{
				runAction(s->luaNrAction, time, nr_selector, nrChannel, type_msg, d1, d2, s->param,
					m + 1, 0,
					0, 0, 0);
				//mlog_in("selectorSearch #%d found or", nr_selector);
				return true;
			}
		}
		break;
	default:
		break;
	}
	return false;
}
bool selectorSearch(double time, int nrDevice, int nrChannel, int type_msg, int d1, int d2)
{
	// search a note/program/control ( in midimsg u ) , within the selectors
	// the selectors are created with the LUA funtion selector()

	if (! g_selector_active )
		return false ;

	bool found = false ;
	for (int nr_selector = 0; nr_selector < g_selectormax; nr_selector++)
	{
		T_selector *s = &(g_selectors[nr_selector]);
		//mlog_in("selectorSearch #%d/%d d1=%d", nr_selector, g_selectormax, d1);
		if (((s->type_msg == type_msg) || (s->type_msg == NOTEONOFF && ((type_msg == NOTEON) || (type_msg == NOTEOFF))))
			&& ((s->nrDevice == -1) || (s->nrDevice == nrDevice))
			&& ((s->nrChannel == -1) || (s->nrChannel == nrChannel))
			)
		{
			if (selectorTrigger(nr_selector, time, nrDevice, nrChannel, type_msg, d1, d2))
			{
				found = true;
				if (g_selectors[nr_selector].stopOnMatch)
					return true;
			}
		}
	}
	return found ;
}
static void initSelector()
{
	for (int i = 0; i < SELECTORMAX; i++)
	{
		T_selector *t = &(g_selectors[i]);
		t->luaNrAction = 0;
		t->op = 'b';
		t->nrDevice = -1;
		t->nrChannel = -1;
		t->type_msg = NOTEONOFF;
		for (int j = 1; j < SELECTORMAXPITCH; j++)
			t->pitch[j] = 0;
		t->nbPitch = 0;
		t->stopOnMatch = false;
		t->param[0] = '\0';
	}
	g_selectormax = 0;
}
static void midi_init()
{
	lua_pushnil(g_LUAstate);
	lua_setglobal(g_LUAstate, smidiToOpen);
}
static void pitch_init()
{
/*
	for (int n = 0; n < MIDIIN_MAX; n++)
	{
		for (int c = 0; c < MAXCHANNEL; c++)
		{
			for (int p = 0; p < MAXPITCH; p++)
				g_statuspitch[n][c][p] = false;
		}
	}
*/
}
static void midiprocess_msg(int midinr, double time, void *buffer, DWORD length)
{
	// process this new midiin mg with the midiin-LUA-thread , using the expected LUA-function midixxx()
	// Construct the short MIDI message.	
	T_midimsg u , u_Original;
	u.dwData = 0;
	switch (length)
	{
	case 0: return;
	case 1: u.bData[0] = ((BYTE*)buffer)[0];  break;
	case 2: u.bData[0] = ((BYTE*)buffer)[0];  u.bData[1] = (((BYTE*)buffer)[1]) & 0x7F; break;
	default: u.bData[0] = ((BYTE*)buffer)[0];  u.bData[1] = (((BYTE*)buffer)[1]) & 0x7F;  u.bData[2] = (((BYTE*)buffer)[2]) & 0x7F; break;
	}
	
	g_current_t = time;

	//mlog_in("midiprocess_msg receive length=%d %d %d %d (lua_gettop=%d==0!)", length, u.bData[0], u.bData[1], u.bData[2], lua_gettop(g_LUAstate));

	switch (u.bData[0])
	{
	case SYSEX:
		// sysex
		if (! g_process_Sysex)	return; // g_process_ sysex
		if ( lua_getglobal(g_LUAstate, LUAFunctionSysex) != LUA_TFUNCTION)
		{
			lua_pop(g_LUAstate, lua_gettop(g_LUAstate));
			return ;
		}
		lua_pushinteger(g_LUAstate, midinr + 1);
		lua_pushnumber(g_LUAstate, time);
		{
			char *sysexAscii = (char*)malloc(length * 6 + 1);
			sysexAscii[0] = '\0';
			char s[10];
			BYTE *ptbuf = (BYTE*)buffer;
			for (unsigned int n = 0; n < length; n++, ptbuf++)
			{
				sprintf(s, "%X ", *ptbuf);
				strcat(sysexAscii, s);
			}
			if (g_collectLog)
			{
				char s[256];
				sprintf(s, "<midiin device#%d @%f ,sysex = %s\n", midinr + 1, time, sysexAscii);
				mlog_in(s);
			}
			lua_pushstring(g_LUAstate, sysexAscii);
			if (lua_pcall(g_LUAstate, 3, 0, 0) != LUA_OK)
			{
				mlog_in("error call  LUA %s , err: %s", LUAFunctionSysex, lua_tostring(g_LUAstate, -1));
			}
			free(sysexAscii);
		}
		lua_pop(g_LUAstate, lua_gettop(g_LUAstate));
		return;
	case ACTIVESENSING:
		if (! g_process_Activesensing) return; // g_process_ active sensing messages 
		if ( lua_getglobal(g_LUAstate, LUAFunctionActive) != LUA_TFUNCTION)
		{
			lua_pop(g_LUAstate, lua_gettop(g_LUAstate));
			return ;
		}
		lua_pushinteger(g_LUAstate, midinr + 1);
		lua_pushnumber(g_LUAstate, time);
		if ( lua_pcall(g_LUAstate, 2, 0, 0) != LUA_OK )
		{
			mlog_in("error call  LUA %s , err: %s", LUAFunctionActive, lua_tostring(g_LUAstate, -1));
		}
		lua_pop(g_LUAstate, lua_gettop(g_LUAstate));
		return;
	case CLOCK:
		if (! g_process_Clock) return; // g_process_ clock messages 
		if ( lua_getglobal(g_LUAstate, LUAFunctionClock) != LUA_TFUNCTION)
		{
			lua_pop(g_LUAstate, lua_gettop(g_LUAstate));
			return ;
		}
		lua_pushinteger(g_LUAstate, midinr + 1);
		lua_pushnumber(g_LUAstate, time);
		if ( lua_pcall(g_LUAstate, 2, 0, 0)!= LUA_OK )
		{
			mlog_in("error call  LUA %s , err: %s", LUAFunctionClock, lua_tostring(g_LUAstate, -1));
		}
		lua_pop(g_LUAstate, lua_gettop(g_LUAstate));
		return;
	default: break;
	}

	u_Original.dwData = u.dwData ;
	
	BYTE type_msg = u.bData[0] >> 4;
	BYTE channel = u.bData[0] & 0x0F;

	if (g_collectLog)
	{
		if ((type_msg == PROGRAM) || (type_msg == CONTROL) || (type_msg == NOTEON) || (type_msg == NOTEOFF))
		{
			char s[256];
			sprintf(s, "<midiin device#%d channel#%d %X/%d/%d @%f", midinr + 1, channel +1, u.bData[0], u.bData[1], u.bData[2], time);
			mlog_in(s);
		}
	}

	if (g_process_Midi)
	{
		if ( lua_getglobal(g_LUAstate, LUAFunctionMidi) != LUA_TFUNCTION)
		{
			lua_pop(g_LUAstate, lua_gettop(g_LUAstate));
			return ;
		}
		lua_pushinteger(g_LUAstate, midinr + 1);
		lua_pushnumber(g_LUAstate, time);
		lua_pushinteger(g_LUAstate, type_msg);
		lua_pushinteger(g_LUAstate, channel + 1);
		lua_pushinteger(g_LUAstate, u.bData[1]);
		lua_pushinteger(g_LUAstate, u.bData[2]);
		if (lua_pcall(g_LUAstate, 6, LUA_MULTRET, 0) != LUA_OK)
		{
			mlog_in("error calling LUA %s, err: %s", LUAFunctionMidi, lua_tostring(g_LUAstate, -1));
			lua_pop(g_LUAstate, lua_gettop(g_LUAstate));
			return;
		}
		else
		{
			if (lua_gettop(g_LUAstate) == 4)
			{
				type_msg = lua_tonumber(g_LUAstate, -4);
				channel = lua_tonumber(g_LUAstate, -3) - 1;
				u.bData[0] = (type_msg << 4) + channel;
				u.bData[1] = lua_tonumber(g_LUAstate, -2);
				u.bData[2] = lua_tonumber(g_LUAstate, -1);
			}
			else
			{
				lua_pop(g_LUAstate, lua_gettop(g_LUAstate));
				return;
			}
		}
		lua_pop(g_LUAstate, lua_gettop(g_LUAstate));
	}
	if ((type_msg == NOTEON) && (u.bData[2] == 0))
	{
		type_msg = NOTEOFF;
		u.bData[0] = (NOTEOFF << 4) + channel;
	}


	bool tbd = false ;
	bool isSelected = false ;
	// mlog_in("midiin midinr=%d , type_mssg=%d , channel=%d , d1=%d , d2=%d", midinr, type_msg, channel, u.bData[1], u.bData[2]);
	switch (type_msg)
	{
	case CHANNELPRESSURE : 
		if (g_process_ChannelPressure)
		{
			tbd = true ;  
			if ( lua_getglobal(g_LUAstate, LUAFunctionChannelPressure) != LUA_TFUNCTION)
			{
				lua_pop(g_LUAstate, lua_gettop(g_LUAstate));
				return ;
			}
		}
		break;
	case KEYPRESSURE: 
		if (g_process_KeyPressure)
		{
			tbd = true ;
			if ( lua_getglobal(g_LUAstate, LUAFunctionKeyPressure) != LUA_TFUNCTION)
			{
				lua_pop(g_LUAstate, lua_gettop(g_LUAstate));
				return ;
			}
		}
		break;
	case SYSTEMCOMMON: 
		if (g_process_SystemCommon)
		{
			tbd = true ;
			if ( lua_getglobal(g_LUAstate, LUAFunctionSystemCommon) != LUA_TFUNCTION)
			{
				lua_pop(g_LUAstate, lua_gettop(g_LUAstate));
				return ;
			}
		}
		break;
	case CONTROL:
		isSelected = selectorSearch(time, midinr, channel, type_msg, u.bData[1], u.bData[2]);
		if (g_process_Control)
		{
			tbd = true ;
			if ( lua_getglobal(g_LUAstate, LUAFunctionControl) != LUA_TFUNCTION)
			{
				lua_pop(g_LUAstate, lua_gettop(g_LUAstate));
				return ;
			}
		}
		break;
	case PROGRAM:
		isSelected = selectorSearch(time, midinr, channel, type_msg, u.bData[1], 0);
		if (g_process_Program)
		{
			tbd = true ;
			if ( lua_getglobal(g_LUAstate, LUAFunctionProgram) != LUA_TFUNCTION)
			{
				lua_pop(g_LUAstate, lua_gettop(g_LUAstate));
				return ;
			}
		}
		break;
	case NOTEOFF :
		/*
		if (g_statuspitch[midinr][channel][u.bData[1]] == false)
		{
			mlog_in("double note-off %d", u.bData[1]);
			return; 
		}
		g_statuspitch[midinr][channel][u.bData[1]] = false;
		*/
		isSelected = selectorSearch(time, midinr, channel, type_msg, u.bData[1], u.bData[2]);
		if (g_process_NoteOff)
		{
			tbd = true ;
			if ( lua_getglobal(g_LUAstate, LUAFunctionNoteOff) != LUA_TFUNCTION)
			{
				lua_pop(g_LUAstate, lua_gettop(g_LUAstate));
				return ;
			}
		}
		break;
	case NOTEON:
		/*
		if (g_statuspitch[midinr][channel][u.bData[1]] == true)
		{
			mlog_in("double note-on %d", u.bData[1]);
			return; 
		}
		g_statuspitch[midinr][channel][u.bData[1]] = true;
		*/
		isSelected = selectorSearch(time, midinr, channel, type_msg, u.bData[1], u.bData[2]);
		if (g_process_NoteOn)
		{
			tbd = true ;
			if ( lua_getglobal(g_LUAstate, LUAFunctionNoteOn) != LUA_TFUNCTION)
			{
				lua_pop(g_LUAstate, lua_gettop(g_LUAstate));
				return ;
			}
		}
		break; // return note_off for note-on with velocity==0
	case PITCHBEND : 
		if (g_process_PitchBend)
		{
			tbd = true ;
			if ( lua_getglobal(g_LUAstate, LUAFunctionPitchBend) != LUA_TFUNCTION)
			{
				lua_pop(g_LUAstate, lua_gettop(g_LUAstate));
				return ;
			}
		}
		break;
	default: 
		mlog_in("unexpected MIDI msg %d", type_msg);
		return;
		break;
	}

	if ( tbd )
	{
		lua_pushinteger(g_LUAstate, midinr + 1);
		lua_pushnumber(g_LUAstate, time);
		int nbParam = 0;
		switch (type_msg)
		{
		case SYSTEMCOMMON:
			lua_pushinteger(g_LUAstate, u.bData[0]); 
			lua_pushinteger(g_LUAstate, u.bData[1]); 
			lua_pushinteger(g_LUAstate, u.bData[2]); 
			nbParam = 5;
			break;
		case CHANNELPRESSURE:
		case PROGRAM:
			lua_pushinteger(g_LUAstate, channel + 1); // channel
			lua_pushinteger(g_LUAstate, u.bData[1]); // program#
			nbParam = 4;
			break;
		case PITCHBEND:
			lua_pushinteger(g_LUAstate, channel + 1); // channel
			lua_pushinteger(g_LUAstate, pitchbend_value(u)); // pitchend value
			nbParam = 4;
			break;
		case NOTEON:
			lua_pushinteger(g_LUAstate, channel + 1); // channel
			lua_pushinteger(g_LUAstate, u.bData[1]); // pitch#
			lua_pushinteger(g_LUAstate, u.bData[2]); // velocity for note, value for control , or msb pitchbend
			nbParam = 5;
			break;
		default:
			lua_pushinteger(g_LUAstate, channel + 1 ); // channel 
			lua_pushinteger(g_LUAstate, u.bData[1]); // control#
			lua_pushinteger(g_LUAstate, u.bData[2]); // value
			nbParam = 5;
			break;
		}
		if (lua_pcall(g_LUAstate, nbParam, 0, 0) != LUA_OK)
		{
			mlog_in("error calling LUA on_midi_msg, err: %s", lua_tostring(g_LUAstate, -1));
		}
		lua_pop(g_LUAstate, lua_gettop(g_LUAstate));
	}
	g_fcallback(time, midinr, u_Original.bData[0] >> 4, u_Original.bData[0] & 0x0F, u_Original.bData[1], u_Original.bData[2], tbd | isSelected);

	lua_pop(g_LUAstate, lua_gettop(g_LUAstate));
}
void CALLBACK midinewmsg(DWORD device, double time, void *buffer, DWORD length, void *ptuser)
{
	lock_mutex_in();
//#ifdef V_VST
//  VstIntPtr intptr = (VstIntPtr)ptuser ;
//  int user = (int)intptr;
//#else
  int user = (int)(reinterpret_cast<long>(ptuser));
//#endif
	midiprocess_msg((int)user, time, buffer, length);
	unlock_mutex_in();
}
void basslua_playbackmsg(int midinr,  int type_msg, int channel, int value1, int value2)
{
	lock_mutex_in();
	char midiMsg[4];
	midiMsg[0] = ( type_msg << 4 ) + channel ;
	midiMsg[1] = value1 ;
	midiMsg[2] = value2 ;
	midiprocess_msg( midinr, g_current_t , midiMsg, 3);
	unlock_mutex_in();
}
#ifdef V_MAC
void completionSysex(MIDISysexSendRequest *request)
{
    free((BYTE*)(request->data));
    free(request);
}
#endif
static void midiclose_device(int n)
{
	assert((n>=0)&&(n<MIDIIN_MAX));

	if (g_midiopened[n] )
	{
		//out_errori("BASS_MIDI_InStop %d\n", n);
		BASS_MIDI_InStop(n);
		//out_errori("BASS_MIDI_InFree %d\n", n);
		BASS_MIDI_InFree(n);
		//out_errori("midiinclose end %d\n", n);
		g_midiopened[n] = false;
	}
}
static void midi_in_close_devices()
{
	for (int n = 0; n < MIDIIN_MAX; n++)
		midiclose_device(n);
}
static void midi_in_init_devices()
{
	for (int n = 0; n < MIDIIN_MAX; n++)
		g_midiopened[n] = false ;
}
static int midiopen_device(int nr_device)
{
	assert((nr_device>=0)&&(nr_device<MIDIIN_MAX));

	if (g_midiopened[nr_device])
		return nr_device; // already open
#ifdef V_VST
    VstIntPtr intptr = nr_device ;
    void *voidptr = (void*)intptr ;
#else
  void *voidptr = (void *)((long)(nr_device));
#endif
	if (BASS_MIDI_InInit(nr_device, (MIDIINPROC *)midinewmsg, voidptr) == TRUE)
	{
		if (BASS_MIDI_InStart(nr_device) == TRUE)
		{
			g_midiopened[nr_device] = true;
			mlog_in("Information : midiIn open device#%d : OK", nr_device + 1);
		}
		else
		{
			mlog_in("Error BASS_MIDI_InStart device#%d ; err=%d", nr_device + 1, BASS_ErrorGetCode());
			return -1;
		}
	}
	else
	{
		mlog_in("Error BASS_MIDI_InInit device#%d ; err=%d", nr_device + 1, BASS_ErrorGetCode());
		return -1;
	}
	return nr_device;
}
static void enableSelectors()
{
	if (lua_getglobal(g_LUAstate, smidiSelector) == LUA_TBOOLEAN)
	{
		g_selector_active = lua_toboolean (g_LUAstate, -1);
	}
	lua_pop(g_LUAstate, 1);
	lua_pushnil(g_LUAstate);
	lua_setglobal(g_LUAstate, smidiSelector);
}
static void midiopen_devices()
{
	if (lua_getglobal(g_LUAstate, smidiToOpen) == LUA_TTABLE)
	{
		midi_in_close_devices();
		/* table is in the stack at index 1 */
		lua_pushnil(g_LUAstate);  /* first key */
		while (lua_next(g_LUAstate, 1) != 0) 
		{
			/* uses 'key' (at index -2) and 'value' (at index -1) */
			if (lua_isinteger(g_LUAstate, -1))
			{
				int nr_device = (int)lua_tointeger(g_LUAstate, -1) - 1;
				midiopen_device(nr_device);
			}
			/* removes 'value'; keeps 'key' for next iteration */
			lua_pop(g_LUAstate, 1);
		}
	}
	lua_pop(g_LUAstate, 1);
	midi_init();
}
static bool filter_check(const char *buf)
{
	int typev = lua_getglobal(g_LUAstate, buf) ;
	lua_pop(g_LUAstate, 1);
	if (typev == LUA_TFUNCTION)
	{
		mlog_in("Information : bassLUA function %s registered", buf);
		return true;
	}
	return false;
}
static void filter_in_set()
{
	// validate the LUA functions available to process the MIDI messages

	g_process_Midi = filter_check(LUAFunctionMidi);
	g_process_Sysex = filter_check(LUAFunctionSysex);
	g_process_Activesensing = filter_check(LUAFunctionActive);
	g_process_Clock = filter_check(LUAFunctionClock);
	g_process_ChannelPressure = filter_check(LUAFunctionChannelPressure);
	g_process_KeyPressure = filter_check(LUAFunctionKeyPressure);
	g_process_Control = filter_check(LUAFunctionControl);
	g_process_SystemCommon = filter_check(LUAFunctionSystemCommon);
	g_process_Program = filter_check(LUAFunctionProgram);
	g_process_NoteOff = filter_check(LUAFunctionNoteOff);
	g_process_NoteOn = filter_check(LUAFunctionNoteOn);
	g_process_PitchBend = filter_check(LUAFunctionPitchBend);
	g_process_Timer = filter_check(LUAFunctionTimer);
	//mlog_in("filter_in_set gettop=%d %s", lua_gettop(g_LUAstate), (lua_gettop(g_LUAstate) == 0 ? "OK" : "KO"));
}
static void mutex_in_init()
{
    // create a mutex to manipulate safely the inputs ( timer , midiin, gui ) up to LUA
	if ( g_mutex_in_ok )
		return ;
	g_mutex_in_ok = true ;
#ifdef V_PC
	g_mutex_in = CreateMutex(NULL, FALSE, NULL);
	if (g_mutex_in == NULL)
	{
		mlog_in("CreateMutex : error mutex_in_init");
		g_mutex_in_ok = false ;
	}
#endif
#ifdef V_MAC
	if (pthread_mutex_init(&g_mutex_in, NULL) != 0)
	{
		mlog_in("pthread_mutex_init : error mutex_in_init");
		g_mutex_in_ok = false ;
	}
#endif
#ifdef V_LINUX
	if (pthread_mutex_init(&g_mutex_in, NULL) != 0)
	{
		mlog_in("pthread_mutex_init : error mutex_in_init");
		g_mutex_in_ok = false ;
	}
#endif
}
static void mutex_in_free()
{
	if ( ! g_mutex_in_ok )
		return;
	g_mutex_in_ok = false ;
#ifdef V_PC
	CloseHandle(g_mutex_in);
#endif
#ifdef V_MAC
	pthread_mutex_destroy(&g_mutex_in);
#endif
#ifdef V_LINUX
	pthread_mutex_destroy(&g_mutex_in);
#endif
}
static void process_in_timer()
{
	if (g_LUAstate == NULL)
		return;

	// proces the timer in luabass module
	if (lua_getglobal(g_LUAstate, moduleLuabass) == LUA_TTABLE)
	{
		//mlog_in("lua_getglobal moduleLuabass OK");
		if (lua_getfield(g_LUAstate, -1, sexternalTimer) == LUA_TFUNCTION)
		{
			//mlog_in("sexternalTimer OK");
			if (lua_pcall(g_LUAstate, 0, 0, 0) != LUA_OK) // call and pop function & parameters
			{
				mlog_in("error call LUA externalTimer err: %s", lua_tostring(g_LUAstate, -1));
				lua_pop(g_LUAstate, 1);
			}
		}
		else
		{
			lua_pop(g_LUAstate, 1); // pop sexternalTimer function
		}
	}
	else
	{
		mlog_in("lua_getglobal moduleLuabass KO");
	}
	lua_pop(g_LUAstate, 1); // pop table moduleLuabass

	// process the timer in LUA script
	if (g_process_Timer && g_LUAstate )
	{
		if ( lua_getglobal(g_LUAstate, LUAFunctionTimer) != LUA_TFUNCTION)
		{
			lua_pop(g_LUAstate, 1);
			return;
		}
		lua_pushnumber(g_LUAstate, g_current_t);
		g_current_t = g_current_t + (float)(g_timer_in_dt) / 1000.0;
		if (lua_pcall(g_LUAstate, 1, 0, 0) != LUA_OK)
		{
			mlog_in("error calling LUA %s, err: %s", LUAFunctionTimer, lua_tostring(g_LUAstate, -1));
			lua_pop(g_LUAstate, 1);
		}
	}

	// check if there is any new midiin device to open
	if (g_countmidiin > 100)
	{
		midiopen_devices(); // check if there is any new midiin device to open
		g_countmidiin = 0;
		enableSelectors(); // check if the selectors are valid or not
	}
	g_countmidiin++;

}
#ifdef V_PC
VOID CALLBACK timer_in_callback(PVOID lpluaparam, BOOLEAN TimerOrWaitFired)
{
	if ( try_lock_mutex_in())
	{
		process_in_timer();
		unlock_mutex_in();
	}	
}
#endif
#ifdef V_MAC
void timer_in_callback(CFRunLoopTimerRef timer, void *info)
{
	if ( try_lock_mutex_in())
	{
		process_in_timer();
		unlock_mutex_in();
	}	
}
void *loop_in_run(void *void_ptr)
{
	int n;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,&n) ;
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,&n);
	__CFRunLoopTimer *mLoopTimer = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent() , (float)(g_timer_in_dt) / 1000.0, 0, 0, timer_in_callback, NULL);
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
static void timer_in_callback(int sig, siginfo_t *si, void *uc)
{
	if ( try_lock_mutex_in())
	{
		process_in_timer();
		unlock_mutex_in();
	}	
}
void *loop_in_run(void *void_ptr)
{
	struct sigaction msigaction;
    msigaction.sa_flags = SA_SIGINFO | SA_RESTART ;
    msigaction.sa_sigaction = timer_in_callback;
    sigemptyset(&(msigaction.sa_mask));
    if (sigaction(MTIMERSIGNALIN, &msigaction, NULL) == -1)
	{
		mlog_in("timer_in_init : sigaction error=%d",errno);
		g_timer_in_ok = false ;
	}
	else
	{
	 	struct sigevent msigevent;
		msigevent.sigev_notify = SIGEV_SIGNAL;
		msigevent.sigev_signo = MTIMERSIGNALIN;
		msigevent.sigev_value.sival_ptr = &g_timer_in_id;
		if (timer_create(CLOCK_REALTIME, &msigevent, &g_timer_in_id) == -1)
		{
			mlog_in("timer_in_init : timer_create error=%d",errno);
		    g_timer_in_ok = false ;
		}
		else
		{
			 //mlog_in("debug timer_in_init : timer_create OK");
	 		 struct itimerspec mitimerspec;
			 mitimerspec.it_value.tv_sec =  g_timer_in_dt / 1000;
			 mitimerspec.it_value.tv_nsec =  (g_timer_in_dt % 1000 )* 1000000;
			 mitimerspec.it_interval.tv_sec = mitimerspec.it_value.tv_sec;
			 mitimerspec.it_interval.tv_nsec = mitimerspec.it_value.tv_nsec;
			 if (timer_settime(g_timer_in_id, 0, &mitimerspec, NULL) == -1)
			 {
				mlog_in("timer_in_init : timer_settime error=%d",errno);
				g_timer_in_ok = false ;
			 }
			 else
			 	mlog_in("Information : timer_in_init.timer_settime OK");
	   }
	}
	return NULL ;
}
#endif
static void timer_in_init(bool externalTimer , int timerDt)
{
	g_timer_in_dt = timerDt ;

	if ( externalTimer )
	{
		g_timer_in_ok = false ;
		return ;
	}

	// create a periodic timer to flush-out the queud midiout msg
	if ( g_timer_in_ok )
		return ;
	g_timer_in_ok = true ;

#ifdef V_PC
	if (! CreateTimerQueueTimer(&g_timer_in, NULL, timer_in_callback, 0, g_timer_in_dt, g_timer_in_dt, 0))
	{
		mlog_in("timer_in_init : error");
		g_timer_in_ok = false ;
	}
#endif
#ifdef V_MAC
	if ( pthread_create(&g_loop_in_run_thread, NULL, loop_in_run, NULL)) 
	{
		mlog_in("timer_in_init : pthread_create error");
		g_timer_in_ok = false ;
	}
#endif
#ifdef V_LINUX
	if ( pthread_create(&g_loop_in_run_thread, NULL, loop_in_run, NULL)) 
	{
		mlog_in("timer_in_init : pthread_create error");
		g_timer_in_ok = false ;
	}
#endif
}
static void timer_in_free()
{
	if ( ! g_timer_in_ok )
		return ;
	g_timer_in_ok = false ;
#ifdef V_PC
    DeleteTimerQueueTimer(NULL, g_timer_in, NULL);
#endif
#ifdef V_MAC
	pthread_cancel(g_loop_in_run_thread);
#endif
#ifdef V_LINUX
	timer_delete(g_timer_in_id);
	pthread_cancel(g_loop_in_run_thread);
#endif
}
static void init_in(bool externalTimer , int timerDt)
{
	pitch_init();
	// mlog_in("debug init_in OK : pitch_init");
	midi_in_init_devices();
	// mlog_in("debug init_in OK : midi_in_init_devices");
	initSelector();
	// mlog_in("debug init_in OK : initSelector");
	srand((unsigned)time(NULL));
	filter_in_set();
	// mlog_in("debug init_in OK : filter_in_set");
	mutex_in_init();
	// mlog_in("debug init_in OK : mutex_in_init");
	timer_in_init( externalTimer ,  timerDt);
	// mlog_in("debug init_in OK : timer_in_init");
}
static void free()
{
	timer_in_free();
	mutex_in_free();
	midi_in_close_devices();
}

void basslua_setSelector(int i, int luaNrAction, char op, int nrDevice, int nrChannel, const char *type_msg, const int *pitch, int nbPitch, bool stopOnMatch , const char* param)
{
	lock_mutex_in();
	if (luaNrAction < 0)
	{
		g_selectormax = i;
	}
	else
	{
		T_selector *t = &(g_selectors[i]);
		t->luaNrAction = luaNrAction;
		t->op = op;
		t->nrDevice = nrDevice;
		t->nrChannel = nrChannel;
		if (strcmp(type_msg, snoteonoff) == 0)
			t->type_msg = NOTEONOFF;
		if (strcmp(type_msg, snoteononly) == 0)
			t->type_msg = NOTEON;
		if (strcmp(type_msg, scontrol) == 0)
			t->type_msg = CONTROL;
		if (strcmp(type_msg, sprogram) == 0)
			t->type_msg = PROGRAM;
		for (int j = 0; j < nbPitch; j++)
			t->pitch[j] = pitch[j];
		t->nbPitch = nbPitch;
		t->stopOnMatch = stopOnMatch;
		strcpy(t->param, param);
		if (i >= g_selectormax)
			g_selectormax = i + 1;
	}
	unlock_mutex_in();
}
void basslua_setMode(int mode)
{
	lock_mutex_in();
	actionMode =  mode ;
	unlock_mutex_in();
}
bool basslua_selectorSearch(int nrDevice, int nrChannel, int type_msg, int p, int v)
{
	lock_mutex_in();
	bool ret = selectorSearch(g_current_t, nrDevice, nrChannel, type_msg, p, v);
	unlock_mutex_in();
	return ret;
}
bool basslua_selectorTrigger(int nrSelector, int nrDevice, int nrChannel, int type_msg, int p, int v)
{
	lock_mutex_in();
	bool ret = selectorTrigger(nrSelector , g_current_t, nrDevice, nrChannel, type_msg, p, v);
	unlock_mutex_in();
	return ret;
}
int basslua_table(const char *module, const char *table, const int index, const char* field, char*svalue, int *ivalue, int action)
{
	lock_mutex_in();
	int retCode = action_table(module, table, index, field, svalue, ivalue, action);
	unlock_mutex_in();
	return (retCode);

}

bool basslua_getLog(char *buf)
{
	lock_mutex_in();
	bool retCode = false;
	if (buf == NULL)
	{
		char bufnull[1024];
		bool r;
		basslua_call(moduleLuabass, soutGetLog, "i>bs", 0,&r,bufnull);
		g_collectLog = false;
	}
	else
	{
		g_collectLog = true;
		if (nrOutBufLog != nrInBufLog)
		{
			strcpy(buf, bufLog[nrOutBufLog]);
			retCode = true;
			nrOutBufLog++;
			if (nrOutBufLog >= MAXNBLOGOUT)
				nrOutBufLog = 0;
		}
		else
		{
			basslua_call(moduleLuabass, soutGetLog, "i>bs", 1, &retCode, buf);
		}
	}
	unlock_mutex_in();
	return (retCode);
}
bool basslua_openMidiIn(int *nrDevices, int nbDevices)
{
	lock_mutex_in();
	bool retCode = true;
	midi_in_close_devices();
	//mlog_in("basslua / basslua_openMidiIn , nbDevices=%d",nbDevices);
	for (int n = 0; n < nbDevices; n++)
	{
		if (midiopen_device(nrDevices[n]) == false)
			retCode = false;
	}
	unlock_mutex_in();
	return (retCode);
}
void basslua_external_timer()
{
	if ( try_lock_mutex_in())
	{
		process_in_timer();
		unlock_mutex_in();
	}	
}

/**
* \fn void basslua_open()
* \brief Open the dedicated midiin-LUA-thread to process midiin msg.
* This function must be called at the beginning, by any external C Module.
* It starts an LUA thread, and call the LUA function onStart(param).
* \param fluaname : LUA script file to launch. This LUA script must have some predefined functions
* \param luaparam : parameter given to the LUA function onStart
* \return -1 if mlog_in. 0 if no mlog_in.
**/
bool basslua_open(const char* fluaname, const char* luaparam, bool reset, long datefluaname, voidcallback ifcallback , const char *logpath, const char *luapath, bool externalTimer , int timerDt )
{
	if (( reset == false ) && (strcmp(luaparam, pluaparam) == 0) && (strcmp(fluaname, pfluaname) == 0)&& (datefluaname == pluadatefname))
		return true ;
	strcpy(pluaparam, luaparam);
	strcpy(pfluaname, fluaname);
	pluadatefname = datefluaname;

	if (ifcallback == NULL)
		g_fcallback = null_fcallback;
	else
		g_fcallback = ifcallback;

	basslua_close();
	
	log_in_init(logpath);
	//mlog_in("debug basslua_open OK : log_in_init(%s)",logpath);

	// open the dedicated midiin-LUA-thread to process midiin msg
	g_LUAstate = luaL_newstate(); // newthread 
	luaL_openlibs(g_LUAstate);
	mlog_in("debug basslua_open OK : luaL_openlibs getop=%d==0", lua_gettop(g_LUAstate));

 	
	lua_getglobal(g_LUAstate, "package"); // to modify the PATH
	//lua_getfield(g_LUAstate, -1, "path"); // get field "path" from table at top of stack (-1)
	//char cur_path[2056];
	//sprintf(cur_path, "%s;%s?.lua", lua_tostring(g_LUAstate, -1), ressourdir);
	//mlog_in("curpath <%s> <%s> <%s> <%s>",cur_path,lua_tostring(g_LUAstate, -1),ressourdir,fusername);
	//lua_pop(g_LUAstate, 1); // get rid of the string on the stack we just pushed on line 5
	lua_pushstring(g_LUAstate, luapath); // push the new LUA path
	lua_setfield(g_LUAstate, -2, "path"); // set the field "path" in table at -2 with value at top of stack
	lua_pop(g_LUAstate, 1); // get rid of package table from top of stack

	if (luaL_loadfile(g_LUAstate, fluaname) != LUA_OK)
	{
		mlog_in("Error : basslua_open lua_loadfile %s : <%s>", fluaname, lua_tostring(g_LUAstate, -1));
		lua_pop(g_LUAstate, 1);
		return(false);
	}
	mlog_in("debug basslua_open OK : lua_loadfile <%s>  getop=%d==1",fluaname, lua_gettop(g_LUAstate));


#ifdef V_PC
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (!SUCCEEDED(hr))
	{
		mlog_in("Error : CoInitializeEx fails");
	}
#endif

	// require the "chord" module for chord interpretation
	lua_getglobal(g_LUAstate, "require");
	lua_pushstring(g_LUAstate, moduleChord);
	if (lua_pcall(g_LUAstate, 1, 1, 0) != LUA_OK)
	{
		mlog_in("Error basslua_open require %s <%s>", moduleChord, lua_tostring(g_LUAstate, -1));
		lua_pop(g_LUAstate, 1);
		return  false;
	}
	mlog_in("debug basslua_open OK : require <%s>",moduleChord);

	if (!lua_istable(g_LUAstate, -1))
	{
		mlog_in("Error basslua_open require %s : not a table", moduleChord);
		lua_pop(g_LUAstate, 1);
		return false;
	}
	lua_setglobal(g_LUAstate, moduleChord);
	mlog_in("debug basslua_open OK : lua_setglobal <%s>",moduleChord);

	// require the "score" module for score interpretation
	lua_getglobal(g_LUAstate, "require");
	lua_pushstring(g_LUAstate, moduleScore);
	if (lua_pcall(g_LUAstate, 1, 1, 0) != LUA_OK)
	{
		mlog_in("Error basslua_open require %s <%s>", moduleScore, lua_tostring(g_LUAstate, -1));
		lua_pop(g_LUAstate, 1);
		return  false;
	}
	mlog_in("debug basslua_open OK : require <%s>",moduleScore);

	if (!lua_istable(g_LUAstate, -1))
	{
		mlog_in("Error basslua_open require %s : not a table", moduleScore);
		lua_pop(g_LUAstate, 1);
		return false;
	}
	lua_setglobal(g_LUAstate, moduleScore);
	mlog_in("debug basslua_open OK : lua_setglobal <%s> gettop=%d==1",moduleScore, lua_gettop(g_LUAstate));

	// require the "luabass" module for Midi-out
	lua_getglobal(g_LUAstate, "require");
	lua_pushstring(g_LUAstate, moduleLuabass);
	if ( lua_pcall(g_LUAstate, 1, 1,0) != LUA_OK )
	{
		mlog_in("Error : basslua_open error require %s <%s>", moduleLuabass, lua_tostring(g_LUAstate, -1));
		lua_pop(g_LUAstate, 1);
		return false;
	}
	mlog_in("debug basslua_open OK : require <%s> getop=%d==2",moduleLuabass, lua_gettop(g_LUAstate));

	if (! lua_istable(g_LUAstate, -1))
	{
		mlog_in("Error basslua_open require %s : not a table", moduleLuabass);
		lua_pop(g_LUAstate, 1);
		return false;
	}
	lua_setglobal(g_LUAstate, moduleLuabass);
	mlog_in("debug basslua_open OK : lua_setglobal <%s> getop=%d==1",moduleLuabass, lua_gettop(g_LUAstate));
	

	// create the "info" table to receive instructions
	lua_newtable(g_LUAstate);
	lua_setglobal(g_LUAstate, tableInfo);
	//mlog_in("debug basslua_open OK : lua_setglobal <%s>",tableInfo);

	// run the script
	if (lua_pcall(g_LUAstate, 0, 0, 0) != LUA_OK)
	{
		mlog_in("Error basslua_open error lua_pcall <%s>", lua_tostring(g_LUAstate, -1));
		lua_pop(g_LUAstate, 1);
		return false;
	}
	mlog_in("debug basslua_open OK : lua_pcall run script <%s>",fluaname);

	// init the luabass module
	if (!basslua_call(moduleLuabass, sinit, "sii", (logpath == NULL) ? "" : logpath, true, timerDt))
	{
		// ask for the luabass.init out-process : init the module 
		mlog_in("Error basslua_open error : luabass.init()");
		return false;
	}
	mlog_in("debug basslua_open OK : luabass.init()");

	// init the lua script
	if (!basslua_call(moduleGlobal, sonStart, "s", luaparam))
	{
		mlog_in("Error basslua_open error : luabass.onStart()");
		return false;
	}
	mlog_in("debug basslua_open OK : %s %s",fluaname,sonStart);

	// init the static variable of this dll
	init_in(externalTimer, timerDt);
	mlog_in("debug basslua_open OK : init");

	mlog_in("basslua_open OK (gettop=%d %s)",lua_gettop(g_LUAstate), (lua_gettop(g_LUAstate) == 0 ? "OK" : "KO"));
	return (true);
}
/**
* \fn void basslua_close()
* \brief close the dedicated IN-LUA-thread which processes events from midiin, in-timer and gui.
* This function must be called at the end, by any external C Module.
* It call the LUA function onStop(), and stops the LUA thread,.
**/
void basslua_close()
{
#ifdef V_PC
	CoUninitialize();
#endif

	if (g_LUAstate)
	{
		//mlog_in("debug basslua_close");

		if ( ! basslua_call(moduleLuabass, soutAllNoteOff, "s", "a") )
			mlog_in("debug basslua_close moduleLuabass.%s(a) : error",soutAllNoteOff);
		//mlog_in("debug basslua_close moduleLuabass.%s(a) : OK",soutAllNoteOff);

		basslua_call(moduleGlobal, sonStop, "");

		if ( ! basslua_call(moduleLuabass, sfree, ""))
			mlog_in("debug basslua_close moduleLuabass.%s : error",sfree);
		//mlog_in("debug basslua_close moduleLuabass.%s : OK",sfree);

		free();

		//lua_close(g_LUAstate);
		//mlog_in("debug basslua_close free : OK");
	}
	g_LUAstate = 0;

	char fmlog[1024];
	strcpy(fmlog,g_path_in_error_txt);
	strcat(fmlog,".mlog");
#ifdef V_MLOG
	mlogflush(fmlog);
#endif
}
