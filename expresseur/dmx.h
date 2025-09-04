#pragma once

#ifndef DEF_DMX

#define DEF_DMX


class dmx
	: public wxDialog
{

public:
	dmx(wxFrame* parent, wxWindowID id, const wxString& title	);
	~dmx();

private:
	void OnSize(wxSizeEvent& event);

	wxFrame* mParent;
	wxDialog* mThis;
	wxSizerFlags sizerFlagMinimumPlace;
	wxSizerFlags sizerFlagMaximumPlace;
	wxDECLARE_EVENT_TABLE();

};

#endif
