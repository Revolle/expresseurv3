local E={} -- to export the functions

function E.keyboarDisposal()
	-- return the four lines of the FR PC keyboard
	t = { 
		{ "1&" , "2é2" , '3"' , "4'" , "5(" , "6-" , "7è" , "8_" , "9ç" , "0à"  } , 
		{ "Aa" , "Zz" , 'Ee' , "Rr" , "Tt" , "Yy" , "Uu" , "Ii" , "Oo" , "Pp"  } , 
		{ "Qq" , "Ss" , 'Dd' , "Ff" , "Gg" , "Hh" , "Jj" , "Kk" , "Ll" , "Mm"  } , 
		{ "Ww" , 'Xx' , "Cc" , "Vv" , "Bb" , "Nn" , "?," , ".;" , ":/" , "!§"  } 
	}
	return t
end

return E
