// update : 15/11/2016 18:00
#ifndef DEF_BITMAPSCORE

#define DEF_BITMAPSCORE


class bitmapscore
	: public viewerscore
{

public:
	bitmapscore(wxWindow *parent, wxWindowID id, mxconf* lMxconf);
	~bitmapscore();
	void onPaint(wxPaintEvent& event);
	void onIdle(wxIdleEvent& event);
	void OnLeftDown(wxMouseEvent& event);
	void OnLeftUp(wxMouseEvent& event);
	void OnMouse(wxMouseEvent& event);
	virtual bool isOk();
	virtual bool setFile(const wxFileName &lfilename);
	virtual bool displayFile(wxSize sizeClient);
	virtual bool setPosition(int pos, bool playing, bool quick);
	virtual int getTrackCount();
	virtual void zoom(int dzoom);
	virtual wxString getTrackName(int trackNr);
	virtual void gotoPosition();
	virtual void gotoNextPage(bool forward);

private:
	wxWindow *mParent;
	wxImage *mImage;
	mxconf *mConf;
	void newLayout();

	double xScale, yScale;


	wxPoint mPointStart, mPointEnd;
	wxRect prevRect , selectedRect ;
	bool alertSetRect;
	wxRect highlight(bool on, wxPoint start, wxPoint end, wxDC& dc);
	wxRect rectChord[MAX_RECTCHORD];
	int nbRectChord;
	int nrChord;
	int prevNrChord=-1;
	int newNrChord=-1;
	wxFileName fileRectChord;
	void readRectChord();
	void writeRectChord();
	bool newBitmap = false ;
	wxDECLARE_EVENT_TABLE();

};

#endif
