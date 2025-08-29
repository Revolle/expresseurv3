-- postprocess the midiout of luabass.
-- this script must be loaded dynamicallywith from expresseur.lua, with the comand : 
-- luabass.onMidiOut("postluabass.lua")

-- This module sends DMX lighting according to midi-out Note-On/Off

-- DMX-COM port must be already opened 
-- e.g. in expresseur.lua /onStart :
--      luabass.dmxOpen(3) -- DMC on port COM-3
--      luabass.dmxSet(100) -- to have medium tenuto on lights

-- table of DMX values to send
-- 4 lights , with 4 channels RVBW/light
dmxv={1,2,3,0,5,6,7,0,9,10,11,0,13,14,15,0}
-- map from pitch%12 to RVB light
dmxp={1,2,3,5,6,7,9,10,11,13,14,15}

-- cath midi-out note-on to calculate DMX values
function onNoteon(nrTrack,pitch,velocity)
  -- pitch of the scale => velocity of a DMX-channel
  dmxv[dmxp[pitch % 12 + 1]] = velocity * 2
  luabass.dmxSend(table.unpack(dmxv))
  luabass.logmsg("dmxout " .. table.concat(dmxv,".") )
  return nrTrack,"Noteon",pitch,velocity
end
-- cath midi-out note-off to calculate DMX values
function onNoteoff(nrTrack,pitch,velocity)
  -- pitch of the scale => switch off DMX-channel
  dmxv[dmxp[pitch % 12 + 1]] = 0
  luabass.dmxSend(table.unpack(dmxv))
  luabass.logmsg("dmxout " .. table.concat(dmxv,".") )
  return nrTrack,"Noteoff",pitch,velocity
end

