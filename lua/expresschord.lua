-- LUA script started by "basslua.dll"
-- "basslua.dll" manages input ( GUI, MIDI-In, timer ) up to LUA
-- the module "luabass.dll" is loaded by default
-- "luabass.dll" manages output ( MIDI-Out ) from LUA
-- function onStart(param) :  called by "basslua.dll" , at the beginning
-- midiinOpen = { 1 } -- LUA table which contains midiIn devices to open, checked regularly by "basslua.dll"
-- function on<event>(...) : called by "luabass.dll" on event
-- function onStop() : called by "basslua.dll" , before to close

local backgroundPlay = false

values = {
  -- callFunction is used by GUI, through basslua. E.G to change MIDI parameters
  -- basslua, will add fields values[valueName]=value
  { name = "chord_delay" , defaultValue=10 , help="Chord improvisation : delay between notes, in ms" },
  { name = "chord_decay" , defaultValue=40 , help="Chord improvisation : decay beween notes, 64 = no decay" },
  { name = "scale_delay" , defaultValue=0 , help="Scale improvisation : delay beween notes of the chord, in ms" },
  { name = "scale_decay" , defaultValue=0 , help="Scale improvisation : decay beween notes of the chord, 64 = no decay" }
}
local delay = 50

-- list of the tracks, for the GUI
tracks = { 
  -- callFunction is used by GUI, through basslua. e.g. to set the trackNr in the midiout-processor
  callFunction = function (nameTrack, nrTrack) luabass.setVarMidiOut(nameTrack,nrtrack) end  ,
  -- the GUI, through basslua, will add fields tracks[trackName]=TrackNr
  { name = "chord-bass" , help = "bass for improvisation"  } , 
  { name = "chord-background" , help = "background chords for improvisation"  } , 
  { name = "chord-chord" , help = "chords for improvisation"  } ,
  { name = "chord-scale" , help = "scale for improvisation"  } ,
}
local tracks = { ["chord-bass"] = 1 , ["chord-background"] = 2 , ["chord-chord"] = 3 , ["chord-scale"] = 4 }

-- midi pitch to select chord (can be modified with choropen(textfile))
local midiChord = {
	[48]="I", -- Do
	[49]= "II.m.7", [50]="II.m", -- re
	[51]="III.m.7", [52]="III.m", -- mi
	[53]="IV" , -- fa
	[54]="V.7" ,[55]="V" , -- sol
	[56]="VI.m.7" , [57]="VI.m", -- la
	[58]="I.7" , [59]="VII.m.b5" -- si
}

-- midi pitch to play bass
local midiBass = {
	[60]= 0 -- Do
}

-- midi pitch to brush chord up or down , quick or slower
local midiBrush = {
	[61]= -2 , [62] = -1 , -- Re
	[63]= 2 , [64] = 1 , -- Mi
}

-- midi pitch to play chords on white pitch
local midiArpege = {
	[65] = 1 ,
	[66]= -1 , [67] = 2 , -- Sol
	[68]= -2 , [69] = 3 , -- La
	[70]= -3 , [71] = 4 , -- Si
	[72]= 5  -- Do
}

function midiOutIsValid(midiout_name)
  -------================
  -- return -1 if the midiout is not valid for the GUI
  local s = string.lower(midiout_name)
  local invalid = { "wavetable" , "sd%-50 midi" , "sd%-50 control" , "keystation" , "nanokey" , "key25" , "key49" }
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
-- display a list of midi-in device available
  for i,v in ipairs(lIn) do
    if midiInIsValid(v) then
      print("Midiin device" ,i,v)
    else
      print("Midiin invalid device" ,i,"("..v..")")
    end
  end
end
function listout()
-- display a list of midi-out device available
  for i,v in ipairs(lOut) do
    if midiOutIsValid(v) then
      print("Midiout device" ,i,v)
    else
      print("Midiout invalid device" ,i,"("..v..")")
    end
  end
end

function openin(s)
-- open midi-in device

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

function openout(track, midiout)
-- open midi-out device
  local n
  if tonumber(midiout)  and tonumber(midiout) <= #lOut then
    n = tonumber(midiout)
  else
    for i,v in ipairs(lOut) do
      if midiOutIsValid(v) and string.find(v,midiout) then
        n = i
      end  
    end
  end
  if n then
	local trackNr = tracks["chord-" .. track] or tonumber(track) or 1
    if luabass.outTrackOpenMidi(trackNr, trackNr,"",n,3) ~= 0 then
      print("ok")
      print("midiOut open #" , n , lOut[n] , "on Track #" , trackNr , track)
    else
      print("error opening midiOut #", n , lOut[n])
    end
  else
    print("error : no midiOut matches ",s)
    listout()
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

function setchord(fileTxt)
	-- one chord per line for each MIDI pitch 48 (Do) ..59 (Si)
--[[ Exemple : 
I
   II.m.7
II.m
   III.m.7
III.m
IV
   V.7
V
   VI.m.7
VI.m
   I.7
VII.m.b5
]]--

	local p = 48
	for line in io.lines(fileTxt) do
		midiChord[p] = line
		p = p + 1
		if p > 59 then
			break 
		end
	end
end 

function chord(c)
  --print("compile chord(" ..c.. ")" .. " mem="..collectgarbage("count"))
  local mChord = luachord.setChord(c)
  -- print("chord compiled by luachord.setChord(" ..c.. ") " )
end

function listshortcut()
	print("shortcuts MIDI :")
	print() ;
	for i = 1,127,1  do
		if midiChord[i] then
			print("  Chord : " , luachord.pitchToString(i) , "=" , midiChord[i])
		end
	end
	print()
	for i = 1,127,1  do
		if midiBass[i] then
			print("  Bass : " , luachord.pitchToString(i) , "=" , midiBass[i] )
		end
	end
	print()
	for i = 1,127,1  do
		if midiBrush[i] then
			print("  Brush : " , luachord.pitchToString(i) , "=" , midiBrush[i] )
		end
	end
	print()
	for i = 1,127,1  do
		if midiArpege[i] then
			print("  Arpege : " , luachord.pitchToString(i) , "=" , midiArpege[i] )
		end
	end
	print()
end

function pedal(track, pedalI)
    luachord.pedal(0,1,0,0,0,64,track .. " " .. pedalI,0,0,0,0,0,0)
end

function octave(track, octaveI)
    luachord.octave(0,1,0,0,0,64,track .. " " .. octaveI,0,0,0,0,0,0)
end

function program(track, programNr)
	local trackNr = tracks["chord-" .. track] or tonumber(track) or 1
	luabass.outProgram(tonumber(programNr) or 1 , 0 , trackNr )
end

function background(p)
	if p == "on" then
		backgroundPlay = true
	else
		backgroundPlay = false
	end
end


function help()
  print("functions available :")
  print("  listin")
  print("  openin <name or #>" )
  print("  listout")
  print("  openout <bass|background|chord|scale> <midioutName or #> " )
  print("  chord <chordname> ( e.g. C , G.7, D.m )" )
  print("  background on|off " )
  print("  transpose [-12..12] | pitch" )
  print("  setchord <fileTxt> : one chord per line" )
  print("  program  <bass|background|chord|scale> programNr" )
  print("  pedal <bass|background|chord|scale> no|legato|pedal" )
  print("  octave <bass|background|chord|scale> [-2..2]" )
  print("  listshortcut")
  print("  init")
  print("  exit")
  print("  help")
end

function init()

  print("init start")
  
  background("on")
  
  print("init luabass")
  luabass.outSetRandomDelay(5) -- 0..256
  luabass.outSetRandomVelocity(5) -- 0..50
  
  print("init midi in")
  openin("Akai LPK25 Wireless")

  print("init midi out")
  tracks["chord-bass"] = 1
  openout("bass" , "LoopBe Internal MIDI")
  pedal("bass" , "legato")
  octave("bass" , 0 )
  --program("bass",1)

  tracks["chord-background"] = 2 
  openout("background" ,  "LoopBe Internal MIDI")
  pedal("background" , "pedal")
  octave("background" , 0 )
  --program("background",1)

  tracks["chord-chord"] = 3 
  openout("chord" ,  "LoopBe Internal MIDI")
  pedal("chord","legato")
  octave("chord" , 0 )
  --program("chord",1)
  values["chord_decay"] = 20
  values["chord_delay"] = 20

  tracks["chord-scale"] = 4
  openout("scale" ,  "LoopBe Internal MIDI")
  pedal("scale" , "no" )
  octave("scale" , 1 )
  --program("scale",1)
  values["scale_decay"] = 0
  values["scale_delay"] = 0
  
  print("init end")
end

function onStart(param)
  -- after init of the LUA bass module
  print("LUA start onStart")
  print ()
  print("list of MIDI interfaces :")
  lOut = luabass.outGetMidiList()
  lIn = luabass.inGetMidiList() 
  print()
  listin()
  print()
  listout()
  print()
  listshortcut()
  print()
  help()
  print()
  init()
end

function transpose(t)
	local myTranspose = math.tointeger (t)
	if myTranspose then
		print("Transpose = " .. myTranspose)
		luabass.outTranspose(myTranspose)
		return
	end
	myTranspose = luachord.stringToPitch(t) - 1
	if myTranspose then
		print("Transpose = " .. myTranspose)
		luabass.outTranspose(myTranspose)
		return
	end
	print("Transpose = 0")
	luabass.outTranspose(0)
end

function onStop()
end

function onNoteOn(device,t,channel, pitch,velocity)
  -- print("LUA noteon mididevice#", device,"t=",t,"channel#",channel,"pitch#",pitch,"velocity=",velocity)

  -- time,bid,ch,typemsg, nr,velocity,param,index,mediane,whiteindex,whitemediane,black
  local i 
  
	if midiChord[pitch] then
	  offPedal("bass")
	  offPedal("background")
	  offPedal("chord")
	  offPedal("scale")
	  print("SetChord " .. midiChord[pitch])
	  chord(midiChord[pitch])
	  if backgroundPlay then
		values["chord_delay"] = 1 * delay
		luachord.playBackground(0,pitch,0,0,0,velocity,"",0,0,0,0,0)
		luachord.playBass(0,pitch,0,0,0,velocity,"",0,0,0,0,0)
	  end
	  return
	end
  
  if midiBass[pitch] then
	luachord.playBass(0,pitch,0,0,0,velocity,"",0,0,0,0,0)
	return
  end

   if midiBrush[pitch] then
	if midiBrush[pitch] < 0 then
		values["chord_delay"] = - midiBrush[pitch] * delay
		luachord.playChord(0,pitch,0,0,0,velocity,"down",0,0,0,0,0)
	else
		values["chord_delay"] = midiBrush[pitch] * delay
		luachord.playChord(0,pitch,0,0,0,velocity,"up",0,0,0,0,0)
	end
	return
  end
  
  if midiArpege[pitch] then
	if midiArpege[pitch] < 0 then
	    i = - midiArpege[pitch]
		luachord.playScale(0,pitch,0,0,0,velocity,"chord",0,0,0,i,1)
	else
	    i = midiArpege[pitch]
		luachord.playScale(0,pitch,0,0,0,velocity,"chord",0,0,0,i,0)
	end
	return
  end
  
  
end

function onNoteOff(device,t,channel, pitch,velocity)
  -- print("LUA noteoff mididevice#", device,"t=",t,"channel#",channel,"pitch#",pitch,"velocity=",velocity)

	if midiChord[pitch] then
	  if backgroundPlay then
		luachord.playBackground(0,pitch,0,0,0,0,"",0,0,0,0,0)
		luachord.playBass(0,pitch,0,0,0,0,"",0,0,0,0,0)
	  end
	  return
	end
  if midiBass[pitch] then
	luachord.playBass(0,pitch,0,0,0,0,"",0,0,0,0,0)
	return
  end

   if midiBrush[pitch] then
    luachord.playChord(0,pitch,0,0,0,0,"",0,0,0,0,0)
	return
  end

  if midiArpege[pitch] then
	luachord.playScale(0,pitch,0,0,0,0,"",0,0,0,0,0)
	return
  end

end

function onControl(device,t,channel, nrControl,value)
  -- print("LUA control mididevice#", device,"t=",t,"channel#",channel,"control#",nrControl,"value=",value)
  if nrControl == 10 then
	values["chord_decay"] = value
	return
  end
  if nrControl == 7 then
	delay = value
	return
  end
  luabass.outControl(nrControl,value,0,tracks["chord-scale"]);
 end
