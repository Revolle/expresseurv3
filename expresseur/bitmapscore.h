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
	void OnLeftDown(wxMouseEvent& event);
	void OnLeftUp(wxMouseEvent& event);
	void OnMouse(wxMouseEvent& event);
	virtual bool isOk();
	virtual bool setFile(const wxFileName &lfilename);
	virtual bool displayFile(wxSize sizeClient);
	virtual void setPosition(int pos, bool playing);
	virtual int getTrackCount();
	virtual void zoom(int dzoom);
	virtual wxString getTrackName(int trackNr);
	virtual void gotoPosition();
	virtual void gotoNextPage(bool forward);

private:
	void refresh(wxDC& dc, int pos);
	wxWindow *mParent;
	mxconf *mConf;
	
	wxFileName filename;
	wxFileName fileInDC;
	wxFileName fileRectChord;
	wxSize sizePage;
	wxMemoryDC *currentDC;

	bool newLayout(wxSize sizeClient);
	bool setPage();
	void setCursor(wxDC& dc, int pos);

	double xScale, yScale;


	wxPoint mPointStart, mPointEnd;
	wxRect prevRect , selectedRect ;
	int prevPos =1 ;
	int prevPaintPos = -1 ;
	int newPaintPos = -1 ;
	bool alertSetRect;
	wxRect highlight(bool on, wxPoint start, wxPoint end, wxDC& dc);
	wxRect rectChord[MAX_RECTCHORD];
	int nbRectChord;
	int nrChord;
	void readRectChord();
	void writeRectChord();

	wxDECLARE_EVENT_TABLE();

};

#endif
