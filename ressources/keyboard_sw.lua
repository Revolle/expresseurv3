local E={} -- to export the functions

function E.keyboarDisposal()
	-- return the four lines of the SW PC keyboard
	t = 
	{
		{ "1+" , '2"' , "3*" , "4ç" , "5%" , "6&" , "7/" , "8(" , "9)" , "0=" } , 
		{ "Qq" , "Ww" , 'Ee' , "Rr" , "Tt" , "Zz" , "Uu" , "Ii" , "Oo" , "Pp" } , 
		{ "Aa" , "Ss" , 'Dd' , "Ff" , "Gg" , "Hh" , "Jj" , "Kk" , "Ll" , "éö" } , 
		{ "Yy" , "Xx" , "Cc" , "Vv" , "Bb" , "Nn" , "Mm" , ",;" , ".:" , "-_" } 
	}
	return t
end

return E
