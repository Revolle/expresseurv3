# Generate expresseur/Expresseur.app
# platform Linux
# platform MAC Darwin
#
EXPRESSEURAPP := expresseur/Expresseur.app

PLATFORM := $(shell uname)
CPPFLAGS := -Wall -MD -MP
CPPFLAGS := $(CPPFLAGS) -g
CXXFLAGS := -std=c++11 
LDFLAGS  :=

ifeq ($(PLATFORM),Darwin)
	  
    WX_CONFIG =../wxWidgets/build64/wx-config

    CXXFLAGS := $(CXXFLAGS) -mmacosx-version-min=10.15 -D__MACOSX_CORE__
    LDFLAGS := $(LDFLAGS) -mmacosx-version-min=10.15

    # dynamic library for basslua ( called by GUI expresseur )
    DYNLIB_EXT := dylib
    DYNLIB_LDFLAGS := $(LDFLAGS) -dynamiclib
    
    # lua C module for luabass , loaded by lua
    SHAREDLIB_EXT := so
    SHAREDLIB_LDFLAGS := $(LDFLAGS) -undefined dynamic_lookup 

		DIRPLATFORM := osx
		LUADIR := $(DIRPLATFORM)/lua/src
		RTMIDIDIR := ../rtmidi
		BASSDIR := $(DIRPLATFORM)/bass

		LUABASS_LIBS :=  -L$(BASSDIR) -lbass -lbassmidi -lbassmix -framework CoreFoundation -framework CoreAudio -framework CoreMIDI
		BASSLUA_LIBS := $(LUABASS_LIBS) -L$(LUADIR) -llua 
		LILYPOND_X86_TAR := ../lilypond-2.25.28-darwin-x86_64.tar
		LILYPOND_ARM_TAR := ../lilypond-2.25.28-darwin-arm64.tar
		WXWIDGESTLIB = $(shell $(WX_CONFIG) --libs base,net,core,adv,xml)
		EXPRESSCMD_LIBS := 
		EXPRESSEUR_LIBS := $(WXWIDGESTLIB) $(EXPRESSCMD_LIBS) 
		TEST_LIBS := $(WXWIDGESTLIB) 
		EXPRESSEURCONTENT := $(EXPRESSEURAPP)/Contents/MacOS
		EXPRESSEURRESOURCES := $(EXPRESSEURAPP)/Contents/Resources
else
ifeq ($(PLATFORM),Linux)
		
		CPPFLAGS := $(CPPFLAGS) -O -fPIC -D__LINUX_ALSA__ 

		WX_CONFIG=../wxWidgets/buildgtk/wx-config

    # dynamic library for basslua ( called by GUI expresseur )
    DYNLIB_EXT := dylib
    DYNLIB_LDFLAGS := $(LDFLAGS) -O -shared -fpic  
   
    # lua C module for luabass , loaded by lua
    SHAREDLIB_EXT := so
    SHAREDLIB_LDFLAGS := $(LDFLAGS) -O -shared -fpic 

		DIRPLATFORM := linux
		LUADIR := $(DIRPLATFORM)/lua/src
		RTMIDIDIR := $(DIRPLATFORM)/rtmidi
		BASSDIR := $(DIRPLATFORM)/bass
		LILYPOND_X86_TAR := ../lilypond-2.25.28-darwin-x86_64.tar
		LILYPOND_ARM_TAR := ../lilypond-2.25.28-darwin-arm64.tar
		MORIGIN := $$ORIGIN
		EXPRESS_LIBPATH := -Wl,-rpath=$(BASSDIR),-rpath=../$(BASSDIR),-rpath=basslua,-rpath=luabass
		LUABASS_LIBS := -L$(BASSDIR) -lbass -lbassmidi -lbassmix -lrt -lm -ldl -lasound -lpthread $(EXPRESS_LIBPATH)
		BASSLUA_LIBS := $(LUABASS_LIBS)  -L$(LUADIR) -llua $(EXPRESS_LIBPATH)
		WXWIDGESTLIB = $(shell $(WX_CONFIG) --libs base,net,core,adv,xml)
		EXPRESSCMD_LIBS := -lrt -lm -ldl $(EXPRESS_LIBPATH)
		EXPRESSEUR_LIBS := $(WXWIDGESTLIB)  $(EXPRESSCMD_LIBS) 
		TEST_LIBS := $(WXWIDGESTLIB) 
		EXPRESSEURCONTENT := $(EXPRESSEURAPP)/Contents/Linux
		EXPRESSEURRESOURCES := $(EXPRESSEURAPP)/Contents/Resources
else
    $(error Define PLATFORM_DIR containing the required headers and libraries)

    DYNLIB_EXT := so
    DYNLIB_LDFLAGS := -Wl,-z,defs -shared
endif
endif

EXPRESSEUR := expresseur/expresseur
EXPRESSEUR_OBJECTS := $(patsubst %.cpp,%.o,$(wildcard expresseur/*.cpp))
$(EXPRESSEUR_OBJECTS): CPPFLAGS += $(shell $(WX_CONFIG) --cxxflags)

TEST := test/test
TEST_OBJECTS := $(patsubst %.cpp,%.o,$(wildcard test/*.cpp))
$(TEST_OBJECTS): CPPFLAGS += $(shell $(WX_CONFIG) --cxxflags)

EXPRESSCMD := expresscmd/expresscmd
EXPRESSCMDEXE := expresscmd/expresscmd.exe
EXPRESSCMD_OBJECTS := $(patsubst %.cpp,%.o,$(wildcard expresscmd/*.cpp))

LIBBASSLUA := basslua/libbasslua.$(DYNLIB_EXT)
LIBBASSLUA_OBJECTS := $(patsubst %.cpp,%.o,$(wildcard basslua/*.cpp))
RTMIDI_OBJECTS := $(patsubst %.cpp,%.o,$(wildcard $(RTMIDIDIR)/*.cpp))

LUA_OBJECTS := lua/*.*

LIBLUABASS := luabass/luabass.$(SHAREDLIB_EXT)
LIBLUABASS_OBJECTS := $(patsubst %.cpp,%.o,$(wildcard luabass/*.cpp))

CPPFLAGS += -Ibasslua -Iluabass -Iexpresseur -I$(BASSDIR) -I$(LUADIR) -I$(RTMIDIDIR)

print-%  : ; @echo $* = $($*)

ifeq ($(PLATFORM),Darwin)
all: $(LIBBASSLUA) $(LIBLUABASS) $(EXPRESSCMDEXE) $(EXPRESSEURAPP) $(TEST)


$(EXPRESSEURAPP): $(EXPRESSEUR) $(LUA_OBJECTS) expresseur/Info.plist
	-mkdir -p $(EXPRESSEURRESOURCES)
	-mkdir -p $(EXPRESSEURCONTENT)
	-mkdir -p $(EXPRESSEURCONTENT)/example
	-mkdir -p $(EXPRESSEURCONTENT)/ressources
	cp expresseur/AppIcon.icns $(EXPRESSEURRESOURCES)
	cp expresseur/Info.plist $@/Contents
	cp $(EXPRESSEUR) $(LIBLUABASS) $(LIBBASSLUA) $(BASSDIR)/*.dylib $(LUADIR)/liblua.a $(LUA_OBJECTS) $(EXPRESSEURCONTENT)
	cp $(LILYPOND_X86_TAR) $(EXPRESSEURCONTENT)/
	cp $(LILYPOND_ARM_TAR) $(EXPRESSEURCONTENT)/
	cp ressources/*.* $(EXPRESSEURCONTENT)/ressources
	cp example/*.* $(EXPRESSEURCONTENT)/example
	install_name_tool -change @rpath/libbass.dylib @loader_path/libbass.dylib $(EXPRESSEURCONTENT)/luabass.so
	install_name_tool -change @rpath/libbassmidi.dylib @loader_path/libbassmidi.dylib $(EXPRESSEURCONTENT)/luabass.so
	install_name_tool -change @rpath/libbassmix.dylib @loader_path/libbassmix.dylib $(EXPRESSEURCONTENT)/luabass.so
	install_name_tool -change @rpath/libbass.dylib @loader_path/libbass.dylib $(EXPRESSEURCONTENT)/libbasslua.dylib
	install_name_tool -change @rpath/libbassmidi.dylib @loader_path/libbassmidi.dylib $(EXPRESSEURCONTENT)/libbasslua.dylib
	install_name_tool -change @rpath/libbassmix.dylib @loader_path/libbassmix.dylib $(EXPRESSEURCONTENT)/libbasslua.dylib
	install_name_tool -change basslua/libbasslua.dylib @loader_path/libbasslua.dylib $(EXPRESSEURCONTENT)/expresseur
	cp lua/*.* $(EXPRESSEURCONTENT)
	rm -f ~/Documents/ExpresseurV3/ressources/*.txt
	
$(EXPRESSCMDEXE) : $(EXPRESSCMD)
	-mkdir -p $(EXPRESSEURCONTENT)
	cp $(EXPRESSCMD) $(LIBLUABASS) $(LIBBASSLUA) $(BASSDIR)/*.dylib $(LUADIR)/liblua.a $(LUA_OBJECTS) $(EXPRESSEURCONTENT)
	install_name_tool -change basslua/libbasslua.dylib @loader_path/libbasslua.dylib $(EXPRESSEURCONTENT)/expresscmd
	cp lua/*.* $(EXPRESSEURCONTENT)
endif

        
ifeq ($(PLATFORM),Linux)
all:  $(LIBLUABASS) $(LIBBASSLUA) $(EXPRESSCMDEXE) $(EXPRESSEURAPP) 

$(EXPRESSEURAPP): $(EXPRESSEUR) $(LUA_OBJECTS)
	-mkdir -p $(EXPRESSEURCONTENT)/example
	-mkdir -p $(EXPRESSEURCONTENT)/ressources
	-mkdir -p $(EXPRESSEURCONTENT)/basslua
	-mkdir -p $(EXPRESSEURCONTENT)/$(BASSDIR)
	cp $(BASSDIR)/lib*.so $(EXPRESSEURCONTENT)/$(BASSDIR)
	cp $(EXPRESSEUR) $(LIBLUABASS) $(LUA_OBJECTS) $(EXPRESSEURCONTENT)
	cp $(LIBBASSLUA) $(EXPRESSEURCONTENT)/basslua
	cp $(LILYPOND_X86_TAR) $(EXPRESSEURCONTENT)/
	cp $(LILYPOND_ARM_TAR) $(EXPRESSEURCONTENT)/
	cp ressources/*.* $(EXPRESSEURCONTENT)/ressources
	cp example/*.* $(EXPRESSEURCONTENT)/example
	cp lua/*.* $(EXPRESSEURCONTENT)
	cp expresseur/*.ico $(EXPRESSEURCONTENT)
	rm -f ~/Documents/ExpresseurV3/ressources/*.txt
	
$(EXPRESSCMDEXE) : $(EXPRESSCMD)
	-mkdir -p $(EXPRESSEURCONTENT)/basslua
	-mkdir -p $(EXPRESSEURCONTENT)/$(BASSDIR)
	cp $(BASSDIR)/lib*.so $(EXPRESSEURCONTENT)/$(BASSDIR)
	cp $(EXPRESSCMD) $(LIBLUABASS) $(LUA_OBJECTS) $(EXPRESSEURCONTENT)
	cp $(LIBBASSLUA) $(EXPRESSEURCONTENT)/basslua
	cp lua/*.* $(EXPRESSEURCONTENT)
endif

$(EXPRESSEUR):  $(EXPRESSEUR_OBJECTS) $(LIBBASSLUA) 
	$(CXX) $(LDFLAGS) -o $@ $^ $(EXPRESSEUR_LIBS)

$(EXPRESSCMD): $(EXPRESSCMD_OBJECTS)  $(LIBBASSLUA) 
	$(CXX) $(LDFLAGS) -o $@ $^ $(EXPRESSCMD_LIBS)

$(LIBBASSLUA): $(LIBBASSLUA_OBJECTS)
	$(CXX) $(DYNLIB_LDFLAGS) -o $@ $^ $(BASSLUA_LIBS)

$(LIBLUABASS): $(LIBLUABASS_OBJECTS) $(RTMIDI_OBJECTS)
	$(CXX) $(CXXFLAGS) $(SHAREDLIB_LDFLAGS) -o $@ $^ $(LUABASS_LIBS)

$(TEST): $(TEST_OBJECTS)  
	$(CXX) $(LDFLAGS) -o $@ $^ $(TEST_LIBS)

clean:
	-$(RM) -r $(EXPRESSEURAPP)
	$(RM) $(EXPRESSEUR_OBJECTS:.o=.d) $(EXPRESSEUR_OBJECTS) $(EXPRESSEUR) \
	    $(EXPRESSCMD_OBJECTS:.o=.d) $(EXPRESSCMD_OBJECTS) $(EXPRESSCMD) \
	    $(LIBBASSLUA_OBJECTS:.o=.d) $(LIBBASSLUA_OBJECTS) $(LIBBASSLUA) \
	    $(TEST_OBJECTS:.o=.d) $(TEST_OBJECTS) $(TEST) \
	    $(LIBLUABASS_OBJECTS:.o=.d) $(LIBLUABASS_OBJECTS) $(LIBLUABASS)

.PHONY: all clean

-include $(EXPRESSEUR_OBJECTS:.o=.d) $(EXPRESSCMD_OBJECTS:.o=.d) $(LIBBASSLUA_OBJECTS:.o=.d) $(LIBLUABASS_OBJECTS:.o=.d) $(TEST_OBJECTS:.o=.d)

