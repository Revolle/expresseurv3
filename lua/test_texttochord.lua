local texttochord = require("texttochord")

function interpret(chord , next_chord , tone )
    print("interpret chord :<" .. chord .. ">" , "next chord=<"..(next_chord or "nil")..">" , "tone=<"..(tone or "")..">")

    texttochord.setTone(tone)
    local r 
    r = texttochord.stringToChord(chord  ,next_chord)
    print("root=" .. texttochord.ptos(r.root - 1 ), "bass=" .. texttochord.ptos(degreeToPitch(r.root,r.bass,0)) , "nextBass=" .. texttochord.ptos(degreeToPitch(r.root,r.nextBass,0)) , "chord","penta","scale",r.text)
    local pitchNames = {"I","IIb","II","IIIb","III","IV","IV#", "V", "V#" , "VI", "VII", "VIIM" }
    for j,v in pairs(r.pitchRole) do
      if v.chord or v.scale or v.penta then
        if v.chord then schord = v.chord else schord = " " end
        if v.penta then spenta = v.penta else spenta = " " end
        if v.scale then sscale = v.scale else sscale = " " end
        print ( "", "","", pitchNames[j] ,schord, spenta , sscale )
      end
    end
    print("")
    texttochord.dumpPitch(r.pitch,"bass")
    texttochord.dumpPitch(r.pitch,"chord")
    texttochord.dumpPitch(r.pitch,"penta")
    texttochord.dumpPitch(r.pitch,"scale")
end

function automatic_test()
--=========================  
  print ( "===========================")
  print ( "test the texttochord.stringToDegree()")
  print ( "===========================")
  
  print ( "c/c" , texttochord.stringToDegree("c",1))
  print ( "f/c" , texttochord.stringToDegree("f",1))
  print ( "g/d" , texttochord.stringToDegree("g",texttochord.stringToDegree("d")))
  print ( "c/g" , texttochord.stringToDegree("c",texttochord.stringToDegree("g")))
  local snote = { "b" , "do" , "do#" , "#do" , "#IV",  "3", "e",  "2#", "2b" , "b2", "g" , "gb", "bg", "g#", "#g", "II", "7", "-", "M7" , "Solb.x" , "xyz" }
  for i,v in ipairs(snote) do
    print(i,v, texttochord.stringToDegree(string.lower(v),texttochord.stringToDegree("c")))
  end
  --

--
  --
  print ( "==========================")
  print ( "test the texttochord.stringToChord()")
  print ( "==========================")

  local stest = { 
    { schord="Do" , stone="C" } ,
    { schord="A.-" , stone="C" } ,
    { schord="G.7"} ,
    { schord="Do-" } ,
    { schord="Do.M7" } ,
    { schord="Do.m.M7" } ,
    { schord="I.0" } ,
    { schord="I.O" } ,
    { schord="C.Add9" } ,
    { schord="C.9" } ,
    { schord="A.-.9" } ,
    { schord="A.-" } ,
    { schord="C.sus4" } ,
    { schord="D.sus4" } ,
    { schord="D.11" } ,
    { schord="D.-.9" } ,
    { schord="D.-.9(#4-)/G" } ,
    { schord="D.9(-#4)" } ,
    { schord="D.-.11(7)" }, 
    { schord="Sol"} ,
    { schord="G.7" , stone="G" } ,
    { schord="D" } ,
    { schord="Re.7" },
    { schord="C/G" , stone="C" },
    { schord="G/B" },
    { schord="G/I" },
    { schord="G/VII"  },
    { schord="C[dorien]"  },
    { schord="C[!dorien]" },
    { schord="C[!balkan]" },
    { schord="C[!balkan/C]" },
    { schord="G[!balkan/G]" },
    { schord="G[!balkan/C]" },
    { schord="G[!balkan]"  },
    { schord="G"},
    { schord="C[|balkan]" },
    { schord="G[balkan/C]" },
    { schord="C[!bertha]" },
    {  }
  }
  local sbass, schord, spenta , sscale , root , tone
  for i,c in ipairs(stest) do
    if c.schord then
      print("++++++++++++++++++")
      print("interpretation of " , c.schord , "tone=", c.stone , ">>" )
      if stest[i+1] then
        print("   followed by" , stest[i+1].schord )
        interpret(c.schord , stest[i+1].schord , c.stone )
      else
        interpret(c.schord , nil , c.stone )
      end
    end
  end
end

function manual_test()
--=====================

  previous_tone = "C"
  while(true) do

    print("chord ? ")
    chord = io.read()
    if ( chord == "" ) then
      return
    end

    print("next chord ? ")
    next_chord = io.read()
    if ( next_chord == "" ) then
      next_chord = nil
    end

    print("tone ? ")
    tone = io.read()
    if ( tone == "" ) then
      tone = previous_tone
    end
    previous_tone = tone

    interpret(chord , next_chord, tone)
    
  end
end

manual_test()
--automatic_test()
