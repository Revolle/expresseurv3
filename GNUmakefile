# Generate expresseur/Expresseur.app
# platform MAC Darwin
#
EXPRESSEUR_APP := expresseur/Expresseur.app

PLATFORM := $(shell uname)

CPPFLAGS := -Wall -MD -MP
CPPFLAGS := $(CPPFLAGS) -g -Wno-deprecated-declarations -Wno-unused-variable
CXXFLAGS := -std=c++11 
LDFLAGS  :=

LILYPOND := lilypond-2.26.0
LILYPOND_X86_64_TAR := ../lilypond/$(LILYPOND)-darwin-x86_64.tar
LILYPOND_ARM64_TAR := ../lilypond/$(LILYPOND)-darwin-arm64.tar
	  
WX_CONFIG =/Users/revolle/wxWidgets/build64/wx-config

CXXFLAGS := $(CXXFLAGS) -mmacosx-version-min=10.15 -D__MACOSX_CORE__
LDFLAGS := $(LDFLAGS) -mmacosx-version-min=10.15

# dynamic library for basslua ( called by GUI expresseur )
DYNLIB_EXT := dylib
DYNLIB_LDFLAGS := $(LDFLAGS) -dynamiclib

# lua C module for luabass , loaded by lua
SHAREDLIB_EXT := so
SHAREDLIB_LDFLAGS := $(LDFLAGS) -undefined dynamic_lookup 

PLATFORM_DIR := osx

LUA_DIR := ../lua/src
RTMIDI_DIR := ../rtmidi

BASS_DIR := ../bass/$(PLATFORM_DIR)

WXWIDGEST_LIB = $(shell $(WX_CONFIG) --libs base,net,core,adv,xml)
EXPRESSCMD_LIBS := 
EXPRESSEUR_LIBS := $(WXWIDGEST_LIB) $(EXPRESSCMD_LIBS) 
TEST_LIBS := $(WXWIDGEST_LIB) 

EXPRESSEUR_CONTENT := $(EXPRESSEUR_APP)/Contents/MacOS
EXPRESSEUR_RESOURCES := $(EXPRESSEUR_APP)/Contents/Resources

EXPRESSEUR := expresseur/expresseur
EXPRESSEUR_OBJECTS := expresseur/bitmapscore.o expresseur/editshortcut.o expresseur/emptyscore.o \
						expresseur/expresseur.o	expresseur/expression.o expresseur/logError.o \
						expresseur/luafile.o expresseur/midishortcut.o expresseur/mixer.o \
						expresseur/musicxml.o expresseur/musicxmlcompile.o expresseur/musicxmlscore.o \
						expresseur/mxconf.o expresseur/textscore.o expresseur/viewerscore.o 
$(EXPRESSEUR_OBJECTS): CPPFLAGS += $(shell $(WX_CONFIG) --cxxflags)
#$(info	EXPRESSEUR_OBJECTS = $(EXPRESSEUR_OBJECTS) )

EXPRESSCMD := expresscmd/expresscmd
EXPRESSCMD_EXE := expresscmd/expresscmd.exe
EXPRESSCMD_OBJECTS := expresscmd/expresscmd.o

SERIALPORT_DIR := ../libserialport

LIBBASSLUA := basslua/libbasslua.$(DYNLIB_EXT)
LIBBASSLUA_OBJECTS := basslua/basslua.o
BASSLUA_LIBS := -L$(LUA_DIR) -llua  -L$(BASS_DIR) -lbass -lbassmidi -lbassmix -framework CoreFoundation -framework CoreAudio -framework CoreMIDI

LIBLUABASS := luabass/luabass.$(SHAREDLIB_EXT)
LIBLUABASS_OBJECTS := luabass/luabass.o $(RTMIDI_DIR)/RtMidi.o $(SERIALPORT_DIR)/serialport.o $(SERIALPORT_DIR)/timing.o $(SERIALPORT_DIR)/macosx.o
LUABASS_LIBS :=  -L$(LUA_DIR) -llua  -L$(BASS_DIR) -lbass -lbassmidi -L$(SERIALPORT_DIR) -lserialport -framework CoreFoundation -framework CoreAudio -framework CoreMIDI

CPPFLAGS += -Ibasslua -Iluabass -Iexpresseur -I$(BASS_DIR) -I$(LUA_DIR) -I$(RTMIDI_DIR) -I$(SERIALPORT_DIR)
        
all: $(LIBBASSLUA) $(LIBLUABASS) $(EXPRESSCMD_EXE) $(EXPRESSEUR_APP)

serialport: $(SERIALPORT)
luabass: $(LIBLUABASS)
basslua: $(LIBBASSLUA)
expresscmd: $(EXPRESSCMD)
expresseur: $(EXPRESSEUR)
expresseurapp: $(EXPRESSEUR_APP)
expresscmdapp: $(EXPRESSCMD_EXE)


$(EXPRESSEUR_APP): $(EXPRESSEUR)
	-mkdir -p $(EXPRESSEUR_RESOURCES)
	-mkdir -p $(EXPRESSEUR_CONTENT)
	-mkdir -p $(EXPRESSEUR_CONTENT)/example
	-mkdir -p $(EXPRESSEUR_CONTENT)/ressources
	cp expresseur/AppIcon.icns $(EXPRESSEUR_RESOURCES)
	cp expresseur/Info.plist $@/Contents
	cp $(EXPRESSEUR) $(EXPRESSEUR_CONTENT)
	cp $(LIBLUABASS) $(EXPRESSEUR_CONTENT)
	cp $(LIBBASSLUA) $(EXPRESSEUR_CONTENT)
	cp $(BASS_DIR)/*.dylib $(EXPRESSEUR_CONTENT)
	cp $(LUA_DIR)/liblua.a $(EXPRESSEUR_CONTENT)
	-mkdir -p $(EXPRESSEUR_CONTENT)/arm64
	tar -xf $(LILYPOND_ARM64_TAR) -C $(EXPRESSEUR_CONTENT)/arm64/
	mv $(EXPRESSEUR_CONTENT)/arm64/$(LILYPOND) $(EXPRESSEUR_CONTENT)/arm64/lilypond
	-mkdir -p $(EXPRESSEUR_CONTENT)/x86_64
	tar -xf $(LILYPOND_X86_64_TAR) -C $(EXPRESSEUR_CONTENT)/x86_64/
	mv $(EXPRESSEUR_CONTENT)/x86_64/$(LILYPOND) $(EXPRESSEUR_CONTENT)/x86_64/lilypond
	cp ressources/*.* $(EXPRESSEUR_CONTENT)/ressources
	cp example/*.* $(EXPRESSEUR_CONTENT)/example
	install_name_tool -change @rpath/libbass.dylib @loader_path/libbass.dylib $(EXPRESSEUR_CONTENT)/luabass.so
	install_name_tool -change @rpath/libbassmidi.dylib @loader_path/libbassmidi.dylib $(EXPRESSEUR_CONTENT)/luabass.so
	install_name_tool -change @rpath/libbassmix.dylib @loader_path/libbassmix.dylib $(EXPRESSEUR_CONTENT)/luabass.so
	install_name_tool -change @rpath/libbass.dylib @loader_path/libbass.dylib $(EXPRESSEUR_CONTENT)/libbasslua.dylib
	install_name_tool -change @rpath/libbassmidi.dylib @loader_path/libbassmidi.dylib $(EXPRESSEUR_CONTENT)/libbasslua.dylib
	install_name_tool -change @rpath/libbassmix.dylib @loader_path/libbassmix.dylib $(EXPRESSEUR_CONTENT)/libbasslua.dylib
	install_name_tool -change basslua/libbasslua.dylib @loader_path/libbasslua.dylib $(EXPRESSEUR_CONTENT)/expresseur
	cp lua/*.* $(EXPRESSEUR_CONTENT)
	cp /usr/local/lib/libserialport.dylib $(EXPRESSEUR_CONTENT)
	install_name_tool -change /usr/local/lib/libserialport.0.dylib  @loader_path/libserialport.dylib luabass.so
$(EXPRESSCMD_EXE) : $(EXPRESSCMD)
	-mkdir -p $(EXPRESSEUR_CONTENT)
	cp $(EXPRESSCMD) $(LIBLUABASS) $(LIBBASSLUA) $(BASS_DIR)/*.dylib $(LUA_DIR)/liblua.a $(EXPRESSEUR_CONTENT)
	install_name_tool -change basslua/libbasslua.dylib @loader_path/libbasslua.dylib $(EXPRESSEUR_CONTENT)/expresscmd
	cp lua/*.* $(EXPRESSEUR_CONTENT)

$(EXPRESSEUR):  $(EXPRESSEUR_OBJECTS) $(LIBBASSLUA) 
	$(CXX) $(LDFLAGS) -o $@ $^ $(EXPRESSEUR_LIBS)

$(EXPRESSCMD): $(EXPRESSCMD_OBJECTS)  $(LIBBASSLUA) 
	$(CXX) $(LDFLAGS) -o $@ $^ $(EXPRESSCMD_LIBS)

$(LIBBASSLUA): $(LIBBASSLUA_OBJECTS)
	$(CXX) $(DYNLIB_LDFLAGS) -o $@ $^ $(BASSLUA_LIBS)

$(LIBLUABASS): $(LIBLUABASS_OBJECTS)
	$(CXX) $(CXXFLAGS) -fPIC -shared $(SHAREDLIB_LDFLAGS) -o $@ $^ $(LUABASS_LIBS)

$(SERIALPORT_DIR)/serialport.o:
$(SERIALPORT_DIR)/timing.o:
$(SERIALPORT_DIR)/macosx.o:
	
clean:
	-$(RM) -r $(EXPRESSEUR_APP)
	$(RM) $(EXPRESSEUR_OBJECTS:.o=.d) $(EXPRESSEUR_OBJECTS) $(EXPRESSEUR) \
	    $(EXPRESSCMD_OBJECTS:.o=.d) $(EXPRESSCMD_OBJECTS) $(EXPRESSCMD) \
	    $(LIBBASSLUA_OBJECTS:.o=.d) $(LIBBASSLUA_OBJECTS) $(LIBBASSLUA) \
	    $(TEST_OBJECTS:.o=.d) $(TEST_OBJECTS) $(TEST) \
	    $(LIBLUABASS_OBJECTS:.o=.d) $(LIBLUABASS_OBJECTS) $(LIBLUABASS)

.PHONY: all clean

-include $(EXPRESSEUR_OBJECTS:.o=.d) $(EXPRESSCMD_OBJECTS:.o=.d) $(LIBBASSLUA_OBJECTS:.o=.d) $(LIBLUABASS_OBJECTS:.o=.d) $(TEST_OBJECTS:.o=.d)

