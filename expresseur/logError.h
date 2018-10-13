#ifndef DEF_LOGERROR

#define DEF_LOGERROR

class logerror
	: public wxDialog
{

public:
	logerror(wxFrame *parent, wxWindowID id, const wxString &title);
	~logerror();

	void OnSize(wxSizeEvent& event);
	
	void OnLogerrorClose(wxCommandEvent& event);
	void OnLogerrorClear(wxCommandEvent& event);

	void scanLog();

private:
	wxSizerFlags sizerFlagMinimumPlace;
	wxSizerFlags sizerFlagMaximumPlace;

	wxListBox *mlog;

	wxDECLARE_EVENT_TABLE();
};

#endif