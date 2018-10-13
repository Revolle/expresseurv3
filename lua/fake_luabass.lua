local E={} -- to export the functions

local listMidi = {}
local idchord = 1
function E.outControl(ctrlnumber,value,delay,track)
  table.insert(listMidi,"ct#"..ctrl.."/"..value.."@"..track.."+"..delay)
end
function E.outSetTrackInstrument(instrument,track)
  table.insert(listMidi,"is#"..instrument.."@"..track)
end
function E.outNoteOff(pitch,velo,id,delay,track)
  table.insert(listMidi,"of#"..pitch.."@"..track.."+"..delay)
end
function E.outNoteOn(pitch,velo,id,delay,track)
  table.insert(listMidi,"on#"..pitch.."v"..velo.."@"..track.."+"..delay)
end
function E.outChordSet(id,transpose,delay,decay,pstart,pend,p1,p2,p3,p4)
  local idreturned = id
  if id == -1 then
    idchord = idchord + 1
    idreturned = idchord
  end
  table.insert(listMidi,"chordset id#"..id..">"..idreturned.." transpose="..transpose.." delay="..delay.." decay="..decay.." ["..pstart..".."..pend.."] p="..(p1 or "")..","..(p2 or "")..","..(p3 or "")..","..(p4 or ""))
  return idreturned
end
function E.outChordOn(id,velocity,dt,track)
  table.insert(listMidi,"chordon id#"..id.." velocity="..velocity.." dt="..dt.." track#"..track)
end
function E.outChordOff(id,velocity,dt)
  table.insert(listMidi,"chordof id#"..id.." velocity="..(velocity or "").." dt="..(dt or ""))
end

function E.dumpListMidi()
  for nilvalue,s in ipairs(listMidi) do
    print(s)
  end
  listMidi = {}
end

return E