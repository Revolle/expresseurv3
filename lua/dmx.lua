-- postprocess the midiout of luabass.

-- This module sends DMX lighting according to midi-out Note-On/Off

-- this script must be loaded dynamically with from expresseur.lua, with the comand : 
-- luabass.onMidiOut("postluabass.lua")
-- DMX-COM port must be already opened 
-- e.g. in expresseur.lua /onStart :
--      luabass.dmxOpen(3) -- DMX on port COM-3
--      luabass.dmxSet(100) -- to have medium tenuto on lights

-- table of DMX values to send
-- 4 lights , with 4 channels RVBW/light
-- map from pitch%12 to RVB light
dmxp={1,2,3,5,6,7,9,10,11,13,14,15}

-- cath midi-out note-on to calculate DMX values
function onNoteon(nrTrack,pitch,velocity)
  -- return the noteon to play without change, and the DMX values
  return nrTrack,pitch,velocity,"Noteon", (dmxp[pitch % 12 + 1]) , ( velocity * 2 ) , "DMX" 
end
-- cath midi-out note-off to calculate DMX values
function onNoteoff(nrTrack,pitch,velocity)
  -- return the noteoff to play without change, and the DMX values
  return nrTrack,pitch,velocity,"Noteoff" ,  (dmxp[pitch % 12 + 1]) , 0 , "DMX" 
end
