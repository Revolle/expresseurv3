// update : 17/11/2016 17:00

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

	void OnNew(wxCommandEvent& WXUNUSED(event));
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
	void OnZoom(wxCommandEvent& WXUNUSED(event));
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
	void OnRecordPlayback(wxCommandEvent& event);
	void OnSavePlayback(wxCommandEvent& WXUNUSED(event));
	void OnPlayback(wxCommandEvent& event);
	
	void OnMixer(wxCommandEvent& WXUNUSED(event));
	void OnGoto(wxCommandEvent& WXUNUSED(event));
	void OnMidishortcut(wxCommandEvent& WXUNUSED(event));
	void OnExpression(wxCommandEvent& WXUNUSED(event));
	void OnLuafile(wxCommandEvent& WXUNUSED(event));
	void OnReset(wxCommandEvent& WXUNUSED(event));
	void OnLog(wxCommandEvent& WXUNUSED(event));
	void OnSettingOpen(wxCommandEvent& WXUNUSED(event));
	void OnSettingSave(wxCommandEvent& WXUNUSED(event));
	void OnSettingSaveas(wxCommandEvent& WXUNUSED(event));

	void OnAudioChoice(wxCommandEvent& event);
	void OnMidioutChoice(wxCommandEvent& event);
	void OnAsioSet(wxCommandEvent& event);
	void OnAudioTest(wxCommandEvent& WXUNUSED(event));
	int setAudioDefault();

	void OnAbout(wxCommandEvent& WXUNUSED(event));
	void OnHelp(wxCommandEvent& WXUNUSED(event));
	void OnUpdate(wxCommandEvent& WXUNUSED(event));
	void OnResetConfiguration(wxCommandEvent& WXUNUSED(event));

	void OnActivate(wxActivateEvent& event);

	void OnHorizontalScroll(wxScrollEvent& event);
	void OnVerticalScroll(wxScrollEvent& event);

	bool OnKeyUp(wxChar k);
	bool OnKeyDown(wxChar k);
	bool OnLeft(wxPoint p, wxSize s , bool down);

	void OnTimer(wxTimerEvent& event);
	void OnSize(wxSizeEvent& event);
	bool timerTask(bool compile , bool longwait );
	void postInit();
	void preClose();

	mixer *mMixer = NULL;
	midishortcut *mMidishortcut = NULL;
	expression *mExpression = NULL;
	logerror *mLog;

private:
	viewerscore *mViewerscore;
	textscore *mTextscore;
	wxScrollBar *mScrollHorizontal;
	wxScrollBar *mScrollVertical;
	int posScrollHorizontal, posScrollVertical;
	wxBoxSizer *sizer_text_viewer;
	wxSizerItem *sizer_A, *sizer_B;

	void setOrientation(int v, int h);

	int waitToRefresh = 1;
	int waitToCompile = 1; 
	int timerDt = 20 ;

	mxconf *mConf;
	bool end_ok = true;
	wxMenuBar *mMenuBar;
	wxTimer *mtimer;

	void setRightDisplay(bool right);
	void setZoom();

	// management of the file
	wxFileName fileName;
	wxFileHistory* fileHistory;
	void FileOpen();
	void FileSave();
	void setWindowsTitle();
	void checkUpdate();

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
	void SetMenuAction(bool all);
	void ornamentAdd(bool absolute);

	void Open(wxString s);
	void wizard(bool audio_only = false);
	void initFirstUse(bool force);
	wxSpinCtrl *mupdatems , *mbufferms;
	wxButton *mAsioSet;
	wxListBox *mlistAudio;
	wxArrayString nameaudioDevices;
	wxListBox *mlistMidiout;
	wxArrayString nameMidiOutDevices;
 
	int getListAudio();
	void setAudioChoice(int nrDevice);

	wxChar  ckeychar, ckeys[MAX_KEYS];
	int typeViewer;

	int mode = modeChord ;
	bool playScoreOn = false ;
	bool editMode;
	bool localoff;
	wxToolBar *toolBar;

	int prev_absolute_measure_nr= NULL_INT ;
	bool playback = false;
	bool recordPlayback = false ;
	
	wxSize image_right =  wxSize(0,0);

	wxDECLARE_EVENT_TABLE();
};
#endif
