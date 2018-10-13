local texttochord = require("texttochord")

function interpret(chord , next_chord , tone )
    print("interpret chord :<" .. chord .. ">" , "next chord=<"..(next_chord or "nil")..">" , "tone=<"..(tone or "")..">")

    texttochord.setTone(tone)
    local r 
    r = texttochord.stringToChord(chord  ,next_chord)
    print("root=" .. r.root, "bass=" .. r.bass , "nextBass=" .. (r.nextBass or "") , "text="..r.text)
    print("root=" .. texttochord.ptos(r.root - 1 ), "bass=" .. texttochord.ptos(degreeToPitch(r.root,r.bass,0)) , "nextBass=" .. texttochord.ptos(degreeToPitch(currentTone,r.nextBass,0)) , "chord","chord","penta","scale",r.text)
    local pitchNames = {"I","IIb","II","IIIb","III","IV","IV#", "V", "V#" , "VI", "VII", "VIIM" }
    for j,v in pairs(r.pitchRole) do
      if v.chord or v.scale or v.penta then
        if v.chord then schord = v.chord else schord = " " end
        if v.penta then spenta = v.scale else spenta = " " end
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
  print ( "test the E.stringToDegree()")
  print ( "===========================")
  
  print ( "c/c" , E.stringToDegree("c",1))
  print ( "f/c" , E.stringToDegree("f",1))
  print ( "g/d" , E.stringToDegree("g",E.stringToDegree("d")))
  print ( "c/g" , E.stringToDegree("c",E.stringToDegree("g")))
  local snote = { "b" , "do" , "do#" , "#do" , "#IV",  "3", "e",  "2#", "2b" , "b2", "g" , "gb", "bg", "g#", "#g", "II", "7", "-", "M7" , "Solb.x" , "xyz" }
  for i,v in ipairs(snote) do
    print(i,v, E.stringToDegree(string.lower(v),E.stringToDegree("c")))
  end
  --

--
  --
  print ( "==========================")
  print ( "test the E.stringToChord()")
  print ( "==========================")

  local stest = { 
    { schord="Do" , stone="C" } ,
    { schord="A-" , stone="C" } ,
    { schord="G7"} ,
    { schord="Do-" } ,
    { schord="DoM7" } ,
    { schord="DomM7" } ,
    { schord="I0" } ,
    { schord="IO" } ,
    { schord="CAdd9" } ,
    { schord="C9" } ,
    { schord="A-9" } ,
    { schord="A-" } ,
    { schord="Csus4" } ,
    { schord="Dsus4" } ,
    { schord="D11" } ,
    { schord="D-9" } ,
    { schord="D-9(#4-)/G" } ,
    { schord="D9(-#4)" } ,
    { schord="D-11(7)" }, 
    { schord="Sol"} ,
    { schord="G7" , stone="G" } ,
    { schord="D" } ,
    { schord="Re7" },
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
