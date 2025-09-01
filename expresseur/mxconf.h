// update : 31 / 10 / 2016 10 : 00
#ifndef DEF_MXCONF

#define DEF_MXCONF

void configSetPrefix(std::vector <wxString> nameOpenMidiOutDevices);
wxConfig *configGet();
void setConfig();
void delConfig();
void configErase();

wxString configWriteFile(wxTextFile *lfile, wxString key, wxString default_value, bool prefix = false, wxString name = "");
long configWriteFile(wxTextFile *lfile, wxString key, long default_value, bool prefix = false, wxString name = "");

bool configReadFile(wxTextFile *lfile, wxString key, wxString default_value, bool prefix = false, wxString name = "");
bool configReadFile(wxTextFile *lfile, wxString key, long default_value, bool prefix = false, wxString name = "");
	
wxString configGet(wxString key, wxString default_value, bool prefix = false, wxString name = "");
long configGet(wxString key, long default_value, bool prefix = false, wxString name = "");

void configSet(wxString key, wxString s, bool prefix = false, wxString name = "");
void configSet(wxString key, long l, bool prefix = false , wxString name = "");

void configRemove(wxString key, bool prefix = false , wxString name = "");
bool configExists(wxString key, bool prefix = false, wxString name = "");
	
wxString getAppDir();
wxString getCwdDir();
wxString getTmpDir();
wxString getResourceDir();
wxString getUserDir();
wxString getConfPath();

#endif
