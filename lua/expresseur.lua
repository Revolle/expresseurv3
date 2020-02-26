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

-- disposal of the four lines of keyboard
keyboarDisposal = nil
keyboardDisposals=
	{
		qwerty = 
		{ 
			{ "1!" , "2@" , "3#" , "4$" , "5%^" , "6&" , "7*" , "8(" , "9)" , "0_" } , 
			{ "Qq" , "Ww" , 'Ee' , "Rr" , "Tt" , "Yy" , "Uu" , "Ii" , "Oo" , "Pp" } , 
			{ "Aa" , "Ss" , 'Dd' , "Ff" , "Gg" , "Hh" , "Jj" , "Kk" , "Ll" , ":;" } , 
			{ "Zz" , "Xx" , "Cc" , "Vv" , "Bb" , "Nn" , "Mm" , "<?" , ">." , "/?"  } 
		} ,
		azerty = 
		{
			{ "1&" , "2é2" , '3"' , "4'" , "5(" , "6-" , "7è" , "8_" , "9ç" , "0à"  } , 
			{ "Aa" , "Zz" , 'Ee' , "Rr" , "Tt" , "Yy" , "Uu" , "Ii" , "Oo" , "Pp"  } , 
			{ "Qq" , "Ss" , 'Dd' , "Ff" , "Gg" , "Hh" , "Jj" , "Kk" , "Ll" , "Mm"  } , 
			{ "Ww" , 'Xx' , "Cc" , "Vv" , "Bb" , "Nn" , "?," , ".;" , ":/" , "!§"  } 
		} ,
		qwertz = 
		{
			{ "1+" , '2"' , "3*" , "4ç" , "5%" , "6&" , "7/" , "8(" , "9)" , "0=" } , 
			{ "Qq" , "Ww" , 'Ee' , "Rr" , "Tt" , "Zz" , "Uu" , "Ii" , "Oo" , "Pp" } , 
			{ "Aa" , "Ss" , 'Dd' , "Ff" , "Gg" , "Hh" , "Jj" , "Kk" , "Ll" , "éö" } , 
			{ "Yy" , "Xx" , "Cc" , "Vv" , "Bb" , "Nn" , "Mm" , ",;" , ".:" , "-_" } 
		}
	}


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
	-- change keyboard for shortcuts,using -k option
	local typeKeyboard = (string.match(param,"-k (%a+)")) or "qwerty"
	-- luabass.logmsg("param=" .. param .. " / typeKeyboard=" .. typeKeyboard )
	keyboarDisposal = keyboardDisposals[typeKeyboard]
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
--===========================================================
	-- play a midi message 
	trackNr = string.match(paramString or "" , "(%d+)")
	luabass.outSystem((ch - 1 ) + (typemsg * 16 ) , d1, d2 , math.tointeger(trackNr) or 1)
end

function allNoteOff( )
--====================
	-- All Note-Off
	luabass.outAllNoteOff()
end

function setLuaValue( t, bid, ch, typemsg, pitch, velo , paramString )
--====================================================================
	-- set an LUA value according to param (and optional velo)
	local luaparam 
	local vol 
	luaparam , vol = string.match(paramString or "" , "(%g+) (%d+)")
	if luaparam then
		values[luaparam]=math.tointeger(vol)
		return
	end
	luaparam  = string.match(paramString or "" , "(%g+)")
	if typemsg == 12 then 
		-- Program
		if luaparam and pitch then
			values[luaparam]=pitch
			return
		end
	else
		if luaparam and velo then
			values[luaparam]=velo
			return
		end
	end 
end

function mainVolume( t, bid, ch, typemsg, pitch, velo , paramString )
--===================================================================
	-- set volume according to param
	vol = string.match(paramString or "" , "(%d+)")
	luabass.outSetVolume(math.tointeger(vol) or ( velo or 64 )) 
end

function trackVolume( t, bid, ch, typemsg, pitch, velo , paramString )
--====================================================================
	-- set track volume
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
--===================================================
	-- move to next file of teh list
	if ( velo or 64 ) > 0 then info.action = "-" end
end
function previousFile( t, bid, typemsg, ch, pitch, velo )
--===================================================
	-- move to previous file of teh list
	if ( velo or 64 ) > 0 then info.action = "+" end
end

--========================= midi thru
function onNoteOn(deviceNr , timestamp, channel , pitch, velocity )
	-- echo a MidiIn noteOn on first track
	if (midiinThru or false) then
		luabass.outNoteOn(pitch,velocity )
	end
end
function onNoteOff(deviceNr , timestamp, channel , pitch, velocity )
	-- echo a MidiIn noteOut on first track
	if (midiinThru or false) then
		luabass.outNoteOff(pitch )
	end
end

--========================= PC Keyboard shortcuts 


-- help message for keydown function
function helpkeydown()
	local chhelp
	if keyboarDisposal and keyboarDisposal[1] and keyboarDisposal[4] and keyboarDisposal[1][10] and keyboarDisposal[4][10] then
		chhelp= [[
Shortcuts defined in expresseur.lua :
Select disposal with -k option in LUA-parameter (qwerty,azerty,qwertz)
* Mixer : 8 tracks ( tacet/p/mf/f) on the left of the four lines of the keyboard 
* Move : arrows, page, home , end , backspace 
* Transpose : ]] .. string.sub(keyboarDisposal[2][10],1,1) .. " " .. string.sub(keyboarDisposal[3][10],1,1) .. [[ 
* Silence : ]] .. string.sub(keyboarDisposal[1][10],1,1) .. [[

* Goto : ]] .. string.sub(keyboarDisposal[1][9],1,1) .. [[

* play view : ]] .. string.sub(keyboarDisposal[4][10],1,1) .. [[

* Midithru : ]] .. string.sub(keyboarDisposal[2][9],1,1) .. " " .. string.sub(keyboarDisposal[3][9],1,1) 
	else
		chhelp = [[
No keyboard disposal in expresseur.lua
Select disposal with -k option in LUA-parameter (qwerty,azerty,qwertz)]]
	end
	return chhelp
end

function keydown ( keyLetter, keyCode, modifiers)
--===============================================
-- when a computer key is pressed, this function is called
-- return true if the process of the keydown will not continue

	info.status = "keydown LUA " .. (keyLetter or "") .. " / " .. (keyCode or "") .. "/" .. (modifiers or "")

-- help
	if (keyCode or -1) == -1 then
		info.action = "!" .. helpkeydown()
		return true
	end

	if keyboarDisposal == nil then 
		return false
	end 

-- move in the score
	if keyCode == 313 then -- WXK_HOME
		luabass.outAllNoteOff()
		luascore.firstPart() ;
		info.status = "first part"
		return true
	elseif keyCode == 312 then -- WXK_END
		luabass.outAllNoteOff()
		luascore.lastPart() 
		info.status= "last part"
		return true
	elseif keyCode == 314 then -- WXK_LEFT
		luabass.outAllNoteOff()
		luascore.previousEvent() 
		info.status= "previous note"
		return true
	elseif keyCode == 316 then -- WXK_RIGHT
		luabass.outAllNoteOff()
		luascore.nextEvent() 
		info.status = "next note"
		return true
	elseif keyCode == 315 then -- WXK_UP
		luabass.outAllNoteOff()
		luascore.previousMeasure() 
		info.status = "previous measure"
		return true
	elseif keyCode == 317 then -- WXK_DOWN
		luabass.outAllNoteOff()
		luascore.nextMeasure() 
		info.status = "next measure"
		return true
	elseif keyCode == 366 then -- WXK_PAGE_UP
		luabass.outAllNoteOff()
		luascore.previousPart() 
		info.status  = "previous part"
		return true
	elseif keyCode == 367 then -- WXK_PAGE_DOWN
		luabass.outAllNoteOff()
		luascore.nextPart() 
		info.status= "next part"
		return true
	elseif keyCode == 8 then -- WXK_BACK
		luabass.outAllNoteOff()
		luascore.previousPos() 
		info.status  = "previous move"
		return true
	end

	if modifiers ~= 0 then 
		return false 
	end
	
-- standard keystrokes checked according to keyboard disposal
	if keyLetter == nil or string.len(keyLetter) < 1 then
		return false
	end
	for i,v in ipairs(keyboarDisposal) do
		-- line by line of the keyboad disposal
		local p = nil
		for j,w in ipairs(v) do
			if string.find(w,keyLetter,1,true) then
				p = j
				break ;
			end
		end
		if p then
			-- column by column of the keyboard disposal
			if (p < 9) then
				-- mixer for 8 first columns (track#). Line is the action (forte, mp, piano, tacet)
				if (i == 1) then
					info.status = "track ".. p .. " forte"
					info.action = "=+" .. p
					luabass.outSetTrackVolume (100,p)
				elseif (i == 2) then
					info.status = "track ".. p .. " meso"
					info.action = "=+" .. p
					luabass.outSetTrackVolume (64,p)
				elseif (i == 3) then
					info.status = "track ".. p .. " piano"
					info.action = "=+" .. p
					luabass.outSetTrackVolume (30,i)
				elseif (i == 4) then
					luabass.outAllNoteOff()
					info.status = "track ".. p .. " tacet"
					info.action = "=-" .. p
				end
				return true 
			elseif (p == 9) then
				if (i==1) then
					luabass.outAllNoteOff()
					info.action = "@"
					info.status = "goto" 
					return true 
				elseif (i==2) then
					luabass.outAllNoteOff()
					midiinSelector = true
					midiinThru = false
					info.status = "MIDI Keyboard is assisted" 
					return true 
				elseif (i==3) then
					luabass.outAllNoteOff()
					midiinSelector = false
					midiinThru = true
					info.status = "MIDI Keyboard is NOT assisted" 
					return true 
				end
			elseif (p == 10) then
				if (i==1) then
					info.status = "All Note Off"
					luabass.outAllNoteOff()
					return true 
				elseif (i==2) then
					valueTranspose = (valueTranspose or 0) + 1
					luabass.outTranspose(valueTranspose)
					info.status = "Transpose " .. valueTranspose
					return true 
				elseif (i==3) then
					valueTranspose = (valueTranspose or 0) - 1
					luabass.outTranspose(valueTranspose)
					info.status = "Transpose " .. valueTranspose
					return true 
				elseif (i==4) then
					luabass.outAllNoteOff()
					info.action = "="
					info.status = "play/view" 
					return true 
				end
			end
		end
	end
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



 

