
luabass = require("fake_luabass") -- to simulate midiout luabass
tracks = {}
tracks["chord-bass"] = 1
tracks["chord-background"] = 2
tracks["chord-chord"] = 3
tracks["chord-scale"] = 4
 values = {}
values["chord delay"] = 100
values["chord decay"] = 40
values["scale decay"] = 0

local luachord = require("luachord")

--print("stringtodegree=",texttochord.stringToDegree("c",1))
--print("stringtopitch=",texttochord.stringToPitch("c4",1))
  local f = io.open("test_song.txt")
  local contentf 
  if f then
print("=========")
print("READ FILE")
print("=========")
    contentf = f:read("a")
    print(contentf)
  else
    print("cannot read file")
    return
  end
  
 print("==============")
 print("ANALIZE SCORE")
 print("==============")
   luachord.setScore(contentf)
  
   luachord.trace()
  
  print("===============")
  print("RECOGNIZED TEXT")
  print("===============")
  local send, sstart
  local sp = 1
  while (true) do
    sstart , send = luachord.getRecognizedScore()
    if sstart and ( sstart ~= -1 ) then
      if ( sstart > 0 ) then
        print ( sstart , send , string.sub(contentf,sstart,send))
      end
    else
      break ;
    end
    sp = sp + 1
    if ( sp > 100 ) then break end
  end
  
  print("=============")
  print("TEST POSITION")
  print("=============")
  luachord.setPosition(20)
  print("20:getPosition" , luachord.getPosition())
  print("20:getTextPosition" , luachord.getTextPosition())
  luachord.setPosition(40)
  print("40:getPosition" , luachord.getPosition())
  print("40:getTextPosition" , luachord.getTextPosition())
  
  print("=================")
  print("SIMULATION OF USE")
  print("=================")
  print("RESTART with high level function and luafakluachord.lua module")
  luachord.firstPart()
  n = 1 
  while ( true ) do
    if ( luachord.getIdChord() == 0 ) then print("end of song") break end
    --luachord.playBass(time,bid,ch,nr,velocity,param,index,mediane,whiteindex,whitemediane,black)
    print(">>on background")
    luachord.playBackground(1,1,1,1,64,"",1,1,1,1,0)
    luabass.dumpListMidi()
    print(">>of background")
    luachord.playBackground(2,1,1,1,0,"",1,1,1,1,0)
    luabass.dumpListMidi()
    print(">>on chord up")
    luachord.playChordUp(3,3,1,3,64,"",1,1,1,1,0)
    luabass.dumpListMidi()
    print(">>of chord up")
    luachord.playBackground(4,3,4,1,0,"",1,1,1,1,0)
    luabass.dumpListMidi()
    print(">>change chord")
    luachord.changeChordOn(3,2,1,2,64,"",1,1,1,1,0)
    luabass.dumpListMidi()
    print(">>on background")
    luachord.playBackground(1,1,1,1,64,"",1,1,1,1,0)
    luabass.dumpListMidi()
    print(">>of background")
    luachord.playBackground(2,1,1,1,0,"",1,1,1,1,0)
    luabass.dumpListMidi()
     n = n + 1 
     if ( n > 100 ) then break end
    luachord.changeChordOn(n,0,0,0,64,"")
end

