LUABASS is a library wich manages the MIDI-Outputs and the AUDIO-outputs.  
It is used as a LUA module. 

BASSLUA automtically loads the LUABASS module when it starts the main LUA script.
    
It is written in C, and compiled for 64 bits (Mac, Windows, Linux).

Dependencies :
  * bass 64 bits : www.un4see.com. To manage the Midi-outputs, SF2, VSTi
  * winmm/pc, coremidi/Mac . To manage the audio-MIDI BIOS of teh computer (Windows, Mac, Linux)
