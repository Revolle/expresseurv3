// update : 15/11/2016 18:00
#ifndef DEF_EMPTYSCORE

#define DEF_EMPTYSCORE


class emptyscore
	: public viewerscore
{

public:
	emptyscore(wxWindow *parent, wxWindowID id, mxconf* lMxconf);
	~emptyscore();
	void onPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	virtual bool isOk();
	virtual bool setFile(const wxFileName &lfilename);
	virtual bool displayFile(wxSize sizeClient);
	virtual void setPosition(int pos, bool playing);
	virtual int getTrackCount();
	virtual void zoom(int dzoom);
	virtual wxString getTrackName(int trackNr);
	virtual void gotoPosition(wxString gotovalue);
	virtual void gotoNextPage(bool forward);


private:
	wxWindow *mParent;
	mxconf *mConf;
	void newLayout();


	wxDECLARE_EVENT_TABLE();

};

#endif
