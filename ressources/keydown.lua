local E={} -- to export the functions

function E.keydown ( keyLetter, keyCode, modifiers)
-- when a computer key is pressed, this function is called
-- return bool + string
-- returned string started with :
--    ! => messagebox
--    * => change view play
-- other string is displayed in statusbar
--    with nil parameter : return false + help 
--     if key is processed : true + "detail" 
--     else : false + ""
	luabass.logmsg("keydown(" .. (keyLetter or "" ).. "," .. (keyCode or "") .. "," .. (modifiers or "")  .. ")")
	if (keyCode or -1) == -1 then
		return true , "!Help about the keydown lua function\nfunctions are described here\n..\n.."
	end
	if keyLetter == "A" then
		return true , "**/"
	elseif keyLetter == "B" then
		return true , "*1/2"
	end
	return false , keyLetter .. " is not processed by keydown.lua"
end

return E -- to export the functions