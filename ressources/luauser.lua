local E={} -- to export the functions
--[[
This LUA module is on the user directory ExpresseruV3/ressources/luauser.lua
It is used to customize the behavior of Expresseur

It can write 
	info.status = "status message to display in the gui" 
	info.action = "!message-box to display in the gui" 
	info.action = "=3/2" track to play/view
	info.action = "+" increment file 
	info.action = "-" decrement file 
	info.action = "0" first file 
	info.action = "#" last file 
	info.action = "@B*2" goto marker B for second time

It contains information about valid or non valid MIDI
	E.valid_midiout 
	E.invalid_midiout 
	E.valid_midiin 
	E.invalid_midiin

It can catch computer keydown with function E.keydown ( keyLetter, keyCode, modifiers)

It can catch MIDI-Event following this specifications :
Functions E.on<event>(...) : LUA functions to take actions on midi events
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
--]]

--===================== 
-- list of (non) valid midi-out and midi-in
E.valid_midiout = { "bus" , "iac" , "loop" , "sd%-50" , "internal" , "through", "buran" }
E.invalid_midiout = { "teensy", "wavetable" , "sd%-50 midi" , "sd%-50 control" , "keystation" , "nanokey" , "key25" , "key49" }
E.valid_midiin = { "sd%-50 midi" }
E.invalid_midiin = { "bus" , "iac" , "loop" , "sd%-50" , "internal" , "through", "buran" }

--===================== 
-- initialization
function E.onStart(param)
 	-- at the initialization
	luabass.logmsg("luauser.onStart")
end
--===================== 
-- on stop
function E.onStop()
	-- before stopping
end

--===================== 
-- catch Noteon MIDI event
function E.onNoteOn(deviceNr , timestamp, channel , pitch, velocity )
	if (midiinThru or false) then
		luabass.outNoteOn(pitch,velocity )
	end
end
function E.onNoteOff(deviceNr , timestamp, channel , pitch, velocity )
	if (midiinThru or false) then
		luabass.outNoteOff(pitch )
	end
end

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
function E.keydown ( keyLetter, keyCode, modifiers)
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

return E -- to export the functions
