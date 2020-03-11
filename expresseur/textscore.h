#ifndef DEF_TEXTSCORE

#define DEF_TEXTSCORE


class textscore
	: public wxTextCtrl
{

public:
	textscore(wxWindow *parent, wxWindowID id, mxconf* lMxconf);
	~textscore();

	void OnSize(wxSizeEvent& event);
	void OnLeftDown(wxMouseEvent& event);

	bool setFile(const wxFileName &lfilename);
	void saveFile(const wxFileName &filename);

	void setEditMode(bool editMode);
	void compileText();
	int scanPosition();
	void scanTextPosition();
	bool needToSave();
	void noNeedToSave();
	void zoom(int z);
	void savePlayback(wxString f);

private:
	wxWindow *mParent;
	mxconf *mConf;
	bool editMode = false ;
	void setFontSize(int t);
	int sizeFont;
	long previousLinePos = -1;
	int oldchordStart, oldchordEnd;
	int prevInsertionPoint = -1;
	wxTextAttr textAttrRecognized;
	wxTextAttr textAttrNormal;
	wxTextAttr textAttrPosition;
	wxString oldText;


	wxDECLARE_EVENT_TABLE();

};

#endif
