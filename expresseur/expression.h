// update : 31 / 10 / 2016 10 : 00
#ifndef DEF_EXPRESSION

#define DEF_EXPRESSION

class expression
	: public wxDialog
{

public:
	expression(wxFrame *parent, wxWindowID id, const wxString &title);
	~expression();

	void OnSize(wxSizeEvent& event);

	void reset();
	void write(wxTextFile *lfile);
	void read(wxTextFile *lfile);

	void OnValue(wxEvent& event);

	void scanValue();

	void savePos();

private:
	wxFrame *mParent;
	wxDialog *mThis;
	std::vector <wxString> nameValue;
	std::vector <wxString> helpValue;
	wxStaticText *txtValue;

	wxSizerFlags sizerFlagMinimumPlace;
	wxSizerFlags sizerFlagMaximumPlace;

	wxSlider *mValue[MAX_EXPRESSION];

	wxDECLARE_EVENT_TABLE();
};

#endif