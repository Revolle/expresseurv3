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
-- catch Program MIDI event
function E.onProgram(deviceNr , timestamp, channel , programNr )
	luabass.logmsg("Program#" .. programNr)
end

--===================== 
-- catch PC keydown
function E.keydown ( keyLetter, keyCode, modifiers)
-- when a computer key is pressed, this function is called
-- return true if the process of the keydown will not continue

 	luabass.logmsg("keydown(" .. (keyLetter or "" ).. "," .. (keyCode or "") .. "," .. (modifiers or "")  .. ")")
	if (keyCode or -1) == -1 then
		info.action = "!Help about the keydown lua function\nfunctions are described here\n..\n.." 
		return true
	end
	if keyLetter == "Y" then
		info.status = "track 1 not played" 
		info.action = "=-1" 
		return true 
	elseif keyLetter == "X" then
		info.status = "track 2 not played" 
		info.action = "=-2" 
		return true 
	elseif keyLetter == "C" then
		info.status = "track 3 not played" 
		info.action = "=-3" 
		return true 
	elseif keyLetter == "V" then
		info.status = "track 4 not played" 
		info.action = "=-4" 
		return true 
	elseif keyLetter == "B" then
		info.status = "track 5 not played" 
		info.action = "=-5"  
		return true 
	elseif keyLetter == "N" then
		info.status = "track 6 not played" 
		info.action = "=-6"  
		return true 
	elseif keyLetter == "M" then
		info.status = "track 7 not played" 
		info.action = "=-7" 
		return true 
	elseif keyLetter == "," then
		info.status = "track 8 not played" 
		info.action = "=-8"  
		return true 
	elseif keyLetter == "A" then
		info.status = "track 1 piano" 
		info.action = "=+1" 
		luabasss.outSetTrackVolume (30,1)
	elseif keyLetter == "S" then
		info.status = "track 2 piano" 
		info.action = "=+2" 
		luabasss.outSetTrackVolume (30,2)
		return true 
	elseif keyLetter == "D" then
		info.status = "track 3 piano" 
		info.action = "=+3" 
		luabasss.outSetTrackVolume (30,3)
	elseif keyLetter == "F" then
		info.status = "track 4 piano" 
		info.action = "=+4" 
		luabasss.outSetTrackVolume (30,4)
		return true 
	elseif keyLetter == "G" then
		info.status = "track 5 piano" 
		info.action = "=+5" 
		luabasss.outSetTrackVolume (30,5)
	elseif keyLetter == "H" then
		info.status = "track 6 piano" 
		info.action = "=+6" 
		luabasss.outSetTrackVolume (30,6)
		return true 
	elseif keyLetter == "J" then
		info.status = "track 7 piano" 
		info.action = "=+7" 
		luabasss.outSetTrackVolume (30,7)
	elseif keyLetter == "K" then
		info.status = "track 8 piano" 
		info.action = "=+8" 
		luabasss.outSetTrackVolume (30,8)
		return true 
	elseif keyLetter == "Q" then
		info.status = "track 1 mesoforte" 
		info.action = "=+1" 
		luabasss.outSetTrackVolume (70,1)
	elseif keyLetter == "W" then
		info.status = "track 2 mesoforte" 
		info.action = "=+2" 
		luabasss.outSetTrackVolume (70,2)
		return true 
	elseif keyLetter == "E" then
		info.status = "track 3 mesoforte" 
		info.action = "=+3" 
		luabasss.outSetTrackVolume (70,3)
	elseif keyLetter == "R" then
		info.status = "track 4 mesoforte" 
		info.action = "=+4" 
		luabasss.outSetTrackVolume (70,4)
		return true 
	elseif keyLetter == "T" then
		info.status = "track 5 mesoforte" 
		info.action = "=+5" 
		luabasss.outSetTrackVolume (70,5)
	elseif keyLetter == "Z" then
		info.status = "track 6 mesoforte" 
		info.action = "=+6" 
		luabasss.outSetTrackVolume (70,6)
		return true 
	elseif keyLetter == "U" then
		info.status = "track 7 mesoforte" 
		info.action = "=+7" 
		luabasss.outSetTrackVolume (70,7)
	elseif keyLetter == "I" then
		info.status = "track 8 mesoforte" 
		info.action = "=+8" 
		luabasss.outSetTrackVolume (70,8)
		return true 
	elseif keyLetter == "1" then
		info.status = "track 1 forte" 
		info.action = "=+1" 
		luabasss.outSetTrackVolume (100,1)
	elseif keyLetter == "2" then
		info.status = "track 2 forte" 
		info.action = "=+2" 
		luabasss.outSetTrackVolume (100,2)
		return true 
	elseif keyLetter == "3" then
		info.status = "track 3 forte" 
		info.action = "=+3" 
		luabasss.outSetTrackVolume (100,3)
	elseif keyLetter == "4" then
		info.status = "track 4 forte" 
		info.action = "=+4" 
		luabasss.outSetTrackVolume (100,4)
		return true 
	elseif keyLetter == "5" then
		info.status = "track 5 forte" 
		info.action = "=+5" 
		luabasss.outSetTrackVolume (100,5)
	elseif keyLetter == "6" then
		info.status = "track 6 forte" 
		info.action = "=+6" 
		luabasss.outSetTrackVolume (100,6)
		return true 
	elseif keyLetter == "7" then
		info.status = "track 7 forte" 
		info.action = "=+7" 
		luabasss.outSetTrackVolume (100,7)
	elseif keyLetter == "8" then
		info.status = "track 8 forte" 
		info.action = "=+8" 
		luabasss.outSetTrackVolume (100,8)
		return true 
	end
	info.status = "the " .. keyLetter .. " is not processed by keydown.lua" -- info in status bar
	return false 
end

return E -- to export the functions
