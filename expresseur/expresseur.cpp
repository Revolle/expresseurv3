/////////////////////////////////////////////////////////////////////////////
// Purpose:     expresseur V3
// Author:      Franck REVOLLE
// Copyright:   (c) Franck REVOLLE Expresseur
// Licence:    Expresseur licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
 
#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "wx/toolbar.h"
#include "wx/log.h"
#include "wx/image.h"
#include "wx/filedlg.h"
#include "wx/colordlg.h"
#include "wx/srchctrl.h"
#include "wx/spinctrl.h"
#include "wx/textfile.h"
#include "wx/ffile.h"
#include "wx/filefn.h"
#include "wx/filename.h"
#include "wx/filehistory.h"
#include "wx/tglbtn.h"
#include "wx/config.h"
#include "wx/listctrl.h"
#include "wx/filepicker.h"
#include "wx/msgdlg.h"
#include "wx/scrolbar.h"
#include "wx/choicdlg.h"
#include "wx/xml/xml.h"
#include "wx/dynarray.h"
#include "wx/sstream.h"
#include "wx/protocol/http.h"
#include "wx/wizard.h"
#include "wx/clipbrd.h"
#include "wx/stdpaths.h"
#include "wx/dir.h"
#include "wx/dynlib.h"
#include "wx/statbmp.h"
#include "wx/stopwatch.h"
#include "wx/tokenzr.h"
#include "wx/kbdstate.h"

#include "version.h"
#include "global.h"
#include "basslua.h"
#include "luabass.h"
#include "mxconf.h"
#include "viewerscore.h"
#include "mixer.h"
#include "editshortcut.h"
#include "midishortcut.h"
#include "expression.h"
#include "logError.h"
#include "emptyscore.h"
#include "bitmapscore.h"
#include "musicxml.h"
#include "musicxmlcompile.h"
#include "musicxmlscore.h"
#include "textscore.h"
#include "luafile.h"
#include "expresseur.h"

// define this to use XPMs everywhere (by default, BMPs are used under Win)
// BMPs use less space, but aren't compiled into the executable on other platforms
#ifdef __WINDOWS__
    #define USE_XPM_BITMAPS 1
#else
    #define USE_XPM_BITMAPS 1
#endif


#ifndef wxHAS_IMAGES_IN_RESOURCES
#endif

/*
#if USE_XPM_BITMAPS
    #include "bitmaps/new.xpm"
    #include "bitmaps/open.xpm"
    #include "bitmaps/save.xpm"
    #include "bitmaps/copy.xpm"
    #include "bitmaps/cut.xpm"
    #include "bitmaps/preview.xpm"  // paste XPM
    #include "bitmaps/print.xpm"
    #include "bitmaps/help.xpm"
#endif // USE_XPM_BITMAPS
*/

Expresseur *frame = NULL;

// Define a new application
class MyApp : public wxApp
{
public:
    	bool OnInit();
	
	int FilterEvent(wxEvent& event);
};

// timer value for compilation in ms
#define periodCompile 1000
// timer value between screen refresh in ms
#define periodRefresh 500

enum
{
    // toolbar menu items
	ID_MAIN_LIST_NEW = ID_MAIN,
	ID_MAIN_LIST_OPEN,
	ID_MAIN_LIST_SAVE,
	ID_MAIN_LIST_SAVEAS,
	ID_MAIN_LIST_ADD,
	ID_MAIN_LIST_REMOVE,
	ID_MAIN_LIST_UP,
	ID_MAIN_LIST_DOWN,

	ID_MAIN_ORNAMENT_ADD_ABSOLUTE,
	ID_MAIN_ORNAMENT_ADD_RELATIVE,
	ID_MAIN_UNZOOM_3,
	ID_MAIN_UNZOOM_2,
	ID_MAIN_UNZOOM_1,
	ID_MAIN_ZOOM_0,
	ID_MAIN_ZOOM_1,
	ID_MAIN_ZOOM_2,
	ID_MAIN_ZOOM_3,
	ID_MAIN_RECORD_IMPRO,
	ID_MAIN_SAVE_IMPRO,
	ID_MAIN_PLAYVIEW,
	ID_MAIN_SOLO0,
	ID_MAIN_SOLO1,
	ID_MAIN_SOLO2,
	ID_MAIN_SOLO3,
	ID_MAIN_SOLO4,
	ID_MAIN_SOLO5,
	ID_MAIN_SOLO6,
	ID_MAIN_SOLO7,
	ID_MAIN_SOLO8,
	ID_MAIN_SOLO9,

	ID_MAIN_LIST_PREVIOUS_FILE,
	ID_MAIN_LIST_NEXT_FILE,

	ID_MAIN_TEXT_SONG ,

	ID_MAIN_SCROLL_HORIZONTAL,
	ID_MAIN_SCROLL_VERTICAL,

	ID_MAIN_MIXER,
	ID_MAIN_GOTO,
	ID_MAIN_MIDISHORTCUT,
	ID_MAIN_KEYDOWNCONFIG,
	ID_MAIN_EXPRESSION,
	ID_MAIN_LUAFILE,
	ID_MAIN_SETTING_OPEN,
	ID_MAIN_SETTING_SAVEAS,
	ID_MAIN_RESET,
	ID_MAIN_LOG,
	ID_MAIN_MIDILOG,
	ID_MAIN_UPDATE,
	ID_MAIN_FIRSTUSE,
	ID_MAIN_RECORD_PLAYBACK,
	ID_MAIN_SAVE_PLAYBACK,
	ID_MAIN_PLAYBACK,

	ID_MAIN_TIMER,

	ID_MAIN_HELP_LUASHORTCUT,

	ID_MAIN_TEST,
	ID_MAIN_CHECK_CONFIG,
	ID_MAIN_DELETE_CACHE,

	ID_MAIN_LOCAL_OFF,
	ID_MAIN_AUDIO_SETTING,
	ID_MAIN_MIDI_SETTING,

	ID_MAIN_PREVIOUS_PAGE,
	ID_MAIN_NEXT_PAGE,

	ID_MAIN_LIST_FILE = ID_MAIN + 100,  // large range for the files in the list
	ID_MAIN_LIST_FILE_END = ID_MAIN_LIST_FILE + 99,  // large range for the files in the list
	ID_MAIN_ACTION = ID_MAIN_LIST_FILE_END + 1, // large range for actions
	ID_MAIN_ACTION_END = ID_MAIN_ACTION + 99 , // large range for actions
	ID_MAIN_KEY_SHORTCUT = ID_MAIN_ACTION_END + 1, // large range for shortcuts
	ID_MAIN_KEY_SHORTCUT_END = ID_MAIN_KEY_SHORTCUT + 99 , // large range for shortcuts
	ID_MAIN_SETTINGS_FILE = ID_MAIN_KEY_SHORTCUT_END + 1 , // large range for confgi presets
	ID_MAIN_SETTINGS_FILE_END = ID_MAIN_SETTINGS_FILE + 99
};

// ----------------------------------------------------------------------------
// event tables
// ----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(Expresseur, wxFrame)

EVT_MENU(ID_MAIN_TEST, Expresseur::OnTest)

//EVT_MENU(wxID_NEW, Expresseur::OnNew)
EVT_MENU(wxID_OPEN, Expresseur::OnOpen)
EVT_MENU(wxID_SAVE, Expresseur::OnSave)
EVT_MENU(wxID_SAVEAS, Expresseur::OnSaveas)
EVT_MENU(wxID_EXIT, Expresseur::OnExit)
EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, Expresseur::OnRecentFile)

EVT_MENU(wxID_UNDO, Expresseur::OnUndo)
EVT_MENU(wxID_REDO, Expresseur::OnRedo)
EVT_MENU(wxID_COPY, Expresseur::OnCopy)
EVT_MENU(wxID_CUT, Expresseur::OnCut)
EVT_MENU(wxID_PASTE, Expresseur::OnPaste)
EVT_MENU_RANGE(ID_MAIN_SOLO1, ID_MAIN_SOLO9, Expresseur::OnPlayviewSolo)
EVT_MENU(ID_MAIN_SOLO0, Expresseur::OnPlayviewAll)
EVT_MENU(ID_MAIN_RECORD_IMPRO, Expresseur::OnRecordImpro)
EVT_MENU(ID_MAIN_SAVE_IMPRO, Expresseur::OnSaveImpro)
EVT_MENU(ID_MAIN_PLAYVIEW, Expresseur::OnPlayview)
EVT_MENU_RANGE(ID_MAIN_UNZOOM_3, ID_MAIN_ZOOM_3, Expresseur::OnZoom)
EVT_MENU(ID_MAIN_ORNAMENT_ADD_RELATIVE, Expresseur::OnOrnamentAddRelative)
EVT_MENU(ID_MAIN_ORNAMENT_ADD_ABSOLUTE, Expresseur::OnOrnamentAddAbsolute)

EVT_MENU(wxID_EDIT, Expresseur::OnEdit)
EVT_MENU(ID_MAIN_LOCAL_OFF, Expresseur::OnLocaloff)
EVT_MENU(ID_MAIN_AUDIO_SETTING, Expresseur::OnAudioSetting)
EVT_MENU(ID_MAIN_MIDI_SETTING, Expresseur::OnMidiSetting)

EVT_MENU(ID_MAIN_PREVIOUS_PAGE,Expresseur::OnPreviousPage)
EVT_MENU(ID_MAIN_NEXT_PAGE, Expresseur::OnNextPage)

EVT_MENU(ID_MAIN_LIST_PREVIOUS_FILE, Expresseur::OnListPreviousFile)
EVT_MENU(ID_MAIN_LIST_NEXT_FILE, Expresseur::OnListNextFile)
EVT_MENU(ID_MAIN_LIST_NEW, Expresseur::OnListNew)
EVT_MENU(ID_MAIN_LIST_OPEN, Expresseur::OnListOpen)
EVT_MENU(ID_MAIN_LIST_SAVE, Expresseur::OnListSave)
EVT_MENU(ID_MAIN_LIST_SAVEAS, Expresseur::OnListSaveas)
EVT_MENU(ID_MAIN_LIST_ADD, Expresseur::OnListAdd)
EVT_MENU(ID_MAIN_LIST_REMOVE, Expresseur::OnListRemove)
EVT_MENU(ID_MAIN_LIST_UP, Expresseur::OnListUp)
EVT_MENU(ID_MAIN_LIST_DOWN, Expresseur::OnListDown)
EVT_MENU_RANGE(ID_MAIN_LIST_FILE, ID_MAIN_LIST_FILE_END, Expresseur::OnListFile)

EVT_MENU_RANGE(ID_MAIN_ACTION, ID_MAIN_ACTION_END, Expresseur::OnMenuAction)
EVT_MENU_RANGE(ID_MAIN_KEY_SHORTCUT, ID_MAIN_KEY_SHORTCUT_END, Expresseur::OnMenuShortcut)
EVT_MENU_RANGE(ID_MAIN_SETTINGS_FILE, ID_MAIN_SETTINGS_FILE_END, Expresseur::OnMenuSettings)

EVT_MENU(ID_MAIN_MIXER, Expresseur::OnMixer)
EVT_MENU(ID_MAIN_GOTO, Expresseur::OnGoto)
EVT_MENU(ID_MAIN_MIDISHORTCUT, Expresseur::OnMidishortcut)
EVT_MENU(ID_MAIN_KEYDOWNCONFIG, Expresseur::OnKeydowInfoLua)
EVT_MENU(ID_MAIN_EXPRESSION, Expresseur::OnExpression)
EVT_MENU(ID_MAIN_LUAFILE, Expresseur::OnLuafile)
EVT_MENU(ID_MAIN_RESET, Expresseur::OnReset)
EVT_MENU(ID_MAIN_DELETE_CACHE, Expresseur::OnDeleteCache)
EVT_MENU(ID_MAIN_LOG, Expresseur::OnLog)
EVT_MENU(ID_MAIN_MIDILOG, Expresseur::OnMidiLog)
EVT_MENU(ID_MAIN_SETTING_OPEN, Expresseur::OnSettingOpen)
EVT_MENU(ID_MAIN_SETTING_SAVEAS, Expresseur::OnSettingSaveas)
EVT_MENU(ID_MAIN_CHECK_CONFIG, Expresseur::OnCheckConfig)

EVT_MENU(ID_MAIN_HELP_LUASHORTCUT, Expresseur::OnHelpluashortcut)

EVT_MENU(ID_MAIN_RECORD_PLAYBACK, Expresseur::OnRecordPlayback)
EVT_MENU(ID_MAIN_SAVE_PLAYBACK, Expresseur::OnSavePlayback)
EVT_MENU(ID_MAIN_PLAYBACK, Expresseur::OnPlayback)

EVT_MENU(wxID_ABOUT, Expresseur::OnAbout)
EVT_MENU(wxID_HELP, Expresseur::OnHelp)
EVT_MENU(ID_MAIN_UPDATE, Expresseur::OnUpdate)
EVT_MENU(ID_MAIN_FIRSTUSE, Expresseur::OnResetConfiguration)

EVT_COMMAND_SCROLL_THUMBRELEASE(ID_MAIN_SCROLL_HORIZONTAL, Expresseur::OnHorizontalScroll)
EVT_COMMAND_SCROLL_THUMBRELEASE(ID_MAIN_SCROLL_VERTICAL, Expresseur::OnVerticalScroll)

EVT_TIMER(ID_MAIN_TIMER, Expresseur::OnTimer)
EVT_IDLE(Expresseur::OnIdle)
EVT_SIZE(Expresseur::OnSize)

wxEND_EVENT_TABLE()

// ============================================================================
// implementation
// ============================================================================

 
// The `main program' equivalent, creating the windows and returning the
// main frame
bool MyApp::OnInit()
{
    if ( !wxApp::OnInit() )
    {
        return false;
    }

	SetAppName(APP_NAME);

	wxInitAllImageHandlers();

    // Create the main frame window
	frame = new Expresseur(NULL, wxID_ANY, APP_NAME, wxPoint(0, 0), wxSize(500, 400), wxMINIMIZE_BOX | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN);

	// Give it an icon (this is ignored in MDI mode: uses resources)
#ifdef RUN_WIN
	frame->SetIcon(wxIcon("MAINICON"));
#endif
#ifdef RUN_LINUX
	frame->SetIcon(wxIcon("expresseur.ico",wxBITMAP_TYPE_ICON));
#endif
#ifdef RUN_MAC
	//frame->SetIcon(wxIcon("expresseur.ico",wxBITMAP_TYPE_ICON));
#endif

#if wxUSE_STATUSBAR
    frame->SetStatusText("",1);
#endif

  return true;
}
int MyApp::FilterEvent(wxEvent& event)
{ 
	if ((event.GetEventType() == wxEVT_KEY_DOWN ) && frame)
	{
		if (frame->OnKeyDown((wxKeyEvent&)event) )
			return Event_Processed;
	}
	return Event_Skip;
}

// ----------------------------------------------------------------------------
// MyApp
// ----------------------------------------------------------------------------

IMPLEMENT_APP(MyApp)


Expresseur::Expresseur(wxFrame* parent,wxWindowID id,const wxString& title,const wxPoint& pos,const wxSize& size,long style)
 :wxFrame(parent, id, title, pos, size, style)
{
    // Give it a status line
	wxStatusBar *mStatusBar = CreateStatusBar(3);
	int proportionStatusBar[3] = { -10,-10,-1 };
	mStatusBar->SetStatusWidths(3, proportionStatusBar);

	// configuration object ( with memory of all parameters
	mConf = new mxconf();

	mxconf::getAppDir();
	//CreateExpresseurV3();

	mode = modeNil;
	listChanged = false;
	listName.Clear();

	typeViewer = EMPTYVIEWER;
	listSelection = -1;
	fileName.Clear();
	mViewerscore = NULL;
	mTextscore = NULL;
	mMixer = NULL;
	mExpression = NULL;
	mMidishortcut = NULL;
	mLog = NULL;
	mtimer = NULL;
	fileHistory = NULL;

	if (mConf->get(CONFIG_VERSION_CHECKED, VERSION_EXPRESSEUR) < VERSION_EXPRESSEUR)
		mConf->set(CONFIG_VERSION_CHECKED, VERSION_EXPRESSEUR);

	// set the menus
	wxMenu *fileMenu = new wxMenu;
	//fileMenu->Append(wxID_NEW, _("New"));
	fileMenu->Append(wxID_OPEN, _("Open...\tCtrl+O"));
	fileMenu->Append(wxID_SAVE, _("Save\tCtrl+S"));
	fileMenu->Append(wxID_SAVEAS, _("Save as..."));
	fileMenu->AppendSeparator();
	wxMenu* menuRecent = new wxMenu;
	fileMenu->AppendSubMenu(menuRecent, "Open Recent");
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_EXIT, "Exit\tCtrl+Q");
	fileHistory = new wxFileHistory();
	fileHistory->UseMenu(menuRecent);
	fileHistory->AddFilesToMenu(menuRecent);
	fileHistory->Load(*mConf->getConfig());

	editMenu = new wxMenu;
	editMenu->Append(wxID_UNDO, _("Undo\tCtrl+Z"));
	editMenu->Append(wxID_REDO, _("Redo"));
	editMenu->AppendSeparator();
	editMenu->Append(wxID_COPY, _("Copy\tCtrl+C"));
	editMenu->Append(wxID_CUT, _("Cut\tCtrl+X"));
	editMenu->Append(wxID_PASTE, _("Paste\tCtrl+V"));
	editMenu->AppendSeparator();
	menuEditMode = editMenu->AppendCheckItem(wxID_EDIT, _("Edit\tCtrl+E"), _("Allow text change. Else, keystrokes are used for shortcuts"));
	editMenu->AppendSeparator();
	editMenu->Append(ID_MAIN_MIXER, _("Mixer\tCTRL+M"));
	editMenu->Append(ID_MAIN_EXPRESSION, _("Expression\tCTRL+P"));
	editMenu->AppendSeparator();
	editMenu->Append(ID_MAIN_GOTO, _("Goto measure...\tCTRL+G"), _("Goto a measure in the score"));
	editMenu->Append(ID_MAIN_PREVIOUS_PAGE, _("Previous page\tSHIFT+CTRL+UP"), _("Goto previous page of the score"));
	editMenu->Append(ID_MAIN_NEXT_PAGE, _("Next Page\tSHIFT+CTRL+DOWN"), _("Goto next page of the score"));
	editMenu->Append(ID_MAIN_ORNAMENT_ADD_ABSOLUTE, _("Add Ornament absolute...\tSHIFT+CTRL+A"), _("Add an ornament in the score, at the expresseur position"));
	editMenu->Append(ID_MAIN_ORNAMENT_ADD_RELATIVE, _("Add Ornament relative...\tSHIFT+CTRL+R"), _("Add an ornament in the score, at the score position"));
	editMenu->AppendSeparator();

	zoomMenu = new wxMenu;
	zoomMenu->AppendRadioItem(ID_MAIN_UNZOOM_3, _("very small"));
	zoomMenu->AppendRadioItem(ID_MAIN_UNZOOM_2, _("small"));
	zoomMenu->AppendRadioItem(ID_MAIN_UNZOOM_1, _("smaller"));
	zoomMenu->AppendRadioItem(ID_MAIN_ZOOM_0, _("normal"));
	zoomMenu->AppendRadioItem(ID_MAIN_ZOOM_1, _("bigger"));
	zoomMenu->AppendRadioItem(ID_MAIN_ZOOM_2, _("big"));
	zoomMenu->AppendRadioItem(ID_MAIN_ZOOM_3, _("very big"));
	editMenu->AppendSubMenu(zoomMenu , "Zoom");

	wxMenu *viewplayMenu = new wxMenu;
	viewplayMenu->Append(ID_MAIN_PLAYVIEW, _("Play/View ...\tCTRL+L"), _("Quick-select tracks to play/view in the score"));
	viewplayMenu->Append(ID_MAIN_SOLO0, _("Play & View all tracks\tALT+0"), _("extraquick play/view all tracks"));
	viewplayMenu->Append(ID_MAIN_SOLO1, _("Solo part 1\tALT+1"), _("extraquick solo play/view track #1"));
	viewplayMenu->Append(ID_MAIN_SOLO2, _("Solo part 2\tALT+2"), _("extraquick solo play/view track #2"));
	viewplayMenu->Append(ID_MAIN_SOLO3, _("Solo part 3\tALT+3"), _("extraquick solo play/view track #3"));
	viewplayMenu->Append(ID_MAIN_SOLO4, _("Solo part 4\tALT+4"), _("extraquick solo play/view track #4"));
	viewplayMenu->Append(ID_MAIN_SOLO5, _("Solo part 5\tALT+5"), _("extraquick solo play/view track #5"));
	viewplayMenu->Append(ID_MAIN_SOLO6, _("Solo part 6\tALT+6"), _("extraquick solo play/view track #6"));
	viewplayMenu->Append(ID_MAIN_SOLO7, _("Solo part 7\tALT+7"), _("extraquick solo play/view track #7"));
	viewplayMenu->Append(ID_MAIN_SOLO8, _("Solo part 8\tALT+8"), _("extraquick solo play/view track #8"));
	viewplayMenu->Append(ID_MAIN_SOLO9, _("Solo part 9\tALT+9"), _("extraquick solo play/view track #9"));
	editMenu->AppendSubMenu(viewplayMenu, "View/Play score-tracks");

	editMenu->AppendSeparator();
	editMenu->AppendCheckItem(ID_MAIN_RECORD_PLAYBACK, _("record score playback\tCTRL+R"), _("Start to record score playing"));
	editMenu->Append(ID_MAIN_SAVE_PLAYBACK, _("save score playback\tCTRL+T"), _("Save score playing in PLAYBACK section"));
	editMenu->AppendCheckItem(ID_MAIN_PLAYBACK, _("score playback\tCTRL+P"), _("Playback the score as described in PLAYBACK section"));
	editMenu->AppendSeparator();
	editMenu->Append(ID_MAIN_RECORD_IMPRO, "record improvisation", "start recording improvisation");
	editMenu->Append(ID_MAIN_SAVE_IMPRO, "save improvisation", "save improvisation in musicxml file, within tmp directory");
	editMenu->Enable(ID_MAIN_SAVE_IMPRO, false);
	editMenu->Enable(ID_MAIN_RECORD_IMPRO, true);

	actionMenu = new wxMenu;

	listMenu = new wxMenu;
	listMenu->AppendSeparator();
	listMenu->Append(ID_MAIN_LIST_NEW, _("New List"));
	listMenu->Append(ID_MAIN_LIST_OPEN, _("Open List..."));
	listMenu->Append(ID_MAIN_LIST_SAVE, _("Save List"));
	listMenu->Append(ID_MAIN_LIST_SAVEAS, _("Save List as..."));
	listMenu->AppendSeparator();
	listMenu->Append(ID_MAIN_LIST_ADD, _("Add current file in list"));
	listMenu->Append(ID_MAIN_LIST_REMOVE, _("Remove current file from list"));
	listMenu->Append(ID_MAIN_LIST_UP, _("Up current file in list"));
	listMenu->Append(ID_MAIN_LIST_DOWN, _("Down current file in list"));
	listMenu->AppendSeparator();
	listMenu->Append(ID_MAIN_LIST_PREVIOUS_FILE, _("Previous file\tCTRL+LEFT"));
	listMenu->Append(ID_MAIN_LIST_NEXT_FILE, _("Next file\tCTRL+RIGHT"));

	listSettingMenu = new wxMenu;
	readListSettings();
	for(unsigned int i = 0 ; i < listSettings.GetCount(); i ++ )
	{
		wxString s = listSettings[i] ;
		if ( s.Contains("|"))
		{
			wxString f = s.Left(s.Find('|'));
			wxString h = s.Mid(s.Find('|') + 1) ;
			listSettingMenu->Append(ID_MAIN_SETTINGS_FILE + i ,f,h);
		}
	}


	wxMenu *settingMenu = new wxMenu;
	settingMenu->Append(ID_MAIN_MIDISHORTCUT, _("MIDI/keyboard config..."), _("Configure interaction from MIDI or keyboard"));
	settingMenu->AppendSubMenu(listSettingMenu,_("Setting presets"), _("list of settings already available in the Expresseur/Resource directory"));
	settingMenu->Append(ID_MAIN_SETTING_OPEN, _("Import setting..."));
	settingMenu->Append(ID_MAIN_SETTING_SAVEAS, _("Export setting as..."));
	settingMenu->AppendSeparator();
	settingMenu->AppendCheckItem(ID_MAIN_LOCAL_OFF, _("Send MIDI local-off"), _("Send local-off on MIDI-out opening, i.e. to unlink keyboard and soud-generator on electronic piano"));
	settingMenu->Append(ID_MAIN_AUDIO_SETTING, _("Audio config..."), _("Audio settings, to decrease latency, and to select the default audio output"));
	settingMenu->Append(ID_MAIN_MIDI_SETTING, _("Midi config..."), _("Midi settings, to select MIDI Inputs and Outputs, and the default MIDI output"));
	settingMenu->Append(ID_MAIN_KEYDOWNCONFIG, _("One-key config..."), _("Configuration of the computer keyboard to use one-key shortcuts coded in LUA"));
	settingMenu->Append(ID_MAIN_LUAFILE, _("LUA Files..."));
	//settingMenu->Append(ID_MAIN_RESET, _("Reset audio/midi"), _("Reset the audio/midi configuration"));
	//settingMenu->Append(ID_MAIN_DELETE_CACHE, _("Delete cache"), _("Delete the MuseScore pages, kept in cache to save computing"));
	settingMenu->Append(ID_MAIN_FIRSTUSE, _("Reset configuration"), _("Restart the initialization wizard"));
	// settingMenu->AppendCheckItem(ID_MAIN_MIDILOG, _("Log MIDI"), _("log output MIDI messages in log file"));
	settingMenu->Append(ID_MAIN_CHECK_CONFIG, _("Check config"), _("Check the configuration (files, .. )"));

	wxMenu *helpMenu = new wxMenu;
	helpMenu->Append(wxID_HELP, _("help"));
	helpMenu->Append(wxID_ABOUT, _("About"));
	helpMenu->Append(ID_MAIN_UPDATE, _("Check update"), _("check if an update is available on the www.expresseur.com web site"));

    mMenuBar = new wxMenuBar( wxMB_DOCKABLE );

	mMenuBar->Append(fileMenu, _("File"));
	mMenuBar->Append(editMenu, _("Edit"));
	mMenuBar->Append(actionMenu, _("Action"));
	mMenuBar->Append(listMenu, _("List"));
	mMenuBar->Append(settingMenu, _("Setting"));
	mMenuBar->Append(helpMenu, _("Help"));

    // Associate the menu bar with the frame
	SetMenuBar(mMenuBar);

	toolBar = CreateToolBar();


	editMode = false;
	editMenu->Check(wxID_EDIT, false);
	logMidiMsg = false;
	//settingMenu->Check(ID_MAIN_MIDILOG, false);

	localoff = (bool)(mConf->get(CONFIG_LOCALOFF, true));
	settingMenu->Check(ID_MAIN_LOCAL_OFF, localoff);

	// scroll horizontal
	posScrollHorizontal = mConf->get(CONFIG_MAIN_SCROLLHORIZONTAL, 20);
	mScrollHorizontal = new	wxScrollBar(this, ID_MAIN_SCROLL_HORIZONTAL, wxDefaultPosition, wxDefaultSize, wxSB_HORIZONTAL);
	mScrollHorizontal->SetToolTip(_("split horizontally the text and the image of the Score"));
	mScrollHorizontal->SetScrollbar(posScrollHorizontal, 1, 100, 1, false);
	posScrollVertical = mConf->get(CONFIG_MAIN_SCROLLVERTICAL, 50);
	mScrollVertical = new	wxScrollBar(this, ID_MAIN_SCROLL_VERTICAL, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);
	mScrollVertical->SetScrollbar(posScrollVertical, 1, 100, 1, false);
	mScrollVertical->SetToolTip(_("split vertically the text and the image of the Score"));


	waitToCompile = 1 ;
	waitToRefresh = 1 ;
	timerDt = mConf->get(CONFIG_TIMERDT, 20);

	sizer_text_viewer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *sizer_scroll_horizontal = new wxBoxSizer(wxHORIZONTAL);
	sizer_scroll_horizontal->AddSpacer(mScrollVertical->GetSize().GetWidth());
	sizer_scroll_horizontal->Add(mScrollHorizontal, wxSizerFlags().Expand().Proportion(1));
	wxBoxSizer *sizer_scroll_vertical = new wxBoxSizer(wxHORIZONTAL);
	sizer_scroll_vertical->Add(mScrollVertical, wxSizerFlags().Expand());
	sizer_scroll_vertical->Add(sizer_text_viewer, wxSizerFlags().Expand().Proportion(1));
	wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->Add(sizer_scroll_horizontal, wxSizerFlags().Expand());
	topSizer->Add(sizer_scroll_vertical, wxSizerFlags().Expand().Proportion(1));
	SetSizer(topSizer);

	image_right.SetWidth(0);
	
	// start to propose something to the user ( wizard, bug managmnt, new hardware config .. )
	mtimer = new wxTimer(this, ID_MAIN_TIMER);
	mtimer->Start(500);
}
Expresseur::~Expresseur()
{
	checkUpdate();

	musicxmlscore::cleanCache(mConf->get(CONFIG_DAYCACHE, 15));
	
	if ( fileHistory)
		fileHistory->Save(*mConf->getConfig());

	if (listName.IsFileReadable())
		mConf->set(CONFIG_LISTNAME, listName.GetFullPath());
	else
		mConf->remove(CONFIG_LISTNAME);

	if (fileName.IsFileReadable())
		mConf->set(CONFIG_FILENAME, fileName.GetFullPath());
	else
		mConf->remove(CONFIG_FILENAME);

	mConf->set(CONFIG_MAINMAXIMIZED, IsMaximized());
	if (!IsMaximized())
	{
		wxSize msize = GetSize() ;
		mConf->set(CONFIG_MAINWIDTH, msize.GetWidth());
		mConf->set(CONFIG_MAINHEIGHT, msize.GetHeight());
	}

	mConf->set(CONFIG_MAIN_SCROLLHORIZONTAL, posScrollHorizontal);
	mConf->set(CONFIG_MAIN_SCROLLVERTICAL, posScrollVertical);

	mConf->set(CONFIG_LOCALOFF, localoff);

	preClose();

	delete fileHistory;

	delete mConf;

	basslua_close();
}
wxString Expresseur::checkFile(wxString dir, wxString fullName)
{
	wxFileName f;
	f.AssignDir(dir);
	f.SetFullName(fullName);
	if ( ! f.FileExists())
		return (f.GetFullPath() + " does not exist\n") ;
	if ( ! f.IsFileReadable())
		return (f.GetFullPath() + " exists but not readable\n") ;
	return wxEmptyString ;
}
bool Expresseur::checkConfig()
{
	wxString merrors ;
	merrors += checkFile(mxconf::getCwdDir(), "test.wav");
	merrors += checkFile(mxconf::getCwdDir(), "scan_position.qml");
	merrors += checkFile(mxconf::getCwdDir(), "wizard_audio.jpg");
	merrors += checkFile(mxconf::getCwdDir(), "all_note_off.png");
	merrors += checkFile(mxconf::getCwdDir(), "expresscmd.lua");
	merrors += checkFile(mxconf::getCwdDir(), "luachord.lua");
	merrors += checkFile(mxconf::getCwdDir(), "luascore.lua");
	merrors += checkFile(mxconf::getCwdDir(), "texttochord.lua");
	merrors += checkFile(mxconf::getCwdDir(), "expresseur.lua");
	merrors += checkFile(mxconf::getResourceDir(), "default_piano.sf2");
	merrors += checkFile(mxconf::getResourceDir(), "guitare.sf2");
	merrors += checkFile(mxconf::getResourceDir(), "default_piano.txt");
	merrors += checkFile(mxconf::getUserDir(), "A_la_claire_fontaine.txt");
	merrors += checkFile(mxconf::getUserDir(), "A_la_claire_fontaine.mxl");
	merrors += checkFile(mxconf::getUserDir(), "fairy_chords.txb");
	merrors += checkFile(mxconf::getUserDir(), "fairy_chords.txt");
	merrors += checkFile(mxconf::getUserDir(), "fairy_chords.png");
	wxString msg;
	bool ret = true;
	if (merrors.IsEmpty())
	{
		msg = "check files : OK\n";
	}
	else
	{
		msg = "check files : FAIL\n";
		msg += merrors;
		ret = false;
	}
	msg += "\n";
	int nbIn = nameValideMidiInDevices.GetCount();
	if (nbIn == 0)
	{
		msg += "No valid MIDI-in\n";
	}
	else
	{
		wxString sv;
		sv.Printf("%d valid Midi-in\n", nbIn);
		msg += sv;
	}
	for (int i = 0; i < nbIn; i++)
	{
		if (nameOpenMidiInDevices.Index(nameValideMidiInDevices[i]) == wxNOT_FOUND)
			msg += "    - " + nameValideMidiInDevices[i] + " : NOT opened (cf. MIDI configuration)\n";
		else
			msg += "    - " + nameValideMidiInDevices[i] + " : opened\n";
	}
	msg += "\n";
	int nbOut = nameValideMidiOutDevices.GetCount();
	if (nbOut == 0)
	{
		msg += "No valid MIDI-out\n";
	}
	else
	{
		wxString sv;
		sv.Printf("%d valid Midi-out\n", nbOut);
		msg += sv;
	}
	for (int i = 0; i < nbOut ; i++)
	{
		if (nameOpenMidiOutDevices.Index(nameValideMidiOutDevices[i]) == wxNOT_FOUND)
			msg += "    - " + nameValideMidiOutDevices[i] + " : NOT opened (cf. MIDI configuration)\n";
		else
			msg += "    - " + nameValideMidiOutDevices[i] + " : opened\n";
	}
	msg += "\n";

	int nbAudio = getListAudio();
	if (nbAudio == 0)
	{
		msg += "No valid Audio output\n";
	}
	else
	{
		wxString sv;
		sv.Printf("%d valid Audio output\n", nbAudio);
		msg += sv;
	}
	for (unsigned int i = 0; i < nameaudioDevices.GetCount(); i++)
	{
		if (nameDefaultaudioDevices == nameaudioDevices[i])
			msg += "    - " + nameaudioDevices[i] + " : DEFAULT sound output\n";
		else
			msg += "    - " + nameaudioDevices[i] + "\n";
	}

	msg += "\nConf=" + mConf->getConfPath() + "\n";
	msg += "Working directory=" + mConf->getCwdDir() + "\n";
	msg += "User directory=" + mConf->getUserDir() + "\n";
	msg += "Temp directory=" + mConf->getTmpDir() + "\n";
	msg += "App directory=" + mConf->getAppDir() + "\n";

	wxMessageBox(msg,"Config check");
	return ret ;
}
void Expresseur::OnCheckConfig(wxCommandEvent& WXUNUSED(event))
{
	checkConfig();
}
void Expresseur::OnHelpluashortcut(wxCommandEvent& WXUNUSED(event))
{
	wxLaunchDefaultBrowser("http://expresseur.com/home/user-guide/#keydown");
}
void Expresseur::preClose()
{
	if (mtimer)
	{
		mtimer->Stop();
		delete mtimer;
	}
	mtimer = NULL;

	if (mExpression)
	{
		mExpression->savePos();
		mConf->set(CONFIG_EXPRESSIONVISIBLE, (mExpression->IsVisible()));
		delete mExpression;
	}
	mExpression = NULL;
	if (mMidishortcut)
	{
		mMidishortcut->savePos();
		delete mMidishortcut;
	}
	mMidishortcut = NULL;
	if (mMixer)
	{
		mMixer->savePos();
		mConf->set(CONFIG_MIXERVISIBLE, (mMixer->IsVisible()));
		delete mMixer;
	}
	mMixer = NULL;
	if (mLog)
		delete mLog;
	mLog = NULL;
	if (mTextscore)	
		delete mTextscore;
	mTextscore = NULL;
	if ( mViewerscore)
		delete mViewerscore;
	mViewerscore=NULL;
}
void Expresseur::postInit()
{
	//preClose();
	
	// resize the main frame
	//bool tobeMaximized = false;
	sizeFrame.SetWidth(mConf->get(CONFIG_MAINWIDTH, 1010) );
	sizeFrame.SetHeight(mConf->get(CONFIG_MAINHEIGHT, 780) );
	//tobeMaximized = mConf->get(CONFIG_MAINMAXIMIZED, false);
	if (sizeFrame.GetWidth() < 600)
		sizeFrame.SetWidth(600);
	if (sizeFrame.GetHeight() < 400)
		sizeFrame.SetHeight(400);
	wxRect sizeToSet;
	sizeToSet.SetWidth(sizeFrame.GetWidth() - mConf->get(CONFIG_MAINDELTAWIDTH, 0));
	sizeToSet.SetHeight(sizeFrame.GetHeight() - mConf->get(CONFIG_MAINDELTAHEIGHT, 0));
	frame->SetSize(sizeToSet);
	

	//if (tobeMaximized)
	//	frame->Maximize(true);
	//else
	frame->Maximize(false);
	//frame->CenterOnScreen();
	frame->Show(true);

	// check if it the first use ( for intialization, wizard, .. )
	initFirstUse(false);

	// read the list of scores
	listName.Assign(mConf->get(CONFIG_LISTNAME, ""));
	if (listName.IsFileReadable())
		ListOpen();

	fileName.Assign(mConf->get(CONFIG_FILENAME, ""));
	// text for the score
	if (mTextscore)
		delete mTextscore;
	mTextscore = new textscore(this, ID_MAIN_TEXT_SONG, mConf);
	mTextscore->SetMinSize(wxSize(0, 0));
	// empty viewer for the score
	if (mViewerscore)
		delete mViewerscore;
	mViewerscore = new emptyscore(this, wxID_ANY, mConf);
	setOrientation(posScrollVertical, posScrollHorizontal);
	FileOpen(true);
}
void Expresseur::setRightDisplay(bool right)
{
	int nbItem = sizer_text_viewer->GetItemCount();
	while (nbItem > 0)
	{
		sizer_text_viewer->Remove(0);
		nbItem = sizer_text_viewer->GetItemCount();
	}
	if ( right )
	{
		sizer_A = sizer_text_viewer->Add(mViewerscore, wxSizerFlags().Expand().Proportion(50));
		sizer_B = sizer_text_viewer->Add(mTextscore, wxSizerFlags().Expand().Proportion(50));
	}
	else
	{
		sizer_A = sizer_text_viewer->Add(mTextscore, wxSizerFlags().Expand().Proportion(50));
		sizer_B = sizer_text_viewer->Add(mViewerscore, wxSizerFlags().Expand().Proportion(50));
	}
}
void Expresseur::setOrientation(int v, int h )
{
	if (v == 50)
	{
		if ( h < 20 )
			h = 20 ;
		if ( h > 80 )
			h = 80 ;
		sizer_text_viewer->SetOrientation(wxHORIZONTAL);
		setRightDisplay(h > 50);
		sizer_A->SetProportion(h);
		sizer_B->SetProportion(100 - h);
	}
	else
	{
		if ( v < 20 )
			v = 20 ;
		if ( v  > 80 )
			v = 80 ;
		sizer_text_viewer->SetOrientation(wxVERTICAL);
		setRightDisplay(v > 50);
		sizer_A->SetProportion(v);
		sizer_B->SetProportion(100 - v);
	}
}
void Expresseur::OnHorizontalScroll(wxScrollEvent& event)
{
	posScrollHorizontal = event.GetPosition();
	if (posScrollHorizontal == 50) posScrollHorizontal = 51;
	posScrollVertical = 50;
	mScrollVertical->SetThumbPosition(50);
	setOrientation(posScrollVertical, posScrollHorizontal);
	waitToRefresh = periodRefresh / timerDt;
	Layout();
	mTextscore->SetFocus();
}
void Expresseur::OnVerticalScroll(wxScrollEvent& event)
{
	posScrollVertical = event.GetPosition();
	if (posScrollVertical == 50) posScrollVertical = 51;
	posScrollHorizontal = 50;
	mScrollHorizontal->SetThumbPosition(50);
	setOrientation(posScrollVertical, posScrollHorizontal);
	waitToRefresh = periodRefresh / timerDt;
	Layout();
	mTextscore->SetFocus();
}
void Expresseur::OnSize(wxSizeEvent& WXUNUSED(event))
{
	waitToRefresh = periodRefresh / timerDt;
	Layout();
}
bool Expresseur::OnKeyDown(wxKeyEvent& event)
{
	if (nbkeydown >= 0)
	{
		// stock les evenements claviers, pour un éventuel réglage du clavier, pendant un OnKeydowInfoLua
		keyboardDisposal[nbkeydown] = event.GetKeyCode();
		nbkeydown++;
		wxString s;
		s.sprintf("Register key#=%d/%d : keycode=%d", nbkeydown, MAXKEYBOARDDISPOSAL, event.GetKeyCode());
		SetStatusText(s, 1);
		if (nbkeydown == MAXKEYBOARDDISPOSAL)
		{
			// record the last MAXKEYBOARDDISPOSAL as the config of the 4 lines of 10 keys of the keyboard
			wxString sDisposal;
			for (unsigned int i = 0; i < MAXKEYBOARDDISPOSAL; i++)
			{
				sDisposal.sprintf("%s%d|", sDisposal, keyboardDisposal[i]);
			}
			mConf->set(CONFIG_KEYBOARDCONFIG, sDisposal);
			settingReset(true);
			nbkeydown = -1;
			SetStatusText("Keyboard config is done", 1);
		}
		return true;
	}

	if (editMode)
		return false;
	bool ret = false;
	int modifiers = event.GetModifiers();
	basslua_call(moduleGlobal, "keydown", "iii>b", event.GetKeyCode(), modifiers, mode , &ret);
	return ret;
}
void Expresseur::OnIdle(wxIdleEvent& evt)
{ 
	static int endCalledBack = 0;
	if (firstTimer) return ;

	switch (mode)
	{
	case modeChord: 
		if (mTextscore == NULL) 
			return; 
		else break;
	case modeScore:	
		if ( mViewerscore == NULL ) 
			return ; 
		else 
			break;
	default : return;
	}


	int nr_device , type_msg , channel , value1 , value2 ;
	bool isProcessed , oneIsProcessed ;
	wxLongLong time ;
	bool calledBack = luafile::isCalledback(&time, &nr_device, &type_msg, &channel, &value1, &value2, &isProcessed , &oneIsProcessed);
	switch (mode)
	{
	case modeChord:
	{
		// compile the text of chords
		if (waitToCompile < 1 )
		{
			mTextscore->compileText();
			waitToCompile = periodCompile / timerDt;
		}


		if (!editMode)
		{
			// scan the current position given by LUA module, according to MID events
			int nrChord = mTextscore->scanPosition();
			mViewerscore->setPosition(nrChord, true);
			mTextscore->scanTextPosition();
		}
		break;
	}
	case modeScore:
	{
		if (playback)
		{
			if (!(((musicxmlscore *)(mViewerscore))->playback()))
			{
				editMenu->Check(ID_MAIN_PLAYBACK, false);
				playback = false;
			}
		}

		if ( recordPlayback && calledBack )
		{
			do {
				((musicxmlscore *)(mViewerscore))->recordPlayback(time, nr_device, type_msg, channel, value1, value2);
			} while ( luafile::isCalledback(&time, &nr_device, &type_msg, &channel, &value1, &value2, &isProcessed , &oneIsProcessed) );
		}

		int nrEvent, playing;
		basslua_call(moduleScore, functionScoreGetPosition, ">ii", &nrEvent, &playing);
		mViewerscore->setPosition(nrEvent - 1, (playing>0) );
	}
	break;
	default:
		break;
	}

	if (calledBack)
	{
		if (mMidishortcut)
			mMidishortcut->scanMidi(nr_device, type_msg, channel, value1, value2); // scan midi-in events for the potential shortcuts needed in the definition of the GUI
		if (mLog && (mLog->IsVisible()))
			mLog->scanLog(); // updates any log from LUA to the window of this GUI
		endCalledBack = 10000;
		/*
		switch (type_msg)
		{
		case NOTEON:
			if (value2 > 0)
			{
				if (isProcessed)
					SetStatusText("note on !", 1);
				else
					SetStatusText("note on ?", 1);
				break;
			}
		case NOTEOFF:
			if (isProcessed)
				SetStatusText("note off !", 1);
			else
				SetStatusText("note off ?", 1);
			break;
		case PROGRAM:
			if (isProcessed)
				SetStatusText("program !", 1);
			else
				SetStatusText("program ?", 1);
			break;
		case CONTROL:
			if (isProcessed)
				SetStatusText("control", 1);
			break;
		default:
			if (isProcessed)
				SetStatusText("midi msg", 1);
			break;
		}
		*/
	}
	if (endCalledBack)
	{
		endCalledBack--;
		if (endCalledBack == 0)
			SetStatusText("", 1);
	}

	if ( waitToRefresh < 1 )
	{
		waitToRefresh = periodRefresh / timerDt;
		// scan pendings useful events from LUA
		if (mMixer && (mMixer->IsVisible()))
			mMixer->scanVolume(); // updates the volume of the mixer of thuis GUI, with the values in LUA
		if (mExpression && (mExpression->IsVisible()))
			mExpression->scanValue(); // updates the values of the "expression" of thuis GUI, with the values in LUA

		// scan if the LUA status text has been changed
		char ch[1024];
		if ((basslua_table(moduleGlobal, tableInfo, -1, fieldAction, ch, NULL, tableGetKeyValue | tableNilKeyValue) & tableGetKeyValue) == tableGetKeyValue)
		{
			switch (ch[0])
			{
			case '0': ListSelect(0);  break;
			case '#': ListSelect(-1);  break;
			case '+': ListSelectNext(1);  break;
			case '-': ListSelectNext(-1); break;
			case '!': wxMessageBox(ch + 1, "LUA message"); break;
			case '=': selectPlayview(ch + 1); break;
			case '@': 
				if (mode == modeScore) 
				{ 
					editMode = true ;
					mViewerscore->gotoPosition(ch + 1) ;
					editMode = false ;
				}
				break;
			default: SetStatusText(ch, 1); break;
			}
		}
		if ((basslua_table(moduleGlobal, tableInfo, -1, fieldStatus, ch, NULL, tableGetKeyValue | tableNilKeyValue) & tableGetKeyValue) == tableGetKeyValue)
		{
			wxString sch(ch);
			if (sch.StartsWith("1"))
				SetStatusText(sch.Mid(1),0);
			else if (sch.StartsWith("2"))
				SetStatusText(sch.Mid(1),1);
			else SetStatusText(sch,1);

		}

		if (image_right != mViewerscore->GetClientSize())
		{
			if ( image_right.GetWidth() == 0)
			{
				wxSize sizeResult = GetSize() ;
				if (( sizeResult.GetWidth() != sizeFrame.GetWidth()) || ( sizeResult.GetHeight() != sizeFrame.GetHeight()))
				{
					mConf->set(CONFIG_MAINDELTAWIDTH, sizeResult.GetWidth() - sizeFrame.GetWidth());
					mConf->set(CONFIG_MAINDELTAHEIGHT, sizeResult.GetHeight() - sizeFrame.GetHeight());
					frame->SetSize(sizeFrame);
				}
			}
			Layout();
			image_right = mViewerscore->GetClientSize();
			mViewerscore->displayFile(image_right);
		}
	}

	evt.RequestMore(); 
}
void Expresseur::OnTimer(wxTimerEvent& WXUNUSED(event))
{
	if (firstTimer)
	{

		firstTimer = false;
		postInit();
		return;
	}
	// trigger external timer for luabass and basslua
	basslua_external_timer();
	waitToCompile--;
	waitToRefresh--;
}
void Expresseur::setWindowsTitle()
{
	SetTitle(APP_NAME << wxString(" - ") << fileName.GetFullName()); // Set the Title to reflect the file open

}
void Expresseur::FileOpen(bool all)
{
	if (fileName.IsFileReadable())
	{
		if (settingReset(all))
		{
			fileHistory->AddFileToHistory(fileName.GetFullPath());
			ListCheck();
		}
	}
	setWindowsTitle();
}
void Expresseur::FileSave()
{
	fileName.SetExt(SUFFIXE_TEXT);
	mTextscore->saveFile(fileName);
	fileHistory->AddFileToHistory(fileName.GetFullPath());
	setWindowsTitle();
	editMode = false;
	menuEditMode->Check(false);
	switch (mode)
	{
	case modeScore:
	  settingReset(false);
	  break;
	default:
		break;
	}
}
void Expresseur::getLuaAction(bool all, wxMenu *newActionMenu)
{
	wxBitmap b;
	wxFileName fb;
	fb.Assign(mxconf::getCwdDir());
	fb.SetExt("png");

	wxString s;
	nameAction.Clear();
	char name[512];
	int nrAction = 0;
	while (basslua_table(moduleGlobal, tableActions, nrAction, fieldName, name, NULL, tableGetKeyValue) == tableGetKeyValue)
	{
		nameAction.Add(name);
		char shortcut[64] = "";
		char help[512] = "";
		basslua_table(moduleGlobal, tableActions, nrAction, fieldShortcut, shortcut, NULL, tableGetKeyValue);
		basslua_table(moduleGlobal, tableActions, nrAction, fieldHelp, help, NULL, tableGetKeyValue);
		if (strlen(shortcut) != 0)
		{
			if ((editMode) || (strlen(shortcut) == 0))
				s.Printf("%s", _(name));
			else
				s.Printf("%s\t%s", _(name), shortcut);
			if (newActionMenu != NULL)
				newActionMenu->Append(ID_MAIN_ACTION + nrAction, s, _(help));
		}
		if (all)
		{
			char icone[64] = "";
			basslua_table(moduleGlobal, tableActions, nrAction, fieldIcone, icone, NULL, tableGetKeyValue);
			fb.SetName(icone);
			s.Printf("%s %s", _(name), shortcut);
			if (fb.IsFileReadable())
			{
				if (b.LoadFile(fb.GetFullPath(), wxBITMAP_TYPE_PNG))
				{
					toolBar->AddTool(ID_MAIN_ACTION + nrAction, _(name), b, _(help));
				}
			}
		}
		nrAction++;
	}
}
void Expresseur::getShortcutAction(wxMenu *newActionMenu)
{
	if (newActionMenu == NULL)
		return ;
	if (mMidishortcut == NULL)
		return;

	// get the list of shortcuts : name+ALT+key
	wxArrayString ls = mMidishortcut->getShortcuts();
	for (unsigned int nrSelector = 0; nrSelector < ls.GetCount(); nrSelector++)
	{
		wxString s = ls[nrSelector];
		newActionMenu->Append(ID_MAIN_KEY_SHORTCUT + nrSelector, s);
	}
}
void Expresseur::SetMenuAction(bool all)
{
	// create the actionMenu and the associated icones in the toolbar, according to actions known in the LUA script
	mTextscore->setEditMode(editMode);
	wxString s;
	wxMenu *newActionMenu = new wxMenu;
	wxBitmap b;
	wxFileName fb;
	fb.Assign(mxconf::getCwdDir());
	fb.SetExt("png");

	if (all)
	{
		fb.SetName("exit");
		int w = 15;
		int h = 15;
		s = fb.GetFullPath();
		if (fb.IsFileReadable())
		{
			if (b.LoadFile(fb.GetFullPath(), wxBITMAP_TYPE_PNG))
			{
				w = b.GetWidth();
				h = b.GetHeight();
			}
		}
		toolBar->ClearTools();
		toolBar->SetToolBitmapSize(wxSize(w, h));
		fb.SetName("open");
		if (fb.IsFileReadable())
			if (b.LoadFile(fb.GetFullPath(), wxBITMAP_TYPE_PNG))
				toolBar->AddTool(wxID_OPEN, _("Open..."), b, _("Open..."));
		fb.SetName("save");
		if (fb.IsFileReadable())
			if (b.LoadFile(fb.GetFullPath(), wxBITMAP_TYPE_PNG))
				toolBar->AddTool(wxID_SAVE, _("Save"), b, _("Save"));
		fb.SetName("edit");
		if (fb.IsFileReadable())
			if (b.LoadFile(fb.GetFullPath(), wxBITMAP_TYPE_PNG))
				toolBar->AddTool(wxID_EDIT, _("Edit"), b, _("edit mode"), wxITEM_CHECK);
		fb.SetName("mixer");
		if (fb.IsFileReadable())
			if (b.LoadFile(fb.GetFullPath(), wxBITMAP_TYPE_PNG))
				toolBar->AddTool(ID_MAIN_MIXER, _("Mixer"), b, _("Mixer"));
		fb.SetName("help");
		if (fb.IsFileReadable())
			if (b.LoadFile(fb.GetFullPath(), wxBITMAP_TYPE_PNG))
				toolBar->AddTool(wxID_HELP, _("Help"), b, _("Help"));
		fb.SetName("exit");
		if (fb.IsFileReadable())
			if (b.LoadFile(fb.GetFullPath(), wxBITMAP_TYPE_PNG))
				toolBar->AddTool(wxID_EXIT, _("Exit"), b, _("Exit"));
		toolBar->AddSeparator();
		fb.SetName("goto");
		if (fb.IsFileReadable())
			if (b.LoadFile(fb.GetFullPath(), wxBITMAP_TYPE_PNG))
				toolBar->AddTool(ID_MAIN_GOTO, _("Goto"), b, _("Goto measure number"));
		toolBar->AddSeparator();
	}

	if (toolBar->FindById(wxID_EDIT))
		toolBar->ToggleTool(wxID_EDIT, editMode);
	editMenu->Check(wxID_EDIT, editMode);

	getLuaAction(all, newActionMenu);
	newActionMenu->AppendSeparator();
	getShortcutAction(newActionMenu);

	newActionMenu->AppendSeparator();
	newActionMenu->Append(ID_MAIN_HELP_LUASHORTCUT, _("One-key shortcuts diagram"), _("link to web page for default configuraton of the One-key shortcuts"));

	if ( all )
		toolBar->Realize();

	wxMenu *oldactionMenu = mMenuBar->Replace(2, newActionMenu, _("Action"));
	delete oldactionMenu;

	mTextscore->SetEditable(editMode);
	if (editMode == false)
	{
		mTextscore->SelectNone();
		//mTextscore->HideNativeCaret();;
	}
	/*
	else
	{
		mTextscore->ShowNativeCaret(true);;
	}
	*/
}
void Expresseur::OnMenuAction(wxCommandEvent& event)
{
	int nrAction = event.GetId() - ID_MAIN_ACTION;
	if (basslua_table(moduleGlobal, tableActions, nrAction, fieldCallFunction, NULL, NULL, tableCallKeyFunction) != tableCallKeyFunction)
	{
		if (basslua_table(moduleGlobal, tableActions, nrAction, (mode == modeChord) ? fieldCallChord : fieldCallScore, NULL, NULL, tableCallKeyFunction) == tableCallKeyFunction)
			luafile::functioncallback(NULL_INT, NULL_INT, NULL_INT, NULL_INT, NULL_INT, NULL_INT, true);
		else
		{
			wxString s;
			s.Printf(_("Error calling LUA action %d : %s "), nrAction + 1, nameAction[nrAction]);
			SetStatusText(s, 0);
		}
	}
}
void Expresseur::OnMenuShortcut(wxCommandEvent& event)
{
	int nrShortcut = event.GetId() - ID_MAIN_KEY_SHORTCUT;
	if (mMidishortcut == NULL)
		return ;
	mMidishortcut->onShortcut(nrShortcut);
}
void Expresseur::OnMenuSettings(wxCommandEvent& event)
{
	unsigned int nrSetting = event.GetId() - ID_MAIN_SETTINGS_FILE;
	if ((nrSetting < 0 ) || (nrSetting >= listSettings.GetCount()))
		return;
	wxString s = listSettings[nrSetting] ;
	if ( ! s.Contains("|"))
		return ;
	wxString f = s.Left(s.Find('|'));
	wxFileName fn ;
	fn.AssignDir(mxconf::getResourceDir());
	fn.SetName(f);
	fn.SetExt("txt");
	settingName.Assign(fn.GetFullPath());
	settingOpen();
	settingReset(false);
}
void Expresseur::OnRecordPlayback(wxCommandEvent& event)
{
	// start recording for future playback
	bool new_recordPlayback = event.GetInt();
	if (new_recordPlayback != recordPlayback)
	{
		if (new_recordPlayback)
		{
			editMenu->Check(ID_MAIN_PLAYBACK, false);
			if (basslua_call(moduleScore, "firstPart", ""))
				luafile::functioncallback(NULL_INT, NULL_INT, NULL_INT, NULL_INT, NULL_INT, NULL_INT, true);
			((musicxmlscore *)(mViewerscore))->initRecordPlayback();
		}
	}
	recordPlayback = new_recordPlayback;
}
void Expresseur::OnSavePlayback(wxCommandEvent& WXUNUSED(event))
{
	wxString f = ((musicxmlscore *)(mViewerscore))->getPlayback();
	mTextscore->savePlayback(f);
}
void Expresseur::OnPlayback(wxCommandEvent& event)
{
	// playback
	bool new_playback = event.GetInt() ;
	if ( new_playback != playback )
	{
		if ( new_playback )
		{
			editMenu->Check(ID_MAIN_RECORD_PLAYBACK,false);
			if (basslua_call(moduleScore,"firstPart", ""))
				luafile::functioncallback(NULL_INT, NULL_INT, NULL_INT, NULL_INT, NULL_INT, NULL_INT, true);
			((musicxmlscore *)(mViewerscore))->initPlayback();
		}
	}
	playback = new_playback ;
}
/*
void Expresseur::OnNew(wxCommandEvent& WXUNUSED(event)) 
{
	if (mTextscore->needToSave())
	{
		if (wxMessageBox(_("Current file has not been saved! Proceed?"), _("File modified"),
			wxICON_QUESTION | wxYES_NO, this) == wxNO)
			return;
	}
	mTextscore->SetValue("");
	fileName.SetName("newsong");
	fileName.SetExt(SUFFIXE_TEXT);
	wxTextFile tfile;
	if (fileName.IsFileWritable() == false)
		tfile.Create(fileName.GetFullPath());
	tfile.Write();
	tfile.Close();
	Open(fileName.GetFullPath());
	mTextscore->noNeedToSave();
}
*/
void Expresseur::OnOpen(wxCommandEvent& WXUNUSED(event)) 
{
	if (mTextscore->needToSave())
	{
		if (wxMessageBox(_("Current file has not been saved! Proceed?"), _("File modified"),
			wxICON_QUESTION | wxYES_NO, this) == wxNO)
			return;
	}
	wxFileDialog
		openFileDialog(this, _("Open file"), "", "",
		"music file (*.txt;*.png;*.xml;*.mxl)|*.txt;*.png;*.xml;*.mxl", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return; // the user changed idea...
	Open(openFileDialog.GetPath());
	mTextscore->noNeedToSave();
}
void Expresseur::OnSave(wxCommandEvent& WXUNUSED(event)) 
{
	if (fileName.IsFileWritable() == false)
	{
		wxFileDialog
			openFileDialog(this, _("Save file"), "", "",
			"song file (*.txt)|*.txt", wxFD_SAVE);
		if (openFileDialog.ShowModal() == wxID_CANCEL)
			return; // the user changed idea...
		fileName.Assign(openFileDialog.GetPath());
	}
	FileSave();
}
void Expresseur::OnSaveas(wxCommandEvent& WXUNUSED(event)) 
{
	wxFileDialog
		openFileDialog(this, _("Save file"), "", "",
		"song file (*.txt)|*.txt", wxFD_SAVE);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return; // the user changed idea...
	fileName.Assign(openFileDialog.GetPath());
	FileSave();
}
void Expresseur::OnExit(wxCommandEvent& WXUNUSED(event)) 
{
	if (mTextscore->needToSave())
	{
		if (wxMessageBox(_("Current file has not been saved! Proceed?"), _("File modified"),
			wxICON_QUESTION | wxYES_NO, this) == wxNO)
			return;
	}
	Close(true);
}
void Expresseur::OnUndo(wxCommandEvent& WXUNUSED(event)) 
{
	mTextscore->Undo();
}
void Expresseur::OnRedo(wxCommandEvent& WXUNUSED(event)) 
{
	mTextscore->Redo();
}
void Expresseur::OnCopy(wxCommandEvent& WXUNUSED(event)) 
{
	mTextscore->Copy();
}
void Expresseur::OnCut(wxCommandEvent& WXUNUSED(event)) 
{
	mTextscore->Cut();
}
void Expresseur::OnPaste(wxCommandEvent& WXUNUSED(event))
{
	mTextscore->Paste();
}
void Expresseur::setZoom()
{
	if (mViewerscore != NULL)
	{
		int zoom;
		if (typeViewer == MUSICXMLVIEWER)
		{
			zoom = mConf->get(CONFIG_ZOOM_MUSICXML, 0);
			//mTextscore->zoom(0);
			mViewerscore->zoom(zoom);
		}
		else
		{
			zoom = mConf->get(CONFIG_ZOOM_TEXT, 0);
			mTextscore->zoom(zoom);
			//mViewerscore->zoom(0);
		}
		wxMenuItem *mmenuItem = zoomMenu->FindChildItem(zoom + ID_MAIN_ZOOM_0);
		if (mmenuItem)
			mmenuItem->Check();
	}

}
void Expresseur::OnZoom(wxCommandEvent& event)
{
	int zoom = event.GetId() - ID_MAIN_ZOOM_0;
	if (typeViewer == MUSICXMLVIEWER)
		mConf->set(CONFIG_ZOOM_MUSICXML, zoom);
	else
		mConf->set(CONFIG_ZOOM_TEXT, zoom);
	setZoom();

	// mlog_in("Expresseur / OnZoom / : displayFile");
	mViewerscore->displayFile(mViewerscore->GetClientSize());
}
void Expresseur::setPlayView(wxString s)
{
	if (((musicxmlscore*)mViewerscore)->setPlayVisible(s))
	{
		mTextscore->setFile(fileName);
		fileName.SetExt(SUFFIXE_TEXT);
		mViewerscore->setFile(fileName);
		mViewerscore->displayFile(mViewerscore->GetClientSize());
	}
}
void Expresseur::OnPlayviewSolo(wxCommandEvent& event)
{
	if (mode != modeScore)
		return;
	int tracknr = event.GetId() - ID_MAIN_SOLO1;
	wxString s;
	s.Printf("%d/", tracknr + 1);
	setPlayView(s);
}
void Expresseur::OnPlayviewAll(wxCommandEvent& WXUNUSED(event))
{
	if (mode != modeScore)
		return;
	setPlayView("*/");
}
void Expresseur::selectPlayview(wxString s)
{
	wxString ts ;
	if (s.IsEmpty())
	{
		editMode = true;
		wxTextEntryDialog mdialog(NULL, "Tracks to play/view.\n14 : play track#1 & #4\n* : play all tracks\n12/ play and view track #1 & #2\n1/2 play track #1 and view track #2\n/3 view track #3\n/* view all tracks\n*/ play and view all tracks\n+2 change only track #2 as played\n-2 change only track #2 as not played", "Expresseur");
		if (mdialog.ShowModal() == wxID_OK)
		{
			ts = mdialog.GetValue();
		}
		editMode = false;
	}
	else
		ts = s;
	if ( ! ts.IsEmpty() )
		setPlayView(ts);
}
void Expresseur::OnPlayview(wxCommandEvent& WXUNUSED(event))
{
	if (mode != modeScore)
		return;
	selectPlayview("");
}
void Expresseur::OnRecordImpro(wxCommandEvent& WXUNUSED(event))
{
	if (mode != modeChord)
		return;
	basslua_call(moduleChord, "logRecord", "");
	editMenu->Enable(ID_MAIN_SAVE_IMPRO, true);
	editMenu->Enable(ID_MAIN_RECORD_IMPRO, false);
}
void Expresseur::OnSaveImpro(wxCommandEvent& WXUNUSED(event))
{
	if (mode != modeChord)
		return;
	basslua_call(moduleChord, "logRecord", "");
}
void Expresseur::OnOrnamentAddAbsolute(wxCommandEvent& WXUNUSED(event))
{
	ornamentAdd(true);
}
void Expresseur::OnOrnamentAddRelative(wxCommandEvent& WXUNUSED(event))
{
	ornamentAdd(false);
}
void Expresseur::ornamentAdd(bool absolute)
{
	if (mode != modeScore)
		return;
	int absolute_measure_nr, measure_nr, repeatNr , beat, t;
	bool ret = ((musicxmlscore *)(mViewerscore))->getScorePosition(&absolute_measure_nr, &measure_nr, &repeatNr , &beat, &t);
	if (ret)
		return;
	wxArrayString list_ornament = musicxmlcompile::getListOrnament();
	editMode = true;
	wxString ornament = wxGetSingleChoice("Select ornament", "Add ornament", list_ornament, this);
	editMode = false;
	if (ornament.IsEmpty())
		return;
	wxString line;
	wxString measure;
	wxString repeat;
	wxString ti;
	if (absolute)
		measure.Printf("!%d", absolute_measure_nr);
	else
	{
		if ( repeatNr == 0 )
			measure.Printf("%d", measure_nr);
		else
			measure.Printf("%d*%d", measure_nr , repeatNr + 1);
	}
	if (t > 0)
		ti.Printf(".%d", t);
	line.Printf("%s.%d%s%s:%s", measure, beat + 1, ti, repeat, ornament);
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData(new wxTextDataObject(line));
		wxTheClipboard->Close();
		wxMessageBox("You can 'paste' the line in the Score Description", "Add ornament");
	}
	else
	{
		wxMessageBox("Error clipboard ...");
	}
}
void Expresseur::OnEdit(wxCommandEvent& WXUNUSED(event))
{
	editMode = !editMode;
	SetMenuAction(false);
}
void Expresseur::OnLocaloff(wxCommandEvent& WXUNUSED(event))
{
	localoff = !localoff;
}
void Expresseur::OnPreviousPage(wxCommandEvent& WXUNUSED(event))
{
	if (mViewerscore == NULL)
		return;
	mViewerscore->gotoNextPage(false);
}
void Expresseur::OnNextPage(wxCommandEvent& WXUNUSED(event))
{
	if (mViewerscore == NULL)
		return;
	mViewerscore->gotoNextPage(true);
}
void Expresseur::readListSettings()
{
	// reads settings available in resources
	listSettings.Clear() ;
	wxDir dirSettings(mxconf::getResourceDir());
	if ( dirSettings.IsOpened() )
	{
		wxString filename  ;
		bool cont = dirSettings.GetFirst(&filename, "*.txt", wxDIR_FILES);
		while ( cont )
		{
			wxFileName ffilename;
			ffilename.AssignDir(mxconf::getResourceDir());
			ffilename.SetFullName(filename);
			wxTextFile tfile;	
			tfile.Open(ffilename.GetFullPath());
			if (tfile.IsOpened())
			{
				wxString str = tfile.GetFirstLine(); // must contain CONFIG-FILE
				if (str == CONFIG_FILE)
				{
					str = tfile.GetNextLine(); // contain a comment about the usage
					if (str.StartsWith("--mode score") || str.StartsWith("--mode improvisation"))
					{
						str = tfile.GetNextLine(); // contain mode score|improvisation
					}
					if (str.StartsWith("--"))
					{
						wxString sf ;
						sf = ffilename.GetName() + "|" + str.Mid(2) ;
						listSettings.Add(sf);
					}
				}
				tfile.Close();
			}
			cont = dirSettings.GetNext(&filename);
		}
	}
}
void Expresseur::ListClearMenu()
{
	for (unsigned int i = ID_MAIN_LIST_FILE; i < ID_MAIN_LIST_FILE_END; i++)
	{
		if (listMenu->FindItem(i) != NULL)
			listMenu->Delete(i);
	}
}
void Expresseur::ListUpdateMenu()
{
	ListClearMenu();
	wxFileName f;
	for (int i = listFiles.Count() - 1; i >= 0; i--)
	{
		f.Assign(listFiles.Item(i));
		wxMenuItem *mfilelist = listMenu->PrependCheckItem(ID_MAIN_LIST_FILE + i, f.GetFullName() , f.GetFullPath() );
		mfilelist->Check(false);
		wxString slabel ;
		if ( i < 9 )
			slabel.Printf("%s\tCTRL+%d\n",f.GetFullName(), i + 1 );
		else if ( i == 9 )
			slabel.Printf("%s\tCTRL+%d\n",f.GetFullName() , 0);
		else if ( i < 19 )
			slabel.Printf("%s\tSHIFT+CTRL+%d\n",f.GetFullName() , i + 1 - 10 );
		else if ( i == 19 )
			slabel.Printf("%s\tSHIFT+CTRL+%d\n",f.GetFullName(), 0);
		if ( slabel.length() > 0 )
			mfilelist->SetItemLabel(slabel);
	}
}
void Expresseur::ListSave()
{
	wxString s;
	wxFileName f;
	wxTextFile tfile;
	if (listName.IsFileWritable() == false)
		tfile.Create(listName.GetFullPath());
	fileHistory->AddFileToHistory(listName.GetFullPath());
	tfile.Open(listName.GetFullPath());
	if (tfile.IsOpened() == false)
		return;
	tfile.Clear();
	tfile.AddLine(LIST_FILE);
	for (unsigned int i = 0; i < listFiles.Count(); i++)
	{
		f.Assign(listFiles.Item(i));
		f.MakeRelativeTo(listName.GetPath());
		tfile.AddLine(f.GetFullPath());
	}
	tfile.Write();
	tfile.Close();
	listChanged = false;
	fileHistory->AddFileToHistory(listName.GetFullPath());
}
void Expresseur::ListOpen()
{
	wxString        str;
	wxFileName f;
	// open the file
	wxTextFile      tfile;
	if (listName.IsFileReadable() == false)
		return;
	tfile.Open(listName.GetFullPath());
	if (tfile.IsOpened() == false)
	{
		return;
	}
	str = tfile.GetFirstLine();
	if (str != LIST_FILE)
	{
		wxMessageBox(_("This file is not a list"),_("Open list"));
		tfile.Close();
		return;
	}
	fileHistory->AddFileToHistory(listName.GetFullPath());
	str = tfile.GetNextLine();
	while (!tfile.Eof())
	{
		if (str.IsEmpty() == false)
		{
			f.Assign(str);
			f.MakeAbsolute(listName.GetPath());
			listFiles.Add(f.GetFullPath());
		}
		str = tfile.GetNextLine();
	}
	tfile.Close();
	ListUpdateMenu();
	listSelection = -1;
	ListCheck();
}
void Expresseur::ListNew()
{
	ListClearMenu();
	listFiles.Clear();
	listName.Clear();

}
void Expresseur::ListCheck()
{
	wxFileName f;
	for (int i = listFiles.Count() - 1; i >= 0; i--)
	{
		f.Assign(listFiles.Item(i));
		if (f.GetFullPath() == fileName.GetFullPath())
		{
			listMenu->Check(ID_MAIN_LIST_FILE + i, true);
			listSelection = i;
		}
		else
		{
			listMenu->Check(ID_MAIN_LIST_FILE + i, false);
		}
	}

}
void Expresseur::ListSelectNext(int df)
{
	if (listSelection == -1)
	{
		if (df > 0)
			ListSelect(0);
		else
			ListSelect(listFiles.Count() - 1);
		return;
	}
	ListSelect(listSelection + df);
}
void Expresseur::ListSelect(int id)
{
	int nrfile;
	if ((int)(listFiles.Count()) == 0)
		return;
	nrfile = id;
	if (id < 0)
		nrfile = (int)(listFiles.Count()) - 1;
	if (id >= (int)(listFiles.Count()))
		nrfile = 0;
	Open(listFiles.Item(nrfile));
	listSelection = nrfile;
	for (int i = listFiles.Count() - 1; i >= 0; i--)
	{
		listMenu->Check(ID_MAIN_LIST_FILE + i, (i == nrfile));
	}
}
void Expresseur::OnListNew(wxCommandEvent& WXUNUSED(event)) 
{ 
	ListNew();
}
void Expresseur::OnListOpen(wxCommandEvent& WXUNUSED(event)) 
{
	if (listChanged)
	{
		if (wxMessageBox(_("Current list has not been saved! Proceed?"), _("Please confirm"),
			wxICON_QUESTION | wxYES_NO, this) == wxNO)
			return;
	}
	wxFileDialog
		openFileDialog(this, _("Open list file"), "", "",
		"list files (*.txt)|*.txt", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return; // the user changed idea...

	listFiles.Clear();
	listName.Assign(openFileDialog.GetPath());
	ListOpen();
}
void Expresseur::OnListSave(wxCommandEvent& WXUNUSED(event)) 
{
	if (listName.IsFileWritable() == false)
	{
		wxFileDialog
			openFileDialog(this, _("Save list file"), "", "",
			"list files (*.txt)|*.txt", wxFD_SAVE );
		if (openFileDialog.ShowModal() == wxID_CANCEL)
			return; // the user changed idea...
		listName.Assign(openFileDialog.GetPath());
	}
	ListSave();
}
void Expresseur::OnListSaveas(wxCommandEvent& WXUNUSED(event)) 
{
	wxFileDialog
		openFileDialog(this, _("Save list file"), "", "",
		"list files (*.txt)|*.txt", wxFD_SAVE);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return; // the user changed idea...
	listName.Assign(openFileDialog.GetPath());
	ListSave();
}
void Expresseur::OnListAdd(wxCommandEvent& WXUNUSED(event)) 
{
	if (fileName.IsOk() == false)
		return;
	listFiles.Add(fileName.GetFullPath());
	ListUpdateMenu();
	listChanged = true;
	ListCheck();
}
void Expresseur::OnListRemove(wxCommandEvent& WXUNUSED(event)) 
{
	if (listFiles.Index(fileName.GetFullPath()) != wxNOT_FOUND)
	{
		listFiles.Remove(fileName.GetFullPath());
		ListUpdateMenu();
		listChanged = true;
	}
}
void Expresseur::OnListUp(wxCommandEvent& WXUNUSED(event)) 
{
	if ((listFiles.Index(fileName.GetFullPath()) != wxNOT_FOUND)
		&& (listFiles.Index(fileName.GetFullPath()) != 0 ))
	{
		int p = listFiles.Index(fileName.GetFullPath());
		listFiles.Insert(fileName.GetFullPath(), p-1);
		listFiles.RemoveAt(p+1);
		ListUpdateMenu();
		listChanged = true;
	}
}
void Expresseur::OnListDown(wxCommandEvent& WXUNUSED(event)) 
{
	if ((listFiles.Index(fileName.GetFullPath()) != wxNOT_FOUND)
		&& (listFiles.Index(fileName.GetFullPath()) != ((int)(listFiles.Count()) - 1 )))
	{
		int p = listFiles.Index(fileName.GetFullPath());
		listFiles.Insert(fileName.GetFullPath(), p + 2);
		listFiles.RemoveAt(p);
		ListUpdateMenu();
		listChanged = true;
	}
}
void Expresseur::OnListFile(wxCommandEvent& event) 
{
	ListSelect(event.GetId() - ID_MAIN_LIST_FILE);
}
void Expresseur::OnListPreviousFile(wxCommandEvent& WXUNUSED(event))
{
	ListSelectNext(-1);
}
void Expresseur::OnListNextFile(wxCommandEvent& WXUNUSED(event)) 
{
	ListSelectNext(1);
}
void Expresseur::OnMixer(wxCommandEvent& WXUNUSED(event))
{
	editMode = true;
	if ( mMixer == NULL )
		settingReset(true);
	mMixer->Show();
	mMixer->Raise();
	editMode = false;
}
void Expresseur::OnGoto(wxCommandEvent& WXUNUSED(event))
{
	if (mode != modeScore)
		return;
	editMode = true;
	mViewerscore->gotoPosition("");
	editMode = false;

}
void Expresseur::OnMidishortcut(wxCommandEvent& WXUNUSED(event))
{
	editMode = true;
	if (mMidishortcut->ShowModal() == wxOK)
	{
		editMode = false;
		settingReset(true);
	}
	editMode = false;
}
void Expresseur::OnKeydowInfoLua(wxCommandEvent& WXUNUSED(event))
{
	editMode = true;
	int retcode = wxMessageBox(_("To setup your keyboard disposal ( http://expresseur.com/home/user-guide/#keydown ) :\n\
- identifiy the 4 red lines of 10 keys which are approximatively  :\n\
      - the line of numbers 1 2 3 4 5 6 7 8 9 0\n\
      - three lines above, with letters and various signs (eg QWERTYUIOP, ...)\n\
- close this dialog-box with the OK button\n\
- keydown on your keyboard the 4 lines, line by line, left to right, top to down (status bar indicates the progress)\n"), "keyboard setting", wxCANCEL | wxOK | wxCANCEL_DEFAULT);
	if (retcode == wxOK)
	{
		SetStatusText(_("Waiting for keydowns"), 1);
		nbkeydown = 0;
	}
	else
	{
		SetStatusText(_("Setting cancelled"), 1);
		nbkeydown = -1;
	}
	editMode = false;
}
void Expresseur::OnExpression(wxCommandEvent& WXUNUSED(event))
{
	mExpression->Show();
	mExpression->Raise();
}
void Expresseur::OnLuafile(wxCommandEvent& WXUNUSED(event))
{
	editMode = true ;
	luafile mLuafile(this, wxID_ANY, _("Lua script"), mConf);
	if ( mLuafile.ShowModal() == 1 )
		settingReset(true);
	editMode = false ;
}
void Expresseur::settingSave()
{
	wxArrayString lChoice;
	lChoice.Add(_("Mixer    :  (tuning from menu edit/Mixer)"));
	lChoice.Add(_("Expression   : (tuning from menu edit/Expression)"));
	lChoice.Add(_("MIDI Shortcuts : (settings from menu settings/MIDI-keyborad configuration)"));
	lChoice.Add(_("Lua Files :  (settings from menu setting/LUA files)"));
	wxArrayInt listToSave ;
	do 
	{
		wxMultiChoiceDialog mChoice(this, _("Select the settings to save."), _("Savec setting"), lChoice, wxOK | wxCANCEL);
		if (mChoice.ShowModal() != wxID_OK)
			return;
		listToSave.Clear() ;
		listToSave = mChoice.GetSelections() ;
	} while (listToSave.GetCount() < 1) ;

	wxString str;
	wxTextFile tfile;
	if (settingName.IsFileWritable() == false)
		tfile.Create(settingName.GetFullPath());
	tfile.Open(settingName.GetFullPath());
	if (tfile.IsOpened() == false)
	{
		wxMessageBox("error opening file for write");
		return;
	}

	wxString comment;
	for (str = tfile.GetFirstLine(); !tfile.Eof(); str = tfile.GetNextLine())
	{
		if (str.StartsWith("--"))
		{
			comment += str.Mid(2) + "\n";
		}
	}

	wxTextEntryDialog *mtextentry = new wxTextEntryDialog ( this, _("description"), _("Setting description"), comment, wxTextEntryDialogStyle | wxTE_MULTILINE ); 
	if (mtextentry->ShowModal() != wxID_OK)
	{
		delete mtextentry;
		return;
	}
	comment = mtextentry->GetValue();
	delete mtextentry;

	
	tfile.Clear();

	tfile.AddLine(CONFIG_FILE);
	wxString lmode ;
	lmode.Printf("--mode %s", ((mode == modeScore)?"score":((mode == modeChord)?"improvisation":"na")));
	tfile.AddLine(lmode );
	if (!comment.IsEmpty())
	{
		wxStringTokenizer tokenizer(comment, "\n");
		while (tokenizer.HasMoreTokens())
		{
			wxString token = tokenizer.GetNextToken();
			tfile.AddLine("--" + token);
		}
	}

	for (unsigned int i = 0; i < listToSave.GetCount(); i++)
	{
		switch (listToSave[i])
		{
		case 0: mMixer->write(&tfile); break;
		case 1: mExpression->write(&tfile); break;
		case 2:mMidishortcut->write(&tfile);break;
		case 3: luafile::write(mConf, &tfile); break;
		default: break;
		}
	}
	tfile.Write();
	tfile.Close();
}
bool Expresseur::testModeMidi()
{
	if ((mode == modeScore) && (mConf->get(CONFIG_MIDI_SETTING, modeScore) == modeChord))
	{
		wxMessageBox(_("MIDI setting seems not done for Score purpose (Menu Setting/MIDI presets)"),
			"MIDI settings" , wxOK|wxCENTRE|wxICON_QUESTION );
		return false ;
	}
	if ((mode == modeChord) && (mConf->get(CONFIG_MIDI_SETTING, modeScore) == modeScore))
	{
		wxMessageBox(_("MIDI setting seems not done for Improvisation purpose (Menu Setting/MIDI presets)"),
			"MIDI settings" , wxOK|wxCENTRE|wxICON_QUESTION );
		return false ;
	}
	return true ;
}
void Expresseur::settingOpen()
{
	wxString        str;
	wxFileName f;
	// open the file
	wxString sf = settingName.GetFullPath();
	wxTextFile      tfile;
	if (settingName.IsFileReadable() == false)
		return;
	tfile.Open(sf);
	if (tfile.IsOpened() == false)
		return;
	str = tfile.GetFirstLine();
	if (str != CONFIG_FILE)
	{
		wxString s;
		s.sprintf("This setting file does start with the expected first line %s", CONFIG_FILE);
		wxMessageDialog(this, s, "read setting error", wxICON_ERROR | wxOK );
		return;
	}
	wxString comment;
	bool firstLine = true ;
	for (str = tfile.GetFirstLine(); !tfile.Eof(); str = tfile.GetNextLine())
	{
		if (str.StartsWith("--"))
		{
			if (firstLine )
			{
				if (str.StartsWith("--mode score"))
				{
					mConf->set(CONFIG_MIDI_SETTING, modeScore);
				}
				if (str.StartsWith("--mode improvisation"))
				{
					mConf->set(CONFIG_MIDI_SETTING, modeChord);
				}
			}
			comment += str.Mid(2) + "\n";
		}
	}
	if (!comment.IsEmpty())
	{
		int retcode = wxMessageBox(comment, settingName.GetName(), wxCANCEL | wxOK );
		if (retcode != wxOK)
		{
			tfile.Close();
			return;
		}
	}
	if ( mMixer != NULL ) mMixer->read( &tfile);
	if (mMidishortcut != NULL) mMidishortcut->read( &tfile);
	if (mExpression != NULL) mExpression->read(&tfile);
	luafile::read(mConf, &tfile);

	tfile.Close();


}
bool Expresseur::settingReset(bool all)
{
	wxBusyCursor wait;

	bool retcode = true;
	// stop the timer to be quite
	mtimer->Stop();

	// close and load the right LUA script
	luafile::reset(mConf , all , timerDt );
	if (all)
	{
		openMidiIn();
		openMidiOut();
	}

	basslua_call(moduleLuabass, soutAllNoteOff, "s", "a");
	getLuaAction(false, NULL);

	// load the shortcuts
	if (mMidishortcut != NULL)
	{
		mMidishortcut->Show(false);
		delete mMidishortcut;
	}
	mMidishortcut = NULL;
	mMidishortcut = new midishortcut(this, wxID_ANY, _("shortcut"), mConf, nameAction, nameMidiInDevices , nameOpenMidiInDevices);
	// setup the menus
	mMidishortcut->reset();
	SetMenuAction(true);

	// load the expression
	if (mExpression != NULL)
	{
		mExpression->Show(false);
		delete mExpression;
	}
	mExpression = NULL;
	mExpression = new expression(this, wxID_ANY, _("Expression"), mConf);
	mExpression->reset();

	// caculate the prefix of settings, according to valid midi-out devices opened
	mConf->setPrefix(nameOpenMidiOutDevices);

	int h = posScrollHorizontal;
	int v = posScrollVertical;

	setAudioDefault();
	
	viewerscore *newViewerscore = NULL;
	typeViewer = EMPTYVIEWER;
	mode = modeNil;
	if (fileName.IsFileReadable())
	{
		wxString ext = fileName.GetExt();
		if ((ext == SUFFIXE_MUSICXML) || (ext == SUFFIXE_MUSICMXL))
		{
			newViewerscore = new musicxmlscore(this, wxID_ANY, mConf);
			if (newViewerscore->setFile(fileName))
			{
				typeViewer = MUSICXMLVIEWER;
				mode = modeScore;
			}
			else
			{
				delete newViewerscore;
				newViewerscore = NULL;
				typeViewer = EMPTYVIEWER;
			}
		}
		if (ext == SUFFIXE_BITMAPCHORD)
		{
			newViewerscore = new bitmapscore(this, wxID_ANY, mConf);
			if (newViewerscore->setFile(fileName))
			{
				typeViewer = BITMAPVIEWER;
				mode = modeChord;
			}
			else
			{
				delete newViewerscore;
				newViewerscore = NULL;
				typeViewer = EMPTYVIEWER;
			}
		}
		if (ext == SUFFIXE_TEXT)
		{
			newViewerscore = new musicxmlscore(this, wxID_ANY, mConf);
			if (newViewerscore->setFile(fileName))
			{
				typeViewer = MUSICXMLVIEWER;
				mode = modeScore;
			}
			else
			{
				delete newViewerscore;
				newViewerscore = NULL;
				typeViewer = EMPTYVIEWER;
				mode = modeChord;
			}
		}
	}

	if (newViewerscore == NULL)
	{
		// empty viewer for the score
		newViewerscore = new emptyscore(this, wxID_ANY, mConf);
		v = 50;
		h = 50;
	}

	basslua_setMode(mode);
	
	editMenu->Enable(ID_MAIN_RECORD_IMPRO, mode == modeChord);
	editMenu->Enable(ID_MAIN_RECORD_PLAYBACK, mode == modeScore);
	editMenu->Enable(ID_MAIN_SAVE_PLAYBACK, mode == modeScore);
	editMenu->Enable(ID_MAIN_PLAYBACK, mode == modeScore);
	editMenu->Enable(ID_MAIN_GOTO, mode == modeScore);
	editMenu->Enable(ID_MAIN_PREVIOUS_PAGE, mode == modeScore);
	editMenu->Enable(ID_MAIN_NEXT_PAGE, mode == modeScore);
	editMenu->Enable(ID_MAIN_ORNAMENT_ADD_RELATIVE , mode == modeScore);
	editMenu->Enable(ID_MAIN_ORNAMENT_ADD_ABSOLUTE , mode == modeScore);
	editMenu->Check(ID_MAIN_RECORD_PLAYBACK,false);
	editMenu->Check(ID_MAIN_PLAYBACK,false);


	if (mode != modeScore)
	{
		musicxmlcompile::clearLuaScore();
	}
	// load the text file, suffixe .txt , attached to the image or musicxml file
	if (fileName.IsFileReadable())
	{
		mTextscore->setFile(fileName);
	}
	
	waitToCompile = periodCompile / timerDt;

	sizer_text_viewer->Replace(mViewerscore, newViewerscore);
	delete mViewerscore;
	mViewerscore = newViewerscore;
	setOrientation(v, h);


	// load the mixer
	if (mMixer != NULL)
	{
		mMixer->Show(false);
		delete mMixer;
	}
	mMixer = NULL;
	mMixer = new mixer(this, wxID_ANY, _("mixer"), mConf, mViewerscore, nameMidiOutDevices , nameOpenMidiOutDevices, true );
	mMixer->reset(localoff, true);

	// set the size of the windows
	int x, y, width, height;
	x = mConf->get(CONFIG_MIXERX, 30);
	y = mConf->get(CONFIG_MIXERY, 30);
	width = mConf->get(CONFIG_MIXERWIDTH, 500);
	height = mConf->get(CONFIG_MIXERHEIGHT, 350);
	if ((x > 0) && (y > 0) && (width > 100) && (height > 60))
		mMixer->SetSize(x, y, width, height);
	mMixer->Show(mConf->get(CONFIG_MIXERVISIBLE,false));

	x = mConf->get(CONFIG_SHORTCUTX, 50);
	y = mConf->get(CONFIG_SHORTCUTY, 50);
	width = mConf->get(CONFIG_SHORTCUTWIDTH, 500);
	height = mConf->get(CONFIG_SHORTCUTHEIGHT, 300);
	if ((x > 0) && (y > 0) && (width > 100) && (height > 60))
		mMidishortcut->SetSize(x, y, width, height);
	mMidishortcut->Show(false);

	x = mConf->get(CONFIG_EXPRESSIONX, 80);
	y = mConf->get(CONFIG_EXPRESSIONY, 80);
	width = mConf->get(CONFIG_EXPRESSIONWIDTH, 500);
	height = mConf->get(CONFIG_EXPRESSIONHEIGHT, 250);
	if ((x > 0) && (y > 0) && (width > 100) && (height > 60))
		mExpression->SetSize(x, y, width, height);
	mExpression->Show(mConf->get(CONFIG_EXPRESSIONVISIBLE, false));

	setZoom();

	waitToCompile = 1 ;
	waitToRefresh = 1 ;
	
	mTextscore->SetFocus();

	// restart the timer
	mtimer->Start(timerDt);

	return retcode;
}
void Expresseur::OnReset(wxCommandEvent& WXUNUSED(event))
{
	settingReset(true);
}
void Expresseur::OnDeleteCache(wxCommandEvent& WXUNUSED(event))
{
	musicxmlscore::cleanCache(-1);
}
void Expresseur::OnLog(wxCommandEvent& WXUNUSED(event))
{
	// load the log
	if (mLog != NULL)
	{
		delete mLog;
	}
	mLog = NULL;
	mLog = new logerror(this, wxID_ANY, _("log !!! timeline bottom->up : last-event is the first-line !!!"));
	mLog->Show();
}
void Expresseur::OnMidiLog(wxCommandEvent& WXUNUSED(event))
{
	logMidiMsg = !logMidiMsg;
	basslua_call(moduleLuabass, "logmidimsg", "i", logMidiMsg?1:0);
}
void Expresseur::OnSettingOpen(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog
		openFileDialog(this, _("Open setting file"), "", "",
		"setting files (*.txt)|*.txt", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return; // the user changed idea...

	settingName.Assign(openFileDialog.GetPath());
	settingOpen();
	settingReset(true);

}
void Expresseur::OnSettingSaveas(wxCommandEvent& WXUNUSED(event))
{
	editMode = true ;
	wxFileDialog
		openFileDialog(this, _("Save setting file"), "", "",
		"list files (*.txt)|*.txt", wxFD_SAVE);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
	{
		editMode = false ;
		return; // the user changed idea...
	}
	settingName.Assign(openFileDialog.GetPath());
	settingName.SetExt("txt");
	settingSave();
	editMode = false ;
}
void Expresseur::OnAbout(wxCommandEvent& WXUNUSED(event)) 
{	
	wxString s;
	s.Printf("Expresseur 3.%d\n(C) 2019 REVOLLE Franck <frevolle@gmail.com>", VERSION_EXPRESSEUR);
 	wxMessageBox(s,"about");
}
void Expresseur::OnHelp(wxCommandEvent& WXUNUSED(event)) 
{
	wxLaunchDefaultBrowser("http://www.expresseur.com");
}
void Expresseur::CreateExpresseurV3()
{
	// get the user directory in its documents folder
	wxFileName fname;
	fname.AssignDir(mxconf::getResourceDir());
	fname.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);

	// copy examples from example-folder in documents/expresseurV3-folder
	wxFileName fdirExample;
	fdirExample.Assign(mxconf::getCwdDir());
	fdirExample.AppendDir(DIR_EXAMPLE);
	wxString sExample = fdirExample.GetFullPath();
	//wxMessageBox(sExample,"example dir");
	bool firstExample = true ;
	wxDir dirExample(sExample);
	if (dirExample.IsOpened())
	{
		wxString file1, file2;
		bool cont = dirExample.GetFirst(&file1, "*.*", wxDIR_FILES);
		while (cont)
		{
			wxFileName ffile1(file1);
			wxFileName ffile2(file1);
			ffile1.SetPath(fdirExample.GetPath());
			file1 = ffile1.GetFullPath();
			ffile2.SetPath(mxconf::getUserDir());
			file2 = ffile2.GetFullPath();
			if ( file2.Contains(wxT("fontaine.txt")))
			{
				mConf->set(CONFIG_FILENAME, file2);
				firstExample = false;
			}
			//wxMessageBox(file1 + " to " + file2 , "copy example");
			wxCopyFile(file1, file2);
			cont = dirExample.GetNext(&file1);
		}
	}

	// copy instruments-ressources from ressources-folder in documents/expresseurV3/ressources-folder
	wxFileName fdirRessources;
	fdirRessources.Assign(mxconf::getCwdDir());
	fdirRessources.AppendDir(DIR_RESOURCES);
	wxString sRessources = fdirRessources.GetFullPath();
	//wxMessageBox(sRessources,"ressources dir");
	wxDir dirRessources(sRessources);
	if (dirRessources.IsOpened())
	{
		wxString file1, file2;
		bool cont = dirRessources.GetFirst(&file1, "*.*", wxDIR_FILES);
		while (cont)
		{
			wxFileName ffile1(file1);
			wxFileName ffile2(file1);
			ffile1.SetPath(fdirRessources.GetPath());
			ffile2.SetPath(mxconf::getResourceDir());
			wxString ext = ffile1.GetExt();
			ext.MakeLower();
			bool tocopy = true;
			if ((ext == "sf2") && (ffile2.FileExists()) && (ffile1.GetSize() == ffile2.GetSize()))
					tocopy = false;
			file1 = ffile1.GetFullPath();
			file2 = ffile2.GetFullPath();
			if (tocopy)
			{
				// wxMessageBox(file1 + " to " + file2 , "copy ressource");
				wxCopyFile(file1, file2);
			}
			cont = dirRessources.GetNext(&file1);
		}
	}

}
void Expresseur::initFirstUse(bool force)
{
	// is is the first time the Expresseur start ?
	bool initialized = mConf->get(CONFIG_INITIALIZED, false);
	if ((initialized) && (!force))
	{
		testMidisetting();
		return;
	}

	CreateExpresseurV3();

	// set as already initialized
	mConf->set(CONFIG_INITIALIZED, true);
	mConf->set(CONFIG_CORRECTINCH, 1000);
	mConf->set(CONFIG_DAYCACHE, 15);

	// open the LUA script
	luafile::reset(mConf, true, timerDt);

	// set a prefix on the actual Midi config
	GetListMidiOut();
	mConf->setPrefix(nameValideMidiOutDevices);
	
	// get the actions from the LUA script
	getLuaAction(false, NULL);

	// run the wizard to tune up the audio, and to inform the user
	wizard(false, false);

	// clean everything
	if (mMixer != NULL) delete mMixer;
	if (mMidishortcut != NULL) delete mMidishortcut;
	if (mExpression != NULL) delete mExpression;
	mMixer = NULL;
	mMidishortcut = NULL;
	mExpression = NULL;

	mMidishortcut = new midishortcut(this, wxID_ANY, _("shortcut"), mConf, nameAction, nameMidiInDevices, nameOpenMidiInDevices);
	mExpression = new expression(this, wxID_ANY, _("Expression"), mConf);
	mMixer = new mixer(this, wxID_ANY, _("mixer"), mConf, mViewerscore, nameMidiOutDevices, nameOpenMidiOutDevices, true);

	// load the dfautl setting for the shorcuts, ...
	settingName.AssignDir(mxconf::getResourceDir());
	settingName.SetFullName("score.txt");
	settingOpen();

	if (mMixer != NULL) delete mMixer;
	if (mMidishortcut != NULL) delete mMidishortcut;
	if (mExpression != NULL) delete mExpression;
	mMixer = NULL;
	mMidishortcut = NULL;
	mExpression = NULL;
}
void Expresseur::OnAudioSetting(wxCommandEvent& WXUNUSED(event))
{
	wizard(true,false);
	settingReset();
}
void Expresseur::OnMidiSetting(wxCommandEvent& WXUNUSED(event))
{
	wizard(false, true);
	settingReset();
}
int Expresseur::GetListMidiIn()
{
	nameValideMidiInDevices.Clear();
	nameMidiInDevices.Clear();
	int nrMidiInDevice = 0;
	int nbMidiInDevice = 0;
	char nameMidiInDevice[MAXBUFCHAR];
	*nameMidiInDevice = '\0';
	while (true)
	{
		basslua_call(moduleLuabass, sinGetMidiName, "i>s", nrMidiInDevice + 1, nameMidiInDevice);
		if (*nameMidiInDevice == '\0')
			break;
		nameMidiInDevices.Add(nameMidiInDevice);
		bool valid = false;
		basslua_call(moduleGlobal, sinMidiIsValid, "s>b", nameMidiInDevice, &valid);
		if (valid)
		{
			nameValideMidiInDevices.Add(nameMidiInDevice);
			nbMidiInDevice++;
		}
		nrMidiInDevice++;
	}
	return nbMidiInDevice;
}
void Expresseur::testMidisetting()
{
	if (mConf->exists(CONFIG_MIDIIN, false, wxString::Format("%d", 0)))
		return;
	wizard(false,true);
}
void Expresseur::openMidiIn()
{
	// open the device in
	int nrDevicesToOpen[MIDIIN_MAX];
	int nbDevicesToOpen = 0;
	GetListMidiIn();
	nameOpenMidiInDevices.Clear();
	for (unsigned int i = 0; i < MIDIIN_MAX; i++)
	{
		wxString smididevice = mConf->get(CONFIG_MIDIIN, "", false, wxString::Format("%d", i));
		int nrDevice = nameMidiInDevices.Index(smididevice);
		if (nrDevice != wxNOT_FOUND)
		{
			nameOpenMidiInDevices.Add(smididevice);
			nrDevicesToOpen[nbDevicesToOpen] = nrDevice;
			nbDevicesToOpen++;
		}
	}
	if (nbDevicesToOpen > 0 )
		basslua_openMidiIn(nrDevicesToOpen, nbDevicesToOpen);
}

int Expresseur::GetListMidiOut()
{
	nameValideMidiOutDevices.Clear();
	nameMidiOutDevices.Clear();
	int nrMidiOutDevice = 0;
	int nbMidiOutDevice = 0;
	char nameMidiOutDevice[MAXBUFCHAR];
	*nameMidiOutDevice = '\0';
	while (true)
	{
		basslua_call(moduleLuabass, soutGetMidiName, "i>s", nrMidiOutDevice + 1, nameMidiOutDevice);
		if (*nameMidiOutDevice == '\0')
			break;
		nameMidiOutDevices.Add(nameMidiOutDevice);
		bool valid = false;
		basslua_call(moduleGlobal, soutMidiIsValid, "s>b", nameMidiOutDevice, &valid);
		if (valid)
		{
			nameValideMidiOutDevices.Add(nameMidiOutDevice);
			nbMidiOutDevice++;
		}
		nrMidiOutDevice++;
	}
	return nbMidiOutDevice;
}
void Expresseur::openMidiOut()
{
	// open the device in
	GetListMidiOut();
	nameOpenMidiOutDevices.Clear();
	for (unsigned int i = 0; i < MIDIOUT_MAX; i++)
	{
		wxString smididevice = mConf->get(CONFIG_MIDIOUT, "", false, wxString::Format("%d", i));
		int nrMidiOutDevice = nameMidiOutDevices.Index(smididevice);
		if (nrMidiOutDevice != wxNOT_FOUND)
		{
			nameOpenMidiOutDevices.Add(smididevice);
			basslua_call(moduleLuabass, soutOpenMidi, "i", nrMidiOutDevice + 1);
		}
	}
}
void Expresseur::wizard(bool audio_only, bool midi_only)
{
	luafile::reset(mConf, true , timerDt);

	wxFileName fWizardJpeg ;
	fWizardJpeg.AssignDir(mxconf::getCwdDir());
	fWizardJpeg.SetExt("jpg");

	wxSizerFlags sizerFlagMinimumPlace;
	wxSizerFlags sizerFlagMaximumPlace;
	//sizerFlagMaximumPlace.Proportion(1);
	sizerFlagMaximumPlace.Expand();
	sizerFlagMaximumPlace.Border(wxALL, 5);
	//sizerFlagMinimumPlace.Proportion(1);
	sizerFlagMinimumPlace.Border(wxALL, 5);

	wxString labelWizard;
	if (audio_only)
		labelWizard = "Audio setting";
	else if (midi_only)
		labelWizard = "Midi setting";
	else
		labelWizard = "Wizard Expresseur";
	wxWizard *mwizard = new wxWizard(this, wxID_ANY, labelWizard);
	mwizard->SetPageSize(wxSize(400,700));

	////// welcome 

	wxWizardPageSimple *pwizard_welcome = new wxWizardPageSimple(mwizard );
	wxBoxSizer *topsizer_welcome = new wxBoxSizer(wxVERTICAL);
	wxString sstart = _("\
Welcome to Expresseur Wizard.\n\n\
Next screens will hep you to setup\n\
MIDI and audio devices.\n\
Last screens will describe the default\n\
basic tunings to play.\n");
	fWizardJpeg.SetName("wizard_welcome");
	topsizer_welcome->Add(new wxStaticBitmap(pwizard_welcome,wxID_ANY,wxBitmap(fWizardJpeg.GetFullPath(), wxBITMAP_TYPE_JPEG )), sizerFlagMaximumPlace);
	topsizer_welcome->Add(new wxStaticText(pwizard_welcome, wxID_ANY,sstart ), sizerFlagMaximumPlace);
	pwizard_welcome->SetSizerAndFit(topsizer_welcome);

	///// Midi-in

	fWizardJpeg.SetName("wizard_midi_in");
	wxWizardPageSimple *pwizard_midi_in = new wxWizardPageSimple(mwizard);
	wxBoxSizer *topsizer_midi_in = new wxBoxSizer(wxVERTICAL);
	topsizer_midi_in->Add(new wxStaticBitmap(pwizard_midi_in,wxID_ANY,wxBitmap(fWizardJpeg.GetFullPath(), wxBITMAP_TYPE_JPEG )), sizerFlagMaximumPlace);
	wxString smidi_in;
	int nbMidiInDevice = GetListMidiIn();
	if (nbMidiInDevice == 0)
	{
		smidi_in = _("\
No valid MIDI-in keyboard connected.\n\
You can play a score on the computer\n\
keyboard with space-bar.\n\
It will be a limited experience.\n\
With a MIDI-in keyboard, it will be\n\
easier to play music, adding sensivity\n\
and velocity.\n\n");
	}
	else
	{
	  mlistMidiin = new	wxListBox(pwizard_midi_in, wxID_ANY, wxDefaultPosition, wxDefaultSize, nameValideMidiInDevices, wxLB_MULTIPLE);
	  mlistMidiin->Bind(wxEVT_LISTBOX, &Expresseur::OnMidiinChoice, this);
	  topsizer_midi_in->Add(mlistMidiin, sizerFlagMaximumPlace);
		smidi_in += _("\
MIDI-in detected : it can be used to\n\
play music, adding sensivity and velocity.\n\
Select the MIDI-inputs you want to use.\n\
Menu Settings/MIDI-keyboard can be used\n\
to tune triggers from these Midi-inputs.\n\n");
	}
	topsizer_midi_in->Add(new wxStaticText(pwizard_midi_in, wxID_ANY, smidi_in), sizerFlagMaximumPlace);
	pwizard_midi_in->SetSizerAndFit(topsizer_midi_in);

	///// Midi-out
	mConf->set(CONFIG_MIXERDEVICEDEFAULT, "" , true);
	wxWizardPageSimple *pwizard_midi_out = new wxWizardPageSimple(mwizard);
	wxBoxSizer *topsizer_midi_out = new wxBoxSizer(wxVERTICAL);
	fWizardJpeg.SetName("wizard_midi_out");
	topsizer_midi_out->Add(new wxStaticBitmap(pwizard_midi_out,wxID_ANY,wxBitmap(fWizardJpeg.GetFullPath(), wxBITMAP_TYPE_JPEG )), sizerFlagMaximumPlace);
	wxString smidi_out;
	int nbMidiOutDevice = GetListMidiOut() ;
	if (nbMidiOutDevice == 0)
	{
		smidi_out = _("\
No valid MIDI-out sound expander.\n\
To generate music sound, you will have\n\
to use the basic SF2 on your sound-card.\n\
Next screen will help you for the tuning.\n\n");
	}
	else
	{
		mlistMidiout = new	wxListBox(pwizard_midi_out, wxID_ANY, wxDefaultPosition, wxDefaultSize, nameValideMidiOutDevices, wxLB_MULTIPLE);
		mlistMidiout->Bind(wxEVT_LISTBOX, &Expresseur::OnMidioutChoice, this);
		topsizer_midi_out->Add(mlistMidiout, sizerFlagMaximumPlace);
		wxButton *mDefaultMidiOut = new wxButton(pwizard_midi_out, wxID_ANY, _("Default Midi Out"));
		mDefaultMidiOut->Bind(wxEVT_BUTTON, &Expresseur::OnDefaultMidiOut, this);
		topsizer_midi_out->AddSpacer(5);
		topsizer_midi_out->Add(mDefaultMidiOut);
		smidi_out = _("\
MIDI-out sound expander detected.\n\
1) Select the defaut MIDI-output with the button.\n\
2) Select the MIDI-outputs you want to use.\n\n");
	}
	topsizer_midi_out->Add(new wxStaticText(pwizard_midi_out, wxID_ANY, smidi_out + _("\
With an electronic piano, you will\n\
add the possibility to play the sound\n\
of this piano, with a good reactivity.\n\n\
If you have a software instrument (e.g.\n\
Pianoteq ), connect it on the virtual\n\
midi-in cable, and connect Expresseur\n\
on the virtual midi-out cable.\n")), sizerFlagMaximumPlace);
	pwizard_midi_out->SetSizerAndFit(topsizer_midi_out);

	/////// Audio

	wxWizardPageSimple *pwizard_audio = new wxWizardPageSimple(mwizard);
	wxBoxSizer *topsizer_audio = new wxBoxSizer(wxVERTICAL);
	fWizardJpeg.SetName("wizard_audio");
	topsizer_audio->Add(new wxStaticBitmap(pwizard_audio,wxID_ANY,wxBitmap(fWizardJpeg.GetFullPath(), wxBITMAP_TYPE_JPEG )), sizerFlagMaximumPlace);
	getListAudio();
	int defaultNrDevice = setAudioDefault();
	wxString saudio = _("\
Audio is used to play VSTi & SF2.\n\
Select the audio device to use.\n\
Decrease the buffer sizes to\n\
decrease latency.\n\
VALIDATE THE GOOD QUALITY OF SOUND\n");
	topsizer_audio->Add(new wxStaticText(pwizard_audio, wxID_ANY, saudio));
	mlistAudio = new	wxListBox(pwizard_audio, wxID_ANY, wxDefaultPosition, wxDefaultSize, nameaudioDevices, wxLB_SINGLE);
	mlistAudio->Bind(wxEVT_LISTBOX, &Expresseur::OnAudioChoice, this);
	if (( defaultNrDevice >= 0 ) && ( defaultNrDevice < (int)(nameaudioDevices.GetCount())))
		mlistAudio->SetSelection(defaultNrDevice);
	topsizer_audio->Add(mlistAudio, sizerFlagMaximumPlace);

	wxGridSizer *msaudio = new wxGridSizer(2, 2, 2);
	msaudio->Add(new wxStaticText(pwizard_audio, wxID_ANY, _("update period ms")));
	mupdatems = new wxSpinCtrl(pwizard_audio, wxID_ANY, "1000", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 10, 100, 0);
	msaudio->Add(mupdatems);
	msaudio->Add(new wxStaticText(pwizard_audio, wxID_ANY, _("add buffer length ms")));
	mbufferms = new wxSpinCtrl(pwizard_audio, wxID_ANY, "1000", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -100, 100, 0);
	msaudio->Add(mbufferms);
	mAsioSet = new wxButton(pwizard_audio, wxID_ANY, _("Asio setting"));
	mAsioSet->Bind(wxEVT_BUTTON, &Expresseur::OnAsioSet, this);
	msaudio->AddSpacer(5);
	msaudio->Add(mAsioSet);
	wxButton *mTest = new wxButton(pwizard_audio, wxID_ANY, _("TEST AUDIO"));
	mTest->Bind(wxEVT_BUTTON, &Expresseur::OnAudioTest, this);
	msaudio->AddSpacer(5);
	msaudio->Add(mTest);
	topsizer_audio->Add(msaudio, sizerFlagMaximumPlace);

	pwizard_audio->SetSizerAndFit(topsizer_audio);
	setAudioChoice(defaultNrDevice);

	////// play score

	wxWizardPageSimple *pwizard_playscore = new wxWizardPageSimple(mwizard);
	wxBoxSizer *topsizer_playscore = new wxBoxSizer(wxVERTICAL);
	fWizardJpeg.SetName("wizard_playscore");
	topsizer_playscore->Add(new wxStaticBitmap(pwizard_playscore,wxID_ANY,wxBitmap(fWizardJpeg.GetFullPath(), wxBITMAP_TYPE_JPEG )), sizerFlagMaximumPlace);
	wxString splayscore = _("\
To play a score : open a musicXML\n\
file, and play on your \n\
MIDI keyboard, or with space-bar\n\
on your computer.\n\n\
Some example of musicXML files have\n\
been installed.");
	topsizer_playscore->Add(new wxStaticText(pwizard_playscore, wxID_ANY, splayscore), sizerFlagMaximumPlace);
	pwizard_playscore->SetSizerAndFit(topsizer_playscore);

	///// improvise

	wxWizardPageSimple *pwizard_improvise = new wxWizardPageSimple(mwizard);
	wxBoxSizer *topsizer_improvise = new wxBoxSizer(wxVERTICAL);
	fWizardJpeg.SetName("wizard_improvise");
	topsizer_improvise->Add(new wxStaticBitmap(pwizard_improvise,wxID_ANY,wxBitmap(fWizardJpeg.GetFullPath(), wxBITMAP_TYPE_JPEG )), sizerFlagMaximumPlace);
	wxString simprovise = _("\
To improvise on a grid : \n\
  - select a MIDI-keyboard\n\
    preset in the setting menu.\n\
  - open a chord file or image\n\
Then, improvise with pitches\n\
in the chord using white keys\n\
and black keys for pithes\n\
out of the chords.\n\
Some example of text files with \n\
chords have been installed.");
	topsizer_improvise->Add(new wxStaticText(pwizard_improvise, wxID_ANY, simprovise), sizerFlagMaximumPlace);
	pwizard_improvise->SetSizerAndFit(topsizer_improvise);

	///// pckeyboard

	wxWizardPageSimple *pwizard_pckeyboard = new wxWizardPageSimple(mwizard);
	wxBoxSizer *topsizer_pckeyboard = new wxBoxSizer(wxVERTICAL);
	fWizardJpeg.SetName("wizard_pckeyboard");
	topsizer_pckeyboard->Add(new wxStaticBitmap(pwizard_pckeyboard,wxID_ANY,wxBitmap(fWizardJpeg.GetFullPath(), wxBITMAP_TYPE_JPEG )), sizerFlagMaximumPlace);
	wxString spckeyboard = _("\
MIDI actions & ALT+shortcuts are set with\n\
menu setting/MIDI-keyboard.\n\n\
One-key shortcuts are defined\n\
in LUA script ( mixer, move, ...)\n\
Set keyboard configuration with\n\
menu Setting/One-key configuration\n\n\
Menu Actions displays all shortcuts.");
	topsizer_pckeyboard->Add(new wxStaticText(pwizard_pckeyboard, wxID_ANY, spckeyboard), sizerFlagMaximumPlace);
	mConf->set(CONFIG_KEYBOARDCONFIG, DEFAULTKEYBOARDDISPOSAL);
	pwizard_pckeyboard->SetSizerAndFit(topsizer_pckeyboard);

	////// end of wizard

	wxWizardPageSimple *pwizard_end = new wxWizardPageSimple(mwizard);
	wxBoxSizer *topsizer_end = new wxBoxSizer(wxVERTICAL);
	fWizardJpeg.SetName("wizard_end");
	topsizer_end->Add(new wxStaticBitmap(pwizard_end,wxID_ANY,wxBitmap(fWizardJpeg.GetFullPath(), wxBITMAP_TYPE_JPEG )), sizerFlagMaximumPlace);
	wxString send = _("\
Please consult the web help, to benefit\n\
all the features, or to change the \n\
configuration-behavior.\n\n\
To come back later in this wizard,\n\
select the menu setup/wizard.");
	topsizer_end->Add(new wxStaticText(pwizard_end, wxID_ANY, send), sizerFlagMaximumPlace);
	wxButton *bHelp = new wxButton(pwizard_end, wxID_ANY, _("web help"));
	bHelp->Bind(wxEVT_BUTTON, &Expresseur::OnHelp, this);
	topsizer_end->Add(bHelp);
	pwizard_end->SetSizerAndFit(topsizer_end);

	if (audio_only)
	{
		pwizard_audio->SetPrev(NULL);
		pwizard_audio->SetNext(NULL);
		mwizard->RunWizard(pwizard_audio);
	}
	else if (midi_only)
	{
		pwizard_midi_in->SetPrev(NULL);
		pwizard_midi_in->SetNext(pwizard_midi_out);
		pwizard_midi_out->SetPrev(pwizard_midi_in);
		pwizard_midi_out->SetNext(NULL);
		 mwizard->RunWizard(pwizard_midi_in);
	}
	else
		{
		pwizard_welcome->SetPrev(NULL);
		pwizard_welcome->SetNext(pwizard_midi_in);
		pwizard_midi_in->SetPrev(pwizard_welcome);
		pwizard_midi_in->SetNext(pwizard_midi_out);
		pwizard_midi_out->SetPrev(pwizard_midi_in);
		pwizard_midi_out->SetNext(pwizard_audio);
		pwizard_audio->SetPrev(pwizard_midi_out);
		pwizard_audio->SetNext(pwizard_playscore);
		pwizard_playscore->SetPrev(pwizard_audio);
		pwizard_playscore->SetNext(pwizard_improvise);
		pwizard_improvise->SetPrev(pwizard_playscore);
		pwizard_improvise->SetNext(pwizard_pckeyboard);
		pwizard_pckeyboard->SetPrev(pwizard_improvise);
		pwizard_pckeyboard->SetNext(pwizard_end);
		pwizard_end->SetPrev(pwizard_pckeyboard);
		pwizard_end->SetNext(NULL);
		mwizard->RunWizard(pwizard_welcome);
	}

	mwizard->Destroy();
}
int Expresseur::getListAudio()
{
	nameaudioDevices.Clear();
	int nraudioDevice = 0;
	char nameaudioDevice[MAXBUFCHAR];
	*nameaudioDevice = '\0';
	while (true)
	{
		basslua_call(moduleLuabass, "getAudioName", "i>s", nraudioDevice + 1, nameaudioDevice);
		if (*nameaudioDevice == '\0')
			break;
		nameaudioDevices.Add(nameaudioDevice);
		nraudioDevice++;
	}
	return nraudioDevice;
}
void Expresseur::OnMidioutChoice(wxCommandEvent& WXUNUSED(event))
{
	wxArrayInt selections;
	mlistMidiout->GetSelections(selections);
	for (unsigned int i = 0; i < MIDIOUT_MAX; i++)
	{
		if ( i < selections.GetCount())
		{
			mConf->set(CONFIG_MIDIOUT, nameValideMidiOutDevices[selections[i]], false, wxString::Format("%d", i));
		}
		else
			mConf->set(CONFIG_MIDIOUT, "" , false, wxString::Format("%d", i));
	}
}
void Expresseur::OnDefaultMidiOut(wxCommandEvent& WXUNUSED(event))
{
	wxArrayInt selections;
	mlistMidiout->GetSelections(selections);
	if ((selections.GetCount() == 0) || (selections.GetCount() > 1))
	{
		wxMessageBox("Select one Midi-Out as default", "Default error");
		return;
	}
	wxString s;
	s.Printf("%s:%s", SMIDI, nameValideMidiOutDevices[selections[0]]);
	mConf->set(CONFIG_MIXERDEVICEDEFAULT, s, true);
}
void Expresseur::OnMidiinChoice(wxCommandEvent& WXUNUSED(event))
{
	wxArrayInt selections;
	mlistMidiin->GetSelections(selections);
	for (unsigned int i = 0; i < MIDIIN_MAX; i++)
	{
		if (i < selections.GetCount())
		{
			mConf->set(CONFIG_MIDIIN, nameValideMidiInDevices[selections[i]], false, wxString::Format("%d", i));
		}
		else
			mConf->set(CONFIG_MIDIIN, "", false, wxString::Format("%d", i));
	}
}
void Expresseur::OnAudioChoice(wxCommandEvent& event)
{
	int nrDevice = event.GetSelection();
	setAudioChoice(nrDevice);
}
void Expresseur::setAudioChoice(int nrDevice)
{
	if ((nrDevice < 0 ) || (nrDevice >= (int)(mlistAudio->GetCount())))
		return ;
	wxString name_device = mlistAudio->GetString(nrDevice);
	if (name_device.StartsWith("asio_"))
	{
		mAsioSet->Enable();
		mupdatems->Disable();
		mbufferms->Disable();
	}
	else
	{
		mAsioSet->Disable();
		mupdatems->Enable();
		int vupdate = mConf->get(CONFIG_AUDIO_UPDATE, 25, false, name_device);
		mupdatems->SetValue(vupdate);
		mbufferms->Enable();
		int vbuffer = mConf->get(CONFIG_AUDIO_BUFFER, 25, false, name_device);
		mbufferms->SetValue(vbuffer);
	}
	mConf->set(CONFIG_DEFAULT_AUDIO, name_device);
}
void Expresseur::OnAsioSet(wxCommandEvent& WXUNUSED(event))
{
	int nrDevice = setAudioDefault() ;
	basslua_call(moduleLuabass, "asioSet", "i", nrDevice + 1);
}
void Expresseur::OnAudioTest(wxCommandEvent& WXUNUSED(event))
{
	wxString name_device = mConf->get(CONFIG_DEFAULT_AUDIO, "");
	int vupdate = mupdatems->GetValue();
	mConf->set(CONFIG_AUDIO_UPDATE, vupdate, false, name_device);
	int vbuffer = mbufferms->GetValue();
	mConf->set(CONFIG_AUDIO_BUFFER, vbuffer, false, name_device);

	setAudioDefault();
	
	wxFileName fsound;
	fsound.AssignDir(mxconf::getCwdDir());
	fsound.SetFullName("test.wav");
	char buff[MAXBUFCHAR];
	wxString fs = fsound.GetFullPath();
	strcpy(buff, fs.c_str());
	basslua_call(moduleLuabass, "outSoundPlay", "s", buff);
}
int Expresseur::setAudioDefault()
{
	nameDefaultaudioDevices = mConf->get(CONFIG_DEFAULT_AUDIO,"");
	basslua_call(moduleLuabass, "audioClose", "");
	getListAudio();
	int nrDevice = nameaudioDevices.Index(nameDefaultaudioDevices);
	if (nrDevice == wxNOT_FOUND)
	{
		switch ( nameaudioDevices.GetCount() )
		{
			case 0 : nrDevice = -1 ; break ;
			case 1 : nrDevice = 0 ; break ;
			default : nrDevice = 1 ; break ;
		}
	}
	basslua_call(moduleLuabass, "audioDefaultDevice", "i", nrDevice + 1 );
	for (unsigned int n = 0; n < nameaudioDevices.GetCount(); n++)
	{
		int vupdate = mConf->get(CONFIG_AUDIO_UPDATE, 25, false, nameaudioDevices[n]);
		int vbuffer = mConf->get(CONFIG_AUDIO_BUFFER, 25, false, nameaudioDevices[n]);
		basslua_call(moduleLuabass, "audioSet", "iii", n + 1, vupdate, vbuffer);
	}
	return nrDevice;
}
void Expresseur::OnUpdate(wxCommandEvent& WXUNUSED(event))
{
	checkUpdate(true);;
}
void Expresseur::checkUpdate(bool interactive)
{
	wxHTTP get;
	wxInputStream *httpStream ;
	{
		wxBusyCursor wait;

		get.SetHeader(_T("Content-type"), _T("text/html; charset=utf-8"));
		get.SetTimeout(2);

		// this will wait until the user connects to the internet. It is important in case of dialup (or ADSL) connections
		if (!get.Connect(_T("www.expresseur.com")))
		{
			get.Close();
			return;// only the server, no pages here yet ...
		}

		wxApp::IsMainLoopRunning(); // should return true

		// use _T("/") for index.html, index.php, default.asp, etc.
		httpStream = get.GetInputStream(_T("/update/"));

		// wxLogVerbose( wxString(_T(" GetInputStream: ")) << get.GetResponse() << _T("-") << ((resStream)? _T("OK ") : _T("FAILURE ")) << get.GetError() );
	}

	if (get.GetError() == wxPROTO_NOERR)
	{
		wxString res;
		wxStringOutputStream out_stream(&res);
		httpStream->Read(out_stream);

		int pos = res.Find("Version#");
		if (pos != wxNOT_FOUND)
		{
			pos += 10;
			int epos = pos ;
			while ((res[epos] >= '0') && (res[epos] <= '9') && (epos < (pos + 10)))
				epos++;
			wxString sv = res.Mid(pos,epos - pos);
			long l;
			if (sv.ToLong(&l))
			{
				int vo = mConf->get(CONFIG_VERSION_CHECKED, VERSION_EXPRESSEUR);
				mConf->set(CONFIG_VERSION_CHECKED, l);
				if (l > vo)
				{
					wxString mes;
					mes.Printf("New version 3.%d available. Go to the web-page ?", l);
					if (wxMessageBox(mes, "update", wxYES_NO) == wxYES)
					{
						wxDELETE(httpStream);
						get.Close();
						wxLaunchDefaultBrowser("http://www.expresseur.com/update/");
						return;
					}
				}
				else
				{
					if ( interactive )
						wxMessageBox("this version is up to date");
				}
			}
		}
		else
		{
			if ( interactive )
				wxMessageBox("error : no version available on www.expresseur.com/update/");
		}
	}
	else
	{
		if ( interactive )
		wxMessageBox("www.expresseur.com not acessible");
	}
	wxDELETE(httpStream);
	get.Close();

}

void Expresseur::OnResetConfiguration(wxCommandEvent& WXUNUSED(event))
{
	int manswer = wxMessageBox(_("Delete and reset all the configuration ?"),_("Confirm"),wxYES_NO,this);
	if ( manswer == wxYES ) 
	{
		mConf->deleteConf() ;
		postInit();
	}
}
void Expresseur::OnTest(wxCommandEvent& WXUNUSED(event))
{
	int nbPaint = ((musicxmlscore *)mViewerscore)->getNbPaint();
	int nbSetPosition = ((musicxmlscore *)mViewerscore)->getNbSetPosition();
	wxString s ;
	s.Printf("NbPaint=%d nbSetPosition=%d",nbPaint,nbSetPosition);
	wxMessageBox(s);
}
void Expresseur::OnRecentFile(wxCommandEvent& event)
{
	wxString f(fileHistory->GetHistoryFile(event.GetId() - wxID_FILE1));

	if (!f.empty())
	{
		Open(f);
	}
}
void Expresseur::Open(wxString f)
{
	wxBusyCursor waitcursor;
	wxFileName fn(f);
	wxString ext = fn.GetExt();
	if ( ext == SUFFIXE_TEXT)
	{
		// read the fist line of the file
		wxTextFile      tfile;
		if (fn.IsFileReadable() == false)
			return;
		tfile.Open(fn.GetFullPath());
		if (tfile.IsOpened() == false)
			return;
		wxString str = tfile.GetFirstLine();
		tfile.Close();

		if (str == CONFIG_FILE)
		{
			settingName.Assign(f);
			settingOpen();
			settingReset(true);
			return;
		}
		if (str == LIST_FILE)
		{
			listName.Assign(f);
			ListOpen();
			return;
		}
		fileName.Assign(f);
		FileOpen();
		testModeMidi() ;
	}
	if ((ext == SUFFIXE_MUSICMXL) || (ext == SUFFIXE_MUSICXML))
	{
		fileName.Assign(f);

		wxFileName txtFilename = fileName;
		txtFilename.SetExt(SUFFIXE_TEXT);
		if (txtFilename.IsFileReadable())
		{
			// a musicxml file is loaded, and the txt file already exists : warning 
			if (wxMessageBox(_("Current text file already exists ! Overwrite ?"), _("File txt exists"),
				wxICON_QUESTION | wxYES_NO, NULL) == wxNO)
				return;
		}
		FileOpen();
		testModeMidi() ;
	}
	if (ext == SUFFIXE_BITMAPCHORD)
	{
		fileName.Assign(f);
		FileOpen();
		testModeMidi() ;
	}
}
