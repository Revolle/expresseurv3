# HANDOVER — texttochord.lua refactor (Expresseur v3)

This file is a resumption context for an LLM agent picking up this task cold, in a new
session, with no access to the prior conversation. It is not written for human readability.
Read this file fully before touching any code or asking the user anything already answered
here. Do not re-ask questions already resolved in "DECISIONS LOCKED" below.

## REPO / ENVIRONMENT STATE

- Origin repo: https://github.com/Revolle/expresseurv3 , cloned at `C:\workspace\expresseurv3` (branch `master`, clean).
- A git worktree was created manually (the `EnterWorktree` tool could not attach because the
  session's primary directory is not itself a git repo — this is a harness quirk, not a repo
  issue) via:
  `git worktree add -b thomas/testclaude ../expresseurv3-worktrees/thomas-testclaude`
- **All work happens in**: `C:\workspace\expresseurv3-worktrees\thomas-testclaude` on branch
  `thomas/testclaude`. Do not work in `C:\workspace\expresseurv3` (that's the `master` checkout).
- No code has been modified yet anywhere. This has been a pure investigation +
  architecture-discussion session. Zero commits made on `thomas/testclaude` beyond the branch
  creation itself.
- No formal plan has been drafted or approved yet. The user explicitly wants a plan agreed
  upon (with them, conversationally) BEFORE any file is edited. Do not jump to implementation.

## TASK

Target file: `lua/texttochord.lua` (Lua, ~1310 lines). It parses a chord-description string
(e.g. `"G7/D"`, `"C[!dorien]"`) and currently *eagerly precomputes* full keyboard-range tables
of MIDI pitches for four "roles": bass, chord, scale, penta.

The user's goal (their own words, translated): replace the static/eager precomputed pitch
tables with **on-demand methods** — i.e. instead of building one giant table of notes up
front, expose functions you call to ask "give me note N for this chord" computed lazily.
Motivation: the current code was written quickly/prototyped, is not scalable/extensible, and
the user feels blocked by its rigidity (see "OPEN QUESTION" below — this is the most important
unresolved thread).

## SCOPE — LOCKED

- **IN SCOPE**: how `texttochord.lua` computes and exposes the actual MIDI pitches to play
  (currently: `interpretedChord.pitch.{bass,chord,scale,penta}[index][alteration]`, built by
  `fillPitches`/`fillScale`/`addChord`/`addChordNotsorted`/`blackKeys`/`fillBass`/
  `calculateWalkingBass`, lines 435-844 approx).
- **OUT OF SCOPE — DO NOT TOUCH**: the `modes` table (lines 56-93 of texttochord.lua). This is
  25 named musical modes/scales (ionian, dorian, balkan, etc.) hardcoded as 12-element degree
  arrays. The user was explicit: *"c'est un concept musical qu'on ne peut pas toucher, donc
  touche pas à ça, c'est off scope."* Do not refactor, regenerate, or restructure this table in
  this task, even if it looks like the same "static table" smell as the in-scope problem.
- Also out of scope (not discussed, assume untouched unless user says otherwise): chord-string
  parsing logic itself (`stringToDegree`, `stringToChord`'s modifier-parsing cascade around
  lines 960-1078) and the `pitchRole[1..12]` degree-role model it produces — this part is
  considered reasonably fine as-is; it is the *output/pitch-generation* layer that's the
  problem, not the *parsing* layer. Do not assume this is settled forever, but do not start
  refactoring it without raising it with the user first.

## WHO CONSUMES texttochord.lua (only real external caller)

`lua/luachord.lua` is the **only** file that requires `texttochord`. The full consumed surface:

```lua
-- luachord.lua:105    texttochord.stringToPitch(s)
-- luachord.lua:109    texttochord.setiTone(valueTone)
-- luachord.lua:112    texttochord.ptos(valueTone)
-- luachord.lua:324    texttochord.stringToPitch(string.lower(sk))
-- luachord.lua:417    texttochord.setiTone(section[nrSection].key)
-- luachord.lua:456    texttochord.stringToChord(sg)
-- luachord.lua:613    texttochord.stringToChord(currenChord.chord)
-- luachord.lua:863    texttochord.stringToChord(chordText)

-- luachord.lua:555-558
function E.getPitches(scale)
  return currentChord.interpretedChord.pitch[scale]
end

-- luachord.lua:559-566  <-- THE key function to redesign the internals behind
function E.getIndexPitches(scale, index, alteration)
  -- scale: "bass" | "chord" | "scale" | "penta"
  -- index: signed integer, keyboard position (see below)
  -- alteration: -1, 0, or 1 (flat/natural/sharp neighbor)
  return currentChord.interpretedChord.pitch[scale][index][alteration]
end
```

`E.getIndexPitches` is called from `E.playPitches` (luachord.lua:944, line 983 body), which is
called by `E.playChord`, `E.playScale`, `E.playBass`, `E.playBackground`
(luachord.lua:1006-1029) — these are the actual MIDI-note-on handlers triggered when a
physical key is pressed.

## CONFIRMED SEMANTICS OF `index` / `alteration` (do not re-litigate)

`index` = **keyboard position**, not "rank within the chord" (i.e. NOT 1=root, 2=third,
3=fifth...). This was explicitly confirmed by tracing the call chain: `E.playChord` /
`E.playScale` / `E.playBass` pass `whitemediane` (or a literal `0`/`1`) as `index`, and
`whitemediane` is documented in `lua/expresseur.lua:58-71` as:

```
--   medianeKey[-n/2..0..n/2] : index of the pitch within the range of the selector. 0 = middle.
--   whiteMediane : idem medianeKey, but taking in account only "white keys"
--   black[0,1] : 0 = white key, 1 = black key   (texttochord side calls this -1/0/1: flat/natural/sharp)
```

**Important architectural fact discovered**: `whiteMediane`/`mediane`/`black` are NOT specific
to chords/music. They are a **generic contract produced by the native engine**
(`basslua.dll`, implemented in C++ in `basslua/basslua.cpp`, `expresseur/midishortcut.cpp`,
`expresseur/editshortcut.cpp`), shared by every Lua script in the app (the mixer, MIDI
shortcuts, `lua/dmx.lua`, etc.), not something owned by `texttochord.lua` or `luachord.lua`.
Redesigning that contract is a large, separate, cross-language (C++) project touching the
whole app's input pipeline — **not** the same task as refactoring `texttochord.lua`'s internal
note-generation. This distinction was raised with the user and is the crux of the **currently
open, unresolved question** below.

## DECISIONS LOCKED (agreed with user, do not re-ask)

1. **Scope = "Option A"**: redesign how output pitches are generated/exposed, not how modes
   are cataloged (see SCOPE above).
2. **Behavior change is acceptable**: this does not need to be a byte-for-byte-identical-output
   refactor. User's words: open to changing behavior "if it helps [simplify]", prioritize
   simplicity over strict backward compatibility of exact musical output.
3. **luachord.lua may also be changed freely.** User's words: "on va tout changer, donc on est
   libre de remodeler comme on veut." Nothing in the calling code is sacred except the
   external native contract (whiteMediane/black — see open question, likely also not to be
   touched, but see below).
4. **Eliminate global mutable module state.** Today `texttochord.lua` has module-level
   `local` mutable globals: `currentTone`, `modeCurrent`, `modeDefault`, `modeMaster`,
   `modeRemanent`, `modeCurrentRoot`, `centerPitch`, `centerOctave`, `blackScale`, `prevBass`
   (declared ~lines 94-103). User agreed: replace with an **explicit context object** owned by
   the caller (`luachord.lua`) and threaded through calls explicitly, e.g. conceptually:
   ```lua
   local context = texttochord.newContext()
   local chord = texttochord.stringToChord(context, "G7", "C")  -- may mutate context (tone/mode remanence)
   local pitch = texttochord.getNote(context, chord, "chord", index, alteration)
   ```
   Rationale given to user and accepted: enables multiple independent chord "voices"/tracks to
   coexist without shared hidden state — impossible today.
5. **Replace eager whole-keyboard precomputation with lazy on-demand computation + cache.**
   Today, every chord change triggers `fillPitches` → `fillScale`/`addChord`/`fillBass`/
   `blackKeys`, which walks the *entire* keyboard range (`while p <= 127`, `while p >= 0`) and
   builds one big table per role, up front, whether or not those notes are ever requested.
   Agreed direction: a single generic `getNote(context, chord, role, index, alteration)`
   function computes the pitch for that specific (role, index, alteration) only when asked,
   optionally memoizing per-chord the first time it's computed. Adding a new "role" in the
   future should require adding data (which degrees belong to that role in `pitchRole`), not a
   new bespoke `fillX`/`addX` function.
   - Exception: **bass** is inherently a short precomputed sequence (4-5 notes forming an
     optimal walking bass line into the *next* chord, via `calculateWalkingBass`, lines
     622-739) — this cannot be made purely lazy/per-index in the same way since it depends on
     next-chord voice-leading. Agreed approach: keep it as a small (4-5 element) precomputed
     structure, but compute it lazily on first bass request for that chord rather than
     unconditionally inside `fillPitches`.
6. **Concrete rigidities identified in the current Lua implementation** (these are candidate
   things the redesign should fix, pending confirmation — see open question):
   - `addChord` (line 578) hardcodes a **5-note voicing cap** for every role, unconditionally.
   - A single global `centerPitch`/`centerOctave` — no way to have different centers per role
     or per simultaneous voice.
   - Magic/hardcoded loop bounds scattered through the fill functions: `while p <= 127`,
     `while p >= 0`, `for nb = 2, 14`, `ichord0 = -50 ... < 50`, `for q = p-1, prevP+1, -1`.
   - **Black-key (alteration) resolution is a single hardcoded strategy** in `blackKeys`
     (lines 482-525): nearest active scale-tone neighbor, or chromatic ±1 fallback (controlled
     by the global `blackScale` flag) — no way to plug in a different strategy.

## OPEN / UNRESOLVED — THIS IS WHERE THE CONVERSATION STOPPED

The user said the current `index`/`black` (whiteMediane) model itself is "beaucoup trop
rigide" and they want to "sortir de ce système" (get out of this system) — they feel
genuinely blocked by it, not just by the internal Lua computation.

After investigation (see "Important architectural fact discovered" above), the agent's working
hypothesis — **not yet confirmed by the user** — is that the actual rigidity is NOT in the
native `whiteMediane`/`black` contract itself (which is generic, shared, load-bearing
infrastructure across the whole app, and would be a much larger separate C++ project to
change), but in what `texttochord.lua` *does* with that number internally (the 6 rigidities
listed in point 6 above: fixed 5-note cap, single global center, magic bounds, single
hardcoded black-key strategy).

**The very last thing the agent asked, unanswered when the session paused**: asked the user
for a *concrete, specific example* of something they currently cannot do, to determine whether
the redesign needs to stay Lua-only (agent's recommendation) or must also touch the native
C++ engine (`basslua/basslua.cpp`, `expresseur/midishortcut.cpp`, `expresseur/editshortcut.cpp`)
— a much bigger undertaking. Example prompts given to the user to help them answer: "je veux un
voicing à 3 notes pour tel rôle", "je veux deux centres différents", "je veux qu'une touche
noire fasse autre chose que la note la plus proche", or something else entirely.

**NEXT ACTION for whoever resumes this**: get that concrete example from the user first. Do
not propose final function signatures or start writing code until this is resolved, since it
determines whether scope is "Lua module refactor" (contained, days) or "Lua + native C++ input
pipeline redesign" (large, cross-language, weeks, high risk, affects every other Lua script in
the app that relies on the same whiteMediane/black/mediane contract — mixer, MIDI shortcuts,
`lua/dmx.lua`, etc.). Getting this wrong burns significant time.

Once that's resolved, the next deliverable (per the user's explicit process) is a **precise
function-signature-level plan** (not code) for the user to approve, covering at minimum:
`newContext`, the exact shape of the context object, `stringToChord`'s new signature, and
`getNote`'s exact signature/behavior/caching strategy — before any file is edited.

## KEY FILE/LINE MAP (for quick re-orientation, verify line numbers haven't drifted)

- `lua/texttochord.lua`
  - 56-93: `modes` table — **OFF-SCOPE, do not touch**.
  - 94-103: module-level mutable globals to be eliminated (see decision 4).
  - 435-480: `addPitchChord`
  - 482-525: `blackKeys` — hardcoded black-key resolution strategy (rigidity #4).
  - 528-576: `addChordNotsorted`
  - 578-619: `addChord` — hardcoded 5-note cap (rigidity #1).
  - 622-638: `closestPitch`
  - 640-666: `calculateWalkingBass`
  - 668-739: `fillBass`
  - 742-814: `fillScale` — eager whole-keyboard walk (rigidity #3, magic bounds).
  - 815-844: `fillPitches` — top-level eager orchestrator, called once per chord.
  - 846-1262: `stringToChord` — string parsing + `pitchRole` construction (considered fine,
    not in active redesign scope, see SCOPE section) + calls `fillPitches` at the end (line
    1259).
  - 1267-1309: commented-out manual test block at bottom of file.
- `lua/luachord.lua`
  - 555-566: `E.getPitches`, `E.getIndexPitches` — the two real entry points into
    `texttochord`'s output, called by:
  - 944-1029ish: `E.playPitches`, `E.playChord`, `E.playScale`, `E.playBass`,
    `E.playBackground` — where `index`/`alteration` actually originate as
    `whitemediane`/`black` parameters passed in from the native engine's callback signature.
- `lua/expresseur.lua:40-89`: doc comment block defining the generic native event contract
  (`indexKey`, `medianeKey`, `whiteIndex`, `whiteMediane`, `black`) shared across all Lua
  scripts in the app — not chord-specific.
- `lua/test_texttochord.lua`: existing manual/automatic test harness for `texttochord.lua`
  (`automatic_test()`, `manual_test()`, `test1()`). Useful to sanity-check no regression once
  changes start, even though exact-output-preservation is not a hard requirement (decision 2).

## STYLE / PROCESS NOTES FOR WHOEVER RESUMES

- The user (Thomas) wants to co-design this iteratively and conversationally: propose a
  concrete recommendation, let them react, refine. Do not deliver a finished design
  unilaterally without checkpoints — but also do not just ask generic open questions; ground
  recommendations in what the code actually shows (they pushed back once on the agent being
  "too generic").
- Do not touch any file (edit) until a plan is explicitly approved by the user in
  conversation.
- Conversation with the user has been in French; continue in French unless they switch.
