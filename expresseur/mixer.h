// update : 18/112016 10:00

#ifndef DEF_MIXER

#define DEF_MIXER



class mixer
	: public wxDialog
{

public:
	mixer(wxFrame *parent, wxWindowID id, const wxString &title, mxconf* lMxconf , viewerscore *mscore );
	~mixer();

	void reset(bool localoff = true, bool doreset = false);
	void write(wxTextFile *lfile);
	void read(wxTextFile *lfile);

	void OnSize(wxSizeEvent& event);
	void OnClose(wxCloseEvent& event);

	void scanVolume();

	void savePos();

	void setMixerVolume(int nrTrack, int value);
	void setMainVolume(int value);

	void OnMainMixerVolume(wxEvent& event);
	void OnMixerVolume(wxEvent& event);
	void OnMasterVolume(wxEvent& event);
	void OnNeutralMixer(wxCommandEvent& event);
	void OnDefaultMixer(wxCommandEvent& event);
	void OnSettingAllNoteOff(wxCommandEvent& event);
	void OnClose(wxCommandEvent& event);

	void OnSoundDevice(wxEvent& event);
	void OnSoundChannel(wxEvent& event);
	void OnSoundInstrument(wxEvent& event);

	void allNoteOff();

	wxArrayString getMidiOutDevices();

private:
	wxFrame *mParent;
	wxDialog *mThis;
	mxconf* mConf;
	viewerscore *mViewerscore;

	bool loading = true;

	wxStaticText *txtValue;

	void BuildSizer();
	wxFlexGridSizer *mixchannelSizer;
	wxBoxSizer *topsizer;

	void getTracks();
	int nbTrack = 0;
	wxArrayString nameTrack;
	wxArrayString helpTrack;
	wxSizerFlags sizerFlagMinimumPlace;
	wxSizerFlags sizerFlagMaximumPlace;

	void createMainMixer();
	void createMixers();

	void replicate(int nrTrack);

	void AddMixerName(wxString label);
	void AddMixerVolume(int nrTrack);

	wxArrayString nameChannel;
	wxArrayString listMidioutDevice[MAX_MIDIOUT_DEVICE];
	bool valideMidioutDevice[MAX_MIDIOUT_DEVICE];
	wxArrayString nameMidioutDevice;
	wxString defaultDevice , lastDevice ;

	bool createViList(wxString fileName, wxString ext);

	void InitListChannel();
	int nbMidioutDevice; 
	int nbOutDevice;

	int mainVolume;
	wxSlider *slmainVolume;
	int trackVolume[MAX_TRACK];
	wxSlider *sltrackVolume[MAX_TRACK];

	wxChoice *mSoundDevice[MAX_TRACK];
	wxComboBox *mInstrument[MAX_TRACK];

	wxArrayString listVIused;

	void getMidioutDevices();
	void getListMidioutDevice(wxString fileName , int nrDevice);

	void getMidiVi(wxString fullNameDevice, int *nrMidiDevice, wxString *nameVi);
	wxString setMidiVi(bool valid , wxString nameMidi, wxString nameVi , wxString extVi);

	void AddSoundDevice(int nrTrack);
	void AddSoundChannel(int nrTrack);
	void AddSoundInstrument(int nrTrack);

	void close();

	wxDECLARE_EVENT_TABLE();
};

#endif