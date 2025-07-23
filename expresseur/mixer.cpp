/////////////////////////////////////////////////////////////////////////////
// Name:        mixer.cpp
// Purpose:     non-modal dialog for the mixer /  expresseur V3
// Author:      Franck REVOLLE
// Modified by:
// Created:     03/04/2015
// update : 03/12/2016 18:00
// Copyright:   (c) Franck REVOLLE Expresseur
// Licence:    Expresseur licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


#include "wx/dialog.h"
#include "wx/filename.h"
#include "wx/stattext.h"
#include "wx/sizer.h"
#include "wx/notebook.h"
#include "wx/bitmap.h"
#include "wx/tglbtn.h"
#include "wx/spinctrl.h"
#include "wx/textfile.h"
#include "wx/statline.h"
#include "wx/config.h"
#include "wx/dir.h"
#include "wx/xml/xml.h"

#include "global.h"
#include "mxconf.h"
#include "viewerscore.h"
#include "mixer.h"
#include "basslua.h"
#include "luabass.h"

enum
{
	ID_MIXER_NEUTRAL=ID_MIXER ,
	ID_MIXER_DEFAULT,
	ID_MIXER_SOLO,
	IDM_MIXER_EXTENSION,
	ID_MIXER_SETTING_ALLNOTEOFF,
	IDM_MIXER_CLOSE,
	ID_MIXER_MAIN_VOLUME,
	ID_MIXER_VOLUME,
	ID_MIXER_SOUND_DEVICE = ID_MIXER_VOLUME + MAX_TRACK,
	ID_MIXER_VIEW = ID_MIXER_SOUND_DEVICE + MAX_TRACK,
	ID_MIXER_CHANNEL = ID_MIXER_VIEW + MAX_TRACK,
	ID_MIXER_INSTRUMENT = ID_MIXER_CHANNEL + MAX_TRACK,
};

wxBEGIN_EVENT_TABLE(mixer, wxDialog)
EVT_SIZE(mixer::OnSize)
EVT_CLOSE(mixer::OnClose)
EVT_BUTTON(ID_MIXER_NEUTRAL, mixer::OnNeutralMixer)
EVT_BUTTON(ID_MIXER_DEFAULT, mixer::OnDefaultMixer)
EVT_BUTTON(ID_MIXER_SETTING_ALLNOTEOFF, mixer::OnSettingAllNoteOff)
EVT_BUTTON(IDM_MIXER_CLOSE, mixer::OnClose)
wxEND_EVENT_TABLE()

mixer::mixer(wxFrame *parent, wxWindowID id, const wxString &title, viewerscore *lviewerscore , wxArrayString lMidiout, wxArrayString lOpenMidiout, bool audio)
: wxDialog(parent, id, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	mParent = parent;
	mThis = this;
	mViewerscore = lviewerscore;
	nbMidioutDevice = lMidiout.GetCount();
	topsizer = NULL;
	// some sizerFlags commonly used
	sizerFlagMaximumPlace.Proportion(1);
	sizerFlagMaximumPlace.Expand();
	sizerFlagMaximumPlace.Border(wxALL, 2);

	sizerFlagMinimumPlace.Proportion(0);
	sizerFlagMinimumPlace.Border(wxALL, 2);

	// set the list of channels
	InitListChannel();
	// get info about the tracks
	getTracks();
	// get info about the midi out devices
	getMidioutDevices(lMidiout, lOpenMidiout, audio);

	BuildSizer();
}
mixer::~mixer()
{
	for (int i = 0; i < MAX_TRACK; i++)
	{
		listInstrumensDevices[i].Clear();
	}
}
void mixer::close()
{
	configSet( CONFIG_MIXER_EXTENSION, mCheckBox->GetValue()?1:0 , true);
}
void mixer::OnClose(wxCloseEvent& event)
{
	if (event.CanVeto())
	{
		Hide();
		event.Veto(true);
	}
	close();
}
void mixer::OnClose(wxCommandEvent& WXUNUSED(event))
{
	Hide();
	close();
}
void mixer::savePos()
{
	wxRect mrect = GetRect();
	configSet( CONFIG_MIXERWIDTH, mrect.GetWidth());
	configSet( CONFIG_MIXERHEIGHT, mrect.GetHeight());
	configSet( CONFIG_MIXERX, mrect.GetX());
	configSet( CONFIG_MIXERY, mrect.GetY());

}
void mixer::BuildSizer()
{
	topsizer = new wxBoxSizer(wxVERTICAL);

	mixchannelSizer = new wxFlexGridSizer(5, wxSize(5, 5));
	mixchannelSizer->AddGrowableCol(1, 1);
	createMixers();
	createMainMixer();
	topsizer->Add(mixchannelSizer, sizerFlagMaximumPlace);
	wxBoxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);


	button_sizer->Add(new wxButton(this, ID_MIXER_DEFAULT, "Default..."), sizerFlagMinimumPlace.Border(wxALL, 10));
	button_sizer->Add(new wxButton(this, ID_MIXER_NEUTRAL, "Neutral"), sizerFlagMinimumPlace.Border(wxALL, 10));

	button_sizer->Add(new wxButton(this, ID_MIXER_SETTING_ALLNOTEOFF, "All Note off"), sizerFlagMinimumPlace.Border(wxALL, 10));
	mCheckBox = new wxCheckBox(this, IDM_MIXER_EXTENSION, "extended channels") ;
	mCheckBox->SetValue(configGet(CONFIG_MIXER_EXTENSION,1, true));
	button_sizer->Add(mCheckBox, sizerFlagMinimumPlace.Border(wxALL, 10));
	button_sizer->Add(new wxButton(this, IDM_MIXER_CLOSE, "Close"), sizerFlagMinimumPlace.Border(wxALL, 10));

	txtValue = new wxStaticText(this, wxID_ANY, "");
	txtValue->SetForegroundColour(*wxLIGHT_GREY);
	txtValue->SetMinSize(wxSize(80, 20));
	button_sizer->Add(txtValue, sizerFlagMaximumPlace);

	topsizer->Add(button_sizer, sizerFlagMinimumPlace);

	// resize all the frame and components
	SetSizerAndFit(topsizer);
}
void mixer::OnSize(wxSizeEvent& WXUNUSED(event))
{
	Layout();
}
void mixer::scanVolume()
{
	// scan the volume which are currently in place, and update the GUI if necessary
	// this scan is called periodically by a global recurrent timer on main thread
	int v;
	basslua_call(moduleLuabass, soutGetVolume , ">i", &v);
	if (v != mainVolume)
	{
		mainVolume = v;
		slmainVolume->SetValue(v);
	}
	for (int nrTrack = 0; nrTrack < nbTrack; nrTrack++)
	{
		basslua_call(moduleLuabass, soutGetTrackVolume, "i>i", nrTrack + 1, &v);
		if (v != trackVolume[nrTrack])
		{
			trackVolume[nrTrack] = v;
			sltrackVolume[nrTrack]->SetValue(v);
		}
	}
}
void mixer::getTracks()
{
	nameTrack.Clear();
	helpTrack.Clear();
	int nrTrack = 0 ;
	if (mViewerscore != NULL)
	{
		nbTrack = mViewerscore->getTrackCount();
		// from the score 
		for (int trackNr = 0; trackNr < nbTrack; trackNr++)
		{
			wxString name;
			wxString help;
			name = mViewerscore->getTrackName(trackNr);
			nameTrack.Add("score:" + name);
			help.Printf(_("track#%d of the score : %s"), trackNr, name);
			helpTrack.Add(help);
		}
	}
	// from lua
	char name[MAXBUFCHAR];
	while (basslua_table(moduleGlobal, tableTracks, nrTrack, fieldName, name, NULL, tableGetKeyValue) == tableGetKeyValue)
	{
		nameTrack.Add(name);
		char help[MAXBUFCHAR] = "";
		basslua_table(moduleGlobal, tableTracks, nrTrack, fieldHelp, help, NULL, tableGetKeyValue);
		helpTrack.Add(help);
		nrTrack++;
	}
	nbTrack = nameTrack.GetCount();
}
void mixer::createMainMixer()
{
	wxStaticText *mControl = new wxStaticText(this, wxID_ANY, "MAIN volume");
	mixchannelSizer->Add(mControl, sizerFlagMinimumPlace);

	long value = configGet(CONFIG_MIXERMAIN,64, true);
	slmainVolume = new wxSlider(this, ID_MIXER_MAIN_VOLUME, value, 0, 127);
	slmainVolume->SetHelpText("Main volume which influence all tracks");
	slmainVolume->Bind(wxEVT_SLIDER, &mixer::OnMainMixerVolume, this);
	mixchannelSizer->Add(slmainVolume, sizerFlagMaximumPlace);

}
void mixer::OnMainMixerVolume(wxEvent& WXUNUSED(event))
{
	if ( mainVolume != slmainVolume->GetValue() )
	{
		mainVolume = slmainVolume->GetValue();

		configSet( CONFIG_MIXERMAIN, mainVolume, true);

		basslua_call(moduleLuabass, soutSetVolume, "i", mainVolume);

		wxString s;
		s.Printf("main %d", mainVolume);
		txtValue->SetLabel(s);
	}
}
void mixer::allNoteOff()
{
	basslua_call(moduleLuabass, soutAllNoteOff, "si", "n",0);
}
void mixer::OnSettingAllNoteOff(wxCommandEvent& WXUNUSED(event))
{
	allNoteOff();
}
void mixer::createMixers()
{
	for (unsigned int nrTrack = 0; nrTrack < nameTrack.GetCount(); nrTrack++)
	{
		AddMixerName(nameTrack[nrTrack]);
		AddMixerVolume(nrTrack);
		AddSoundDevice(nrTrack);
		AddSoundChannel(nrTrack);
		AddSoundInstrument(nrTrack);
	}
}
void mixer::AddMixerName( wxString label)
{
	wxStaticText *mControl = new wxStaticText(this, wxID_ANY, label);
	mixchannelSizer->Add(mControl, sizerFlagMinimumPlace);
}
void mixer::AddMixerVolume(int nrTrack)
{
	trackVolume[nrTrack] = configGet(CONFIG_MIXERVOLUME, 64, true, nameTrack[nrTrack]);

	sltrackVolume[nrTrack] = new wxSlider(this, ID_MIXER_VOLUME + nrTrack, trackVolume[nrTrack], 0, 127);
	sltrackVolume[nrTrack]->SetToolTip(helpTrack[nrTrack]);
	sltrackVolume[nrTrack]->Bind(wxEVT_SLIDER, &mixer::OnMixerVolume, this);
	mixchannelSizer->Add(sltrackVolume[nrTrack], sizerFlagMaximumPlace);
}
void mixer::OnMixerVolume(wxEvent& event)
{
	int nrTrack = event.GetId() - ID_MIXER_VOLUME ;
	if ( trackVolume[nrTrack] != sltrackVolume[nrTrack]->GetValue())
	{
		trackVolume[nrTrack] = sltrackVolume[nrTrack]->GetValue();

		configSet(CONFIG_MIXERVOLUME, trackVolume[nrTrack], true, nameTrack[nrTrack]);
		basslua_call(moduleLuabass, soutSetTrackVolume, "ii", trackVolume[nrTrack], nrTrack + 1);

		wxString s;
		s.Printf("trackVolume%d %d", nrTrack + 1, trackVolume[nrTrack]);
		txtValue->SetLabel(s);
	}
}
void mixer::OnDefaultMixer(wxCommandEvent& WXUNUSED(event))
{
	wxString s;
	s.Printf("%s : %s", lastDevice, _("Set this last device used as default ?"));
	if (wxMessageBox(s, "default device", wxYES_NO, this) == wxYES)
	{
		defaultDevice = lastDevice;
		configSet(CONFIG_MIXERDEVICEDEFAULT, defaultDevice, true);
	}
}
void mixer::OnNeutralMixer(wxCommandEvent& WXUNUSED(event))
{
	for (unsigned int nrTrack = 0; nrTrack < nameTrack.GetCount(); nrTrack++)
	{
		sltrackVolume[nrTrack]->SetValue(64);
		configSet(CONFIG_MIXERVOLUME, 64, true, nameTrack[nrTrack]);
		basslua_call(moduleLuabass, soutSetTrackVolume, "ii", 64, nrTrack + 1);
	}

	slmainVolume->SetValue(64);
	basslua_call(moduleLuabass, soutSetVolume, "i", 64);
	configSet(CONFIG_MIXERMAIN, 64, true);
}
void mixer::setMixerVolume(int nrTrack, int value)
{
	sltrackVolume[nrTrack]->SetValue(value);
	configSet(CONFIG_MIXERVOLUME, value, true, nameTrack[nrTrack]);
}
void mixer::setMainVolume(int value)
{
	slmainVolume->SetValue(value);
	configSet( CONFIG_MIXERMAIN, 64, true);
}
void mixer::InitListChannel()
{
	// list of channels 1..16
	wxString s;
	for (int i = 0; i < 16; i++)
	{
		s.Printf("%s.%d", "ch", i + 1);
		nameChannel.Add(s);
	}
}
void mixer::getMidiVi(wxString fullNameDevice, int *nrDevice, wxString *nameVi)
{
	wxString nameDevice;
	wxString typeDevice = fullNameDevice.BeforeFirst(':', &nameDevice);
	*nameVi = NULL_STRING;
	*nrDevice = NULL_INT;
	if (nameDevice.IsEmpty())
	{
		return;
	}
	if (typeDevice.Contains(NSF2) || typeDevice.Contains(NVST))
	{
		if (typeDevice.Contains(NSF2))
			*nameVi = nameDevice + "." + SSF2;
		else
			*nameVi = nameDevice + "." + SVST;
		if (listVIused.Index(nameDevice) == wxNOT_FOUND)
			listVIused.Add(nameDevice);
		*nrDevice = VI_ZERO + listVIused.Index(nameDevice);
	}
	else
	{
		for (unsigned int i = 0; i < nameMidiDevices.GetCount(); i++)
		{
			wxString s = nameMidiDevices[i].AfterFirst(':');
			if (s == nameDevice)
				*nrDevice = i;
		}
	}
}
wxString mixer::setMidiVi( wxString nameMidi, wxString nameVi, wxString extVi)
{
	wxString s;
	if (nameVi.IsEmpty())
		s.Printf("%s:%s", SMIDI, nameMidi);
	else
	{
		if (extVi.IsSameAs(SSF2,false))
			s.Printf("%s:%s", NSF2, nameVi);
		else
			s.Printf("%s:%s", NVST, nameVi);
	}
	return s;
}
void mixer::getMidioutDevices(wxArrayString lMidiout, wxArrayString lOpenMidiout, bool audio)
{
	// list all midi devices ( midiout and VI )

	wxString s ;
	wxString name;
	nameDevices.Clear();
	nameMidiDevices.Clear();
	nameDevices.Add(_("(no output)"));

	for (int i = 0; i < lOpenMidiout.GetCount(); i++)
	{
		nameDevices.Add(setMidiVi(lOpenMidiout[i], "", ""));
		getListMidioutDevice(lOpenMidiout[i], nameDevices.GetCount() - 1);
	}
	for (int i = 0; i < lMidiout.GetCount(); i++)
	{
		nameMidiDevices.Add(setMidiVi(lMidiout[i], "", ""));
	}

	wxString firstDeviceDefault;

	if (audio)
	{

		wxArrayString filesVI;
		wxString sdir = getResourceDir();
		wxFileName fvi;
		fvi.AssignDir(sdir);
		wxDir::GetAllFiles(sdir, &filesVI, "*.sf2", wxDIR_DEFAULT);
#ifdef VST
		wxDir::GetAllFiles(sdir, &filesVI, "*.dll", wxDIR_DEFAULT);
#endif
		for (unsigned int nrVi = 0; nrVi < filesVI.Count(); nrVi++)
		{
			fvi.Assign(filesVI[nrVi]);
			if ( createViList(fvi.GetName(), fvi.GetExt()) )
				name = setMidiVi("", fvi.GetName(), fvi.GetExt());
			nameDevices.Add(name);
			if (defaultDevice.IsEmpty())
				defaultDevice = name;
			if (name.Contains("default_"))
				firstDeviceDefault = name;
			getListMidioutDevice(fvi.GetName(), nameDevices.GetCount() - 1);
		}
	}
	if (firstDeviceDefault.IsEmpty() == false)
		defaultDevice = firstDeviceDefault;
	
	defaultDevice = configGet(CONFIG_MIXERDEVICEDEFAULT, defaultDevice, true);

	lastDevice = defaultDevice;
}
bool mixer::createViList(wxString fileVI , wxString ext)
{
	// create if necessary the list of programs inside the VI. It is stored int the text file with same name than the VI
	wxFileName flist;
	flist.AssignDir(getResourceDir());
	flist.SetName(fileVI);
	flist.SetExt(ext);
	int retCode;
	char buffileVI[MAXBUFCHAR];
	wxString sff = flist.GetFullPath();
	strcpy(buffileVI, sff.c_str());
	basslua_call(moduleLuabass, soutListProgramVi, "s>i", buffileVI, &retCode);
	return (retCode);
}
void mixer::getListMidioutDevice(wxString fileName , int nrDevice)
{
	// get the list of instrument from the txt file, associated to the name of the device
	wxString fname = fileName;

	// try to suppress the prefix "X- "
	if (fileName.Mid(1, 2) == "- ")
		fname = fileName.Mid(3);

	// suppress the @audio-device
	fname = fname.BeforeFirst('@');

	wxFileName flist;
	flist.AssignDir(getResourceDir());
	flist.SetName(fname);
	flist.SetExt("txt");

	if (!flist.IsFileReadable())
	{
		if ((nrDevice - 1 ) < nbMidioutDevice)
		{
			flist.SetName("gm"); // default general midi list of instruments for MidiOut devices
			if (!flist.IsFileReadable())
				return;
		}
		else
			return;
	}

	wxTextFile tfile;
	tfile.Open(flist.GetFullPath());
	if (tfile.IsOpened() == false)
		return;

	wxString str;
	str = tfile.GetFirstLine();
	while (!tfile.Eof())
	{
		if (str.IsEmpty() == false)
		{
			listInstrumensDevices[nrDevice].Add(str);
		}
		str = tfile.GetNextLine();
	}
	tfile.Close();
}
void mixer::AddSoundDevice(int nrTrack)
{
	mSoundDevice[nrTrack] = new wxChoice(this, ID_MIXER_SOUND_DEVICE + nrTrack, wxDefaultPosition, wxDefaultSize, nameDevices);

	wxString svalue = configGet(CONFIG_MIXERDEVICENAME, defaultDevice, true, nameTrack[nrTrack]);

	int nrItem = mSoundDevice[nrTrack]->FindString(svalue);
	if (nrItem != wxNOT_FOUND)
		mSoundDevice[nrTrack]->SetSelection(nrItem);
	else
	{
		nrItem = mSoundDevice[nrTrack]->FindString(defaultDevice);
		if (nrItem != wxNOT_FOUND)
			mSoundDevice[nrTrack]->SetSelection(nrItem);
		else
		{
			if (mSoundDevice[nrTrack]->GetCount() > 0 )
				mSoundDevice[nrTrack]->SetSelection(0);
		}
	}
	configSet(CONFIG_MIXERDEVICENAME, mSoundDevice[nrTrack]->GetStringSelection(), true, nameTrack[nrTrack]);

	mSoundDevice[nrTrack]->Bind(wxEVT_CHOICE, &mixer::OnSoundDevice, this);
	mixchannelSizer->Add(mSoundDevice[nrTrack], sizerFlagMinimumPlace);
}
void mixer::OnSoundDevice(wxEvent& event)
{
	int nrTrack = event.GetId() - ID_MIXER_SOUND_DEVICE;
	wxChoice *mControl = (wxChoice*)(event.GetEventObject());
	unsigned int nrDevice = mControl->GetSelection();

	wxString s1;
	configSet(CONFIG_MIXERDEVICENAME, nameDevices[nrDevice], true, nameTrack[nrTrack]);
	
	lastDevice = nameDevices[nrDevice];

	wxString instrument;
	if (nrDevice < listInstrumensDevices->GetCount())
	{
		mInstrument[nrTrack]->Set(listInstrumensDevices[nrDevice]);
		if (listInstrumensDevices[nrDevice].GetCount() > 0)
			instrument = listInstrumensDevices[nrDevice].Item(0);
		else
			instrument = "";
		mInstrument[nrTrack]->SetValue(instrument);
	}

	configSet(CONFIG_MIXERINSTRUMENT, instrument, true, nameTrack[nrTrack]);

	replicate(nrTrack);
	Layout();
	reset();
}
void mixer::AddSoundChannel(int nrTrack)
{
	wxChoice *mControl = new wxChoice(this, ID_MIXER_CHANNEL + nrTrack, wxDefaultPosition, wxDefaultSize, nameChannel);

	long value = 0 ;
	if (mSoundDevice[nrTrack]->GetSelection() != wxNOT_FOUND)
	{
		value = configGet(CONFIG_MIXERCHANNEL, 0, true, nameTrack[nrTrack]);
	}

	mControl->SetSelection(value);
	mControl->Bind(wxEVT_CHOICE, &mixer::OnSoundChannel, this);
	mixchannelSizer->Add(mControl, sizerFlagMinimumPlace);
}
void mixer::OnSoundChannel(wxEvent& event)
{
	int nrTrack =event.GetId() - ID_MIXER_CHANNEL ;
	wxChoice *mControl = (wxChoice*)(event.GetEventObject());
	int value = mControl->GetSelection();

	configSet(CONFIG_MIXERCHANNEL, value, true, nameTrack[nrTrack]);

	replicate(nrTrack);

	reset();

}
void mixer::AddSoundInstrument(int nrTrack)
{
	wxString instrument;
	wxString s1;
	if (mSoundDevice[nrTrack]->GetSelection() != wxNOT_FOUND)
	{
		instrument = configGet(CONFIG_MIXERINSTRUMENT, "", true, nameTrack[nrTrack]);
		mInstrument[nrTrack] = new wxComboBox(this, ID_MIXER_INSTRUMENT + nrTrack, instrument, wxDefaultPosition, wxDefaultSize,  listInstrumensDevices[mSoundDevice[nrTrack]->GetSelection()]  );
	}
	else
	{
		wxArrayString e;
		mInstrument[nrTrack] = new wxComboBox(this, ID_MIXER_INSTRUMENT + nrTrack, instrument, wxDefaultPosition, wxDefaultSize, e);
	}

	mInstrument[nrTrack]->Bind(wxEVT_TEXT, &mixer::OnSoundInstrument, this);
	mInstrument[nrTrack]->SetToolTip(_("name : [[Bank_MSB/]Bank_LSB/]Program [Control/Value]*"));
	mixchannelSizer->Add(mInstrument[nrTrack], sizerFlagMaximumPlace);
}
void mixer::OnSoundInstrument(wxEvent& event)
{
	int nrTrack = event.GetId() - ID_MIXER_INSTRUMENT;
	wxString value = mInstrument[nrTrack]->GetValue();

	configSet(CONFIG_MIXERINSTRUMENT, value, true, nameTrack[nrTrack]);

	char bufInstrument[MAXBUFCHAR];
	strcpy(bufInstrument, value.c_str());
	basslua_call(moduleLuabass, soutSetTrackInstrument, "si", bufInstrument, nrTrack + 1);

	replicate(nrTrack);
}
void mixer::replicate(int nrTrack)
{
	int channel_nrTrack = configGet(CONFIG_MIXERCHANNEL, 0, true, nameTrack[nrTrack]);
	wxString device_nrTrack = configGet(CONFIG_MIXERDEVICENAME, defaultDevice, true, nameTrack[nrTrack]);
	for (int n = 0; n < nbTrack; n++)
	{
		if (n != nrTrack)
		{
			int channel_n = configGet(CONFIG_MIXERCHANNEL, 0, true, nameTrack[n]);
			wxString device_n = configGet(CONFIG_MIXERDEVICENAME, defaultDevice, true, nameTrack[n]);
			if ((channel_nrTrack == channel_n) && (device_nrTrack == device_n))
			{
				wxString value = configGet(CONFIG_MIXERINSTRUMENT, "", true, nameTrack[nrTrack]);
				mInstrument[n]->ChangeValue(value);
				configSet(CONFIG_MIXERINSTRUMENT, value, true, nameTrack[n]);
			}
		}
	}
}
void mixer::reset(bool localoff ,bool doreset)
{
	//mlog_in("mixer / reset / start");

	if ((loading == true) && (doreset == false))
		return;
	if (doreset)
		loading = false;

	if ((nbTrack < 0) || (nbTrack >= MAX_TRACK))
		return;

	// reset all the MIDI out configuration

	wxString s1, sdevice[MAX_TRACK], instrument[MAX_TRACK], nameVi[MAX_TRACK];
	int channel[MAX_TRACK], volume[MAX_TRACK], nrDevice[MAX_TRACK];

	for (int i = 0; i < MAX_TRACK; i++)
	{
		channel[i] = NULL_INT;
		volume[i] = NULL_INT;
		nrDevice[i] = NULL_INT;
		sdevice[i] = NULL_STRING;
		instrument[i] = NULL_STRING;
		nameVi[i] = NULL_STRING;
	}

	int mainvolume = configGet( CONFIG_MIXERMAIN,64,true);

	//mlog_in("mixer / reset / nbTrack=%d",nbTrack);
	for (int nrTrack = 0; nrTrack < nbTrack; nrTrack++)
	{
		sdevice[nrTrack] = configGet(CONFIG_MIXERDEVICENAME, defaultDevice, true, nameTrack[nrTrack]);
		getMidiVi(sdevice[nrTrack], &(nrDevice[nrTrack]), &(nameVi[nrTrack]));
		channel[nrTrack] = configGet(CONFIG_MIXERCHANNEL, 0, true, nameTrack[nrTrack]);
		volume[nrTrack] = configGet(CONFIG_MIXERVOLUME, 64, true, nameTrack[nrTrack]);
		instrument[nrTrack] = configGet(CONFIG_MIXERINSTRUMENT, "", true, nameTrack[nrTrack]);
	}

	//mlog_in("mixer / reset / soutTracksClose");
	basslua_call(moduleLuabass, soutTracksClose, "");

	// calculation of extended channel per device
	int channelUsed[OUT_MAX_DEVICE][MAXCHANNEL], channelPerDevice[OUT_MAX_DEVICE], additionnalChannelPerDevice[OUT_MAX_DEVICE];
	for (int nr_device = 0; nr_device < OUT_MAX_DEVICE; nr_device++)
		for (int nr_channel = 0; nr_channel < MAXCHANNEL; nr_channel++)
			channelUsed[nr_device][nr_channel] = false;
	for (int nr_track = 0; nr_track < nbTrack; nr_track++)
	{
		if (nrDevice[nr_track] != NULL_INT)
		{
			channelUsed[nrDevice[nr_track]][channel[nr_track]] = true;
		}
	}
	for (int nr_device = 0; nr_device < OUT_MAX_DEVICE; nr_device++)
	{
		if ( mCheckBox->GetValue() )
		{
			channelPerDevice[nr_device] = 0;
			for (int nr_channel = 0; nr_channel < MAXCHANNEL; nr_channel++)
			{
				if (channelUsed[nr_device][nr_channel])
					channelPerDevice[nr_device]++;
			}
			switch (channelPerDevice[nr_device])
			{
			case 0: additionnalChannelPerDevice[nr_device] = 8; break;
			case 1: additionnalChannelPerDevice[nr_device] = 8; break;
			case 2: additionnalChannelPerDevice[nr_device] = 5; break;
			case 3: additionnalChannelPerDevice[nr_device] = 4; break;
			case 4: additionnalChannelPerDevice[nr_device] = 2; break;
			case 5: additionnalChannelPerDevice[nr_device] = 2; break;
			case 6: additionnalChannelPerDevice[nr_device] = 1; break;
			default: additionnalChannelPerDevice[nr_device] = 0; break;
			}
		}
		else
			additionnalChannelPerDevice[nr_device] = 0;
	}

	//mlog_in("mixer / reset / setTracks");
	for (int nrTrack = nbTrack - 1; nrTrack >= 0; nrTrack--)
	{
		if ((nrDevice[nrTrack] != NULL_INT) && (nrDevice[nrTrack] < VI_ZERO))
		{
			char bufNameTrack[512];
			strcpy(bufNameTrack, nameTrack[nrTrack].c_str());
			char bufInstrument[512];
			strcpy(bufInstrument, instrument[nrTrack].c_str());
			int n = nrTrack + 1;
			//mlog_in("mixer / reset / setTrack %s(%d)",soutTrackOpenMidi,nrTrack);
			if ((nrDevice[nrTrack] < MAX_TRACK) && (nrDevice[nrTrack] >= 0))
			{
				basslua_call(moduleLuabass, soutTrackOpenMidi, "iisiisi", n, channel[nrTrack] + 1, bufInstrument, nrDevice[nrTrack] + 1, additionnalChannelPerDevice[nrDevice[nrTrack]], bufNameTrack, localoff);
				//mlog_in("mixer / reset / setTrack tableSetKeyValue");
				basslua_table(moduleGlobal, tableTracks, -1, bufNameTrack, NULL, &n, tableSetKeyValue);
				//mlog_in("mixer / reset / setTrack tableCallKeyFunction");
				basslua_table(moduleGlobal, tableTracks, n, fieldCallFunction, bufNameTrack, &n, tableCallKeyFunction | tableCallTableFunction);
			}
		}
		else if (nameVi[nrTrack] != NULL_STRING)
		{
			char bufNameTrack[512];
			strcpy(bufNameTrack, nameTrack[nrTrack].c_str());
			char bufInstrument[512];
			strcpy(bufInstrument, instrument[nrTrack].c_str());
			char bufNameVi[512];
			wxFileName fvi;
			fvi.AssignDir(getResourceDir());
			fvi.SetFullName(nameVi[nrTrack]);
			wxString svi = fvi.GetFullPath();
			strcpy(bufNameVi, svi.c_str());
			int n = nrTrack + 1;
			if ((nrDevice[nrTrack] < MAX_TRACK) && (nrDevice[nrTrack] >= 0))
			{
				//mlog_in("mixer / reset / setTrack VI=%d",nrTrack);
				basslua_call(moduleLuabass, soutTrackOpenVi, "iissi", n, channel[nrTrack] + 1, bufInstrument, bufNameVi, additionnalChannelPerDevice[nrDevice[nrTrack]]);
				basslua_table(moduleGlobal, tableTracks, -1, bufNameTrack, NULL, &n, tableSetKeyValue);
				basslua_table(moduleGlobal, tableTracks, n, fieldCallFunction, bufNameTrack, &n, tableCallKeyFunction | tableCallTableFunction);
			}
		}
	}

	
	//mlog_in("mixer / reset / setVolume");
	basslua_call(moduleLuabass, soutSetVolume, "i", mainvolume);
	for (int nrTrack = 0; nrTrack < nbTrack; nrTrack++)
	{
		basslua_call(moduleLuabass, soutSetTrackVolume, "ii", volume[nrTrack], nrTrack + 1);
	}
	//mlog_in("mixer / reset / end");
}
void mixer::write(wxTextFile *lfile)
{
	configWriteFile(lfile, CONFIG_MIXERMAIN,64,true);
	configWriteFile(lfile, CONFIG_MIXER_EXTENSION,1,true);
	for (unsigned int nrTrack = 0; nrTrack < nameTrack.GetCount(); nrTrack++)
	{
		configWriteFile(lfile, CONFIG_MIXERDEVICENAME, defaultDevice, true, nameTrack[nrTrack]);
		configWriteFile(lfile, CONFIG_MIXERCHANNEL, 0, true, nameTrack[nrTrack]);
		configWriteFile(lfile, CONFIG_MIXERINSTRUMENT, "", true, nameTrack[nrTrack]);
		configWriteFile(lfile, CONFIG_MIXERVOLUME, 64, true, nameTrack[nrTrack]);
	}
}
void mixer::read(wxTextFile *lfile)
{
	configReadFile(lfile, CONFIG_MIXERMAIN,64);
	configReadFile(lfile, CONFIG_MIXER_EXTENSION,1);
	for (unsigned int nrTrack = 0; nrTrack < nameTrack.GetCount(); nrTrack++)
	{
		configReadFile(lfile, CONFIG_MIXERDEVICENAME, defaultDevice, true, nameTrack[nrTrack]);
		configReadFile(lfile, CONFIG_MIXERCHANNEL, 0, true, nameTrack[nrTrack]);
		configReadFile(lfile, CONFIG_MIXERINSTRUMENT, "", true, nameTrack[nrTrack]);
		configReadFile(lfile, CONFIG_MIXERVOLUME, 64, true, nameTrack[nrTrack]);
	}
}
