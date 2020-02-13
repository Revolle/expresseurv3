--[[
This LUA script is started by basslua.

Basslua manages input ( GUI, MIDI-In, timer ). These inputs are sent to this LUA script.

Basslua loads in addition, by default, these modules, accessible as global :
- luabass : C-LUA-Module for midi output.
- luachord.lua : script-LUA-module to interpret text chords.
- luascore.lua : script-LUA-module to interpret score.

Function onStart(param) : called by basslua ,when this LUA script is started.
Function onStop() : called by basslua , before to close this LUA script.

basslua uses these tables :
- midiinOpen = { 1, 3 } : LUA table which contains midiIn deviceNrs to open. Checked regularly by basslua.
- midiinSelector = true/false : LUA boolean to unvalidate/validate selectors. Checked regularly by basslua.
- It can write 
	info.status = "status message to display in the gui" 
	info.action = "!message-box to display in the gui" 
	info.action = "=3/2" track to play=3/view=2
	info.action = "+" increment file 
	info.action = "-" decrement file 
	info.action = "0" first file 
	info.action = "#" last file 
	info.action = "@B*2" goto marker B for second time

- values = { {},..} : table of values which can be tuned in the GUI. 
    Read by the GUI through basslua.
	The GUI, through basslua, will add fields : values[valueName]=value
    callFunction is used by GUI, through basslua. e.g. to change MIDI parameters
    Example :
    values = {
      { name = "ctrl7 Bass" , value=60 , callFunction = ctrl7Bass , help="volume Midi track Bass"  } }
	  create values["ctrl7 Bass"] with initial value 60. function ctrl7Bass will be called on each change.

- tracks = { {},..} : table of tracks which can be tuned in the mixer of the GUI. 
    Read by the GUI through basslua.
    Example :
    tracks = { 
       -- callFunction is used by GUI, through basslua. E.G to set the trackNr in the midiout-processor
       callFunction = function (nameTrack, nrTrack) luabass.setVarMidiOut(nameTrack,nrtrack) end  ,
       -- the GUI, through basslua, will add fields tracks[trackName]=TrackNr
       { name = "chord-bass" , help = "track volume for the bass for the chords"  } }
- actions = { {},..} : table of actions which can be used in thh GUI. Example :
    Read by the GUI through basslua.
    Example :
     actions = { 
     -- <name> : displayed in the GUI
     -- <icone> : if file icone.bmp exists, action is displyed in the toolbar
     -- <help> : help displayed in the GUI
     -- <shortcut> : if it exists, action is displayed in the menu bar ( e.g. HOME, END, DOWN, LEFT, RIGHT, CTRL+LEFT , ALT+RIGHT, SHIFT+HOME... )
     -- callFunction is called when the selector is triggered ( by a midi event, or a keystroke )
     -- callFunction parameters are :
     --   time : float, timestamp of the event
     --   uidKey : integer, unique id of the event ( composed of the selector id , channel, and picth )
     --   channel[1..16] of the event
     --   type_msg_MIDI[1..16] of the event
     --   pitch[0..127] : integer, pitch of the event ( or control nr )
     --   velocity[0..127] : integer, velocity of the event ( or control value )
     --   paramString : string , parameter set in the GUI for this selector
     --   indexKey[1..n] : integer, index of the picth within the range of the selector. 1 means the start.
     --   medianeKey[-n/2..0..n/2] : integer, index of the pitch within the the range of the selector. 0 means the middle.
     --   whiteIndex : integer,  idem indexKey, but taking in account only "white keys"
     --   whiteMediane : integer,  idem medianeKey, but taking in account only "white keys"
     --   black[0,1] : integer,  0 means white key. 1 means black key.
 
Functions user.on<event>(...) : LUA functions to take actions on midi events
  called by luabass.dll on midi or timer event.
	
  To process-map midi events before their processing ( change pitch ... ) : 
		onMidi(integer deviceNr 1.. , float timestamp, integer type_msg 0..15, integer channel 1..16,  integer value1 0..127 , integer value2 0..127). 
		return nothing to stop the process of the MIDI message, or return the new (or same) midi message to continue the process :
	     	integer type_msg 0..15  , integer channel 1..16,  integer value1 0..127 , integer value2 0..127
  These functions return nothing
    To catch a specific category of MidiEvent :
		onNoteOn(integer deviceNr 1.. , float timestamp, integer channel 1..16, integer pitch 0..127, integer velocity 0..127). 
		onNoteOff(integer deviceNr 1.. , float timestamp, integer channel 1..16, integer pitch 0..127, integer velocity 0..127)
		onKeypressure(integer deviceNr 1.. , float timestamp, integer channel 1..16, integer pitch 0..127, integer value 0..127)
		onControl(integer deviceNr 1.. , float timestamp, integer channel 1..16, integer controlNr 0..127, integer value 0..127)
		onProgram(integer deviceNr 1.. , float timestamp, integer channel 1..16, integer programNr 0..127)
		onChannelPressure(integer deviceNr 1.. , float timestamp, integer channel 1..16, integer value 0..127 )
		onPitchBend(integer deviceNr 1.. , float timestamp, integer channel 1..16, integer value 0..127*127 )
		onSystemeCommon(integer deviceNr 1.. , float timestamp, integer value1 0..127, integer value2 0..127 , integer value3 0..127 )
    To catch special Midi event :
	    onSysex(integer deviceNr 1.. , float timestamp , string asciiHexSysex e.g. FE4F7C4A... )
        onActive(integer deviceNr 1.. , float timestamp )
        onClock(integer deviceNr 1.. , float timestamp )
	To catch the regulat timer
        onTimer(float timestamp)

This LUA script can starts another LUA script, to manage midi-out events. cf. luabass.onMidiout(LUAfile)
The script should contains ones of these functions :
    onNoteOn(integer trackNr 1.. , integer pitch 0..127, integer velocity 0..127)
    onNoteOff(integer trackNr 1.. , integer pitch 0..127, integer velocity 0..127)
    onKeyPressure(integer trackNr 1.. , integer pitch 0..127, integer value 0..127)
    onControl(integer trackNr 1.. , integer controlNr 0..127, integer value 0..127)
    onProgram(integer trackNr 1.. , integer programNr 0..127)
    onChannelPressure(integer trackNr 1.. , integer value 0..127)
    onPitchBend(integer trackNr 1.. , integer LSB 0..127, integer MSB 0..127)
These functions must return a list of zero or many MIDI messages, to send on midi-out. One message is 4 values :
  integer trackNr 1.., 
  string typeMessage ( NoteOn, noteOff, keyPressure, Control, Program, channelPressure, pitchBend )
  integer value1 0..127 ( pitch for note, programNr, controlNr, LSB for pitchbend )
  integer value2 0.127 ( velcoity for note, 0 fro program, cnotrolValue, MSB for pitchben )

--]]

--========================  Validity of a Midi-Out device, for the GUI  ( used by the GUI and the initialization)
-- list of (non) valid midi-out and midi-in
valid_midiout = { "bus" , "iac" , "loop" , "sd%-50" , "internal" , "through", "buran" }
invalid_midiout = { "teensy", "wavetable" , "sd%-50 midi" , "sd%-50 control" , "keystation" , "nanokey" , "key25" , "key49" }
valid_midiin = { "sd%-50 midi" }
invalid_midiin = { "bus" , "iac" , "loop" , "sd%-50" , "internal" , "through", "buran" }

function midiOutIsValid(midiout_name)
  -------================
  -- return false if the midiout is not valid for the GUI
  local s = string.lower(midiout_name)
  for inil,v in ipairs(valid_midiout) do
    if ( string.find(s,v ) ~= nil) then
      return true ;
    end
  end
  for inil,v in ipairs(invalid_midiout) do
    if ( string.find(s,v ) ~= nil) then
      return false ;
    end
  end
  return true ;
end
-- Validity of a Midi-In device, for the GUI  ( used by the GUI )
function midiInIsValid(midiin_name)
  -------===============
  -- return false if the midiin is not valid for the GUI
  local s = string.lower(midiin_name)
  for inil,v in ipairs(valid_midiin) do
    if ( string.find(s,v ) ~= nil) then
      return true ;
    end
  end
  for inil,v in ipairs(invalid_midiin) do
    if ( string.find(s,v ) ~= nil) then
      return false ;
    end
  end
  return true ;
end

--===================== initialization
function onStart(param)
  -- after init of the LUA bass module
	if ( string.find(param,"--preopen_midiout") ~= nil) then
  	     lOut = luabass.outGetMidiList() -- list of midi-out ports
		for i,v in ipairs(lOut) do
    	  		luabass.outSetMidiValide(i,midiOutIsValid(v)) -- make midi-out valide or not
		end
  	luabass.outPreOpenMidi() -- pre-open valid midi-out, to avoid later conflict 
	end
end

--===================== stop
function onStop()
	-- before stop of the LUA bass module
end


--==================== Expresseur GUI

-- list of values, for the GUI ( throug basslua )
values = {
  -- callFunction is used by GUI, through basslua. E.G to change MIDI parameters
  -- basslua, will add fields values[valueName]=value
  { name = "chord_delay" , defaultValue=10 , help="Chord improvisation : delay between notes, in ms" },
  { name = "chord_decay" , defaultValue=40 , help="Chord improvisation : decay beween notes, 64 = no decay" },
  { name = "scale_delay" , defaultValue=0 , help="Scale improvisation : delay beween notes of the chord, in ms" },
  { name = "scale_decay" , defaultValue=0 , help="Scale improvisation : decay beween notes of the chord, 64 = no decay" },
}

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

function playNote( t, bid, ch, typemsg, d1, d2 , paramString)
	trackNr = string.match(paramString or "" , "(%d+)")
	luabass.outSystem((ch - 1 ) + (typemsg * 16 ) , d1, d2 , math.tointeger(trackNr) or 1)
end
function allNoteOff( )
    luabass.outAllNoteOff()
    t0 = nil 
    dt = nil
    t1 = nil
end
function setLuaValue( t, bid, ch, typemsg, pitch, velo , paramString )
  local luaparam 
  local vol 
  luaparam , vol = string.match(paramString or "" , "(%g+) (%d+)")
  if luaparam then
    values[luaparam]=math.tointeger(vol)
	return
  end
  luaparam  = string.match(paramString or "" , "(%g+)")
  if luaparam and velo then
    values[luaparam]=velo
	return
  end
end
function mainVolume( t, bid, ch, typemsg, pitch, velo , paramString )
  vol = string.match(paramString or "" , "(%d+)")
  luabass.outSetVolume(math.tointeger(vol) or ( velo or 64 )) 
end
function trackVolume( t, bid, ch, typemsg, pitch, velo , paramString )
  --parameter form#1 : volume [optional track#]
  --parameter form#2 : track_name
  --parameter form#3 : track#
  local trackNr 
  local vol 
  vol, trackNr = string.match(paramString or "" , "(%d+) (%d+)")
  if vol then
	  luabass.outSetTrackVolume(math.tointeger(vol) or (velo or 64),trackNr or 1)
	  return
  end
  trackNr = string.match(paramString or "" , "(%d+)")
  if trackNr then
	  luabass.outSetTrackVolume(velo or 64,trackNr or 1)
	  return
  end
  if tracks[paramString or "none"] then
	  luabass.outSetTrackVolume(math.tointeger(velo) or 64,tracks[paramString or "none"] or 1)
	  return
  end
  luabass.logmsg("trackVolume("..paramString.."):unsolved")
end
function nextFile( t, bid, ch, typemsg, pitch, velo )
  if ( velo or 64 ) > 0 then info.action = "-" end
end
function previousFile( t, bid, typemsg, ch, pitch, velo )
  if ( velo or 64 ) > 0 then info.action = "+" end
end

--========================= midi thru
function onNoteOn(deviceNr , timestamp, channel , pitch, velocity )
	if (midiinThru or false) then
		luabass.outNoteOn(pitch,velocity )
	end
end
function onNoteOff(deviceNr , timestamp, channel , pitch, velocity )
	if (midiinThru or false) then
		luabass.outNoteOff(pitch )
	end
end

--========================= PC Keyboard shortcuts 
-- french keyboard
keyboard_line = {} 
keyboard_line[1]={"1" , "2" , "3" , "4", "5" , "6" , "7" , "8", "9", "0" }
keyboard_line[2]={"A" , "Z" , "E" , "R", "T" , "Y" , "U" , "I", "O", "P" }
keyboard_line[3]={"Q" , "S" , "D" , "F", "G" , "H" , "J" , "K", "L", "M" }
keyboard_line[4]={"W" , "X" , "C" , "V", "B" , "N" , "," , ";", ":", "!" }
-- US keyboard
--[[ 
keyboard_line[1]={"1" , "2" , "3" , "4", "5" , "6" , "7" , "8", "9", "0" }
keyboard_line[2]={"Q" , "W" , "E" , "R", "T" , "Y" , "U" , "I", "O", "P" }
keyboard_line[3]={"A" , "S" , "D" , "F", "G" , "H" , "J" , "K", "L", ";" }
keyboard_line[4]={"Z" , "X" , "C" , "V", "B" , "N" , "M" , ",", ".", "/" }
--]]
-- switzerland keyboard
--[[ 
keyboard_line[1]={"1" , "2" , "3" , "4", "5" , "6" , "7" , "8", "9", "0" }
keyboard_line[2]={"Q" , "W" , "E" , "R", "T" , "Z" , "U" , "I", "O", "P" }
keyboard_line[3]={"A" , "S" , "D" , "F", "G" , "H" , "J" , "K", "L", "é" }
keyboard_line[4]={"Y" , "X" , "C" , "V", "B" , "N" , "M" , ",", ".", "-" }
--]]
helpkeydown = [[
Shortcuts defined in ressources/luauser.lua :
Mixer : 8 tracks ( tacet/p/mf/f) on the left of the four lines of the keyboard 
Move : arrows, page, home , end , backspace 
Transpose : ]] .. keyboard_line[2][10] .. " " .. keyboard_line[3][10] .. [[ 
Silence : ]] .. keyboard_line[1][10] .. [[

Goto : ]] .. keyboard_line[1][9] .. [[

play view : ]] .. keyboard_line[4][10] .. [[

Midithru : ]] .. keyboard_line[2][9] .. " " .. keyboard_line[3][9]


--===================== 
-- catch PC keydown
function keydown ( keyLetter, keyCode, modifiers)
-- when a computer key is pressed, this function is called
-- return true if the process of the keydown will not continue

 	luabass.logmsg("keydown(" .. (keyLetter or "" ).. "," .. (keyCode or "") .. "," .. (modifiers or "")  .. ")")
-- help
	if (keyCode or -1) == -1 then
		info.action = "!" .. helpkeydown
		return true
	end
-- mixer
	for i,v in ipairs(keyboard_line[1]) do
    		if (keyLetter == v) and (i < 9) then
      			info.status = "track ".. i .. " forte"
			info.action = "=+" .. i
			luabass.outSetTrackVolume (100,i)
			return true
		end
	end 
	for i,v in ipairs(keyboard_line[2]) do
    		if (keyLetter == v) and (i < 9) then
      			info.status = "track ".. i .. " meso"
			info.action = "=+" .. i
			luabass.outSetTrackVolume (64,i)
			return true
		end
	end 
	for i,v in ipairs(keyboard_line[3]) do
    		if (keyLetter == v) and (i < 9) then
      			info.status = "track ".. i .. " piano"
			info.action = "=+" .. i
			luabass.outSetTrackVolume (30,i)
			return true
		end
	end 
	for i,v in ipairs(keyboard_line[4]) do
    		if (keyLetter == v) and (i < 9) then
      			info.status = "track ".. i .. " tacet"
			info.action = "=-" .. i
			return true
		end
	end 
-- silence
	if keyLetter == keyboard_line[1][10] then
		luabass.outAllNoteOff()
		info.status = "all note off" 
		return true
-- transpose
	elseif keyLetter == keyboard_line[2][10] then
		valueTranspose = (valueTranspose or 0) + 1
		luabass.outTranspose(valueTranspose)
		info.status("Transpose " .. valueTranspose)
		return true
	elseif keyLetter == keyboard_line[3][10] then
		valueTranspose = (valueTranspose or 0) - 1
		luabass.outTranspose(valueTranspose)
		info.status("Transpose " .. valueTranspose)
		return true
-- playview
	elseif keyLetter == keyboard_line[4][10] then
		info.action = "="
		info.status("play view")
		return true
-- goto
	elseif keyLetter == keyboard_line[1][9] then
		info.action = "@"
		info.status("Goto")
		return true
-- move
	elseif keyCode == 314 then -- WXK_HOME
		luascore.firstPart() ;
		info.status("first part")
		return true
	elseif keyCode == 313 then -- WXK_END
		luascore.lastPart() ;
		info.status("last part")
		return true
	elseif keyCode == 315 then -- WXK_LEFT
		luascore.previousEvent() ;
		info.status("previous note")
		return true
	elseif keyCode == 317 then -- WXK_RIGHT
		luascore.nextEvent() ;
		info.status("next note")
		return true
	elseif keyCode == 316 then -- WXK_UP
		luascore.previousMeasure() ;
		info.status("previous measure")
		return true
	elseif keyCode == 318 then -- WXK_DOWN
		luascore.nextMeasure() ;
		info.status("next measure")
		return true
	elseif keyCode == 367 then -- WXK_PAGE_UP
		luascore.previousPart() ;
		info.status("previous part")
		return true
	elseif keyCode == 369 then -- WXK_PAGE_DOWN
		luascore.nextPart() ;
		info.status("next part")
		return true
	elseif keyCode == 8 then -- WXK_BACK
		luascore.previousPos() ;
		info.status("previous move")
		return true
-- midi thru
	elseif keyLetter == keyboard_line[2][9] then
		midiinSelector = true
		midiinThru = false
		luabass.outAllNoteOff()
		info.status = "MIDI Keyboard is assisted" 
		return true
	elseif keyLetter == keyboard_line[3][9] then
		midiinSelector = false
		midiinThru = true
		info.status = "MIDI Keyboard is NOT assisted" 
		return true
	end
	info.status = "the " .. keyLetter .. " / " .. keyCode .. " is not processed by keydown.lua" 
	return false 
end


-- list of actions for the GUI ( throug basslua )
  -- <name> : displayed in the GUI
  -- <icone> : if file icone.bmp exists, action is displyed in the toolbar
  -- <help> : help displayed in the GUI
  -- <shortcut> : if it exists, action is displayed in the menu bar 
  --           ( e.g. HOME, END, DOWN, LEFT, RIGHT, CTRL+LEFT , ALT+RIGHT, SHIFT+HOME, SPACE... )
  -- callFunction is called when the selector is triggered ( by a midi event, or a keystroke )
  --    ( callScore and callChord will be preferred when the "mode" "Score" or "Chord" is set by the GUI throug basslua )
  -- callFunction parameters are :
  --   time : float, timestamp
  --   uidKey : integer, unique id of the event ( composed of the selector id ,channel , and picth )
  --   channel[1..16] : integer, channel of the event ( or control nr , or program nr )
  --   type_msg_MIDI[1..16] of the event
  --   pitch[0..127] : integer, pitch of the event ( or control nr , or program nr )
  --   velocity[0..127] : integer, velocity of the event ( or control value )
  --   paramString : string , pamaeter set in the selector
  --   indexKey[1..n] : index of the picth within the range of the selector. 1 means the minimum of the range
  --   medianeKey[-n/2..0..n/2] : index of the pitch within the the range of the selector. 0 means the middle of the range
  --   whiteIndex : idem indexKey, but taking in account only "white keys"
  --   whiteMediane : idem medianeKey, but taking in account only "white keys"
  --   black[0,1] : 0 means white key. 1 means black key.
actions = { 
  {name="global/all note off", callFunction = allNoteOff ,help="all note off", shortcut = "CTRL+BACK" , icone = "all_note_off" },
  {name="global/play note", callFunction = playNote ,help="play the midi event" },
  {name="global/main volume", callFunction = mainVolume  ,help="set main volume from parameter=value, or value from MIDI data-2" },
  {name="global/track volume", callFunction = trackVolume ,help="set track volume from parameter=track value, or value from MIDI data-2" },
  {name="global/lua value", callFunction = setLuaValue  ,help="set lua vaue from parameter=luaname value, or value from MIDI data-2"},
  {name="global/previous file",  help="go to previous file of the list", callFunction = previousFile  },
  {name="global/next file", help="go to next file of the list", 
    callFunction = nextFile , icone = "next_file" },
  {name="move/previous move", help="go to the position of the previous move",
    callScore = luascore.previousPos,callChord=luachord.previousPos,shortcut = "ALT+HOME",icone = "previous_move" },
  {name="move/first part", help="go to beginning of the score",
    callScore = luascore.firstPart,callChord=luachord.firstPart,shortcut = "CTRL+HOME",icone = "first_part" },
  {name="move/previous part", help="go to previous part of the score",
    callScore = luascore.previousPart,callChord=luachord.previousPart,shortcut="CTRL+PGUP",icone="previous_part" },
  {name="move/previous measure",help="go to previous section/measure of the score",
    callScore = luascore.previousMeasure, callChord = luachord.previousSection, shortcut = "CTRL+UP" , icone = "previous_section"},
  {name="move/previous chord",help="go to previous chord of the score",
    callScore=luascore.previousEvent,callChord=luachord.previousChord,shortcut="CTRL+LEFT",icone="previous_chord"},
  {name="move/next chord",help="go to next chord of the score",
    callScore=luascore.nextEvent,callChord=luachord.nextChord,shortcut="CTRL+RIGHT",icone="next_chord" },
  {name="move/next measure", help="go to next section/measure of the score",
    callScore = luascore.nextMeasure, callChord = luachord.nextSection , shortcut = "CTRL+DOWN" , icone = "next_section" },
  {name="move/next part", help="go to next part of the score",
    callScore = luascore.nextPart,callChord=luachord.nextPart,shortcut = "CTRL+PGDN", icone = "next_part" },
  {name="move/last part", help="go to last part of the score",
    callScore = luascore.lastPart, callChord = luachord.lastPart , shortcut = "CTRL+END", icone = "last_part" },
  {name="move/repeat part",callScore = luascore.repeatPart, callChord = luachord.repeatPart },
  {name="move/first part smooth",callScore=luascore.firstPartSmooth,callChord=luachord.firstPartSmooth},
  {name="move/repeat part smooth",callScore=luascore.repeatPartSmooth,callChord=luachord.repeatPartSmooth},
  {name="move/next part smooth",callScore=luascore.nextPartSmooth,callChord=luachord.nextPartSmooth },
  {name="move/last part smooth",callScore=luascore.lastPartSmooth,callChord=luachord.lastPartSmooth},
  {name="score/play", callFunction = luascore.play} ,
  {name="chord/change chord", callFunction = luachord.changeChord },
  {name="chord/play scale", callFunction = luachord.playScale },
  {name="chord/play chord", callFunction = luachord.playChord },
  {name="chord/play background", callFunction = luachord.playBackground },
  {name="chord/play bass", callFunction = luachord.playBass },
  {name="chord/pedal", callFunction = luachord.pedal},
  {name="chord/octave", callFunction = luachord.octave },
  }



 

