-- postprocess the midiout of luabass.
-- this script is loaded dynamically. Example :  luabass.onMidiOut("postluabass.lua")
-- global variable of this script can be set . Example : luabass.setVarMidiOut("transpose",10)
transpose = 0
function onNoteon(nrTrack,pitch,velocity)
  return nrTrack,"Noteon",pitch+transpose,velocity
end
function onNoteoff(nrTrack,pitch,velocity)
  return nrTrack,"Noteoff",pitch+transpose,velocity
end
function getTranspose()
  return transpose
end

