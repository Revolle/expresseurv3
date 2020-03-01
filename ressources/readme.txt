This directory contains the Virtual Instruments ( VI ).

- SF2 : Sound-Fonts file, with ".sf2" extension
- VSTi : Virtual Instrument, with ".dll" extension. 
   The Graphic User Interface is not displayed. 
   Expresseur builds list of programs with Midi-Programs if available, else with VST-Programs.

A lot of SF2 or VSTi are available on Internet ( free or not ... ). 
Example : VSTi Pianoteq for piano, vibra, harpsichord, steel-drum, an more...

The "luabass" library can also play sound-file ( .wav extension ) with multi-thread capacity.
The audio file "test.wav" is used to test audio in the menu "setting/audio".

Expresseur builds the list of instruments available in the VI.
This list is displayed in the Expresseur-mixer, when the VI is selected.
This list has the same name than the VI, but with suffixe ".txt".
This list can be changed manually ( with more or less instruments ):
- one line per instrument
- syntax example "piano_Left(P1,C10/1)" . Detail : 
     - a name without space, followed by "tuning" betwen paranthesis
     - "tuning" is a :
          - a program with prefix "P" :
                - simple one is the pogram number [0..127] eg "P32"
                - with bank MSB [0..127] eg "P10/32"
                - with banl MSB and LSB [0..127] eg "P10/2/32"
          - a control with prefix "C" :
                - the control number and it value [0..127] eg C10/1 to set the pan on right

For new tracks, Expresseur uses by default the VI with prefix "default_"

The VI is plugged by default on the default sound-ouput set in the menu "settings/audio".
To plug the VI on another sound-output, at the end of the name of the VI, add the name of the audio-output with "@" prefix .
The list of audio-output is visible in the menu "settings/audio".
Example : "guitar@SD-50.sf2" connects this sf2 on the "SD-50" audio output.

Expresseur can also use instruments from MIDI-sound-expanders :
- software ( eg Pianoteq standalone ). Use virtual Midi cable ( eg loopBE ) to connect it.
- hardware. Use MIDI-cable or USB to connect it.
     A MIDI-Sound-expander guarantees low-latency and audio-quality, even on small PC.
     The name of the file with the list of instruments must be the same than the name of the MIDI-driver.
     Example of MIDI-sound-expander hardware : Miditech-pianobox, SD-50 Roland, any electronic-piano...
     "gm.txt" is a standard list of instruments for MIDI-sound-explander compliant with General-MIDI.

This directory can contains a script "expresseur.lua", to embend personal LUA features (modifying the original one from the setup).

