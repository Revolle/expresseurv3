// update : 15/11/2016 18:00

#ifndef DEF_MUSICXMLSCORE

#define DEF_MUSICXMLSCORE


class musicxmlscore
	: public viewerscore
{

public:
	musicxmlscore(wxWindow *parent, wxWindowID id, mxconf* config );
	~musicxmlscore();

	void onPaint(wxPaintEvent& event);
	void OnLeftDown(wxMouseEvent& event);
	bool isOk();
	void cleanTmp();
	static void cleanCache(int nbDayCache);

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
	
	int getNbPaint();
	int getNbSetPosition() ;

	virtual bool displayFile(wxSize sizeClient);
	virtual bool setFile(const wxFileName &lfilename);
	virtual int getTrackCount();
	virtual wxString getTrackName(int trackNr);
	virtual void setPosition(int pos, bool playing);
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
	wxString getNamePage(wxFileName fp , int pageNr);
	wxString musescoreexe, musescorescript , musescorepng , musescorepos;

	bool docOK = false ;
	int nrChord;

	float fzoom = 1.0;
	float resolution_dpi = 100.0;
	float tenths = 40.0;
	float millimeters = 7.0;
	float inch = 25.4;
	bool musescore_def_xml = true ;

	int prevPaintPos = -1;
	bool prevPaintPlaying = true;
	int newPaintPos = -1;
	bool newPaintPlaying = true;
	int prevPos = -1;
	bool prevPlaying = true;

	int totalMeasures = 0;
	int totalStaves = 0;
	int totalPages = 0;;
	int currentPos = 0;

	int nbPaint = 0 ;
	int nbSetPosition = 0 ;
	
	wxMemoryDC *currentDC ;
	wxSize sizePage;
	wxRect buttonPage;
	int currentPageNr = 0 ;
	int currentPageNrPartial = -1 ;
	bool currentTurnPage = true ;

	bool setCursor(wxDC& dc , int nrEvent,bool red);
	bool readPos();
	bool setPage(int pos , wxRect *rectPos);

	void crc_init();
	wxULongLong crc_cumulate_file(wxString fname);
	wxULongLong crc_cumulate_string(wxString buf);

	wxDECLARE_EVENT_TABLE();

};

#endif
