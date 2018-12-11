// update : 31 / 10 / 2016 10 : 00
#ifndef DEF_MXCONF

#define DEF_MXCONF

class mxconf 
{
public:
	mxconf();
	~mxconf();

	void setPrefix();
	wxConfig *getConfig();
	void deleteConf();

	wxString writeFile(wxTextFile *lfile, wxString key, wxString default_value, bool prefix = false, wxString name = "");
	long writeFile(wxTextFile *lfile, wxString key, long default_value, bool prefix = false, wxString name = "");

	bool readFile(wxTextFile *lfile, wxString key, wxString default_value, bool prefix = false, wxString name = "");
	bool readFile(wxTextFile *lfile, wxString key, long default_value, bool prefix = false, wxString name = "");

	wxString get(wxString key, wxString default_value, bool prefix = false, wxString name = "");
	long get(wxString key, long default_value, bool prefix = false, wxString name = "");

	void set(wxString key, wxString s, bool prefix = false, wxString name = "");
	void set(wxString key, long l, bool prefix = false , wxString name = "");

	void remove(wxString key, bool prefix = false , wxString name = "");
	bool exists(wxString key, bool prefix = false, wxString name = "");
	
	static wxString getAppDir();
	static wxString getCwdDir();
	static wxString getTmpDir();
	static wxString getResourceDir();
	static wxString getUserDir();
	
private:
	static void setDir() ;
	wxConfig *mConfig;
	wxString mPrefix;
	wxString readFileLines(wxTextFile *lfile, wxString key);
	wxString prefixKey(wxString key, bool prefix, wxString name = "");
	wxString keyNr(wxString key, int nr);
};

#endif
