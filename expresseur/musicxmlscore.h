// update : 15/11/2016 18:00

#ifndef DEF_MUSICXMLSCORE

#define DEF_MUSICXMLSCORE

struct spos { unsigned int x1, y1, x2, y2; };
struct sfpos { float x1, y1, x2, y2; };
struct sposly { unsigned int line, column; };
class cposnote
{
public:
	bool empty;
	sposly ply;
	spos png;
	sfpos pdf;
	unsigned int page;
};


class musicxmlscore
	: public viewerscore
{

public:
	musicxmlscore(wxWindow *parent, wxWindowID id );
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

	bool setPlayVisible(wxString sin);

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
	virtual void zoom(int) {} ;
	virtual void gotoPosition(wxString gotovalue);
	virtual void gotoNextPage(bool forward);

private:
	wxWindow *mParent;
	wxFileName xmlName;
	musicxmlcompile *xmlCompile = NULL;
	bool xmlExtractXml(wxFileName f);
	bool newLayout(wxSize sizeClient);
	wxString getNamePage(int pageNr);

	bool docOK = false ;
	int nrChord;

	bool lily_def_xml = true ;

	int newPaintPlaying = -1;
	int newPaintPos = -1;
	int prevPos = -1;
	bool prevPlaying = true;

	int totalMeasures = 0;
	int totalStaves = 0;
	int totalPages = 0;;
	int currentPos = 0;

	int nbPaint = 0 ;
	int nbSetPosition = 0 ;
	
	wxSize sizePage;
	wxRect buttonPage;
	wxRect prevRectPos;
	bool prevNrOrnament = false ;
	int currentPageNr = 0 ;
	int currentPageNrPartial = -1 ;
	bool currentTurnPage = true ;
	wxString lilypos, expresseurpos , lilysrc;

	void setCursor(wxDC& dc , int nrEvent,bool playing ,bool redraw);
	bool readlilypos();
	bool readlilypdf(uint32_t page , uint32_t xpng, uint32_t ypng);
	bool readpngsize(uint32_t* xpng, uint32_t* ypng);
	bool musicxmlscore::readlilypond();
	bool readPos();
	bool setPage(wxDC& dc, int pageNr, bool turnPage, bool redraw);
	wxBitmap *scoreBitmap = NULL ;
	int prev_absolute_measure_nr = NULL_INT;

	void crc_init();
	std::uint64_t crc_cumulate_file(wxString fname);
	std::uint64_t crc_cumulate_string(wxString buf);

	std::vector <cposnote> lposnotes;

	wxDECLARE_EVENT_TABLE();

};

#endif
