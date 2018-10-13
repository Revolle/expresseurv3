#ifndef DEF_TEXTSCORE

#define DEF_TEXTSCORE


class textscore
	: public wxTextCtrl
{

public:
	textscore(wxWindow *parent, wxWindowID id, mxconf* lMxconf);
	~textscore();

	void OnSize(wxSizeEvent& event);

	bool setFile(const wxFileName &lfilename);
	void saveFile(const wxFileName &filename);

	void setEditMode(bool editMode);
	void compileText();
	int scanPosition(bool editMode);
	int getInsertionPoint();
	int getInsertionLine();
	bool needToSave();
	void noNeedToSave();
	void zoom(int z);

	void savePlayback(wxString f);

	void OnKeyUp(wxKeyEvent& event);
	void OnKeyDown(wxKeyEvent& event);
	void OnLeftDown(wxMouseEvent& event);
	void OnLeftUp(wxMouseEvent& WXUNUSED(event));

private:
	wxWindow *mParent;
	mxconf *mConf;

	void setFontSize(int t);
	int sizeFont;

	int oldchordStart, oldchordEnd;
	int oldsectionStart, oldsectionEnd;
	wxTextAttr textAttrRecognized;
	wxTextAttr textAttrNormal;
	wxTextAttr textAttrPosition;
	
	wxString oldText;


	wxDECLARE_EVENT_TABLE();

};

#endif
