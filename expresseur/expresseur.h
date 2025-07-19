#ifndef DEF_EXPRESSEUR

#define DEF_EXPRESSEUR

typedef long (MNLReleaseProc)(void);
typedef long (MNLLoadProc)(char *path, char *validationCode);

class Expresseur : public wxFrame
{
public:
	Expresseur(wxFrame *parent,
		wxWindowID id = wxID_ANY,
		const wxString& title = _(APP_NAME),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_FRAME_STYLE | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE);
	virtual ~Expresseur();


	void OnTest(wxCommandEvent& WXUNUSED(event));

	//void OnNew(wxCommandEvent& WXUNUSED(event));
	void OnOpen(wxCommandEvent& WXUNUSED(event));
	void OnSave(wxCommandEvent& WXUNUSED(event));
	void OnSaveas(wxCommandEvent& WXUNUSED(event));
	void OnRecentFile(wxCommandEvent& event);
	void OnExit(wxCommandEvent& WXUNUSED(event));

	void OnUndo(wxCommandEvent& WXUNUSED(event));
	void OnRedo(wxCommandEvent& WXUNUSED(event));
	void OnCopy(wxCommandEvent& WXUNUSED(event));
	void OnCut(wxCommandEvent& WXUNUSED(event));
	void OnPaste(wxCommandEvent& WXUNUSED(event));
	void OnEdit(wxCommandEvent& WXUNUSED(event));
	void OnLocaloff(wxCommandEvent& WXUNUSED(event));
	void OnAudioSetting(wxCommandEvent& WXUNUSED(event));
	void OnMidiSetting(wxCommandEvent& WXUNUSED(event));
	void OnZoom(wxCommandEvent& WXUNUSED(event));
	void OnPlayviewSolo(wxCommandEvent& WXUNUSED(event));
	void OnSaveImpro(wxCommandEvent& WXUNUSED(event));
	void OnRecordImpro(wxCommandEvent& WXUNUSED(event));
	void OnPlayview(wxCommandEvent& WXUNUSED(event));
	void OnPlayviewAll(wxCommandEvent& WXUNUSED(event));
	void selectPlayview(wxString s);
	void OnOrnamentAddAbsolute(wxCommandEvent& WXUNUSED(event));
	void OnOrnamentAddRelative(wxCommandEvent& WXUNUSED(event));
	void OnPreviousPage(wxCommandEvent& WXUNUSED(event));
	void OnNextPage(wxCommandEvent& WXUNUSED(event));

	void OnListNextFile(wxCommandEvent& event);
	void OnListPreviousFile(wxCommandEvent& event);

	void OnListNew(wxCommandEvent& WXUNUSED(event));
	void OnListOpen(wxCommandEvent& WXUNUSED(event));
	void OnListSave(wxCommandEvent& WXUNUSED(event));
	void OnListSaveas(wxCommandEvent& WXUNUSED(event));
	void OnListAdd(wxCommandEvent& WXUNUSED(event));
	void OnListRemove(wxCommandEvent& WXUNUSED(event));
	void OnListUp(wxCommandEvent& WXUNUSED(event));
	void OnListDown(wxCommandEvent& WXUNUSED(event));
	void OnListFile(wxCommandEvent& event);
	void OnMenuAction(wxCommandEvent& event);
	void OnMenuShortcut(wxCommandEvent& event);
	void OnMenuSettings(wxCommandEvent& event);
	void OnRecordPlayback(wxCommandEvent& event);
	void OnSavePlayback(wxCommandEvent& WXUNUSED(event));
	void OnPlayback(wxCommandEvent& event);
	
	void OnMixer(wxCommandEvent& WXUNUSED(event));
	void OnGoto(wxCommandEvent& WXUNUSED(event));
	void OnMidishortcut(wxCommandEvent& WXUNUSED(event));
	void OnKeydowInfoLua(wxCommandEvent& WXUNUSED(event));
	void OnExpression(wxCommandEvent& WXUNUSED(event));
	void OnLuafile(wxCommandEvent& WXUNUSED(event));
	void OnReset(wxCommandEvent& WXUNUSED(event));
	void OnDeleteCache(wxCommandEvent& WXUNUSED(event));
	void OnLog(wxCommandEvent& WXUNUSED(event));
	void OnMidiLog(wxCommandEvent& WXUNUSED(event));
	void OnSettingOpen(wxCommandEvent& WXUNUSED(event));
	void OnSettingSaveas(wxCommandEvent& WXUNUSED(event));
	void OnCheckConfig(wxCommandEvent& WXUNUSED(event));
	void OnHelpluashortcut(wxCommandEvent& WXUNUSED(event));

	void OnAudioChoice(wxCommandEvent& event);
	void OnMidioutChoice(wxCommandEvent& event);
	void OnMidiinChoice(wxCommandEvent& event);
	void OnAsioSet(wxCommandEvent& event);
	void OnAudioTest(wxCommandEvent& WXUNUSED(event));
	void OnDefaultMidiOut(wxCommandEvent& WXUNUSED(event));
	int setAudioDefault();

	void OnAbout(wxCommandEvent& WXUNUSED(event));
	void OnHelp(wxCommandEvent& WXUNUSED(event));
	void OnUpdate(wxCommandEvent& WXUNUSED(event));
	void OnResetConfiguration(wxCommandEvent& WXUNUSED(event));

	//void OnActivate(wxActivateEvent& event);

	bool OnKeyDown(wxKeyEvent& event );

	void OnHorizontalScroll(wxScrollEvent& event);
	void OnVerticalScroll(wxScrollEvent& event);

	void OnIdle(wxIdleEvent& event);
	void OnTimer(wxTimerEvent& event);
	void OnSize(wxSizeEvent& event);
	void postInit();
	void preClose();
	bool testModeMidi();

	////mixer *mMixer = NULL;
	////midishortcut *mMidishortcut = NULL;
	////expression *mExpression = NULL;
	////logerror *mLog;
	wxTimer *mtimer;

private:
	////viewerscore *mViewerscore;
	////textscore *mTextscore;
	wxScrollBar *mScrollHorizontal;
	wxScrollBar *mScrollVertical;
	int posScrollHorizontal = 20;
	int posScrollVertical = 50 ;
	wxBoxSizer *sizer_text_viewer;
	wxSizerItem *sizer_A, *sizer_B;

	void setOrientation(int v, int h);

	bool firstTimer = true;
	int waitToRefresh = 1;
	int waitToCompile = 1; 
	int timerDt = 20 ;

	////mxconf *mConf;
	wxMenuBar *mMenuBar;

	void setRightDisplay(bool right);
	void setZoom();

	// management of the file
	wxFileName fileName;
	wxFileHistory* fileHistory;
	void FileOpen(bool all = false);
	void FileSave();
	void setWindowsTitle();
	void checkUpdate(bool interactive = false);

	// management of the file list
	void ListClearMenu();
	void ListUpdateMenu();
	void ListSave();
	void ListOpen();
	void ListNew();
	void ListCheck();
	void ListSelectNext(int df);
	void ListSelect(int id);
	bool listChanged;
	wxMenu *listMenu;
	wxArrayString listFiles;
	wxFileName listName;
	int listSelection;

	wxMenu *listSettingMenu;
	void readListSettings();
	wxArrayString listSettings ; 

	wxArrayString nameAction;
	wxArrayString nameValue;
	wxArrayString helpValue;

	wxFileName settingName;
	void settingOpen();
	void settingSave();
	bool settingReset(bool all = true);

	wxMenu *actionMenu;
	wxMenu *editMenu;
	wxMenu *zoomMenu;
	wxMenuItem *menuEditMode;

	void getLuaAction(bool all, wxMenu *newActionMenu);
	void getShortcutAction(wxMenu *newActionMenu);
	void SetMenuAction(bool all);
	void ornamentAdd(bool absolute);

	void Open(wxString s);
	void wizard(bool audio_only = false, bool midi_only = false );
	int GetListMidiIn();
	void openMidiIn();
	void openMidiOut();
	void testMidisetting();
	int GetListMidiOut();
	void initFirstUse(bool force);
	void CreateExpresseurV3();
	wxSpinCtrl *mupdatems , *mbufferms;
	wxButton *mAsioSet;
	wxListBox *mlistMidiout;
	wxListBox *mlistMidiin;
	wxListBox *mlistAudio;
	wxArrayString nameaudioDevices;
	wxString nameDefaultaudioDevices;
	wxArrayString nameMidiOutDevices;
	wxArrayString nameMidiInDevices;
	wxArrayString nameValideMidiOutDevices;
	wxArrayString nameValideMidiInDevices;
	wxArrayString nameOpenMidiOutDevices;
	wxArrayString nameOpenMidiInDevices;
	int getListAudio();
	void setAudioChoice(int nrDevice);

	void setPlayView(wxString s);
	bool checkConfig();
	wxString checkFile(wxString dir, wxString fullname);

	int typeViewer;

	int mode = 0 ;
	bool editMode = true;
	bool logMidiMsg = false;
	bool localoff = false;
	wxToolBar *toolBar;
	
	wxSize sizeFrame ;
	
	bool playback = false;
	bool recordPlayback = false ;
	
	int keyboardDisposal[MAXKEYBOARDDISPOSAL];
	int nbkeydown = -1;

	wxSize image_right =  wxSize(0,0);

	wxDECLARE_EVENT_TABLE();
};
#endif
