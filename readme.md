#Expresseur project

documentation , presentation :
www.expresseur.com

1. basslua
    This library can be used by a GUI, to manage MIDI inputs and LUA scripting (to invent your own MIDI logic).
    C library 64 bits. Started by the GUI, which load :
    * main lua script to catch the midi-event
    * luachord.lua
    * luascore.lua
    * luabass
    
    dependencies :
    * lua5.3 64 bits : www.lua.org
    * bass 64 bits : www.un4see.com 
    * winmm/pc, coremidi/Mac
    
2. luabass :

    This library can be used to manage audio output (MIDI, VST, SF2, wav, DMX) from a LUA script.
    C library 64 bits. Started by lua script, to render midi-out, SF2, VSTi, Wav
 
    dependencies :
    * lua5.3 64 bits : www.lua.org
    * bass, bassmix, bassmidi ( bassasio/PC ) 64 bits: www.un4seen.com
    * winmm/pc, coremidi/Mac
    * vsti sdk (include only, dynamic VSTi-dll loaded dynamically on demand)

3. expresscmd :

    This command-line can be used to launch basslua and luabass, and to interact with the LUA script.
   C exe command-line 64 bits. Starts basslua

   dependencies :
   * basslua 64 bits
   
4. expresseur
   
   This graphical User Interfacepresents two ways to play : improvisation and score-playing.
   C++ wxwidgets Graphic User Iinterface 64 bits. Starts basslua
 
   dependencies
   * wxwidgets3.1 64 bits
   * basslua 64 bits
   * lilypond for the score rendering
          
5. mac

   build wxwidgets 64 bits ( used by expresseur GUI ) : 
   * cd Documents
   * git clone --recurse-submodules https://github.com/wxWidgets/wxWidgets.git
   * cd wxWidgets
   * mkdir build64
   * cd build64
   * ../configure --with-macosx-version-min=10.9 --disable-shared --disable-sys-libs  --prefix="$(pwd)"
   * make
   
   build lua5.3.3 64 bits ( used by basslua & luabass )
   * cd $root_dir/../mac ( $root_dir is the platform where the git project is downloaded )
   * curl -O http://www.lua.org/ftp/lua-5.3.3.tar.gz
   * tar xf lua-5.3.3.tar.gz
   * rm lua-5.3.3.tar.gz
   * modification de src/Makefile : ligne macosx:
   * $(MAKE) $(ALL) SYSCFLAGS="-DLUA_USE_MACOSX -mmacosx-version-min=10.9 " SYSLIBS="-lreadline" CC=cc
   * make macosx
   
   rtmidi :
   https://github.com/thestk/rtmidi.git
   
   sound & Midi 64 bits libraries from www.un4seen.org : bass , bassmix, bassmidi
   
   GNUmakefile builds the app package, with include & directories in mac directory
   * make -f GNUmakefile
   copy Expresseur.app ni Applications folder
   within Expresseur.app/Contents/MacOS : command-line "expresscmd" is available

6. pc

   status : Solution 64 bits solution with Visual Studio available and operationnal. GNU Makefile not available.

7. Linux Ubuntu / Debian

   build wxwidgets 64 bits ( used by expresseur GUI ) : 
   * sudo apt-get install gtk2.0
   * git clone --recurse-submodules https://github.com/wxWidgets/wxWidgets.git
   * cd wxwidgets
   * mkdir buildgtk
   * cd buildgtk
   * ../configure --with-gtk
   * make
   * sudo make install
   * sudo ldconfig

   build lua 64 bits ( used by expresseur GUI ) : 
   * sudo apt-get install libreadline6 libreadline6-dev
   * ou debian : sudo apt-get install libreadline7 libreadline-dev
   * cd linux/lua-5.3.4
   * make linux
   alsa dev package
   * sudo apt-get install libasound2-dev
   
   rtmidi under linux/rtmidi from : https://github.com/thestk/rtmidi.git
   
   sound/midi 64 bits lib under linux/bass .h and .so , from www.un4seen.org : bass , bassmix, bassmidi
   
   GNUmakefile builds the epresseur package 
   * make -f GNUmakefile
   cd expresseur/Expresseur.app/Contents/Linux : 
   * ./expresseur for the GUI, 
   * or ./expresscmd for the command-line
   * to create a Gnome icone : adapt path in expresseurv3.desktop and copy to /usr/share/applications
   
  
