// update : 15/11/2016 18:00

#ifndef DEF_MUSICXMLSCORE

#define DEF_MUSICXMLSCORE


class musicxmlscore
	: public viewerscore
{

public:
	musicxmlscore(wxWindow *parent, wxWindowID id, mxconf* config );
	~musicxmlscore();

	void onIdle(wxIdleEvent& event);
	void onPaint(wxPaintEvent& event);
	void OnLeftDown(wxMouseEvent& event);
	bool isOk();
	void cleanTmp();
	static void cleanCache();

	void xmlProcessScore();
	void xmlProcessPartList(wxXmlNode *xmlParentIn, wxXmlNode *xmlParentOut);
	wxXmlNode *GetXmlPath(wxXmlNode *parent, const wxString name, wxString *content);
	bool getScorePosition(int *absolute_measure_nr, int *measure_nr, int *repeat , int *beat, int *t);

	void setPlayVisible(wxString sin);

	void initRecordPlayback();
	void initPlayback();
	void recordPlayback(wxLongLong time, int nr_device, int type_msg, int channel, int value1, int value2);
	bool playback();
	wxString getPlayback();

	virtual bool displayFile(wxSize sizeClient);
	virtual bool setFile(const wxFileName &lfilename);
	virtual int getTrackCount();
	virtual wxString getTrackName(int trackNr);
	virtual bool setPosition(int pos, bool playing, bool quick);
	virtual void zoom(int zoom);
	virtual void gotoPosition();
	virtual void gotoNextPage(bool forward);

private:
	wxWindow *mParent;
	mxconf *mConf;
	wxFileName xmlName;
	musicxmlcompile *xmlCompile = NULL;
	bool xmlExtractXml(wxFileName f);
	bool newLayout(wxSize sizeClient);
	
	wxString musescoreexe, musescorescript , musescorepng , musescorepos;

	bool docOK = false ;
	int nrChord;

	float fzoom = 1.0;
	float resolution_dpi = 100.0;
	float tenths = 40.0;
	float millimeters = 7.0;
	float inch = 25.4;
	bool musescore_def_xml = true ;

	int newPos = 0 ;
	bool newPlaying = false ;
	bool newQuick = false ;

	int totalMeasures = 0;
	int totalStaves = 0;
	int totalPages = 0;;
	int currentNrEvent = 0;
	wxBitmap currentBitmap;
	int prevPos = -1;
	bool prevPlaying = true;
	wxRect rectPrevPos;
	wxBrush originalBackground;
	bool setCursor(int nrEvent,bool red, wxDC& dc);
	bool readPos();
	bool setPage(int pageNr, bool turnPage, wxDC& dc);
	bool drawpage(int pageNr,wxDC& dc , int full = 0);
	void setPageNr(int ipageNr, wxDC& dc);
	int currentPageNrFull = 0;
	int currentPageNrPartial = 0;
	wxSize sizePage;
	wxRect buttonPage;

	void crc_init();
	wxLongLong crc_cumulate_file(wxString fname);
	wxLongLong crc_cumulate_string(wxString buf);

	wxDECLARE_EVENT_TABLE();

};

#endif
