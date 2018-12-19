EXPRESSCMD is a small command-line C software.  
It can be used to start a LUA script which contains your own MIDI logic. 
There are two optional parameters :
 * the LUA script to be loaded by basslua (default : expresscmd.lua ). This script contains your MIDI logic
 * the parameter to initailize the LUA script, using function onStart. Default : --preopen_midiout

It is written in C, and compiled for 64 bits (Mac, Windows, Linux).

EXPRESSCMD loads basslua library, which loads :
  * main lua script : to catch the midi-event, and create a logic (e.g. expresscmd.lua .. )
  * luachord.lua : LUA script to compile-improvise a text file containing chord-names
  * luascore.lua : LUA script to compile-interpret a pre-compiled score 
  * luabass : library to manage MIDI-outputs and Sound-outputs

After the basslua loading, it proposes a command line interface.  
The command accepted are the LUA functions available in the script.  

Dependencies :
  * basslua library
