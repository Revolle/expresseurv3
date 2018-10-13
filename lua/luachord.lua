--[[
This LUA-module is loaded by default by the bassLUA-midi-driver.
This LUA-module contains functions which are driven by the GUI over bassLUA-midi-driver :
  - setScore
  - setNrEvent
   - getRecognizedScore
#  - getPosition
#  - setPosition
#
This module interprets a song.
The song is a text file.
The song contains text and chords.
Chords cannot be mixed with free-text-line.
A free-text-line starts with a ponctuation character ( #-/ .. )
Chords are recognized by LUA-script-module "texttochord.lua".
Text and chords can be grouped in sections.
The key can be set in a section
Parts can link sections.
Parts can loop.

e.g. of a score 

    - my song

    section(A,C)
    - the text of section A, in key C
      C        G
    - which have two lines
            F        C

    section(B)
    - that section part B
      G7           C

    section(coda)
    - that'all folks
      G7   F   C

    part(tosing)
    A B
    part(toimprovise ,loop)
    A B B
    part(tofinish)
    A coda



]]--

local E={} -- to export the functions


-- module to recognize the chords
local texttochord = require("texttochord")

-- contains the whole score text
local score = "" 

-- contains all the structured information about the score
local section = {} -- sections which contain text + chords   
local part = {}  -- chains of sections

-- postion in the song
local posPart , posSection , posChord , smoothPart
local currentChord = nil
local prevCurrentChord = nil 
local currentPitches = {}
local currentLoopPitches = {}
local currentChordPitches = nil
local restart = true ;

-- keywords to split parts : <keyword>(<name> [, <key>|loop])
local keyword = { "section" , "part" }

-- position for getRecognizedText
local typeRecognizedPosition , NrRecognizedPosition , NrRecognizedSubPosition

function E.pitchToString(p)
  local d = (p%12) + 1
  local o = math.floor(p/12)
  local s = {"C","C#","D","D#","E","F","F#","G","G#","A","Bb","B"}
  return s[d]..o
end

function E.stringToPitch(s)
 -- return chromatic-range 1..12, string after the pitch-processed,  degree-range 1..7 , alteration -1..1
  return texttochord.stringToPitch(s)
end   

function E.getRecognizedScore()
 -- return start-end of recognized text, or -1 -1 if no more
 -- analyse sections first
 
  local sstart , send 
  
   -- analyse chords in current section
  if ( typeRecognizedPosition == "sectionChord" ) then
    if ( NrRecognizedSubPosition <= #(section[NrRecognizedPosition].chord) ) then
      sstart = section[NrRecognizedPosition].chord[NrRecognizedSubPosition].posStart 
      send = section[NrRecognizedPosition].chord[NrRecognizedSubPosition].posEnd 
      NrRecognizedSubPosition = NrRecognizedSubPosition + 1 ;
      return sstart , send 
    else
      -- no more chord, next section
      typeRecognizedPosition = "section"
      NrRecognizedPosition = NrRecognizedPosition + 1 ;
    end
  end
if ( typeRecognizedPosition == "section" ) then
    if ( NrRecognizedPosition <= #section ) then
      sstart = section[NrRecognizedPosition].titleStart 
      send = section[NrRecognizedPosition].titleEnd
      NrRecognizedSubPosition = 1
      typeRecognizedPosition = "sectionChord" -- will search the chords in the current section
      return sstart , send 
    else
      typeRecognizedPosition = "part"
      NrRecognizedPosition = 1
    end
  end
 
  -- analyse the sections in the current part
  if ( typeRecognizedPosition == "partSection" ) then
    if ( NrRecognizedSubPosition <= #(part[NrRecognizedPosition].section) ) then
      sstart = part[NrRecognizedPosition].section[NrRecognizedSubPosition].posStart 
      send = part[NrRecognizedPosition].section[NrRecognizedSubPosition].posEnd 
      NrRecognizedSubPosition = NrRecognizedSubPosition + 1 ;
      return sstart , send 
    else
      -- no more section, next part
      typeRecognizedPosition = "part"
      NrRecognizedPosition = NrRecognizedPosition + 1 ;
    end
  end
 -- analyse the chains
 if ( typeRecognizedPosition == "part" ) then
    if ( NrRecognizedPosition <= #part ) then
      sstart = part[NrRecognizedPosition].titleStart 
      send = part[NrRecognizedPosition].titleEnd 
      NrRecognizedSubPosition = 1
      typeRecognizedPosition = "chainSection" -- will search for the sections in the current part
      return sstart , send 
    else
      typeRecognizedPosition = "chord"
      NrRecognizedPosition = 1
    end
  end
  
  -- no more recognized text
  return -1 , -1 
end

function E.trace()
  -- for debug purpose
  print()
  for nrSection=1 , #section ,  1 do
    print("=============================================")
    print( "section#" .. nrSection , section[nrSection].name , section[nrSection].key , section[nrSection].posStart .. ".." .. section[nrSection].posEnd )
    print("=============================================")
    if ( section[nrSection].chord ) then
      for nrChord = 1 , #(section[nrSection].chord) ,1 do
        local tc = section[nrSection].chord[nrChord]
        print("chord #", tc.nrChord , string.sub(score,tc.posStart,tc.posEnd) , "root=".. tc.interpretedChord.root )
      end
    end
  end
  print()
  for nrChord=1 , #part ,  1 do
    print("*********************************************")
    print( "part#" .. nrChord , part[nrChord].name , part[nrChord].loop , part[nrChord].posStart .. ".." .. part[nrChord].posEnd )
    print("*********************************************")
    for nrSection=1 , #(part[nrChord].section) ,  1 do
      print ("       section#" .. part[nrChord].section[nrSection].nrSection , " = " , section[part[nrChord].section[nrSection].nrSection].name )
    end
  end
  print()
end
local function printPos()
print ("position")
print ("  part#" .. posPart , part[posPart].name )
print ("     section#" .. posSection , section[part[posPart].section[posSection].nrSection].name )
print ("         chord#" .. posChord , section[part[posPart].section[posSection].nrSection].chord[posChord].root)
end

local function extractParts()
  -- read the text file, and extract the Parts
  -- Parts are stored in in the table <parts>
  -- a part starts with the textt: word()
  -- the possible words are in the table <keyword> 
  local posStart = 1
  local pmax = string.len(score)
  local pbest = pmax
  local nbest = nil 
  local pat={}
  -- search for the first keyword : section / part
  for _, v in ipairs(keyword) do
    table.insert(pat,{})
    pat[#pat].pat = v .. "(%b())"
    pat[#pat].posStart, pat[#pat].posEnd , pat[#pat].name = string.find(score , pat[#pat].pat ,posStart)
    if ((pat[#pat].posStart) and (pat[#pat].posStart < pbest)) then
      pbest = pat[#pat].posStart
      nbest = #pat
    end
  end
  if ( nbest == nil ) then
    -- no keyword, only one section in one part is created
    table.insert(section,{})
    section[1].name = "noname"
    section[1].content = score
    section[1].posStart = 1
    section[1].posEnd = string.len (score)
    section[1].titleStart = 0
    section[1].titleEnd =0
    return
  end
  if ( pbest > 2 ) then
    -- the text before the first keyword is a section with no name
    table.insert(section,{})
    section[1].name = "noname"
    section[1].content = string.sub(score,1 , pbest - 1)
    section[1].posStart = 1
    section[1].posEnd =  pbest - 1
    section[1].titleStart = 0
    section[1].titleEnd = 0
 end

  while nbest ~= nil do
    -- store the next sections
    local pp 
    if ( nbest == 1 ) then
      -- section of text and chords
      table.insert(section,{})
      pp = section[#section]
    elseif ( nbest == 2 ) then
      -- part of sections
      table.insert(part,{})
      pp = part[#part]
    end
    pp.titleStart = pat[nbest].posStart
    pp.titleEnd = pat[nbest].posEnd
    -- print("name",pat[nbest].name)
    if ( string.find(pat[nbest].name,",") ) then
      -- there are additional information in the line
      -- extract the name of the section or part, and the additional parameter
      local sk
      pp.name , sk = string.match(pat[nbest].name,"%(%s*(%w+)%s*,%s*(%w+)%s*%)")
      if ( nbest == 1 ) then
        -- tone of the section
        local k = texttochord.stringToPitch(sk)
        if ( k ) then
          pp.key = k % 12
        end
      elseif ( nbest == 2 ) then
        -- loop of the part
        if ( sk == "loop" ) then
          pp.loop = true          
        else
          pp.loop = false          
        end
      end
    else
      -- extract the name of the section or part
      pp.name = string.match(pat[nbest].name,"%(%s*(%w+)%s*%)")
    end
    if ( pp.name == nil ) then pp.name = "" end
    local b ,nrChord 
    b = pat[nbest].posEnd + 1
    pbest = pmax
    nbest = nil
    -- search for next section
    for i, v in ipairs(keyword) do
      pat[i].pat = v .. "(%b())"
      pat[i].posStart, pat[i].posEnd , pat[i].name = string.find(score , pat[i].pat ,b)
      if ((pat[i].posStart) and (pat[i].posStart < pbest)) then
        pbest = pat[i].posStart
        nbest = i
      end
    end
    pp.posStart = b
    if ( nbest == nil ) then
      pp.content = string.sub(score,b)
      pp.posEnd = string.len(score)
    else
      nrChord = pat[nbest].posStart - 1
      pp.content = string.sub(score,b,nrChord)
      pp.posEnd = nrChord
    end
  end
end
local function extractSectionFromPart()
  -- in the loop and part, recognize the sections requested
  -- complete the information in the table <parts>.section
  -- for the type <part> and loop
  
  if (#part == 0) then
    -- no part declared, create a virtual part of all sections
    table.insert(part,{})
    part[1].posStart = 1
    part[1].posEnd = 1
    part[1].section = {}
    for nrSection=1 , #section , 1 do
      part[1].section[#(part[1].section)+1] = {}
      local pc = part[1].section[#(part[1].section)]
      pc.nrSection = nrSection
      pc.posStart = 0
      pc.posEnd = 0          
    end
    return
  end
  for nrPart=1 , #part ,  1 do
    part[nrPart].section = {}
    local posStart , posEnd , word
    posStart = 1
    posEnd = 1
    while(true ) do
      posStart , posEnd , word = string.find(part[nrPart].content,"(%w+)",posEnd + 1)
      if ( posStart == nil ) then break end
      -- search the existence of these sections
      for nrSection=1 , #section , 1 do
       if ( section[nrSection].name == word ) then
          -- the requested section exists
          part[nrPart].section[#(part[nrPart].section)+1] = {}
          local pc = part[nrPart].section[#(part[nrPart].section)]
          pc.nrSection = nrSection
          pc.posStart = posStart + part[nrPart].posStart - 1
          pc.posEnd = posEnd  + part[nrPart].posStart - 1          
          break 
        end
      end
    end
  end
end

local function extractChord()
  -- extract the chords fom the "section"
  -- chords are recognized according to the patterns
  -- line starting with ponctuation is ignored 
  local nrChord = 1
  for nrSection=1 , #section , 1 do
    section[nrSection].chord = {}
    local l = 0 
    local pl = 1 
    local endl = false
    -- analyse each line
    while(endl == false) do
      l = string.find(section[nrSection].content,"\n",l+1 )
      if ( l == nil ) then 
        endl = true 
        l = string.len(section[nrSection].content)
      end
      -- detect free-text-ilne with a ponctuation character  at the beginning of the line
      local sl 
      sl = string.sub(section[nrSection].content,pl,l)
      local ponctuation 
      ponctuation = string.find(sl,"^%p")
      if ( ponctuation == nil ) then
        local gstart
        local gend = 0 
        local sg
        -- analyse each word of the line
        while(true) do
          gstart , gend , sg = string.find(sl,"(%g+)", gend + 1 )
          if ( gstart == nil ) then break end
          if (( sg ~= "[" ) and ( sg ~= "]" ) 
            and ( sg ~= "(" ) and ( sg ~= ")" ) 
            and ( sg ~= "|" ) and ( sg ~= "||" ) and ( sg ~= "/" )) then
            local interpretedChord = texttochord.stringToChord(sg)
            if interpretedChord then
              table.insert(section[nrSection].chord,{})
              local pc = section[nrSection].chord[#(section[nrSection].chord)]
              pc.interpretedChord = interpretedChord
              if ( pc.interpretedChord.sameChord ) and ( #(section[nrSection].chord) > 1 ) then
                pc.interpretedChord.pitch = section[nrSection].chord[#(section[nrSection].chord) - 1].interpretedChord.pitch
              end
              pc.posStart = section[nrSection].posStart + gstart + pl - 2
              pc.posEnd = section[nrSection].posStart + gend + pl - 2
              pc.nrChord = nrChord
              nrChord = nrChord + 1
            end
          end
        end
        pl = l
      end
    end
  end
end  


function analyseScore()
  extractParts()
  extractSectionFromPart()
  extractChord()
end

function E.setScore(sscore)
  -- load the song from string ssong
  -- extract the structured information in table chord_score
  score = sscore
  section = {}    
  part = {} 
  typeRecognizedPosition = "section"
  NrRecognizedPosition = 1 
  posPart = nil 
  
  analyseScore()
  E.firstPart()
  E.letChord()
  
end
function E.setFile(fscore)
  -- load the song from string ssong
  -- extract the structured information in table chord_score
  local f = io.open(fscore)
  if f then
    E.setScore(f:read("a")) 
  end
end

function E.getTextPosition()
  -- return text :
  --    part
  --    section
  --    chord
  
  if ( currentChord ~= nil ) then
    local ph = part[posPart]
    local ps = section[ph.section[posSection].nrSection]
    local pc = ps.chord[posChord]
    local schord = string.sub(score,pc.posStart, pc.posEnd)
    return ph.name , ps.name , schord
  end
end

function E.getIdChord()
  -- return index of the current chord
  if posSection and posChord then
   return posSection * 128 + posChord + 1
 else
   return 0
 end
end
function E.isNoChord()
  -- return true if there is a No Chord
  if ( currentChord == nil ) then return true end
  return ( currentChord.interpretedChord.noChord or false )
end
function E.getPitches(scale)
  if currentChord == nil then return nil end
  return currentChord.interpretedChord.pitch[scale]
end
function E.getIndexPitches(scale,index,alteration)
  -- scale name, index of the pitch, alteration=-1,0,1
  -- return the notes of the scale for the currentChord, starting at index
  if currentChord == nil then return nil end
  --print("scale="..scale,"index="..index,"alteration="..alteration)
  --texttochord.dumpPitch(currentChord.interpretedChord.pitch,scale)
  return currentChord.interpretedChord.pitch[scale][index][alteration]
end
function E.letChord()
  -- set the current chord in pointer currentChord
 local noinfinite = false
  currentChord = nil
  if ( posPart == nil ) then 
    E.firstPart() 
  end
  while(true) do
    if ( posPart > #part ) then
      return -- no more part
    end
    if ( posSection <= #(part[posPart].section) ) then
      if (posChord <= #(section[part[posPart].section[posSection].nrSection].chord)) then 
        break 
      end
    end
    if ( smoothPart > 0 ) then
      posPart = smoothPart
      posSection = 1 
      posChord = 1
      smoothPart = 0 
    else      
      if ( posSection < #(part[posPart].section)) then 
        posSection = posSection + 1 
        posChord = 1 
      else
        if ( part[posPart].loop ) then
          posSection = 1
          posChord = 1
          if ( noinfinite ) then return end
          noinfinite = true
        else
          if ( posPart < #part ) then
            posPart = posPart + 1 
            posSection = 1
            posChord = 1 
          else
            return
          end
        end
      end
    end
  end
  currentChord = section[part[posPart].section[posSection].nrSection].chord[posChord]
end

function E.isRestart()
  local r = restart
  restart = false 
  return r
end
function E.nextChord(time,bid,ch,nr,velocity)
  -- move to the next chord
  if velocity == 0 then return end
  posChord = posChord + 1
  E.letChord()
end
function E.isNewChord()
  if currentChord == prevCurrentChord then return false end
  prevCurrentChord = currentChord
  return true
end
function E.previousChord()
  -- move to the previous chord
  posChord = posChord - 1
  if ( posChord < 1 ) then
    posChord = 1
  end
  E.letChord()
end
function E.nextSection()
  -- go to the next section
  posSection = posSection + 1 
  posChord = 1 
  E.letChord()
end
function E.firstPart()
  -- go to the beginning of the score
  posPart = 1
  posSection = 1
  posChord = 1
  smoothPart = 0
  restart=true
  E.letChord()
end
function E.firstPartSmooth()
  -- wait the end of the current section , go to the beginning of the score
  smoothPart = 1
end
function E.nextPart()
  -- go to the next part ( or loop )
  posPart = posPart + 1
  posSection = 1 
  posChord = 1 
  smoothPart = 0
  E.letChord()
end
function E.nextPartSmooth()
  --  wait the end of the current section, go to the next part ( or loop )
  smoothPart = posPart + 1
end
function E.lastPart()
  -- go to the last part ( tipycally a coda )
  posPart = #part
  posSection = 1 
  posChord = 1 
  smoothPart = 0
   E.letChord()
end
function E.lastPartSmooth()
  -- wait the end of the current section go to the last part ( tipycally a coda )
  smoothPart = #part
end
function E.gotoPart(nrchainh)
  -- go to the part
  if ( type(nrchain) ~= "number" ) then return end
  posPart = nrchain
  posSection = 1 
  posChord = 1 
  smoothPart = 0
  E.letChord()
end
function E.gotoPartSmooth(nrchain)
  -- wait the end of the current section, go to the part
  if ( type(nrchain) ~= "number" ) then return end
  smoothPart = nrchain
end
function E.repeatPart()
  -- repeat the part ( or loop )
  posPart = posPart
  posSection = 1 
  posChord = 1 
  smoothPart = 0
  E.letChord()
end
function E.repeatPartSmooth()
  -- wait the end of the current section, and repeat the part ( or loop )
  smoothPart = posPart
end
function E.getPosition()
  -- return position in the original text-score :
  --    part position <start> and <end>
  --    chord position <start> and <end> 
  --    chord number
  if ( currentChord == nil ) then
    return -1 , -1 , -1 , -1 , -1
  end
  local pscore = part[posPart]
  local posPart = pscore.section[posSection]
  local posSection = section[posPart.nrSection]
  local posChord = posSection.chord[posChord]
  return  posPart.posStart , posPart.posEnd , posChord.posStart , posChord.posEnd , posChord.nrChord
end
function E.setPosition(pos)
  -- set the position in the structure, according to given text position
  restart = true
  E.letChord()
  for nrSection = 1 , #(section) , 1 do
    for nrChord = 1 , #(section[nrSection].chord) , 1 do
      local pc = section[nrSection].chord[nrChord]
      if ( pc == nil ) then 
        print ( "error section[".. nrSection .. "].chord[" .. nrChord .. "]" ) 
        return
      end
      if (( pos >= ( pc.posStart - 1) ) and ( pos <= ( pc.posEnd) )) then
        -- a chord nrChord exists at this position in the section nrSection
        if ( nrSection == part[posPart].section[posSection].nrSection ) then
          -- already the current section. Go to this chord
          posChord = nrChord
          E.letChord()
          return
        end
        for cs = 1 , #(part[posPart].section) , 1 do
          if ( nrSection == part[posPart].section[cs].nrSection ) then
            -- the section exists in the current part. Go to this section/chord 
            posChord = nrChord
            posSection = cs
            E.letChord()
            return
          end
        end
        for cc = 1 , #(part) , 1 do
          for cs = 1 , #(part[cc].section) , 1 do
            if ( nrSection == part[cc].section[cs].nrSection ) then
              -- the section exists in the chains. Gor to this part/section/chord
              posChord = nrChord
              posSection = cs
              posPart = cc
              E.letChord()
              return
            end
          end
        end
        -- chord not found in the chains
        return
      end
    end
  end
  for nrPart = 1 , #(part) , 1 do
    for nrSection = 1 , #(part[nrPart].section) , 1 do
      local pc = part[nrPart].section[nrSection]
      if ( pc == nil ) then 
        print ( "error part[".. nrPart .. "].section[" .. nrSection .. "]" ) 
        return
      end
      if (( pos >= ( pc.posStart - 1 ) ) and ( pos <= ( pc.posEnd + 1))) then
        -- a section in a part exists at this position
        posPart = nrPart 
        posSection = nrSection
        posChord = 1
        E.letChord()
        return
      end
    end
  end
  E.letChord()
end
function E.setNrEvent(askedNrChord)
  E.letChord()
  for nrSection = 1 , #(section) , 1 do
    for nrChord = 1 , #(section[nrSection].chord) , 1 do
      local pc = section[nrSection].chord[nrChord]
      if ( pc == nil ) then 
        print ( "error section[".. nrSection .. "].chord[" .. nrChord .. "]" ) 
        return
      end
      if (pc.nrChord == askedNrChord) then
        -- a chord nrChord exists at this position in the section nrSection
        if ( nrSection == part[posPart].section[posSection].nrSection ) then
          -- already the current section. Go to this chord
          posChord = nrChord
          E.letChord()
          return
        else
          posSection = nrSection
          posChord = nrChord
          E.letChord()
        end
      end
    end
  end  
end
function E.setChord(chordText)
-- set the current chord as specified in the string parameter
  local mInterpretedChord = texttochord.stringToChord(chordText)
  currentChord={}
  currentChord.interpretedChord = mInterpretedChord
  return mInterpretedChord
end

-- functions to play the chords, which can be connected directly to selectors
-------------------------------------------------------------------------------
-- the bid is the "button Identifiant", which is a unique id for a button Midi-in

-- list of bid playing, per scale :
local bidPlayed = { ["scale"] = {} ; ["background"] = {} ; ["chord"] = {} ; ["bass"] = {} }

-- list of bid which are "sustained" ( pedal ), per scale
local bidPedal = { ["scale"] = {} ; ["background"] = {} ; ["chord"] = {} ; ["bass"] = {} }

-- transposition of each scale
E.trackOctave = { ["scale"] = 1*12 ; ["background"] = 0*12 ; ["chord"] = 0*12 ; ["bass"] = 1*12 }

-- type of legato per scale
--   0 = no legato
--   1 = legato up to thext note of the scale
--   2 = legato up to the next chord
local legatoPlay = { ["scale"] = 2 ; ["background"] = 1 ; ["chord"] = 1 ; ["bass"] = 1 }

local currentBlack = 0
local previousBlack = 0

function offPedal(track)
  -- switch off zombi-pedals, and move active-pedal to active-non-pedal
  for k,v in pairs(bidPedal[track]) do
    if v < 0 then
      luabass.outChordOff(v * (-1)) -- kill zombi pedal
    else
      bidPlayed[track][k] = v  -- move non-zombi pedal to non-pedal chord
    end
  end
  bidPedal[track] = {} -- no more pedal pending
end
function legato(track,velocity,param)
  -- set the legato of the track
  -- param contains the status of the pedal : "off"  or "legato" , or "pedal" 
  -- if param is empty : velocity = 0 <=> off , velocity > O <=> pedal
  local v
  if string.len(param) > 0 then
    if string.find(param,"legato") then 
      v = 1
    elseif string.find(param,"off") then
      v = 0
    elseif string.find(param,"pedal") then
      v = 2
    end
  else
    if velocity > 0 then
      v = 2
    else
      v = 0
    end
  end
  legatoPlay[track] = v
  if v == 0 then
    offPedal(track)
    for k,v in pairs(bidPlayed[track]) do
      if v < 0 then
        luabass.outChordOff(v * (-1))
        bidPlayed[track][k] = nil
      end
    end
  elseif v == 1 then
      offPedal(track)
  end
end
function E.pedalScale(time,bid,ch,nr,velocity,param,index,mediane,whiteindex,whitemediane,black)
  legato("scale",velocity,param)
end
function E.pedalBackground(time,bid,ch,nr,velocity,param,index,mediane,whiteindex,whitemediane,black)
  legato("background",velocity,param)
end
function E.pedalChord(time,bid,ch,nr,velocity,param,index,mediane,whiteindex,whitemediane,black)
  legato("chord",velocity,param)
end
function E.pedalBass(time,bid,ch,nr,velocity,param,index,mediane,whiteindex,whitemediane,black)
  legato("bass",velocity,param)
end
function E.octaveScale(time,bid,ch,nr,velocity,param,index,mediane,whiteindex,whitemediane,black)
  E.trackOctave["scale"] = 12 * (tonumber(parameter) or (E.trackOctave["scale"] / 12 ))
end
function E.octaveBackground(time,bid,ch,nr,velocity,param,index,mediane,whiteindex,whitemediane,black)
  E.trackOctave["background"] = 12 * (tonumber(parameter) or (E.trackOctave["background"] / 12 ))
end
function E.octaveChord(time,bid,ch,nr,velocity,param,index,mediane,whiteindex,whitemediane,black)
  E.trackOctave["chord"] = 12 * (tonumber(parameter) or (E.trackOctave["chord"] / 12 ))
end
function E.octaveBass(time,bid,ch,nr,velocity,param,index,mediane,whiteindex,whitemediane,black)
  E.trackOctave["bass"] = 12 * (tonumber(parameter) or (E.trackOctave["bass"] / 12 ))
end

function E.playPitches(bid,velocity,index,black,scale,track,pstart,pend,delay,decay)
  -- play the pitches of the scale, 
  -- pitch is calculated from the index and black, within the scale
  -- bid is the unique id of the MIDI-in button, ith its velocity ( 0 == note-off )
  
  
  local typeLegato = legatoPlay[track]
  
  if velocity == 0 then 
    local playedId = bidPlayed[track][bid]
    if playedId then
      if typeLegato ~= 1 then
        -- non legato. on note-off : switch off the bid chord
        luabass.outChordOff(math.abs(playedId))
        bidPlayed[track][bid]=nil
        return
      end
      bidPlayed[track][bid] = playedId * (-1) -- make it zombi
      return
    end
    local pedalId = bidPedal[track][bid]
    if pedalId then
      bidPedal[track][bid] = pedalId * (-1) -- make it zombi
    end
    return
  end
  --noteon
  if typeLegato == 1 then
    -- switch-off the previous zombi
    for k,v in pairs(bidPlayed[track]) do
      luabass.outChordOff(math.abs(v))
    end
    bidPlayed[track] = {}
  elseif typeLegato == 2 then
    if bidPedal[track][bid] then
      luabass.outChordOff(math.abs(bidPedal[track][bid]))
    end
  end
  -- switch-on the chord
  local tpitch = E.getIndexPitches(scale,index,black)
  if tpitch then
    --luabass.logmsg(track.." decay="..decay.." play=>"..table.concat(tpitch,"/"))
    local id = luabass.outChordSet(-1,E.trackOctave[track],delay or 0,decay or 64,pstart or 1,pend or -1,table.unpack(tpitch))
    luabass.outChordOn(id,velocity,0,tracks["chord-" .. track])
    if typeLegato == 2 and black == 0 then
      bidPedal[track][bid] = id
    else
      bidPlayed[track][bid] = id
     end
  end
end
function E.playScaleChord(time,bid,ch,nr,velocity,param,index,mediane,whiteindex,whitemediane,black)
  E.playPitches(bid,velocity,whitemediane,black,"chord","scale",1,-1,values["scale delay"],values["scale decay"])
end
function E.playScalePenta(time,bid,ch,nr,velocity,param,index,mediane,whiteindex,whitemediane,black)
  E.playPitches(bid,velocity,whitemediane,black,"penta","scale",1,-1,values["scale delay"],values["scale decay"])
end
function E.playChordUp(time,bid,ch,nr,velocity,param,index,mediane,whiteindex,whitemediane,black)
  E.playPitches(bid,velocity,0,0,"chord","chord",-1,1,values["chord delay"],values["chord decay"])
end
function E.playChordDown(time,bid,ch,nr,velocity,param,index,mediane,whiteindex,whitemediane,black)
  E.playPitches(bid,velocity,0,0,"chord","chord",1,-1,values["chord delay"],values["chord decay"])
end
function E.playBackground(time,bid,ch,nr,velocity,param,index,mediane,whiteindex,whitemediane,black)
  E.playPitches(bid,velocity,0,0,"chord","background",1,-1,0,45)
  E.playPitches(bid,velocity,1,0,"bass","bass",1,1,0,0)
end
function E.playBass(time,bid,ch,nr,velocity,param,index,mediane,whiteindex,whitemediane,black)
  E.playPitches(bid,velocity,1,0,"bass","bass",1,1,0,0)
end
function E.playWalkingBass(time,bid,ch,nr,velocity,param,index,mediane,whiteindex,whitemediane,black)
  E.playPitches(bid,velocity,whiteindex,black,"bass","bass",1,1,0,0)
end
function E.changeChord()
  -- change to next chord 
  offPedal("bass")
  offPedal("background")
  offPedal("chord")
  offPedal("scale")
  E.nextChord()
end
function E.changeChordOn(time,bid,ch,nr,velocity,param,index,mediane,whiteindex,whitemediane,black)
  -- change to next chord on noteOn
  if E.isRestart() or (velocity == 0) then
 	return
  end
  E.changeChord()
end
function E.changeChordOff(time,bid,ch,nr,velocity,param,index,mediane,whiteindex,whitemediane,black)
  -- change to next chord on noteOff ( for anticipation )
  if velocity == 0 then
    E.changeChord()
  end
end
function E.alternateChord(time,bid,ch,nr,velocity,param,index,mediane,whiteindex,whitemediane,black)
  -- change to next chord when alternate white <=> black
  if E.isRestart() then 
    previousBlack = black
    currentBlack = black 
  else
    currentBlack = black
  end
  if previousBlack ~= currentBlack  then 
    previousBlack = currentBlack
    E.changeChord()
  end
end

return E -- to export the functions