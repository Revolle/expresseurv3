local E={}
function E.onTimer(f)
	if countTimer then 
		countTimer = (countTimer or 0) + 1
		if countTimer == 100 then
			countTimer = 0 
			luabass.logmsg("OnTimer 100 time")
		end
	else
		countTimer = 0
		luabass.logmsg("OnTimer first time")
	end
end
return E


