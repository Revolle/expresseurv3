--[[
Set an improvisation environment from a string which contains a chord description.
Function return the pitches to play according to user's request, for : bass, chord, penta, scale

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
% : same chord then previous
=pitch : set the tone
@pitch : center the keyboard on this pitch
*.lua : LUA script to execute

Example :
C
CM7
G7/D
Fadd11
C(#4)
Sol.Sus4
D[!dorien]
B.b5
Bb.9
@D3
NC
$tuning.lua
=C
%

accord 7 : => (5)
accord 9 : => (1)(3)
accord 11 : => (1)(3)
]]--
local E={} -- to export the functions

local modes = { -- specify the modes up
  -- example for the standard diatonic C scale :
  --           1 0 2 0 3 4 0 5 0 6 0 7
  --           C   D   E F   G   A   B  -- ( in tone C )
  -- if the mode down is different, it contains the up and the down specification
  -- if the mode down is the same as up, no need to specify the down
  -- majeur
  { degree = { 1,0,2,0,3,4,0,5,0,6,0,7} , names = {"modei","ionien"}},
  { degree = { 1,0,2,3,0,4,0,5,0,6,7,0} , names = {"modeii","dorien"}},
  { degree = { 1,2,0,3,0,4,0,5,6,0,7,0} , names = {"modeiii","phrygien"}},
  { degree = { 1,0,2,0,3,0,4,5,0,6,0,7} , names = {"modeiv","lydien"}},
  { degree = { 1,0,2,0,3,4,0,5,0,6,7,0} , names = {"modev","mixolydien"}},
  { degree = { 1,0,2,3,0,4,0,5,6,0,7,0} , names = {"modevi","eolien"}},
  { degree = { 1,2,0,3,0,4,5,0,6,0,7,0} , names = {"modevii","locrien"}},
  -- mineur melodique ascendant (mm)
  { degree = { 1,0,2,3,0,4,0,5,0,6,0,7, 1,0,2,3,0,4,0,5,6,0,7,0} , names = {"modemi","minor melodic" ,"mineur melodique"}},
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
  { degree = { 1,2,0,0,3,4,0,5,6,0,7,0} , names = {"modehv","phrygien #3"}},
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
local interpretedChord = {} -- résultat de l'analyse de l'accord en corurs
interpretedChord.isOk = false
local lgamme = { "bass" , "chord" , "penta" , "scale" }
local context = {} -- contexte du jeu en cours sur le clavier
context.centerPitch = 64 -- pitch of the middle of the keyboard
context.bass = {}
context.chord = {}
context.penta = {}
context.scale = {} -- current index in the scale 
bassOctave = 2
chordOctave = 4

function dump(o, indent)
    indent = indent or ""
    if type(o) == "table" then
        local s = "{\n"
        for k, v in pairs(o) do
            s = s .. indent .. "  [" .. tostring(k) .. "] = " .. dump(v, indent .. "  ") .. ",\n"
        end
        return s .. indent .. "}"
    else
        return tostring(o)
    end
end


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

--=============================
function E.chordtos(c)
--=============================
  local sc = ""
  for i,p in ipairs(c) do
    sc = sc .. "(".. p .. ")" .. E.ptos(p) .. " "
  end
  return sc
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
function E.setBassOctave(o)
--==================================
  bassOctave = o
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
--================================
function E.stringToChord(isChord)
--================================

  -- interpret the string <isChord> which contains a chord description ( e.g. : C G7/D E(#4) F[balkan] G@D )
  -- set the improvisation environment in interpretedChord

  if isChord == "%" then -- Same chord
    return
  end
  if isChord == "nc" then -- No Chord
    interpretedChord = {}
    interpretedChord.isOk = false
    return
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
    if p and o then
      context.centerPitch = p + 12 * o
      for _ , j  in ipairs(lgamme) do
        for i,v in pairs(v[j]) do
          v[j][i].centerPitch = context.centerPitch
        end
      end
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

   if string.match(rsChord,"%g+") == nil then
    return
  end

  interpretedChord = {}
  interpretedChord.isOk = false

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

  -- extract the differents parts of the chord
  --------------------------------------------

  local root -- root of the chord at the beginning of the string ( e.g. G in "G7" )
  local smodifier -- modifier of the chord ( e.g. 7 in "G7" )
  local degreeRoot -- degree of the root in the tone ( e.g. V for "G7" in C )
  local sbass -- bass of the chord after the / ( e.g D in "G/D" )
  local sscale -- scale modifier between paranthesis ( e.g. #4 in "G(#4)" )
  local smode -- mode between brackets ( e.g. balkan in "G[Balkan]" )
  local smodeRoot -- root of the mode after the / ( e.g. C in "G[Balkan/C]" )

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
  
  -- calculate the mode. e.g. "balkan" in C[balkan]
  -------------------------------------------------

  local degreeModeRoot
  -- print("degreeModeRoot")
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

  print("degreeInChord", table.concat(degreeInChord))


  -- calculate the pitches of the chord
  -------------------------------------

  local nbinchord = 0 
  for d = 1 , 7 do
    if degreeInChord[d] > 0 then
      pitchRole[degreeInChord[d]].chord = d
      nbinchord = nbinchord + 1
    end
  end
  -- simplification if more than 4 pitches in chord
  if nbinchord > 5 then
    -- suppress root
    if degreeInChord[1] > 0 then
      pitchRole[degreeInChord[1]].chord = 0
    end
  end
  if nbinchord > 6 then
    -- suppress fith
    if degreeInChord[5] > 0 then
      pitchRole[degreeInChord[5]].chord = 0
    end
  end
  if nbinchord > 7 then
    -- suppress 3rd
    if degreeInChord[3] > 0 then
      pitchRole[degreeInChord[3]].chord = 0
    end
  end
  
  pitchRole[1].root = 1
  
   -- calculate the scale from the chord and the mode
  --------------------------------------------------

  if modeMaster then
    -- use the mode as-it-isChord ( chord information  isChord not used )
    for p,d in pairs(modeDegree) do --p=1..12..24
      if d ~= 0 then
        if p <= 12 then
          pitchRole[p].scaleup = d
        end
        if p > 12 or #modeDegree < 13 then
          pitchRole[p].scaledown = d
        end
      end
    end
  else
    -- use the mode to complement the definition of the chord
    for degree = 1 , 7 do
      if degreeInChord[degree] ~= 0 then
        if degreeInChord[degree] < 0 then --scale only -12..-1
          pitchRole[(-1) * degreeInChord[degree]].scaleup = degree
          pitchRole[(-1) * degreeInChord[degree]].scaledown = degree
        else --chord and scale 1..12
          pitchRole[degreeInChord[degree]].scaleup = degree
          pitchRole[degreeInChord[degree]].scaledown = degree
        end
      else
        for i,dm in pairs(modeDegree) do --p=1..12..24
          if degree == dm then
            if i <= 12 then
              pitchRole[i].scaleup = degree
            end
            if i > 12 or #modeDegree < 13 then
              pitchRole[i].scaledown = degree
            end
          end
        end
      end
    end
  end

  -- calculate the penta from the scale
  -------------------------------------
  local prevI = 13
  pitchRole[1].pentaup = 1
  for i = 12 , 1 , -1 do
    if pitchRole[i].scaledown then
      if ( prevI - i ) > 1 then
        pitchRole[i].pentaup = pitchRole[i].scaleup
        prevI = i
      else
        if pitchRole[i].chord then
          if ( i ~= 12 ) then
            pitchRole[i+1].pentaup = nil
          end
          pitchRole[i].pentaup = pitchRole[i].scaleup
          prevI = i
        end
      end
    end
  end
  prevI = 13
  pitchRole[1].pentadown = 1
  for i = 12 , 1 , -1 do
    if pitchRole[i].scaledown then
      if ( prevI - i ) > 1 then
        pitchRole[i].pentadown = pitchRole[i].scaledown
        prevI = i
      else
        if pitchRole[i].chord then
          if ( i ~= 12 ) then
            pitchRole[i+1].pentadown = nil
          end
          pitchRole[i].pentadown = pitchRole[i].scaledown
          prevI = i
        end
      end
    end
  end

  interpretedChord.isOk = true
end

--=====================================
function E.getChord(open,octave)
--=====================================

-- return the pitches of the chord 
-- if open=true : open position of the chord

  if ( interpretedChord.isOk or false ) == false then
    return nil
  end

  local pitchRole = interpretedChord.pitchRole
  local pchord = {}
  local idone = {}
  
  prevp = degreeToPitch( interpretedChord.bass , 1 , octave )
  idone[interpretedChord.bass] = true
  table.insert(pchord, prevp )
  -- recherche des notes d accord qui suivent
  local dpmin = 2
  if open then
    dpmin = 5
  end
  
  -- count number of degrees in chord
  local nbp = 0
  for ci = 1 , 12 do
    if pitchRole[ci].chord  then
         if interpretedChord.bass ~= ci then
          print("note accord non basse" , ci)
          nbp = nbp + 1
        else
          print("note accord deja basse" , ci)
        end
    end
  end
  print("accord avec " , nbp, "notes (plus la basse)")
  if nbp == 0 then
    return pchord
  end
  
  local i = 0
  local o = octave
  local p = degreeToPitch( interpretedChord.root , 1 , octave ) - 1
  while true do
    i = i + 1
    p = p + 1
    if i > 12 then
      i = 1
      o = o + 1
    end
    if pitchRole[i].chord then
      if (p - prevp ) > dpmin then
        if idone[i] then
          print("ton " .. i .. " deja insere")
        else
          print("ton " .. i .. " a inserer")
          idone[i] = true
          table.insert(pchord,p)
          prevp = p
          nbp = nbp - 1
          if nbp == 0 then
            break
          end
        end
      end
    end
  end
  return pchord
end

--=====================================
function E.nextPitch(up,full,typ , buttonid)
--=====================================

  -- according to the improvisation environment <context> in <interpretedChord> ( calculated by stringToChord )
  -- give next picth to play, according to
  --     * up :true ( down if false )
  --     * full : 3=root , 2=full step , 1=intermediate pitch , 0= chromatic
  --     * typ : chord, penta , scale , bass
  --     * id : id of the requester to keep context, tipycaly the selector-id

  if ( interpretedChord.isOk or false ) == false then
    return nil
  end
  
  if context[buttonid] == nil then
    context[buttonid] = {}
  end
  
  -----------------------------
  local ctx = context[buttonid]
  
  local pitchRole = interpretedChord.pitchRole
  
  -- for the bass, only bass ( or fith )
  if typ == "bass" then
    if full then
      return degreeToPitch( interpretedChord.bass , 1 , bassOctave )
    else
      if degreeInChord[5] > 0 then
        return degreeToPitch( interpretedChord.root , 5 , bassOctave )
      else
        return degreeToPitch( interpretedChord.bass , 1 ,bassOctave )
      end
    end
  end
  
  local i 
  -- récupération du recentrage de pitch si il y a
  if ctx[typ] == nil then
    -- pas de contexte, on récupère le contexte global
    ctx[typ] = {}
    ctx[typ].centerPitch = context.centerPitch
  end
  if (ctx.centerPitch or -1 ) > 1 then
    -- une demande de recentrage a été déposée, on la récupère
    ctx.currentPitch = ctx.centerPitch
    ctx.centerPitch= -1
    return ctx.currentPitch
  end
  
  ctx = ctx[typ]
  --------------
  
  -- rattrapage du pitch si necessaire ( changement d'accord, de tonalite, .. )
  if ctx.pitch == nil then
    ctx.id = 1
    ctx.octave = chordOctave
    ctx.pitch = degreeToPitch( interpretedChord.root , ctx.id , ctx.octave )
  end
  
  local i = ctx.id
  local o = ctx.octave
  local p = ctx.pitch

  if p ~= degreeToPitch( interpretedChord.root , i , i ) then
    while p < degreeToPitch( interpretedChord.root , i , i ) do
      p = p + 1
      i = i + 1
      if i > 12 then
        i = 1
        o = o + 1
      end
    end
    while p > degreeToPitch( interpretedChord.root , i , o ) do
      p = p - 1
      i = i - 1
      if i < 1 then
        i = 12
        o = o - 1
      end
    end
  end
  
  -- differenciation de la gamme montante ou descendante
  local s = typ
  local di = 1 
  if s == "scale" or s == "penta" then
    if up then
      s = s + "up"
    else
      s = s + "down"
      di = -1
    end
  end
  
  -- recherche du degre suivant demande
  local nbstep = 0 
  local r = nil
  while true do
    nbstep = nbstep + 1
    if nbstep > 24 then
      break 
    end
    i = i + di
    p = p + di
    if i < 1 then
      i = 12
      o = o - 1
    end
    if i > 12 then
      i = 1
      o = o + 1
    end
    if pitchRole[i][s] then
      r = pitchRole[i][s]
      break
    end
  end
  
  if r == nil then
    return nil
  end
  ctx.id = i 
  ctx.octave = o 
  ctx.pitch = p
  return p
  
end

-- return E -- to export the functions


-- test this module

local mchords={"C" , "C[!modemi]" , "Cm" , "C.m",  "C.#5" , "C7" , "C.7" } -- , "C[!.ionien]","C[.!dorien]","G7","Dm", "C[_!balkan]", "F" }

print(_VERSION)

for i, mchord in ipairs(mchords) do
  print("set chord",mchord)
  E.stringToChord(mchord)
  -- print("interpretedChord=" , dump(interpretedChord))
  -- print("mode courant", modeCurrent.names[1] , table.concat(modeCurrent.degree))
  pp = E.getChord(false,1)
  print("accord fermé" , E.chordtos(pp))
  pp = E.getChord(true,1)
  print("accord ouvert" , E.chordtos(pp))
end
