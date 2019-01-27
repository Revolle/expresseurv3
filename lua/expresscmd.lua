-- LUA script started by "basslua.dll"
-- "basslua.dll" manages input ( GUI, MIDI-In, timer ) up to LUA
-- the module "luabass.dll" is loaded by default
-- "luabass.dll" manages output ( MIDI-Out ) from LUA
-- function onStart(param) :  called by "basslua.dll" , at the beginning
-- midiinOpen = { 1 } -- LUA table which contains midiIn devices to open, checked regularly by "basslua.dll"
-- function on<event>(...) : called by "luabass.dll" on event
-- function onStop() : called by "basslua.dll" , before to close

instruments = { piano = 0 , accordeon = 21 , guitare = 24 }
local myDelay = 0 ;
myThrough = false ;

function midiOutIsValid(midiout_name)
  -------================
  -- return -1 if the midiout is not valid for the GUI
  local s = string.lower(midiout_name)
  local invalid = { "teensy", "wavetable" , "sd%-50 midi" , "sd%-50 control" , "keystation" , "nanokey" , "key25" , "key49" }
  for i,v in ipairs(invalid) do
    if ( string.find(s,v ) ~= nil) then
      return false ;
    end
  end
  return true ;
end
function midiInIsValid(midiin_name)
  -------===============
  -- return -1 if the midiin is not valid for the GUI
  local s = string.lower(midiin_name)
  local valid = { "sd%-50 midi" }
  local invalid = { "buran" , "iac" , "loop" , "sd%-50" , "through" , "bus" }
  for i,v in ipairs(valid) do
    if ( string.find(s,v ) ~= nil) then
      return true ;
    end
  end
  for i,v in ipairs(invalid) do
    if ( string.find(s,v ) ~= nil) then
      return false ;
    end
  end
  return true ;
end

function listin()
  for i,v in ipairs(lIn) do
    if midiInIsValid(v) then
      print("Midiin device" ,i,v)
    else
      print("Midiin device" ,i,"("..v..")")
    end
  end
end
function listout()
  for i,v in ipairs(lOut) do
    if midiOutIsValid(v) then
      luabass.outSetMidiValide(i,true)
      print("Midiout device" ,i,v)
    else
      luabass.outSetMidiValide(i,false)
      print("Midiout device" ,i,"("..v..")")
    end
  end
end

function openin(s)
  local n
  if tonumber(s) and tonumber(s) <= #lIn then
    n = tonumber(s)
  else
    for i,v in ipairs(lIn) do
      if midiInIsValid(v) and string.find(v,s) then
        n = i
      end  
    end
  end
  if n then
    print("midiIn open #" , n , lIn[n])
    midiinOpen = {}
    midiinOpen[1] = n
  else
    print("error : no midiIn matches ",s)
    listin()
  end
end

function openout(s)
  local n
  if tonumber(s)  and tonumber(s) <= #lOut then
    n = tonumber(s)
  else
    for i,v in ipairs(lOut) do
      if midiOutIsValid(v) and string.find(v,s) then
        n = i
      end  
    end
  end
  if n then
    local trackName = "track" .. s
    if luabass.outTrackOpenMidi(1, 1,"",n,3) ~= 0 then
      print("ok")
      print("midiOut open #" , n , lOut[n] , "on Track #1")
    else
      print("error opening midiOut #", n , lOut[n])
    end
  else
    print("error : no midiOut matches ",s)
    listout()
  end
end

function instrument(name)
  if instruments[name] and tonumber(instruments[name]) then
    luabass.outProgram(tonumber(instruments[name]),0,1,121,0)
    print("instrument #",tonumber(instruments[name]),name)
  else
    print("instrument unknown",name)
  end
end

function dump(o)
   if type(o) == 'table' then
      local s = '{ '
      for k,v in pairs(o) do
         if type(k) ~= 'number' then k = '"'..k..'"' end
         s = s .. '['..k..'] = ' .. dump(v) .. ','
      end
      return s .. '} '
   else
      return tostring(o)
   end
end

function chord(c)
	print("try to compile chord(" ..c.. ")" .. " mem="..collectgarbage("count"))
  local mChord = luachord.setChord(c)
	print("chord compiled by luachord.setChord(" ..c.. ") " )

  local bassduration=900
  local chordduration=900
  local pentaduration=300

  local pitchbass = luachord.getIndexPitches("bass",1,0)
  local pitches = luachord.pitchToString(pitchbass[1])
  print("bass(" .. c .. ") is ".. pitchbass[1] .. " = " .. pitches )
  local idbass = luabass.outChordSet(-1,12,100,50,1,-1,pitchbass[1])
  if idbass > 0 then
    luabass.outChordOn(idbass,64)
    luabass.outChordOff(idbass,0,bassduration)
  else
    print("error setting bass #"..idbass)
  end

  local tpitchchord = luachord.getIndexPitches("chord",0,0)
  pitches = ""
  local dpitches = ""
  for k,v in pairs(tpitchchord) do
    pitches = pitches .. dpitches .. luachord.pitchToString(v)
    dpitches = ","
  end
  print("chord(" .. c .. ") contains "..table.concat(tpitchchord,",") .. " = " .. pitches )
  local idchord = luabass.outChordSet(-1,0,0,30,1,-1,table.unpack(tpitchchord))
  if idchord > 0 then
    luabass.outChordOn(idchord,64,bassduration)
    luabass.outChordOff(idchord,0,chordduration+bassduration)
  else
    print("error setting chord #"..idchord)
  end

 local tpitchpenta = {}
 for i = 1 , 6 do
  local tpitch = luachord.getIndexPitches("penta",i-1,0)
  tpitchpenta[i]=tpitch[1]
 end
 pitches = ""
 dpitches = ""
  for k,v in pairs(tpitchpenta) do
    pitches = pitches .. dpitches .. luachord.pitchToString(v)
    dpitches = ","
  end
  print("penta(" .. c .. ") contains "..table.concat(tpitchpenta,",") .. " = " .. pitches )
 for k,v in pairs(tpitchpenta) do
  local idpenta = luabass.outChordSet(-1,0,0,50,1,-1,v)
  if idpenta > 0 then
    luabass.outChordOn(idpenta,64, bassduration+chordduration+pentaduration*k)
    luabass.outChordOff(idpenta,0, bassduration+chordduration+pentaduration*k+pentaduration)
  else
    print("error setting chord #"..idpenta)
  end
 end
 
end

function sound(wavfile)
  luabass.outSoundPlay(wavfile)
end

function through(d)
  if d then
    if d == "on" then
      myThrough = true
    else
      myThrough = false
    end
  else
    myThrough =  true
  end
end
function echo(d)
  myDelay =  tonumber(d or 0)
end

function help()
  print("open a MIDI-in using openin. Open a MIDI-out using openout. MIDI-in is going to Midi-out.")
  print("openin <name or #>" )
  print("openout <name or #>" )
  print("listin")
  print("listout")
  print("through on|off")
  print("echo delay ( in ms )")
  print("chord <chordname> ( e.g. C , G7, Dm )" )
  for i,v in pairs(instruments) do
    print("instrument " .. i )
  end
  print("transpose [-12..12]" )
  print("sound <file.wav>")
  print("exit")
  print("help")
end

function onStart(param)
  -- after init of the LUA bass module
  print("LUA start onStart")
  print()
  print("list of MIDI interfaces :")
  lOut = luabass.outGetMidiList()
  lIn = luabass.inGetMidiList() 
  print()
  listin()
  print()
  listout()
	if ( string.find(param,"--preopen_midiout") ~= nil) then
    io.write(">>\n")
    io.write(">>\n")
    io.write("preopen midiout ? (y/n) ")
    local retCode = io.read()
    if retCode == "y" then
    	luabass.outPreOpenMidi() -- pre-open valid midi-out, to avoid later conflict 
    end
	end
  print ()
  help()
  print()
end
function transpose(t)
  luabass.outTranspose(t)
end
function onStop()
end
function openVi(dll)
  luabass.outTrackOpenVi(1,1,"",dll);
end

function onNoteOn(device,t,channel, pitch,velocity)
  print("LUA noteon mididevice#", device,"t=",t,"channel#",channel,"pitch#",pitch,"velocity=",velocity)
  if myThrough then
    print("LUA noteon through")
    luabass.outNoteOn(pitch,velocity,pitch)
    if myDelay > 0 then
      print("LUA noteon echo",myDelay,"ms")
	    luabass.outNoteOn(pitch + 12,velocity,pitch+128,myDelay)
    end
  end
end
function onNoteOff(device,t,channel, pitch,velocity)
  print("LUA noteoff mididevice#", device,"t=",t,"channel#",channel,"pitch#",pitch,"velocity=",velocity)
  if myThrough then
    print("LUA noteoff through")
    luabass.outNoteOff(pitch,0,pitch)
    if myDelay > 0 then
      print("LUA noteoff echo",myDelay,"ms")
	    luabass.outNoteOff(pitch + 12,0,pitch+128)
    end
  end
end
 function onControl(device,t,channel, nrControl,value)
  print("LUA control mididevice#", device,"t=",t,"channel#",channel,"control#",nrControl,"value=",value)
  myDelay = 2 * value ;
 end

 function onProgram(device,t,channel, nrProgram)
  print("LUA program mididevice#", device,"t=",t,"channel#",channel,"program#",nrProgram)
end

