--[[
This LUA-module is loaded by default by the bassLUA.
This LUA-module contains functions which are driven by the GUI over bassLUA :
  - setScore
  - addEvent
  - addEventStarts
  - addEventStops
  - addTracks
  - getNrEvent
  
#Interpret a score.
The score.events is a structure.
The events contains events to play, and parts to play sections.
An event contains notes.


]]--

local E={} -- to export the functions

--[[
--for standalone local LUA debugger test  :
local luachord = require("luachord")
local luabass = require("luabassfake") -- to simulate midiout luabass
local trace = "" -- to trace graphically on the output
--]]

-- the score, with the events and the tracks
local score = { events = {} , tracks = {} }

local nb_events = 0 -- == #events
local c_nrEvent_playing = nil -- current index for note playing 
local c_nrEvent_noteOn = 0 -- current index for the next note-on 
local c_nrEvent_noteOff = 0 -- current index for the next note-off
local c_partNr = 1 -- number of the current part
local c_measureNr = 1   -- number of the current measure
local memEvent = 1 -- last position of the preivious move

-- index of a propery in an event of the events
local eStarts = 1 -- eStarts these events 
local eStops = 2 -- eStops these events
local ePlayed = 3
local eVisible = 4
local eTrackNr = 5 
local ePitch = 6
local eVelocity = 7
local eDelay = 8
local eDynamic = 9
local eRandomDelay = 10
local ePedal = 11
local eLua = 12
local eWillStopIndex = 13
local eStopIndex = 14
local ePartNr = 15
local eMeasureNr = 16
local eMeasureLength = 17
local eStartMeasureNr = 18
local eStartT = 19
local eStartOrder = 20
local eStopMeasureNr = 21
local eStopT = 22
local eStopOrder = 23

-- index of a propery in track
local tName = 1  
local tRandomDelay = 2 
local tDynamic = 3
local tPedal = 4

-- temp tables, used to create the events
local currentEventStarts  = {}
local currentEventStops  = {}

local new_measure = false 
local new_part = false 
local end_score = false

-- list of note-off triggers 
local noteOffStops = {}

function applyLua(luastring,TrackNr)
  -- the "Lua" string, set in an ornament, can be interpreted to do what needed here
  -- in this funtion, the syntax is set of words with space-delimiter
  words={}
  for w in string.gmatch(luastring,"%g+") do
    table.insert(words,w)
  end  
  if #words == 0 then return end
  cmd = string.lower(words[1])
  if cmd == "chord" then -- chord Gm
    if #words ~= 2 then return end
    luachord.setChord(words[2])  
    return
  end
  if cmd == "instrument" then -- instrument myGuitar(P43)
    if #words ~= 2 then return end
    luabass.outSetTrackInstrument(words[2],TrackNr)
    return
  end
  if cmd == "tune" then -- tune 415
    if #words ~= 2 then return end
    if tonumber(words[2]) then 
      luabass.outTune(tonumber(words[2]),TrackNr)
    end
    return
  end
  if cmd == "bendrange" then -- bendrange 1
    if #words ~= 2 then return end
    if tonumber(words[2]) then
      luabass.outTune(tonumber(words[2]),TrackNr)
    end
    return
  end
  if cmd == "gm" then
    if #words ~= 2 then return end
    if words[2] == "1" then -- gm 1
      luabass.outSysex("F0 7E 7F 09 01 F7",TrackNr)
      return
    end
    if words[2] == "2" then -- gm 2
      luabass.outSysex("F0 7E 7F 09 03 F7",TrackNr)
      return
    end
    if words[2] == "off" then -- gm off
      luabass.outSysex("F0 7E 7F 09 02 F7",TrackNr)
      return
    end
  end
  if cmd == "scale" then
    if #words < 2 then return end
    if words[2] == "equal" then -- scale equal
      luabass.outSysex("F0 7E 7F 08 08 7F 7F 7F 40 40 40 40 40 40 40 40 40 40 40 40 F7",TrackNr)
      return
    end
    if words[2] == "arabian" then -- scale arabian
      luabass.outSysex("F0 7E 7F 08 08 7F 7F 7F 3A 6D 3E 34 0D 38 6B 3C 6F 40 36 0F F7",TrackNr)
      return
    end
    if #words < 3 then return end
    if words[2] == "just" then -- scale just G
      local root = luachord.stringToPitch(words[3])
      if root == nil or root < 0 or root > 12 then 
        return
      end
      local sti = { "40", "38", "44", "50", "32", "3E", "36", "42", "4E", "30", "4E", "34" }
      local sto = {}
      for i = root, root +11, 1 do
        local j = i % 12
        table.insert(sto,sti[j+1])
      end
      local ss = "F0 7E 7F 08 08 7F 7F 7F " .. table.concat(sto," ") .. " F7"
      luabass.outSysex(ss,TrackNr) 
    end
  end
  if cmd == "test" then -- bendrange 1
     luabass.logmsg(table.concat(words," - "))
    return
  end
end

function playControl(t)
  
  -- local ns = nil
  
  if t[eDynamic] >= 0 then
    local d = t[eDynamic]
    local p = t[eTrackNr]
    if p > 0 then
      score.tracks[p][tDynamic] = d
    else
      for i=1 , #(score.tracks) ,  1 do
        score.tracks[p][tDynamic] = d
      end
    end
    -- ns = "d"
  end
  
  if t[eRandomDelay] > 0 then
    local d = t[eRandomDelay]
    local p = t[eTrackNr]
    if p > 0 then
      score.tracks[p][tRandomDelay] = d
    else
      for i=1 , #(score.tracks) ,  1 do
        score.tracks[i][tRandomDelay] = d
      end
    end
    -- ns = "r"
  end
  
  if t[ePedal] >= 0 then
    local d = t[ePedal]
    local p = t[eTrackNr]
    if p > 0 then
      luabass.outControl(64,0,0,p) -- ePedal off
      score.tracks[p][tPedal] = d
      if ( d > 0 ) then
        luabass.outControl(64,d,200,p) -- ePedal on
      end
    else
      for i=1 , #(score.tracks) ,  1 do
        luabass.outControl(64,0,0,i) -- ePedal off
        score.tracks[i][tPedal] = d
        if ( d > 0 ) then
          luabass.outControl(64,d,200,i) -- ePedal on
        end
      end
    end
    -- ns = "p"
  end
  
  if string.len(t[eLua]) > 0 then
    local d = t[eLua]
    local p = t[eTrackNr]
    applyLua(d,p)
    -- ns = "l"
  end
  
  -- return ns
end

function nextGroupEvent()
  -- adjust the position on the next playable event
  -- set these booleans :
  --    end of events
  --    start of a measure
  --    start of a part
  c_nrEvent_noteOn = c_nrEvent_noteOn + 1
  if c_nrEvent_noteOn < 1 then
    c_nrEvent_noteOn = 1
  end
  if c_nrEvent_noteOn > nb_events then
    c_nrEvent_noteOn = nb_events
  end
  while ( c_nrEvent_noteOn < nb_events ) and  (#(score.events[c_nrEvent_noteOn][eStarts]) ==  0 ) 
  do
    c_nrEvent_noteOn = c_nrEvent_noteOn + 1
  end
  end_score =  ( c_nrEvent_noteOn >= nb_events  ) 
  local t = score.events[c_nrEvent_noteOn]
  if t == nil then 
	end_score= true
	return 
  end
  new_measure = ( t[eMeasureNr] ~= c_measureNr )
  new_part = ( t[ePartNr]  ~= c_partNr )
  c_measureNr = t[eMeasureNr] 
  c_partNr = t[ePartNr] 
end


function stopEvent(t,nrEvent)
  -- stop an event
  local rtr = luabass.outNoteOff(t[ePitch],0,nrEvent,0,t[eTrackNr])
  --luabass.logmsg("noteoff#"..nrEvent.." p="..t[ePitch].." tr="..t[eTrackNr].." ret="..rtr)
end

function playPendingTuning(a, b )
  -- play pending tunings
  if a < 1 then return end
  --if trace then trace = string.rep("|",nb_events) end
  local e = false
  for nrEvent=a, b , 1 do
    local ts = score.events[nrEvent]
    local ns = playControl(ts)
    --if trace and ns then
    --  trace = string.sub(trace,1,nrEvent - 1) .. ns .. string.sub(trace,nrEvent + 1)
    --  e = true
    --end
  end
  --if trace and e then
  --  print("  "..trace)
  --end
end
function playPendingTuningOff()
  -- play pending tunings
  playPendingTuning(c_nrEvent_noteOff, c_nrEvent_noteOn - 1)
  c_nrEvent_noteOff = c_nrEvent_noteOn
end
function playPendingTuningOn(to_stop_index)
  -- play pending tunings
  playPendingTuning(c_nrEvent_noteOff, to_stop_index - 1)
  c_nrEvent_noteOff = to_stop_index
end

function playEvent(velo_in,t , nrEvent,nbEvent)
  -- play an event
  if nbEvent == 0 then return end
  local velo = t[eVelocity]
  local p = t[ePitch]
  local tr = t[eTrackNr]
  
  if p <= 0 or p >= 127 or velo < 2 then return end
  
  local min_velo_out = 1
  local max_velo_out = 128 
  if velo < 64 then
    min_velo_out = 1
    max_velo_out = 2 * velo
  else
    min_velo_out = ( velo - 63 )
    max_velo_out = 128
  end
  -- rescale the velocity-in
  local dyn = score.tracks[tr][tDynamic]
  local min_velo_in = 64 - dyn / 2 
  local max_velo_in = 64 + dyn / 2
  local v_in = min_velo_in + (( max_velo_in - min_velo_in ) * velo_in ) / 128 
  -- rescale the velocity out 
  local velo_out = math.floor(min_velo_out + ((max_velo_out - min_velo_out ) * v_in ) / 128)
  -- compensation of nbEvent note within one single key-in
  local compensation = 15 -- [0..30]
  velo_out = math.floor(((200 - (compensation * (nbEvent - 1))) * velo_out) / 200);
  -- cap the velocity-out
  if velo_out < 1 then velo_out = 1 end
  if velo_out > 127 then velo_out = 127 end
  -- evaluate delay
  local eDelay = t[eDelay] 
  if score.tracks[tr][tRandomDelay] > 0 then 
    eDelay = math.floor(eDelay + math.random (0,score.tracks[tr][tRandomDelay] )) 
  end
  
  local rtr = luabass.outNoteOn(p,velo_out,nrEvent,eDelay,tr)  
  --luabass.logmsg("noteon#"..nrEvent.." p="..p.." v="..velo_out.." d="..eDelay.." tr="..tr.." ret="..rtr)
end

function noteOn_Event(bid , velo_in)
  ------------------------------------
  -- the button-id ( bid ) play the current group , and go to the next one
  
  local t = score.events[c_nrEvent_noteOn]
  
  --if trace then trace = string.rep("|",nb_events) end
  
  local to_stop_index = t[eStopIndex]
  if to_stop_index > 0 then
    local list_stop = score.events[to_stop_index][eStops]
    for nilvalue,nrEvent in ipairs(list_stop) do
      local ts = score.events[nrEvent]
      stopEvent(ts,nrEvent)
      --luabass.logmsg("noteOn_Event bid#"..bid.." to_stop nrEvent#"..nrEvent)
      --if trace then trace = string.sub(trace,1,nrEvent - 1) .. "X" .. string.sub(trace,nrEvent + 1) end
    end
  end
  
  local list_start = t[eStarts]
  local nb_start = #list_start
  for nilValue ,nrEvent in ipairs(list_start) do
    if nrEvent > 0 then
      local ts = score.events[nrEvent]
      playEvent(velo_in , ts , nrEvent,nb_start)
      -- luabass.logmsg("noteOn_Event bid#"..bid.." play nrEvent#"..nrEvent)
      --if trace then trace = string.sub(trace,1,nrEvent - 1) .. "O" .. string.sub(trace,nrEvent + 1) end
    end
  end
  
  -- reminder for notes to stop on note-off
  to_stop_index = t[eWillStopIndex]
  noteOffStops[bid] = to_stop_index
 
  --if trace then print("o " .. trace) end

  -- apply pending tunings
  playPendingTuningOn(to_stop_index)
  c_nrEvent_noteOff = to_stop_index
   
  -- goto nex event to play for the next noteOn_Event
  nextGroupEvent()
end

function noteOff_Event(bid)
-----------------------------
  
  -- stop the group of event started with the same button-id ( bid )
  --if trace then trace = string.rep("|",nb_events) end
  local to_stop_index = noteOffStops[bid]
  if c_nrEvent_playing and c_nrEvent_playing[1] == bid then
      c_nrEvent_playing =  nil
  end
  if to_stop_index and to_stop_index > 0 and score.events[to_stop_index] then
    local list_stop = score.events[to_stop_index][eStops]
    for nilvalue,nrEvent in ipairs(list_stop) do
      if score.events[nrEvent] then
        local ts = score.events[nrEvent]
        stopEvent(ts,nrEvent)
        --luabass.logmsg("noteOff_Event bid#"..bid.." nrEvent#"..nrEvent)
        --if trace then trace = string.sub(trace,1,nrEvent - 1) .. "X" .. string.sub(trace,nrEvent + 1) end
      end
    end
  end
  
  --if trace then print ("x "..trace) end

  -- apply pending tunings
  playPendingTuningOff()
  
end

function noteOff_Event_All()
-----------------------------
  
  -- stop all the group of events
  for nrBid,nilValue in pairs(noteOffStops) do
	noteOff_Event(nrBid)
  end
 end

function resetPendingTuning()
  c_nrEvent_noteOff = 1
  playPendingTuningOff()
end
function memPos()
  memEvent = c_nrEvent_noteOn
  collectgarbage("restart")
end

function E.play( t, bid, ch, typemsg, pitch, velo , param )
  --luabass.logmsg("play score bid="..bid.." velo="..velo.." param="..param)
  collectgarbage("stop")
  if nb_events == 0 then return end
  if (param or "") == "legato" then
    -- always legato, noteOn up to next noteOn
    if velo ~= 0 then
	    luabass.logmsg("play score legato")
	    noteOff_Event(12532)
	    c_nrEvent_playing = { 12532 , c_nrEvent_noteOn }
	    noteOn_Event(12532 , velo)
    end
  else
	  if velo == 0 then
	    -- noteoff
	    luabass.logmsg("play score noteoff#"..bid)
	    noteOff_Event(bid)
	  else
	    -- noteon
	    c_nrEvent_playing = { bid , c_nrEvent_noteOn }
	    luabass.logmsg("lay score noteon#"..bid.." "..c_nrEvent_noteOn)
	    noteOn_Event(bid , velo)
	  end
  end
end

function E.previousPos()
  -- move to the previsous pos_move 
  noteOff_Event_All()
  local m = memEvent
  -- E.firstPart()
  c_nrEvent_noteOn = m - 1
  nextGroupEvent()
  resetPendingTuning()

end
function E.firstPart()
  -- move to the beginning of the tune 
  noteOff_Event_All()
  nb_events = #(score.events)
  c_nrEvent_noteOn = 0 -- current index for the next note-on 
  c_nrEvent_noteOff = 1 -- current index for the next note-off
  c_partNr = 1 -- number of the current parter
  c_measureNr = 1   -- number of the current measure
  
  nextGroupEvent()
  resetPendingTuning()
  memPos()
end
function E.nextEvent()
  -- move to the next event
  noteOff_Event_All()
  nextGroupEvent()
  playPendingTuningOff()
  memPos()
end
function E.nextMeasure()
  -- move to the next part
  noteOff_Event_All()
  local nr = -1
  local j = 1
  for i,t in ipairs(score.events) do
    if t[eMeasureNr]  ~= nr then
      nr = t[eMeasureNr]
      j = i
      if nr == ( c_measureNr + 1 ) then
        break
      end
    end
  end
  E.gotoNrEvent(j)
  memPos()
end
function E.nextPart()
  -- move to the next part
  noteOff_Event_All()
  local nr = -1
  local j = 1
  for i,t in ipairs(score.events) do
    if t[ePartNr]  ~= nr then
      nr = t[ePartNr]
      j = i
      if nr == ( c_partNr + 1 ) then
        break
      end
    end
  end
  E.gotoNrEvent(j)
  memPos()
end
function E.lastPart()
  -- move to the last part
  noteOff_Event_All()
  local nr = -1
  local j = 1
  local pj = 1
  local ppj = 1
  for i,t in ipairs(score.events) do
    if t[ePartNr]  ~= nr then
      nr = t[ePartNr]
      pj = j
      j = i
    end
  end
  E.gotoNrEvent(pj)
  memPos()
end
function E.previousPart()
  -- move to the previous mark
  noteOff_Event_All()
  local nr = -1
  local j = 1
  local pj = 1
  local p_c_nrEvent_noteOn = c_nrEvent_noteOn
  for i,t in ipairs(score.events) do
    if t[ePartNr]  ~= nr then
      nr = t[ePartNr]
      pj = j 
      j = i
      if nr >= ( c_partNr - 1 ) then
        break
      end
    end
  end
  E.gotoNrEvent(j)
  if c_nrEvent_noteOn == p_c_nrEvent_noteOn then
    E.gotoNrEvent(pj)
  end
  memPos()
end
function E.previousMeasure()
  -- move to the previous measure
  noteOff_Event_All()
  local nr = -1
  local j = 1
  local pj = 1
  local p_c_nrEvent_noteOn = c_nrEvent_noteOn
  for i,t in ipairs(score.events) do
    if t[eMeasureNr]  ~= nr then
      nr = t[eMeasureNr]
      pj = j 
      j = i
      if nr >= c_measureNr  then
        break
      end
    end
  end
  E.gotoNrEvent(j)
  if c_nrEvent_noteOn == p_c_nrEvent_noteOn then
    E.gotoNrEvent(pj)
  end
  memPos()
end

function E.previousEvent()
  -- move to the previous event
  noteOff_Event_All()
  c_nrEvent_noteOn =  c_nrEvent_noteOn  - 1
  while ( c_nrEvent_noteOn > 1) and  (#(score.events[c_nrEvent_noteOn][eStarts]) ==  0 ) 
  do
    c_nrEvent_noteOn = c_nrEvent_noteOn - 1
  end
  c_nrEvent_noteOn = c_nrEvent_noteOn -1
  nextGroupEvent()
  memPos()
end

function E.gotoNrEvent(nrEvent)
  noteOff_Event_All()
  E.firstPart()
  c_nrEvent_noteOn = nrEvent - 1
  nextGroupEvent()
  resetPendingTuning()
  memPos()
end
function E.getPosition()
  -- return the next ebvent to play, or the event playing
  if nb_events == 0 then
  	return -1 , 0 
  end
  if c_nrEvent_playing then
    return c_nrEvent_playing[2] , 1 
  end
  return c_nrEvent_noteOn , 0 
  -- memPos()
end

-- save a score in a file
function E.save(f)
  f=io.open(f..".pck","w")
  io.output(f)
  -- save 32 tracks
  for i = 1 , 32 , 1 do
    local s 
    if i <= #(score.tracks) then
      t = score.tracks[i]
      s = string.pack("zhhh",t[tName],t[tRandomDelay],t[tDynamic],t[tPedal])
    else
      s = string.pack("zhhh","",0,0,0)
    end
    io.write(s) 
  end
  -- save the events
  for inil,t in ipairs(score.events) do
    local s = string.pack("zzhhhhhhhhhzhhhhhhhhhhh",
      table.concat(t[eStarts],"/"),table.concat(t[eStops],"/"),
      t[ePlayed],t[eVisible],
      t[eTrackNr],t[ePitch],t[eVelocity],t[eDelay],
      t[eDynamic],t[eRandomDelay],t[ePedal],
      t[eLua],
      t[eWillStopIndex],t[eStopIndex],
      t[ePartNr],t[eMeasureNr],t[eMeasureLength],
      t[eStartMeasureNr],t[eStartT],t[eStartOrder],
      t[eStopMeasureNr],t[eStopT],t[eStopOrder]     
      )
    io.write(s) 
  end
  io.close(f)
end
-- load a score from a file
function E.load(f)
  collectgarbage("restart")
  E.initScore()
  f=io.open(f..".pck","r")
  io.input(f)
  sf=io.read("a")
  io.close(f)
  local possf = 1
  local lensf = string.len(sf)
  -- read 32 tracks
  for i = 1 , 32 , 1 do
    local name , randomDelay , dynamic , pedal 
    name , randomDelay , dynamic , pedal, possf = string.unpack("zhhh",sf,possf)
    if string.len(name) > 0 then
      table.insert(score.tracks,{name , randomDelay , dynamic , pedal})
    end
  end
  -- read the events
  while possf < lensf do
    local Starts,Stops
    local Played,Visible
    local TrackNr,Pitch,Velocity,Delay
    local Dynamic,RandomDelay,Pedal
    local Lua
    local WillStopIndex,StopIndex
    local PartNr,MeasureNr,MeasureLength
    local StartMeasureNr,StartT,StartOrder
    local StopMeasureNr,StopT,StopOrder
      
    Starts,Stops,
     Played,Visible,
     TrackNr,Pitch,Velocity,Delay,
     Dynamic,RandomDelay,Pedal,
     Lua,
     WillStopIndex,StopIndex,
     PartNr,MeasureNr,MeasureLength,
     StartMeasureNr,StartT,StartOrder,
     StopMeasureNr,StopT,StopOrder,
     possf = string.unpack("zzhhhhhhhhhzhhhhhhhhhhh",sf,possf)
    local tStarts = {}
    for si in string.gmatch(Starts,"%d+") do
      table.insert(tStarts,math.tointeger(si))
    end
    local tStops = {}
    for si in string.gmatch(Stops,"%d+") do
      table.insert(tStops,math.tointeger(si))
    end
    table.insert(score.events, {
     tStarts , tStops ,
    Played,Visible,
    TrackNr,Pitch,Velocity,Delay,
    Dynamic,RandomDelay,Pedal,
    Lua,
    WillStopIndex,StopIndex,
    PartNr,MeasureNr,MeasureLength,
    StartMeasureNr,StartT,StartOrder,
    StopMeasureNr,StopT,StopOrder } )
  end
  E.firstPart()
end

function dumpScore()
  f= io.open ("traceScore.txt", "w")
  io.output(f)
  io.write("Trace events\n")
  
  for nrEvent=1 , #(score.events) ,  1 do
    local t = score.events[nrEvent]
    io.write( "#" .. nrEvent.."/"..t[eStartMeasureNr].."."..t[eStartT].."/"..luachord.pitchToString(t[ePitch]) ," Played="..t[ePlayed]," Visible="..t[eVisible], "\n")
    io.write( "          TrackNr#"..t[eTrackNr]," Pitch#"..t[ePitch]," velo#"..t[eVelocity]," Delay="..t[eDelay]," Pedal="..t[ePedal],"\n")
    if t[eStopIndex] > 0 then
    io.write( "          StopIndex           #" .. t[eStopIndex], "\n")
    end
    if ( #(t[eStarts]) > 0 ) then
    io.write( "          Starts synchronously#",table.concat(t[eStarts],"/"))
      io.write("\n")
    end
    if t[eWillStopIndex] > 0 then
    io.write( "          WillStopIndex       #" .. t[eWillStopIndex] , "\n")
    end
    if ( #(t[eStops]) > 0 ) then
    io.write( "          Stops synchronously #",table.concat(t[eStops],"/"))
      io.write("\n")
    end
    io.write( "          PartNr=" .. t[ePartNr], " MeasureNr=" .. t[eMeasureNr], " MeasureLength=" .. t[eMeasureLength], "\n" )
    io.write( "          StartMeasureNr=" .. t[eStartMeasureNr] , " StartT=" .. t[eStartT], " StartOrder=" .. t[eStartOrder], "\n")
    io.write( "           StopMeasureNr=" .. t[eStopMeasureNr]  , "  StopT=" .. t[eStopT] , "  StopOrder=" .. t[eStopOrder] , "\n")
    if t[eLua] ~= "" then
    io.write( "          Lua=" .. t[eLua] , "\n")
    end
  end
  io.write("Trace end events\n")
  io.close(f)
end

function E.initScore()
  -- innit the score
  collectgarbage("restart")
  score.events = {}
  score.tracks = {}
  nb_events = 0
  c_nrEvent_playing = nil -- current index for note playing 
  c_nrEvent_noteOn = 0 -- current index for the next note-on 
  c_nrEvent_noteOff = 1 -- current index for the next note-off
  c_partNr = 1 -- number of the current part
  c_measureNr = 1   -- number of the current measure
end

function E.addEvent(iplayed,ivisible , 
    itrackNr, ipitch, ivelocity, idelay, 
    idynamic, irandomDelay, ipedal , 
    ilua , 
    iwill_stop_index, istop_index,
    ipartNr,imeasureNr,imeasureLength,
    istart_measureNr,istart_t,istart_order,
    istop_measureNr,istop_t,istop_order)
  currentEventStarts = {}
  currentEventStops = {}
  table.insert(score.events, {currentEventStarts,currentEventStops, 
      iplayed,ivisible,
      itrackNr, ipitch, ivelocity, idelay, 
      idynamic, irandomDelay, ipedal , 
      ilua, 
      iwill_stop_index, istop_index,
      ipartNr,imeasureNr,imeasureLength,
      istart_measureNr,istart_t,istart_order,
      istop_measureNr,istop_t,istop_order} )
  -- followed by sequence of addEventStarts(..) and addEventStops(..)
end
function E.addEventStarts(start_nr_musicxmlevent)
  table.insert(currentEventStarts , start_nr_musicxmlevent )
end
function E.addEventStops(stop_nr_musicxmlevent)
 table.insert(currentEventStops , stop_nr_musicxmlevent)
end

function E.addTrack(track_name)
  table.insert(score.tracks,{track_name , 0 , 128 ,  0 })
end
function E.finishScore()
  -- dumpScore()
  collectgarbage("restart")
  nb_events = #(score.events)
  c_nrEvent_playing = nil -- current index for note playing 
  c_nrEvent_noteOn = 0 -- current index for the next note-on 
  c_nrEvent_noteOff = 1 -- current index for the next note-off
  c_partNr = 1 -- number of the current part
  c_measureNr = 1   -- number of the current measure

  E.firstPart()
end



--[[
test the module 
function testCreateScore()
  E.initScore()
  
  -- beat#1 , a quarter automatic-arpeggiate eChord with 3 pitches 0=40 1=44 2=47 decreshendo
  E.addEvent( -- #1
    1, 1 , 
    1 , 40, 64, 0 ,
    -1, -1, -1 , 
    "" , 
    1, 0, 0,
    1,1,48,
    1,0,0,
    1,1,0 )
  E.addEventStarts(1)
  E.addEventStarts(2)
  E.addEventStarts(3)
  E.addEventStops(1)
  E.addEventStops(2)
  E.addEventStops(3)
  E.addEvent( -- #2
    1, 1 , 
    1 , 44, 54, 10 ,
    -1, -1, -1 , 
    "" , "" , "" , 
    0, 0, 0,
    1,1,48,
    1,0,0,
    1,1,0 )
  E.addEvent( -- #3
    1, 1 , 
    1 , 47, 44, 20 ,
    -1, -1, -1 , 
    "" , "" , "" , 
    0, 0, 0,
    1,1,48,
    1,0,0,
    1,1,0 )
  
  -- beat #2 , two quarters #4=52 #6=56 with a half #5=28 
  E.addEvent( -- #4
    1, 1 , 
    1 , 52, 64, 0 ,
    -1, -1, -1 , 
    "" , "" , "" , 
    4, 0, 0,
    1,1,48,
    1,1,0,
    1,2,0 )
  E.addEventStarts(4)
  E.addEventStarts(5)
  E.addEventStops(4)
  E.addEvent( -- #5
    1, 1 , 
    1 , 28, 54, 0 ,
    -1, -1, -1 , 
    "" , "" , "" , 
    -1, -1, 0,
    1,1,48,
    1,1,0,
    1,3,0 )
  E.addEvent( -- #6
    1 , 1 , 
    1 , 56, 44, 0,
    -1, -1, -1 , 
    "" , "" , "" , 
    6, -1, 0,
    1,1,48,
    1,2,0,
    1,3,0 )
  E.addEventStarts(6)
  E.addEventStops(5)
  E.addEventStops(6)

 -- add an empty playable event at the end of the events, in an empty part
  E.addEvent(0, 0 , 
  0 , 0, 0, 0,
  -1, -1, -1 , 
  "" , "" , "" , 
  -1, -1, 0,
  0,0,48,
  0,0,0,
  0,0,0 )

  -- track#1 : piano
  E.addTrack("piano")
  
  E.finishScore()
end
function testEvent()
  for i = 1 , 20 , 1 do
    --print("on#"..i)
    E.noteOn_Event(i , 64)
    --print("of#"..i)
    E.noteOff_Event(i)
  end
  print("Dump Midi :")
  luabass.dumpListMidi()
end
--
testCreateScore()
E.save("testserialize")
E.load("testserialize")
testEvent()
--]]

return E -- to export the functions
