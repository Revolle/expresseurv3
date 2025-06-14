// expresscmd.cpp 
// update : 31 / 10 / 2016 10 : 00
// console app.
//
// Simple usage of basslua ans luabass module.
//
// Load a LUA script file through the bassua. 
// This LUA script processes :
//     - MIDI inputs
//     - user's commands from this app
//     - generates MIDI out accordiing to its logic
//

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


#if defined(V_LINUX) || defined(V_MAC)
#define strcpy_s strcpy
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <conio.h>
#include <direct.h>

#include "basslua.h"

#define sUsage "type help for list of functions available\n"

int main(int argc, char* argv[])
{

	// define the default lua-script and its empty parameter
	char fname[1024] = "expresscmd.lua";
	char param[1024] = "";
	char line_read[1024] ;

	// use arguments of the command-line to change the lua-scipt and its parameters
	if (argc > 1)
		strcpy_s(fname, argv[1]);
	if (argc > 2)
		strcpy_s(param, argv[2]);
	if (argc > 3)
		_chdir(argv[3]);
	// debug :
	// _chdir("C:\\Users\\franc\\Documents\\GitHub\\expresseurV3_VC\\basslua\\x64\\Debug");
	char full[_MAX_PATH];
	if (_fullpath(full, ".\\", _MAX_PATH) == NULL)
		fprintf(stderr, "cannot calculate working directory\n");

	// starts the basslua module with the lua-scipt
	// this command loads :
	//    - the basslua ( with midi-in management )
	//    - the lua-script ( with its music-logic according to midi-inputs).
	//        The lua-script loads these modules  by default :
	//          - luabass ( for midi-output )
	//          - luascore ( to play a score )
	//          - luachord ( to play chords )
	//        The lua-scriptstarts the lua-function onStart(parameters) :
	fprintf(stderr,"basslua_open argv[1].scriptlua=<%s> argv[2].param=<%s> (in argv[3].working_directory=%s)\n",fname, param , full);
	bool retCode = basslua_open(fname, param, true, 0, NULL ,"expresscmd_log","./?.lua", false/*external timer*/,20);
	fprintf(stderr,"Return code basslua_open =%s\n", retCode?"OK":"Error");

	// print the usage of this command-line tool
	fprintf(stderr,sUsage);

	// read the input
	while (true)
	{
		char *ret_fgets = fgets(line_read , 1000 , stdin );
		if (( ret_fgets != NULL ) && (strlen(line_read) > 0))
		{
			line_read[strlen(line_read) - 1] = '\0' ;
			// read user's command, and forward it to the LUA script through the basslua module
			if ((strcmp(line_read, "exit") == 0) || (strcmp(line_read, "quit") == 0) || (strcmp(line_read, "close") == 0) || (strcmp(line_read, "end") == 0))
				break;
			if ((strcmp(line_read, "?") == 0) || (strcmp(line_read, "help") == 0))
			{
				printf(sUsage);
			}
			char *pt[20];
			int nb;
			pt[0] = strtok(line_read, " ");
			if (pt[0] != NULL)
			{
				nb = 1;
				while ((pt[nb] = strtok(NULL, " ")) != NULL) nb ++;
				char *f[2];
				f[0]=NULL;
				f[1]=NULL;
				f[0] = strtok(pt[0], ".");
				if ( f[0] != NULL)
					f[1] = strtok(NULL, ".");
				char module[256];
				char function[256];
				if (f[1] == NULL)
				{
					strcpy(module, "_G");
					strcpy(function, pt[0]);
				}
				else
				{
					strcpy(module, f[0]);
					strcpy(function, f[1]);
				}
				bool ret_code = false;
				switch (nb)
				{
				case 1: ret_code = basslua_call(module, function, ""); break;
				case 2: ret_code = basslua_call(module, function, "s", pt[1]); break;
				case 3: ret_code = basslua_call(module, function, "ss", pt[1], pt[2]); break;
				case 4: ret_code = basslua_call(module, function, "sss", pt[1], pt[2], pt[3]); break;
				case 5: ret_code = basslua_call(module, function, "ssss", pt[1], pt[2], pt[3], pt[4]); break;
				default: printf("too many arguments\n");
				}
				if (ret_code )
					printf(">Done\n");
				else
					printf(">Error command <%s>\n", line_read);
			}
		}
	}

	basslua_close();

	return 0;
}

