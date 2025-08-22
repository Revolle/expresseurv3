-- postprocess the midiout of luabass.

-- This module sends DMX lighting according to midi-out Note-On/Off

-- this script must be loaded dynamically with from expresseur.lua, with the comand : 
-- luabass.onMidiOut("postluabass.lua")
-- DMX-COM port must be already opened 
-- e.g. in expresseur.lua /onStart :
--      luabass.dmxOpen(3) -- DMX on port COM-3
--      luabass.dmxSet(100) -- to have medium tenuto on lights

-- 4 lights , with 4 channels RVBW/light
-- map ( pitch%12 ) => ( channels DMX RVB light)
dmxp={0,1,2,4,5,6,8,9,10,12,13,14}

-- DmX on track >=
trackDMX = 2

-- cath midi-out note-on to calculate DMX values
function onNoteon(nrTrack,pitch,velocity)
  if nrTrack >= trackDMX then
    -- return the noteon to play without change, and the DMX values
    return nrTrack,pitch,velocity,"Noteon", (dmxp[pitch % 12 + 1]) , ( velocity * 2 ) , "DMX" 
 else
    -- return the noteon to play without change
    return nrTrack,pitch,velocity,"Noteon" 
  end
end
-- cath midi-out note-off to calculate DMX values
function onNoteoff(nrTrack,pitch,velocity)
  if nrTrack >= trackDMX then
    -- return the noteoff to play without change, and the DMX values
    return nrTrack,pitch,velocity,"Noteoff" ,  (dmxp[pitch % 12 + 1]) , 0 , "DMX" 
  else
    -- return the noteoff to play without change
    return nrTrack,pitch,velocity,"Noteoff" 
  end
end
