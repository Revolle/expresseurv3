-- postprocess the midiout of luabass.
-- this script must be loaded dynamicallywith from expresseur.lua, with the comand : 
-- luabass.onMidiOut("postluabass.lua")

-- This module sends DMX lighting according to midi-out Note-On/Off

-- COM port to use for DMX
-- can be set from expresseur.lua, with the command :
-- luabass.setVarMidiOut("dmxport","COM1")
dmxport="COM1"

-- table of DMX values for each DMX-channel
dmxv={1,2,3,4,5,6,7,8,9,10,11,12}
dmxchange = false

-- automatic decrease factor in duration. 
-- can be set  from expresseur.lua,with the command :
-- luabass.setVarMidiOut("dmxdv",0.99)
-- value 1 means no decrease
dmxdv = 0.98

-- cath midi-out note-on to calculate DMX values
function onNoteon(nrTrack,pitch,velocity)
  -- pitch of the scale => velocity of a DMX-channel
  dmxv[pitch % 12 + 1] = velocity * 2
  dmxchange = true
  return nrTrack,"Noteon",pitch,velocity
end
-- cath midi-out note-off to calculate DMX values
function onNoteoff(nrTrack,pitch,velocity)
  -- pitch of the scale => switch off DMX-channel
  dmxv[pitch % 12 + 1] = 0
  dmxchange = true
  return nrTrack,"Noteoff",pitch,velocity
end

-- decrease DMX lighting.
function onTimer()
  if dmxdv != 1 then
    for n=1,12 do
      if dmxv[n] > 0 then
        -- decrease the current dmx values
        dmxv[n]=math.floor(dmxv[n]*dmxdv)
	    dmxchange = true
	  end
    end
  end
  if not dmxchange then return end
  dmxchange = false
  luabass.outDmx(dmxport,table.unpack(dmxv))
end

