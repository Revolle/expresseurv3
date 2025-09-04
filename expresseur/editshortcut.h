
#ifndef DEF_EDITSHORTCUT

#define DEF_EDITSHORTCUT


class editshortcut
	: public wxDialog
{

public:
	editshortcut(wxWindow *parent, wxWindowID id, const wxString &title,
		wxString *lname,
		wxString *laction, std::vector <wxString> nameAction,
		wxString *lkey, std::vector <wxString> nameKey,
		wxString *ldevice, std::vector <wxString> nameDevice,  std::vector <wxString> nameOpenDevice ,
		wxString *lchannel, std::vector <wxString> nameChannel,
		wxString *levent, std::vector <wxString> nameEvent,
		wxString *lmin, std::vector <wxString> nameValueMin,
		wxString *lmax, std::vector <wxString> nameValueMax,
		wxString *lparam,
		wxString *lstopOnMatch, std::vector <wxString> namestopOnMatch
		);
	~editshortcut();

	void OnMidi(wxCommandEvent& event);
	void scanMidi( int nr_device, int type_msg, int channel, int value1, int value2);

private:
	wxListBox *listMidi;

	wxChoice *fEvent;
	wxChoice *fTdevice;
	wxChoice *fTchannel;
	wxChoice *fMin;
	std::vector <wxString> nameDevice;
	std::vector <wxString> nameOpenDevice;
	wxDECLARE_EVENT_TABLE();

};

#endif
