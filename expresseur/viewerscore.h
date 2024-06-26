// update : 15/11/2016 18:00
#ifndef DEF_VIEWERSCORE

#define DEF_VIEWERSCORE

class viewerscore : public wxPanel
{
public:
	viewerscore(wxWindow *parent, wxWindowID id);
	virtual ~viewerscore();
	virtual bool isOk() = 0;
	virtual bool setFile(const wxFileName &lfilename) = 0;
	virtual bool displayFile(wxSize clientSize) = 0;
	virtual int getTrackCount() = 0;
	virtual wxString getTrackName(int trackNr) = 0;
	virtual void setPosition(int pos, bool playing ) = 0;
	virtual void zoom(int zoom) = 0;
	virtual void gotoPosition(wxString gotoposition) = 0;
	virtual void gotoNextPage(bool forward) = 0;
};

#endif
