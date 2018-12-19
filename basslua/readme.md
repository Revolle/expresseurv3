BASSLUA is a library wich manages the MIDI-Inputs and the LUA scipts.  
That's the only "entry-point" for the "main software" :
  * Graphic User Interface like expresseur
  * Command-line like expresscmd
  * your own invention to create your own MIDI logic
    
It is written in C, and compiled for 64 bits (Mac, Windows, Linux).

When started by the mai software, BassLua loads :
  * main lua script : to catch the midi-event, and create a logic (e.g.eexpresseur.lua, expresscmd.lua .. )
  * luachord.lua : LUA script to compile-improvise a text file containing chord-names
  * luascore.lua : LUA script to compile-interpret a pre-compiled score 
  * luabass : library to manage MIDI-outputs and Sound-outputs

Dependencies :
  * lua5.3 64 bits : www.lua.org. To enable the LUA scripting-running
  * bass 64 bits : www.un4see.com. To manage the Midi-inputs
  * winmm/pc, coremidi/Mac . To manage the audio-MIDI BIOS of teh computer (Windows, Mac, Linux)
