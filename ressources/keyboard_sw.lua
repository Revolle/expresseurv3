local E={} -- to export the functions

function E.keyboarDisposal()
	-- return the four lines of the SW PC keyboard
	t = 
	{
		{ "1" , "2" , "3" , "4" , "5" , "6" , "7" , "8" , "9" , "0" } , 
		{ "Q" , "W" , 'E' , "R" , "T" , "Z" , "U" , "I" , "O" , "P" } , 
		{ "A" , "S" , 'D' , "F" , "G" , "H" , "J" , "K" , "L" , "é" } , 
		{ "Y" , "X" , "C" , "V" , "B" , "N" , "M" , "," , "." , "-" } 
	}
	return t
end

return E
