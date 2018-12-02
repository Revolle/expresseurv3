// update : 31 / 10 / 2016 10 : 00
#ifndef DEF_LUAFILE

#define DEF_LUAFILE

class c_eventMidi
{
public:
	c_eventMidi(	wxLongLong itime , int inr_device, int itype_msg, int ichannel, int ivalue1, int ivalue2, bool iisProcessed);
	wxLongLong time ; 
	int nr_device; int type_msg; int channel; int value1; int value2 , isProcessed ; 
	static bool OneIsProcessed ;
};
WX_DECLARE_LIST(c_eventMidi, l_eventMidi);

class luafile
	: public wxDialog
{

public:
	luafile(wxFrame *parent, wxWindowID id, const wxString &title, mxconf* lMxconf);
	~luafile();

	void OnSize(wxSizeEvent& event);

	static void reset(mxconf* mConf, bool all , int timerDt );
	static void write(mxconf* mConf, wxTextFile *lfile);
	static void read(mxconf* mConf, wxTextFile *lfile);
	static void functioncallback(double time , int nr_device , int type_msg , int channel , int value1 , int value2 , bool isProcessed );
	static bool isCalledback(wxLongLong *time , int *nr_device , int *type_msg , int *channel , int *value1 , int *value2, bool *isProcessed, bool *oneIsProcessed);
	void OnLuaFile(wxCommandEvent& event);
	void OnLuaParameter(wxCommandEvent&  event);

private:
	wxFrame *mParent;
	wxDialog *mThis;
	mxconf* mConf;
	wxSizerFlags sizerFlagMinimumPlace;
	wxSizerFlags sizerFlagMaximumPlace;
	wxDECLARE_EVENT_TABLE();
};

#endif
