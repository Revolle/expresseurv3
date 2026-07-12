// update : 15/11/2016 18:00
#ifndef DEF_BITMAPSCORE

#define DEF_BITMAPSCORE


class bitmapscore
	: public viewerscore
{

public:
	bitmapscore(wxWindow *parent, wxWindowID id);
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
	virtual void gotoPosition(wxString gotovalue);
	virtual void gotoNextPage(bool forward);

private:
	void refresh(int pos);
	wxWindow *mParent;
	
	wxFileName filename;
	wxFileName fileInDC;
	wxFileName fileRectChord;
	wxSize sizePage;

	bool newLayout(wxSize sizeClient);
	bool setPage(bool redraw);
	bool setCursor(int pos , bool redraw);

	double xScale, yScale;

	wxBitmap scoreBitmap;

	wxPoint mPointStart, mPointEnd;
	wxRect prevRectPos, currentRectPos;
	wxRect prevRect , selectedRect ;
	int prevNrChord =1 ;
	int prevPaintNrChord = -1 ;
	int newPaintNrChord = -1 ;
	bool alertSetRect;
	wxRect highlight(bool on, wxPoint start, wxPoint end, wxDC& dc);
	wxRect rectChord[MAX_RECTCHORD];
	int nbRectChord;
	int nrChord = -1;
	void readRectChord();
	void writeRectChord();

	wxDECLARE_EVENT_TABLE();

};

#endif
