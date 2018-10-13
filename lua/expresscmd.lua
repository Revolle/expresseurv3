-- update : 03/12/2016 18:00
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
  local invalid = { "iac" , "loop" , "sd%-50" , "through" }
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
	print("chord(" ..c.. ")")
  local mChord = luachord.setChord(c)
	print("luachord.setChord(" ..c.. ")")
  local tpitch = luachord.getIndexPitches("chord",0,0)
  print("chord(" .. c .. ") = "..table.concat(tpitch,","))
  local id = luabass.outChordSet(-1,0,0,30,1,-1,table.unpack(tpitch))
  if id > 0 then
    luabass.outChordOn(id,64)
    luabass.outChordOff(id,0,1000)
  else
    print("error setting chord #"..id)
  end
end

function sound(wavfile)
  luabass.outSoundPlay(wavfile)
end

function echo(d)
  myDelay =  tonumber(d)
end

function help()
  print("open a MIDI-in using openin. Open a MIDI-out using openout. MIDI-in is going to Midi-out.")
  print("openin <name or #>" )
  print("openout <name or #>" )
  print("listin")
  print("listout")
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
  	luabass.outPreOpenMidi() -- pre-open valid midi-out, to avoid later conflict 
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

function onNoteOn(device,t,channel,pitch,velocity)
  print("LUA noteon",device,t,channel,pitch,velocity,"echo=",myDelay,"ms")
  luabass.outNoteOn(pitch,velocity,pitch)
  if myDelay > 0 then
	  luabass.outNoteOn(pitch + 12,velocity,pitch+128,myDelay)
  end
end
function onNoteOff(device,t,channel,pitch,velocity)
  print("LUA noteoff",device,t,channel,pitch,velocity)
  luabass.outNoteOff(pitch,0,pitch)
  if myDelay > 0 then
	  luabass.outNoteOff(pitch + 12,0,pitch+128)
  end
end
 function onControl(device,t,channel,nrControl,value)
   print("LUA control",device,t,channel,nrControl,value)
 myDelay = 2 * value ;
 end


