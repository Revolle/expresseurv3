local E={} -- to export the functions

function E.keyboarDisposal()
	-- return the four lines of the FR PC keyboard
	t = { 
		{ "&" , "é" , '"' , "'" , "(" , "-" , "è" , "_" , "ç" , "à"  } , 
		{ "A" , "Z" , 'E' , "R" , "T" , "Y" , "U" , "I" , "O" , "P"  } , 
		{ "Q" , "S" , 'D' , "F" , "G" , "H" , "J" , "K" , "L" , "M"  } , 
		{ "W" , 'X' , "C" , "V" , "B" , "N" , "," , ";" , ":" , "!"  } 
	}
	return t
end

return E
