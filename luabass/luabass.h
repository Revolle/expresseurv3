// LUA module to drive midi-out
//////////////////////////////////////////////

#define M_SAMPLE_RATE 44100

// name of LUA functions in module luabass
#define soutGetVolume "outGetVolume"
#define soutSetVolume "outSetVolume"
#define soutSetTrackVolume "outSetTrackVolume"
#define soutGetTrackVolume "outGetTrackVolume"
#define soutAllNoteOff "outAllNoteOff"
#define soutGetMidiName "outGetMidiName"
#define soutMidiIsValid "midiOutIsValid"
#define sinMidiIsValid "midiInIsValid"
#define soutListProgramVi "outListProgramVi"
#define soutTrackOpenMidi "outTrackOpenMidi"
#define soutSetTrackInstrument "outSetTrackInstrument"
#define soutSetTrackCurve "outSetTrackCurve"
#define soutTracksClose "outTracksClose"
#define soutTrackOpenVi "outTrackOpenVi"
#define soutTrackMute "outTrackMute"
#define soutSetChordCompensation "outSetChordCompensation"
#define soutSetRandomDelay "outSetRandomDelay"
#define soutSetRandomVelocity "outSetRandomVelocity"
#define soutGetLog "outGetLog"
#define sinGetMidiName "inGetMidiName"
#define sinit "init"
#define sfree "free"
#define sexternalTimer "externalTimer"
