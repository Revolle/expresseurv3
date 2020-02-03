local E={} -- to export the functions
--[[
This LUA module is on the user directory ExpresseruV3/ressources/luauser.lua
It is used to customize the behavior of Expresseur

It can write 
	info.status = "staus message to display in the gui" 
	info.msgbox = "message-box to display in the gui" 
	info.playview => track to play/view
	info.file => value to increment decrement file

It contains information about valid or non valid MIDI

It can catch computer keydown

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

 info.msgbox = "keydown"

	luabass.logmsg("keydown(" .. (keyLetter or "" ).. "," .. (keyCode or "") .. "," .. (modifiers or "")  .. ")")
	if (keyCode or -1) == -1 then
		info.msgbox = "Help about the keydown lua function\nfunctions are described here\n..\n.." 
		return true
	end
	if keyLetter == "A" then
		info.playview = "*/"
		return true 
	elseif keyLetter == "B" then
		info.playview = "1/2" 
		return true 
	end
	info.status = keyLetter .. " is not processed by keydown.lua" -- info in status bar
	return false 
end

return E -- to export the functions
