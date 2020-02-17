local E={} -- to export the functions

function E.keyboarDisposal()
	-- return the four lines of the US PC keyboard
	t = { 
		{ "1!" , "2@" , "3#" , "4$" , "5%^" , "6&" , "7*" , "8(" , "9)" , "0_" } , 
		{ "Qq" , "Ww" , 'Ee' , "Rr" , "Tt" , "Yy" , "Uu" , "Ii" , "Oo" , "Pp" } , 
		{ "Aa" , "Ss" , 'Dd' , "Ff" , "Gg" , "Hh" , "Jj" , "Kk" , "Ll" , ":;" } , 
		{ "Zz" , "Xx" , "Cc" , "Vv" , "Bb" , "Nn" , "Mm" , "<?" , ">." , "/?"  } 
	}
	return t
end

return E
