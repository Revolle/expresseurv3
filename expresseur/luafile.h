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

class luafile
	: public wxDialog
{

public:
	luafile(wxFrame *parent, wxWindowID id, const wxString &title);
	~luafile();

	void OnSize(wxSizeEvent& event);

	static void reset(bool all , int timerDt );
	static void write(wxTextFile *lfile);
	static void read(wxTextFile *lfile);
	static void functioncallback(double time , int nr_device , int type_msg , int channel , int value1 , int value2 , bool isProcessed );
	static bool isCalledback(wxLongLong *time , int *nr_device , int *type_msg , int *channel , int *value1 , int *value2, bool *isProcessed, bool *oneIsProcessed);
	void OnLuaUserFile(wxCommandEvent& event);
	void OnLuaParameter(wxCommandEvent&  event);

private:
	wxFrame *mParent;
	wxDialog *mThis;
	wxSizerFlags sizerFlagMinimumPlace;
	wxSizerFlags sizerFlagMaximumPlace;
	wxDECLARE_EVENT_TABLE();
};

#endif
