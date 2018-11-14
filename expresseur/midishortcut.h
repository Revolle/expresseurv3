// update : 31 / 10 / 2016 10 : 00
#ifndef DEF_MIDISHORTCUT

#define DEF_MIDISHORTCUT


class midishortcut
	: public wxDialog
{

public:
	midishortcut(wxFrame *parent, wxWindowID id, const wxString &title, mxconf* lMxconf, wxArrayString inameAction);
	~midishortcut();

	void reset();
	void write(wxTextFile *lfile);
	void read(wxTextFile *lfile);

	void OnSize(wxSizeEvent& event);
	void OnClose(wxCloseEvent& event);

	void scanMidi(int nr_device, int type_msg, int channel, int value1, int value2);

	void OnDelete(wxCommandEvent& event);
	void OnAdd(wxCommandEvent& event);
	void OnEdit(wxCommandEvent& event);
	void OnUp(wxCommandEvent& event);
	void OnDown(wxCommandEvent& event);
	void OnClose(wxCommandEvent& event);
	void OnDescriptionFunctionMidi(wxCommandEvent& event);

	void OnFunctionMidi(wxCommandEvent& event);
	void OnDevice(wxEvent& event);
	void OnCurve(wxEvent& event);
	void OnChannel(wxEvent& event);
	void OnVolume(wxEvent& event);
	void savePos();

	wxArrayString getShortcuts() ;
	void onShortcut(int nrShortcut);
	//bool hitkey(wxChar c , bool on , wxFrame *parent);

private:
	wxFrame *mParent;
	wxDialog *mThis;
	mxconf *mConf;

	editshortcut *medit;

	wxSizerFlags sizerFlagMinimumPlace;
	wxSizerFlags sizerFlagMaximumPlace;
	wxFlexGridSizer *topsizer;

	wxListView *listShortchut;

	wxArrayString nameAction;
	wxArrayString nameKey;
	wxArrayString nameDevice;
	wxArrayString nameChannel;
	wxArrayString nameEvent;
	wxArrayString nameValueMin;
	wxArrayString nameValueMax;
	wxArrayString nameStopOnMatch;
	wxArrayString nameParam;

	wxArrayString valueName;
	wxArrayString valueAction;
	wxArrayString valueParam;
	wxArrayString valueKey;
	wxArrayInt valueDevice;
	wxArrayInt valueChannel;
	wxArrayString valueEvent;
	wxArrayInt valueMin;
	wxArrayString valueStopOnMatch;

	wxArrayInt shortcutNrSelector;

	int prevShortcutNrSelector = -1 ;

	wxChoice *listFunctionMidi;

	void openMidiIn(bool allValid);

	void loadShortcut();
	void saveShortcut();

	void InitLists();

	int nbDevice;
	bool valideMidiinDevice[MAX_MIDIIN_DEVICE];
	bool deviceToOpen[MAX_MIDIIN_DEVICE];

	void getMidiinDevices();

	int edit(long i);
	void close();

	wxDECLARE_EVENT_TABLE();
};

#endif
