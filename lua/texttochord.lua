--[[
Translate a string which contains a chord description.
Return the pitches for : bass, chord, penta, scale

A pitch = midi_pitch[1..127]    
    ( old + 128*scale_degree )
       e.g. : 
         in chord-C, C4(64) is degree 1 , midi_pitch = 64+1*128
         in chord-C, G4(72) is degree 5, midi-pitch = 72 + 5*128
      To retrieve :
         - midi_pitch = pitch % 128
         - degree (if any) = floor(pitch/128)
  
The chord starts with the root and its optional modifiers ( e.g. C7 )
The chord can have options: /bass (scale_modifiers) [mode/root] @center

Pitch example : C, Bb , F# , Sol, IV , 4 ... 
Modifiers example : sus4, - , M7, 0, ....
Bass is a pitch
Scale_modifiers example : 3, #4 , ...( suppress the pitch from the chord ). 
Mode : name from the list. By default, based on the root 
    prefix !  chord does not influence the mode, and mode can influence the 3rd and 5th of the chord if not specified
    prefix :  chord influences the mode
    prefix _  mode is used for this chord and the next chords
    prefix .  mode is used for this chord only
    /modeRoot : if not specified, based on the root of the chord

Special meanings
NC : no chord
% : same chord dame previous
=pitch : set the tone
@pitch : center the keyboar on this pitch
*.lua : LUA script to execute

Example :
C
CM7
G7/D
Fadd11
C(#4)
SolSus4
D[!dorien] 
B.b5
Bb.9
@D3
NC
$tuning.lua
=C
%

V 1.1 20/8/2015

]]--
local E={} -- to export the functions

local modes = { -- specify the modes
  -- example for the standard diatonic C scale :
  --           1 0 2 0 3 4 0 5 0 6 0 7  
  --           C   D   E F   G   A   B  -- ( in tone C )
  -- majeur
  { degree = { 1,0,2,0,3,4,0,5,0,6,0,7} , names = {"modei","ionien"}},
  { degree = { 1,0,2,3,0,4,0,5,0,6,7,0} , names = {"modeii","dorien"}},
  { degree = { 1,2,0,3,0,4,0,5,6,0,7,0} , names = {"modeiii","phrygien"}},
  { degree = { 1,0,2,0,3,0,4,5,0,6,0,7} , names = {"modeiv","lydien"}},
  { degree = { 1,0,2,0,3,4,0,5,0,6,7,0} , names = {"modev","mixolydien"}},
  { degree = { 1,0,2,3,0,4,0,5,6,0,7,0} , names = {"modevi","eolien"}},
  { degree = { 1,2,0,3,0,4,5,0,6,0,7,0} , names = {"modevii","locrien"}},
  -- mineur melodique ascendant (mm)
  { degree = { 1,0,2,3,0,4,0,5,0,6,0,7} , names = {"modemi","minor melodic" ,"mineur melodique"}},
  { degree = { 1,2,0,3,0,4,0,5,0,6,7,0} , names = {"modemii","dorien b9"}},
  { degree = { 1,0,2,0,3,0,4,0,5,6,0,7} , names = {"modemiii","lydien augmented","lydien augmente"}},
  { degree = { 1,0,2,0,3,0,4,5,0,6,7,0} , names = {"modemiv","lydien b7", "bartok", "vaschaspati"}},
  { degree = { 1,0,2,0,3,4,0,5,6,0,7,0} , names = {"modemv","mixolydien b13"}},
  { degree = { 1,0,2,3,0,4,5,0,6,0,7,0} , names = {"modemvi","locrien #9" , "eolien b5"}},
  { degree = { 1,2,0,3,4,0,5,0,6,0,7,0} , names = {"modemvii","altered","altere", "super locrien"}},
  -- mineur harmonique (mh)
  { degree = { 1,0,2,3,0,4,0,5,6,0,0,7} , names = {"modehi","minor harmonic","mineur harmonique"}},
  { degree = { 1,2,0,3,0,4,5,0,0,6,7,0} , names = {"modehii","locrien naturel","locrien natural"}},
  { degree = { 1,0,2,0,3,4,0,0,5,6,0,7} , names = {"modehiii","ionien augmente","ionien augmented"}},
  { degree = { 1,0,2,3,0,0,4,5,0,6,7,0} , names = {"modehiv","dorien #11"}},
  { degree = { 1,2,0,0,3,4,0,5,6,0,7,0} , names = {"modehv","phtygien #3"}},
  { degree = { 1,0,0,2,3,0,4,5,0,6,0,7} , names = {"modehvi","lydien #9"}},
  { degree = { 1,2,0,3,4,0,5,0,6,7,0,0} , names = {"modehvii","diminue diatonique","diminished diatonic"}},
  -- exotic
  { degree = { 1,2,0,3,0,4,0,5,6,0,0,7} , names = {"balkan"}},
  { degree = { 1,2,0,0,3,4,0,5,6,0,7,0} , names = {"andalou"}},
  { degree = { 1,0,2,3,0,0,4,5,6,0,0,7} , names = {"tzigane"}},
  { degree = { 1,2,0,0,3,4,0,5,6,0,0,7} , names = {"oriental"}},
  --specific
  { degree = { 1,0,2,3,0,4,5,0,6,7,0,8} , names = {"diminue","diminished", "tondemiton", "tonehalftone", "bertha"}},
  { degree = { 1,2,0,3,4,0,5,6,0,7,8,0} , names = {"demitonton","halftonetone"}},
  { degree = { 1,0,2,0,3,0,4,0,5,0,6,0} , names = {"ton" , "tone"}}
}
local modeMaster = false -- the mode isChord used "as-it-isChord" ( true ! ) , or can be altered by the chord ( false | )
local modeRemanent = true -- the mode isChord used for the chord and the next chords ( true ), or for this chord only ( false )
local modeDefault = modes[1] -- the default mode to use
local modeCurrent = modeDefault -- the current mode to use
local modeCurrentRoot = 1 
local currentTone = 1
local centerPitch = 1 -- pitch of the middle of the keyboard
local centerOctave = 4 -- octave for the center
local blackScale = true -- if true then black key tries to be part of scale, else black is chormatic approach only
local prevBass = nill 

--================
function E.ptos(x)
--================
  -- return the english notation of pitch x (pitch+ rolechord*128)
  if x == nil then
    return "?"
  end
  d = math.floor(x/128)
  p = x % 128
  o = math.floor(p/12)
  i = ( p % 12 ) + 1
  local sp = { "C" ,"Db" , "D" , "Eb" , "E" , "F", "F#" , "G" , "G#" , "A", "Bb" , "B" }
  local s
  if o > 0  then
    s = sp[i] .. tostring(o)
  else
    s = sp[i]
  end
  if d > 0  then
    s = s .. "." .. tostring(d) 
  end
  return s
end

--========================
function E.dumpPitch(t,name)
--========================
  -- print the list of pitches in table t, for name ( form chord, scale, penta ) 
  local l = t[name]
  local spres = name .. ": "
  for i = -7 , 7 do
    if l[i] then
      spres = spres .. i.."=(" .. E.ptos(l[i][-1][1]) .. ")"
      for j = 1 , #l[i][0] do
        if j == 2 then
          spres = spres .. "+"
        end
        if l[i][0][j] then
          spres =  spres .. E.ptos(l[i][0][j])
        end
      end
      if i == -1 then
        spres = spres .. " <"
      elseif i == 0 then
        spres = spres .. "> "
      else
        spres = spres .. " "
      end
    end
  end
  print ( spres )
end

--====================================
function degreeToPitch( root , j , o )
--====================================
  -- return the pitch, according to degree J and octave o
  if j == nil then return nil end
  p = (( j - 1 ) + ( root - 1 ) + ( currentTone - 1 ) + 12 * (o or 4 ))
  return p
end

--======================================
function E.stringToDegree(ipitch, iroot)
--======================================
  
  -- convert degree-name ( in lowercase ) to chromatic-range, degree-range, and alteration
  -- root is the root of the chromatic-range 1..12
  -- name can be 
  --     a b c d e f g
  --     do re mi fa sol la si
  --     i ii iii iv v vi vii ( root is irrelevant here )
  --     1 2 3 4 5 6 7 8 9 10 11 12 13  ( root is irrelevant here )
  -- prefix or sufix can be # or b
  -- exception : - == third minor , 7 == 7th minor , m7 ==lower(M7) == 7th major
  -- return : chromatic-range 1..12, string after the pitch-processed,  degree-range 1..7 , alteration -1..1 , octavia
  if ipitch == nil then return end
  
  local root = ( iroot or 1 ) - 1
  local p = nil
  local d = nil
  local spitch 
  local salteration 
  local rstring 
  local alteration = 0 
  if string.len(ipitch) > 2 then
    -- 3 letters
    spitch = string.sub(ipitch,1,3)
    salteration = string.sub(ipitch,4,4)
    if salteration == "#" then 
      alteration = 1 
      rstring = string.sub(ipitch,5)
    elseif salteration == "b" then 
      rstring = string.sub(ipitch,5)
      alteration = -1 
    else
      spitch = string.sub(ipitch,2,4)
      salteration = string.sub(ipitch,1,1)
      if salteration == "#" then 
        alteration = 1 
        rstring = string.sub(ipitch,5)
      elseif salteration == "b" then 
        rstring = string.sub(ipitch,5)
        alteration = -1 
      else
        spitch = string.sub(ipitch,1,3)
        rstring = string.sub(ipitch,4)
      end
    end
    if spitch == "sol" then
      p = 8 - root
     elseif spitch == "iii" then
      p = 5
      d = 3
    elseif spitch == "vii" then
      p = 12
      d = 7
    end
  end
  if p == nil and string.len(ipitch) > 1 then
    -- 2 letters
    spitch = string.sub(ipitch,1,2)
    salteration = string.sub(ipitch,3,3)
    if salteration == "#" then 
      alteration = 1 
      rstring = string.sub(ipitch,4)
    elseif salteration == "b" then 
      alteration = -1 
      rstring = string.sub(ipitch,4)
    else
      spitch = string.sub(ipitch,2,3)
      salteration = string.sub(ipitch,1,1)
      if salteration == "#" then 
        alteration = 1 
        rstring = string.sub(ipitch,4)
      elseif salteration == "b" then 
        alteration = -1 
        rstring = string.sub(ipitch,4)
      else
        spitch = string.sub(ipitch,1,2)
        rstring = string.sub(ipitch,3)
      end
    end
    if spitch == "do" then
      p = 1 - root
    elseif spitch == "re"  then
      p = 3 - root
    elseif spitch == "ii" then
      p = 3
      d = 2
    elseif spitch == "mi" then
      p = 5 - root
    elseif spitch == "10" then
      p = 5
      d = 3
    elseif spitch == "fa"  then
      p = 6 - root
    elseif spitch == "11"  or spitch == "iv" then
      p = 6
      d = 4
    elseif spitch == "12" then
      p = 8
      d = 5
    elseif spitch == "la" then
      p = 10 - root
    elseif spitch == "13" or spitch == "vi" then
      p = 10
      d = 6
     elseif spitch == "si" then
      p = 12 - root
     elseif spitch == "m7" then
      p = 12
      d = 7
      rstring = string.sub(ipitch,3)
    end
  end
  if p == nil then
    -- 1 letter
    if ipitch == "b" then
      p = 12 - root
      rstring = ""
    else
      spitch = string.sub(ipitch,1,1)
      salteration = string.sub(ipitch,2,2)
      if salteration == "#" then 
        alteration = 1 
        rstring = string.sub(ipitch,3)
      elseif salteration == "b" then 
        alteration = -1 
        rstring = string.sub(ipitch,3)
      else
        spitch = string.sub(ipitch,2,2)
        salteration = string.sub(ipitch,1,1)
        if salteration == "#" then 
          alteration = 1 
          rstring = string.sub(ipitch,3)
        elseif salteration == "b" then 
          alteration = -1 
          rstring = string.sub(ipitch,3)
        else
          spitch = string.sub(ipitch,1,1)
          rstring = string.sub(ipitch,2)
        end
      end
      if spitch == "c" then
        p = 1 - root
      elseif spitch == "1" or spitch == "8"  or spitch == "i" then
        p = 1
        d = 1
      elseif spitch == "d" then
        p = 3 - root
      elseif spitch == "2"  or spitch == "9"  then
        p = 3
        d = 2
      elseif spitch == "e" then
        p = 5 - root
      elseif spitch == "3" then
        p = 5
        d = 3
      elseif spitch == "f" then
        p = 6 - root
      elseif spitch == "4" then
        p = 6
        d = 4
      elseif spitch == "g" then
        p = 8 - root
      elseif spitch == "5" or spitch == "v"  then
        p = 8
        d = 5
      elseif spitch == "a"  then
        p = 10 - root
      elseif spitch == "6" then
        p = 10
        d = 6
      elseif spitch == "b" then
        p = 12 - root
      elseif spitch == "7" then -- exeption 7 isChord by convention 7th minor
        p = 12
        d = 7
        alteration = -1
        rstring = string.sub(ipitch,2)
      elseif spitch == "-" then -- exeption - isChord by convention 3rd minor
        p = 5
        d = 3
        alteration = -1
        rstring = string.sub(ipitch,2)
      end
    end
  end
  if p then
    p = (( p + alteration - 1 ) % 12 ) + 1
    if d == nil then
      local pd = {{1,0},{2,-1},{2,0},{3-1},{3,0},{4,0},{5,-1},{5,0},{5,1},{6,0},{7,-1},{7,0}}
      d= pd[p][1]
      alteration = pd[p][2]
    end
    local soctave , ioctave
    ioctave = 4
    soctave = string.match(rstring,"(%d).*")
    if soctave then
      ioctave = tonumber(soctave)
    end
    return p , rstring , d , alteration , ioctave
  end
  return nil, ipitch , nil , nil
end
  
function E.stringToPitch(s)
 -- return chromatic-range 1..12, string after the pitch-processed,  degree-range 1..7 , alteration -1..1
  local chromatic_range , posts, nilv, vilw = E.stringToDegree(s,1)
  return chromatic_range , posts
end   
--=====================================================
function calculateMode(sMode,degreeRoot,degreeModeRoot)
--=====================================================
  -- set the current mode, with optional degreeRoot
  local modeRank= 0
  local modeFound = false
  for i,mode in pairs(modes) do
    for j,name in pairs(mode.names) do
      if string.find(sMode,name) then
        if string.len(name) > modeRank then
          modeRank = string.len(name)
          modeCurrent = mode
          modeFound = true
          print("caculateMode",sMode,"modeCurrent",modeCurrent.names[1], "modeFound" , modeFound)
        end
      end
    end
  end 
  return modeFound
end

--==================================
function E.setModeDefault(s)
--==================================
  if calculateMode(s) then
    modeDefault = modeCurrent
    return true
  else
    return false
  end 
end

--==================================
function E.setTone(s)
--==================================
  -- calculate the tone
  if s == nil then
    return currentTone
  end
  local tone =  E.stringToDegree(string.lower(s))
  currentTone = tone or currentTone
  return currentTone
end

--==================================
function E.setiTone(i)
--==================================
  -- set the tone
  currentTone = i
end

--=========================
function E.setBlackScale(v)
--=========================
  -- set the compilation environment for black keys : if true, black keys tries to use the scale, if false black keys uses chromatic approach
  blackScale = v
end

function addPitchChord(listChord, t)
  --================================
  -- add chords for each pitch in t
  local ichord0 = -50
  -- search the first chord pitch
  while listChord[ichord0] == nil and ichord0 < 50 do
    ichord0 = ichord0 + 1
  end
  if listChord[ichord0] == nil then return end
  
  for i , v in pairs(t) do
    -- for each item of the scale to complete
    local p = v[0][1]
    local ichord = ichord0
    
    
    while listChord[ichord] ~= nil and ichord < 50 and listChord[ichord][0][1] < ( p + 2 ) do
      -- search for the next chord pitch
      ichord = ichord + 1
    end
    local c = {}
    for j = 1 , 7 do
      if listChord[ichord+j] == nil or listChord[ichord+j][0][1] == nil then
        break
      end
      c[j] = listChord[ichord+j][0][1]
    end
    
    local m = {}
    m[p % 12] = true
    if listChord[ichord] then 
      v[0][2] = listChord[ichord][0][1]
      m[(listChord[ichord][0][1]) % 12 ]=true
      for j = 1 , 10 do
        if listChord[ichord+j] == nil or listChord[ichord+j][0][1] == nil then
          break
        end
        local mj = listChord[ichord+j][0][1] % 12
        if m[mj] == nil then
          v[0][j + 2] = listChord[ichord + j][0][1]
          m[mj] = true
        end
      end
    end
  end
end

function blackKeys(listScale , t )
--================================
  -- calculate black keys of t, according to listScale and blackScale
  local r = {}
  for i , v in pairs(listScale) do
    local k = tonumber(v[0][1])
    if k then
      r[k] = true
    end
  end
  for i , v in pairs(t) do
    v[-1] = {}
    v[1] = {}
    local p = (v[0][1]) % 128
    -- calculate the flat
    local prevP 
    if blackScale == true and t[i - 1] then
      prevP = t[i - 1][0][1]
      for q = p - 1 , prevP + 1, -1 do
        if r[q] then
          v[-1][1] = q
          break
        end
      end
    end
    if v[-1][1] == nil then
      v[-1][1] = p - 1
    end
    -- calculate the sharp
    local nextP 
    if blackScale == true and t[i+1] then
      nextP = t[i + 1][0][1]
      for q = p + 1, nextP - 1 , 1 do
        if r[q] then
          v[1][1] = q
          break
        end
      end
    end
    if v[1][1] == nil then
      v[1][1] = p + 1
    end
  end
end
  
--==========================================
function addChordNotsorted(lroot,chord,pitchRole,j,o)
--==========================================
  local sj = { 6,4,2,7,3,1,5}
  local oj = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}
  local p = degreeToPitch(lroot,j,o)
  -- root j,o is obliged
  chord[1] = p 
  oj[j] = o
  local iv = 1
  local v
  local firstLoop = true
  for nb = 2 , 14 do -- loop a long time ... 14 should be enough ?
    v = sj[iv] -- scan degrees in priority sj
    iv = iv + 1
    if iv > #sj then
      firstLoop = false
      iv = 1
    end
    if v ~= pitchRole[j].chord or firstLoop == false then -- original degree is ignored in first scan
      for k = 1 , 12 do
        if pitchRole[k].chord == v then -- this degree is in the chord
          local o = 0 
          if oj[k] == -1 then
            while degreeToPitch(lroot,k,o) < ( p + 2 ) do
              o = o + 1 -- pitch must be higher then the root
            end
          else
            o = oj[k] + 1 -- pitch already include, keep the next octave
          end
          local v = 0
          local t = degreeToPitch(lroot,k,o)
          for l,q in ipairs(chord) do
            if math.abs( t - q ) < 3 then -- one more pitch closer to this one ...
              v = v + 1
            end
            if math.abs( t - q ) < 2 then -- one more pitch closer to this one ...
              v = v + 2
            end
          end
          if v > 1 then -- too much closed to other pitches in the chord : keep the next octave
            o = o + 1
          end
          oj[k] = o
          table.insert(chord,  degreeToPitch(lroot,k,o))
        end
      end
    end
  end
end
--==========================================
function addChord(lroot,chord,pitchRole,j,o)
--==========================================
  -- add pitches degree j octave o, with additional pitches of the chord
  chord[0] = {} -- white keys
  local cwhite = chord[0]
  addChordNotsorted(lroot,cwhite,pitchRole,j,o)
  -- sort the chord and keep 5 pitches with different roles
  table.sort(cwhite)
  while #cwhite > 5 do
    local somethingUseful = false
    for i = 6 , #cwhite do
      local itAlreadyExists = false
      for j = 2 , 5 do
        if ( cwhite[i] % 12 ) ==  ( cwhite[j] % 12 ) then
          itAlreadyExists = true
        end
       end
       if itAlreadyExists == false then
         somethingUseful = true
         break
        end
    end
    local found = false
    if somethingUseful == true then
      for i = 5 , 3, -1 do
        for j = (i - 1 ) ,1 , -1 do
          if ( found == false )  and (( cwhite[i] % 12 ) ==  ( cwhite[j] % 12 )) then
            table.remove(cwhite,i)
            found = true
          end
        end
      end
    end
    if found == false then
      table.remove(cwhite)
    end
  end
  --
  --if pitchRole[j].chord then
  --  cwhite[1] = cwhite[1] + (pitchRole[j].chord) * 128
  -- end
end


function closestPitch(pivot,lroot,degree, o0)
--===========================================
  -- return the closest pitch to pivot, from degree/lroot
  local dmin = 999
  local d
  local p 
  local p1 
  for o = ( o0 - 1 ) , ( o0 + 1 ) do
    p = degreeToPitch(lroot , degree , o )
    d = math.abs( p - pivot )
    if d < dmin then
      p1 = p
      dmin = d
    end
  end
  return p1
end

--==============================
function calculateWalkingBass(t)
--==============================
  -- return the table with 4 pitches, which make an optimal walking bass, according to the possible pitches in t[5][*]
  local p5 = t[5][1]
  local dmin = 9999
  local res = { t[1][1] , t[1][1] , t[1][1] , t[1][1] }
  for i1,p1 in ipairs(t[1]) do
    for i2,p2 in ipairs(t[2]) do
      for i3,p3 in ipairs(t[3]) do
        for i4,p4 in ipairs(t[4]) do
          if ( p1 ~= p2 ) and ( p2 ~= p3 ) and ( p3 ~= p4 ) then
            d = math.abs(p1-p2) + math.abs(p2-p3) + math.abs(p3-p4) + math.abs(p4-p5)
            if d < dmin then
              dmin = d
              res[1] = p1
              res[2] = p2
              res[3] = p3
              res[4] = p4              
            end
          end
        end
     end
    end
  end
  return res
end

--======================================================================
function fillBass(lroot, currentTone , listBass , pitchRole , bass , nextBass)
--======================================================================
  
  -- bass is 4 pitches to drive the bass up to next bass
  local bassPitch = {}
  for i = 1 , 5 do
    bassPitch[i] = {}
  end
  
  local pivot = degreeToPitch(currentTone , 1 , 2 )
  
  -- calculate the first bass
  startPitch = closestPitch(pivot, lroot, bass, 2)
  local firstBass = bass
  --[[
	if prevBass and prevBass == startPitch then
    for j,v in pairs(pitchRole) do
      if v.chord and v.chord == 5 then
         firstBass = j -- first bass is the fith when bass is repeated
      end
    end
  end
  if firstBass == nil then
    firstBass = bass -- first bass is the bass when bass is not repeated
  end
  ]]--
  bassPitch[1][1] = closestPitch(pivot, lroot, firstBass, 2)
  prevBass = bassPitch[1][1] 
  
  -- calculate the target after the fourth pitch
  local targetPitch
  local fourthFixed = false
  if nextBass then
    targetPitch = closestPitch(pivot, lroot, nextBass, 2)
    if targetPitch == bassPitch[1][1] then
      targetPitch = closestPitch(pivot, lroot, nextBass + 7, 2) -- target bass is the fith when bass is repeated
    end
  else
    targetPitch = bassPitch[1][1]
    bassPitch[4][1] = bassPitch[1][1] -- when no next bass, target bass it the bass itself 
    fourthFixed = true
  end
  bassPitch[5][1] = targetPitch
  
  -- calculate the pitches available for the 2nd, 3rd and 4th bass values, from chord values
  local nb = 1
  for j,v in pairs(pitchRole) do
    if v.chord then
      local p = closestPitch(pivot, lroot, j, 2)
      for i = 2 , 4 do
        if ( i ~= 4 ) or ( fourthFixed == false ) then
          bassPitch[i][nb]  = p
        end
      end
      nb = nb + 1
    end
  end
  if fourthFixed == false then
    -- add chromatic approch for the fourth pitch
    bassPitch[4][nb] = bassPitch[5][1] + 1
    nb = nb + 1
    bassPitch[4][nb] = bassPitch[5][1] - 1
  end
  
  local res = calculateWalkingBass(bassPitch)
  for i,v in ipairs(res) do
    listBass[i] = {}
    listBass[i][0] = {}
    listBass[i][0][1] = v
  end  
end

--========================================
function fillScale(lroot,pitchRole , name)
--========================================

  local listScale = interpretedChord.pitch[name]
  local j0 , o0 , j , o 
  local dmax  , d
  local p, c  
  -- center the scale arount the centerPitch
  c = degreeToPitch( 1 ,centerPitch , centerOctave )
  dmax = 99
  d = 99
  j = 1
  o = 2
  repeat
    if pitchRole[j][name] then
      p = degreeToPitch( lroot ,j , o )
      d = math.abs( p - c)
      if d < dmax then
        dmax = d
        j0 = j
        o0 = o
      end
    end
    j = j + 1
    if j == 12 then
      j = 1
      o = o + 1
    end
  until d > dmax
  -- fill scale upper the center
  o = o0
  j = j0
  p = degreeToPitch( lroot , j , o )
  local iscale = 0
  while p <= 127 do
    listScale[iscale] = {}
    addChord(lroot,listScale[iscale],pitchRole,j,o)
    iscale = iscale + 1
    repeat
      j = j + 1
      if j > 12 then 
        j = 1 
        o = o + 1
      end
    until pitchRole[j][name]
    p = degreeToPitch( lroot ,j , o )
  end
  -- fill scale lower the center
  iscale = -1
  j = j0
  o = o0
  repeat
    j = j - 1
    if j == 0 then 
      j = 12 
      o = o - 1
    end
  until pitchRole[j][name]
  p = degreeToPitch( lroot , j , o )
  while p >= 0 do
    listScale[iscale] = {}
    addChord(lroot,listScale[iscale],pitchRole,j,o)
    iscale = iscale - 1
    repeat
      j = j - 1
      if j == 0 then 
        j = 12 
        o = o - 1
      end
    until pitchRole[j][name]
    p = degreeToPitch(lroot , j , o )
  end
end
--===============================================
function  fillPitches(interpretedChord,pitchRole)
--===============================================
  
  interpretedChord.pitch.bass = {}
  interpretedChord.pitch.chord = {}
  interpretedChord.pitch.scale = {}
  interpretedChord.pitch.penta = {}
  local listBass = interpretedChord.pitch.bass
  local listChord = interpretedChord.pitch.chord
  local listScale = interpretedChord.pitch.scale
  local listPenta = interpretedChord.pitch.penta
  
  local lroot = interpretedChord.root
  
  -- fill the chord
  fillScale(lroot,pitchRole,"chord")
  fillScale(lroot,pitchRole,"scale")
  fillScale(lroot,pitchRole,"penta")
 
  -- fill the bass
  fillBass(lroot, currentTone, listBass, pitchRole , interpretedChord.bass , interpretedChord.nextBass)
  
  -- calculate black-keys
  blackKeys(listScale , listBass )
  blackKeys(listScale , listChord )
  blackKeys(listScale , listPenta )
  blackKeys(listScale , listScale )
  
end
--=============================================
function E.stringToChord(isChord , isNextChord)
--=============================================
  
  -- interpret the string <isChord> which contains a chord description ( e.g. : C G7/D E(#4) F[balkan] G@D ) 
  -- fill the <pitchRoles> array with the decrypted information
  -- parameter==nil : empty the <pitchRoles>
  
  interpretedChord = {}
  interpretedChord.pitchRole = {}
  local pitchRole = interpretedChord.pitchRole
  for i = 1 , 12 do
    pitchRole[i] = {}
  end
  interpretedChord.text = isChord
  
  local degreeInChord = {} -- pitch( I...VII )/degrees(1..12) already described by the chord
  for i = 1 , 7 do
    degreeInChord[i] = 0
  end
  
  local sChord = string.lower(isChord)
  local rsChord = sChord
  
  -- analyse special meeanings
  ----------------------------
  
  local scenter -- center of the keyboard after the @ ( e.g. @G5 )
  local stone -- tone after the = ( e.g. =G )
  local sluafile -- lua file to execute ( e.g. $send_my_tuning_to_midi.lua )
  scenter = string.match(rsChord,".*@([%w#-]+).*")
  if scenter then
    rsChord = string.gsub(rsChord,"@[%w#-]+","")
    p , _s , _d , _a , o = E.stringToDegree(scenter , 1)
    if p then
      centerPitch = p
    end
    if o then
      centerOctave = o
    end
  end
  stone = string.match(rsChord,".*=([%w#-]+).*")
  if stone then
    rsChord = string.gsub(rsChord,"=[%w#-]+","")
    E.setTone(stone)
  end
  sluafile = string.match(rsChord,".*$(%w+%.lua).*")
  if sluafile then
    rsChord = string.gsub(rsChord,"%w+%.lua","")
    interpretedChord.luafile = sluafile
  end
  
  if string.sub(rsChord,1,2) == "nc" then -- No Chord
    interpretedChord.noChord = true
    return interpretedChord
  end
  
  if string.sub(rsChord,1,1) == "%" then -- Same chord
    interpretedChord.sameChord = true
    return interpretedChord
  end
  
  if string.match(rsChord,"%g+") == nil then 
    return interpretedChord 
  end

  -- extract the differents parts of the chord
  --------------------------------------------
  
  local root -- root of the chord at the beginning of the string ( e.g. G in "G7" )
  local smodifier -- modifier of the chord ( e.g. 7 in "G7" )
  local degreeRoot -- degree of the root in the tone ( e.g. V for "G7" in C )
  local sbass -- bass of the chord after the / ( e.g D in "G/D" )
  local sscale -- scale modifier between paranthesis ( e.g. #4 in "G(#4)" )
  local smode -- mode between brackets ( e.g. balkan in "G[Balkan]" )
  local smodeRoot -- root of the mode after the / ( e.g. C in "G[Balkan/C]" )
  local snextBbass , inextBass -- next bass and its degree ( e.g. E in "G" "E" )
  
   -- mode between brackets
  smode = string.match(rsChord,".*(%b[]).*")
  if smode then
    rsChord = string.gsub(rsChord,"%b[]","")
    smode = string.sub(smode,2,-2)
    -- mode root after the /
    smodeRoot = string.match(smode,".*/([%w#-]+).*")
    if smodeRoot then
      smode = string.gsub(smode,"/[%w#-]+","")
    end
  end
  -- degree for the scale, between paranthesis 
  sscale = string.match(rsChord,".*(%b()).*")
  if sscale then
    sscale = string.sub(sscale,2,-2)
    rsChord = string.gsub(rsChord,"%b()","")
  end
  -- bass after the /
  sbass = string.match(rsChord,".*/([%w#-]+).*")
  if sbass then
    rsChord = string.gsub(rsChord,"/[%w#-]+","")
  end
  -- next bass 
  if isNextChord then
    local snextChord = string.lower(isNextChord)
    -- after the / of the next chord
    snextBass = string.match(snextChord,".*/([%w#-]+).*")
    if snextBass then
      inextBass = E.stringToDegree(snextBass , currentTone)
    else
      -- root of the next chord
      inextBass = E.stringToDegree(snextChord , currentTone)
    end
  end
  
  -- iterate each word separated by a dot
  local firstWord = true 
  for wordchord in string.gmatch(rsChord, "[#%+%-%w]+") do
    if firstWord == true then
	    firstWord = false
      -- calculate the root with the first word
      -----------------------------------------
	  -- luabass.logmsg("wordchord " .. wordchord .. " " .. currentTone)
      root , smodifier , degreeRoot = E.stringToDegree(wordchord , currentTone)
      if root == nil then
        return nil
      end
      interpretedChord.root = root
      degreeInChord[1] = 1
    else
      smodifier = wordchord
    end
    -- calculate modifiers of the chord ( e.g. "7" in G7, or #5 in G.#5 )
    -------------------------------------------------------
    
    if string.find(smodifier,"o") then -- diminue mb56
      degreeInChord[3] = 4
      degreeInChord[5] = 7
      degreeInChord[7] = 10
    elseif string.find(smodifier,"0") then  -- semi-diminue mb57   
      degreeInChord[3] = 4
      degreeInChord[5] = 7
      degreeInChord[7] = 11
    else
      -- tierce
      if string.find(smodifier,"sus2") then
        degreeInChord[3] = 3
      elseif string.find(smodifier,"sus[4]*") then
        degreeInChord[3] = 6
      elseif string.find(smodifier,"[-m]*") and string.find(isChord,"[-m]") then -- m is case sensitive
        degreeInChord[3] = 4
      --elseif string.find(smodifier,"m[^7]*") and string.find(isChord,"m[^7]*") then -- m is case sensitive
      --  degreeInChord[3] = 5
      end   
      -- quinte
      if string.find(smodifier,"b5") then
        degreeInChord[5] = 7
      elseif string.find(smodifier,"#5") then
        degreeInChord[5] = 9
      elseif string.find(smodifier,"5") then
        degreeInChord[5] = 8
      end
      --sixte
      if string.find(smodifier,"64") or string.find(smodifier,"46")  then
        degreeInChord[4] = 6
        degreeInChord[6] = 10
      elseif string.find(smodifier,"6") then
        degreeInChord[6] = 10
      end
      --septieme
      if (string.find(smodifier,"maj7")  and string.find(isChord,"MAJ7")) or ( string.find(smodifier,"m7") and string.find(isChord,"M7")) then
        degreeInChord[7] = 12
      elseif string.find(smodifier,"7") then
        degreeInChord[7] = 11
      end
    end
    --extension 9
    if string.find(smodifier,"a[d]+9") then
      degreeInChord[2] = 3
    elseif string.find(smodifier,"a[d]+#9") then
      degreeInChord[2] = 4
    elseif string.find(smodifier,"a[d]+b9") then
      degreeInChord[2] = 2
    elseif string.find(smodifier,"9") then
      if degreeInChord[7] == 0 then degreeInChord[7] = 11 end
      degreeInChord[2] = 3
    elseif string.find(smodifier,"#9") then
       if degreeInChord[7] == 0 then degreeInChord[7] = 11 end
      degreeInChord[2] = 4
    elseif string.find(smodifier,"b9") then
      if degreeInChord[7] == 0 then degreeInChord[7] = 11 end
      degreeInChord[2] = 2
    end
    --extension 11
    if string.find(smodifier,"a[d]+11") then
      degreeInChord[4] = 6
    elseif string.find(smodifier,"a[d]+#11") then
      degreeInChord[4] = 7
    elseif string.find(smodifier,"a[d]+b11") then
      degreeInChord[4] = 5
    elseif string.find(smodifier,"11") then
      if degreeInChord[7] == 0 then degreeInChord[7] = 11 end
      if degreeInChord[2] == 0 then degreeInChord[2] = 3  end
      degreeInChord[4] = 6
    elseif string.find(smodifier,"#11") then
      if degreeInChord[7] == 0 then degreeInChord[7] = 11 end
      if degreeInChord[2] == 0 then degreeInChord[2] = 3 end
      degreeInChord[4] = 7
    elseif string.find(smodifier,"b11") then
      if degreeInChord[7] == 0 then degreeInChord[7] = 11 end
      if degreeInChord[2] == 0 then degreeInChord[2] = 3  end
      degreeInChord[4] = 5
    end
      --extension 13
    if string.find(smodifier,"a[d]+13") then
      degreeInChord[6] = 10
    elseif string.find(smodifier,"a[d]+#13") then
      degreeInChord[6] = 11
    elseif string.find(smodifier,"a[d]+b13") then
      degreeInChord[6] = 9
    elseif string.find(smodifier,"13") then
      if degreeInChord[7] == 0 then degreeInChord[7] = 11 end
      if degreeInChord[2] == 0 then degreeInChord[2] = 3  end
      if degreeInChord[4] == 0 then degreeInChord[4] = 6  end
      degreeInChord[6] = 10
    elseif string.find(smodifier,"#13") then
      if degreeInChord[7] == 0 then degreeInChord[7] = 11 end
      if degreeInChord[2] == 0 then degreeInChord[2] = 3  end
      if degreeInChord[4] == 0 then degreeInChord[4] = 6  end
      degreeInChord[6] = 11
    elseif string.find(smodifier,"b13") then
      if degreeInChord[7] == 0 then degreeInChord[7] = 11 end
      if degreeInChord[2] == 0 then degreeInChord[2] = 3  end
      if degreeInChord[4] == 0 then degreeInChord[4] = 6  end
      degreeInChord[6] = 9
    end
  end

  
  -- calculate the scale modifiers. e.g. "#4" in G(#4) 
  ----------------------------------------------------
  
  if sscale then
    -- scale modifier, degree by degree
    while sscale and string.len(sscale) > 0 do
      local p , ss , d = E.stringToDegree(sscale,currentTone)
      if p then
        degreeInChord[d] = p * -1
      end
      sscale = ss
    end
  end
  
  -- calculate the bass. e.g. "E" in C/E
  --------------------------------------
  
  local ibass = E.stringToDegree(sbass , root)
  if ibass then
    interpretedChord.bass = ibass
  else
    interpretedChord.bass = 1 -- default bass = root
  end
  if inextBass then
    interpretedChord.nextBass = inextBass
  else
    interpretedChord.nextBass = nil
  end
    
  -- calculate the mode. e.g. "balkan" in C[balkan]
  -------------------------------------------------
  
  local degreeModeRoot
  print("degreeModeRoot")
  _p , _s , degreeModeRoot = E.stringToDegree(smodeRoot , currentTone)
  if degreeModeRoot then
    modeCurrentRoot = degreeModeRoot
  end
  local modeFound = false
  if smode then
    -- master and remanence of the mode in the prefix : !|_. 
    modeFound = calculateMode(smode,degreeRoot,degreeModeRoot)
    local prefixMode = string.sub(smode,1,2)
    if string.find(prefixMode,"!") then
      modeMaster = true
    elseif string.find(prefixMode,"|") then
      modeMaster = false
    end
    if string.find(prefixMode,"_") then
      modeRemanent = true
      print("modeRemanent")
    elseif string.find(prefixMode,"%.") then
      modeRemanent = false
    end
    print("smode=",smode,"modeFound",modeFound,"modeRemanent",modeRemanent,"modeCurrent",modeCurrent.names[1])
  end
  if modeFound == false and modeRemanent == false then
    print("mode default", "modeFound", modeFound, "modeRemanent", modeRemanent)
    modeCurrent = modeDefault
    modeCurrentRoot = 1
  end
  print("modeCurrent=" , modeCurrent.names[1] )
  -- calculate the degrees of the current mode
  local modeDegree = {}
  local diffDegree = ( degreeRoot or 1 ) - ( modeCurrentRoot or 1 )
  local p0 = 1
  for p , d in pairs(modeCurrent.degree) do -- p=1..12
    if d == ( diffDegree + 1 ) then
      p0 = p
      break
    end
  end
  for p = 1 , 12 do 
    if modeCurrent.degree[p0] == 0 then
      modeDegree[p] = 0
    else
      local nd = modeCurrent.degree[p0] - diffDegree
      if nd <= 0 then
        nd = nd + 7
      elseif nd > 7 then
        nd = nd - 7
      end
      modeDegree[p] = nd
    end
    p0 = p0 + 1
    if p0 > 12 then p0 = 1 end
  end

  --complete the chord with default third and fith, according to mode if imposed with a !
  ---------------------------------------------------------------------------------------
  
  if modeMaster then
    -- use the mode to calculate default third and fith in the chord
    for p,d in pairs(modeDegree) do -- p=1..12
      if d == 3 then
        degreeInChord[3] = p
      elseif d == 5 then
        degreeInChord[5] = p
      end
    end
  else
    -- complete the chord with default 3rd
    if degreeInChord[3] == 0 then
      degreeInChord[3] = 5
    end
    -- complete the chord with default 5th
    if degreeInChord[5] == 0 then
      degreeInChord[5] = 8
    end
  end
  
  -- calculate the pitches of the chord
  -------------------------------------
  
  for d = 1 , 7 do
    if degreeInChord[d] > 0 then
      pitchRole[degreeInChord[d]].chord = d
    end
  end

  -- calculate the scale from the chord and the mode
  --------------------------------------------------
  
  if modeMaster then
    -- use the mode as-it-isChord ( chord information  isChord not used )
    for p,d in pairs(modeDegree) do --p=1..12
      if d ~= 0 then
        pitchRole[p].scale = d
      end
    end
  else
    -- use the mode to complement the definition of the chord
    for degree = 1 , 7 do
      if degreeInChord[degree] ~= 0 then
        if degreeInChord[degree] < 0 then --scale only -12..-1
          pitchRole[(-1) * degreeInChord[degree]].scale = degree
        else --chord and scale 1..12
          pitchRole[degreeInChord[degree]].scale = degree
        end
      else
        for i,dm in pairs(modeDegree) do
          if degree == dm then
            pitchRole[i].scale = degree
            break
          end
        end
      end
    end
  end
  
  -- calculate the penta from the scale
  -------------------------------------
  local prevI = 13
  pitchRole[1].penta = 1
  for i = 12 , 1 , -1 do
    if pitchRole[i].scale then
      if ( prevI - i ) > 1 then
        pitchRole[i].penta = pitchRole[i].scale 
        prevI = i 
      else
        if pitchRole[i].chord then
          if ( i ~= 12 ) then
            pitchRole[i+1].penta = nil
          end
          pitchRole[i].penta = pitchRole[i].scale 
          prevI = i 
        end
      end
    end
  end
  
  
  
  interpretedChord.pitch = {}

  -- calculate the array of pitches
  ---------------------------------
  fillPitches(interpretedChord,pitchRole)
    
  return interpretedChord
end


return E -- to export the functions

--[[
-- test this module

local mchords={"C" , "Cm" , "C.m" ,  "C.#5" , "C7" } -- , "C[!.ionien]","C[.!dorien]","G7","Dm", "C[_!balkan]", "F" }
for i, mchord in ipairs(mchords) do
  local minterpretedChord = E.stringToChord(mchord , nil)
  
  print("mode courant", modeCurrent.names[1] , table.concat(modeCurrent.degree))
  
  local tpitch={}
  local spitch=""
  for i = -10 , 10 , 1 do
    tpitch[i]=minterpretedChord.pitch["chord"][i][0][1]
    spitch = spitch .. " " .. E.ptos(tpitch[i])
  end
  print("chord" , mchord, spitch , table.concat(tpitch,",") )
  print()
  
  tpitch={}
  spitch=""
  for i = -10 , 10 , 1 do
    tpitch[i]=minterpretedChord.pitch["penta"][i][0][1]
    spitch = spitch .. " " .. E.ptos(tpitch[i])
  end
  print("penta" , mchord, spitch , table.concat(tpitch,",") )
  print()

  tpitch={}
  spitch=""
  for i = -10 , 10 , 1 do
    tpitch[i]=minterpretedChord.pitch["scale"][i][0][1]
    spitch = spitch .. " " .. E.ptos(tpitch[i])
  end
  print("scale" , mchord, spitch , table.concat(tpitch,",") )
  print()

  E.dumpPitch(minterpretedChord.pitch,"chord")
  E.dumpPitch(minterpretedChord.pitch,"penta")
  E.dumpPitch(minterpretedChord.pitch,"scale")
  print()

end
]]--
