// update : 31 / 10 / 2016 10 : 00
#ifndef DEF_MIDISHORTCUT

#define DEF_MIDISHORTCUT


class midishortcut
	: public wxDialog
{

public:
	midishortcut(wxFrame *parent, wxWindowID id, const wxString &title, std::vector <wxString> inameAction, std::vector <wxString> lMidiin, std::vector <wxString> lOpenedMidiin);
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

	void OnDevice(wxEvent& event);
	void OnCurve(wxEvent& event);
	void OnChannel(wxEvent& event);
	void OnVolume(wxEvent& event);
	void savePos();

	std::vector<wxString> getShortcuts() ;
	void onShortcut(int nrShortcut);
	//bool hitkey(wxChar c , bool on , wxFrame *parent);

private:
	wxFrame *mParent;
	wxDialog *mThis;

	editshortcut *medit;

	wxSizerFlags sizerFlagMinimumPlace;
	wxSizerFlags sizerFlagMaximumPlace;
	wxFlexGridSizer *topsizer;

	wxListView *listShortchut;

	std::vector <wxString> nameAction;
	std::vector <wxString> nameKey;
	std::vector <wxString> nameOpenDevice;
	std::vector <wxString> nameDevice;
	std::vector <wxString> nameChannel;
	std::vector <wxString> nameEvent;
	std::vector <wxString> nameValueMin;
	std::vector <wxString> nameValueMax;
	std::vector <wxString> nameStopOnMatch;
	std::vector <wxString> nameParam;

	std::vector <wxString> valueName;
	std::vector <wxString> valueAction;
	std::vector <wxString> valueParam;
	std::vector <wxString> valueKey;
	std::vector <int> valueDevice;
	std::vector <int> valueChannel;
	std::vector <wxString> valueEvent;
	std::vector <int> valueMin;
	std::vector <wxString> valueStopOnMatch;

	std::vector <int> shortcutNrSelector;

	int prevShortcutNrSelector = -1 ;

	wxChoice *listFunctionMidi;


	void loadShortcut();
	void saveShortcut();

	void InitLists();

	int edit(long i);
	void close();
	bool changed = false;

	wxDECLARE_EVENT_TABLE();
};

#endif
