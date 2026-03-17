# Modular Audio Patcher (Windows) — Execution Plan

## 1) Goal

Build a desktop Windows app where users can:

- Drag module boxes on a canvas (`Drum Machine`, `Synth`, `Effects`, etc.)
- Connect modules with virtual cables
- Hear audio in real time from the patch graph
- Save/load patches

Recommended stack:

- **Language**: `C++17+`
- **Framework**: `JUCE` (UI + audio + plugin hosting support)
- **Build**: `CMake`
- **Tests**: Catch2 or JUCE UnitTest

---

## 2) Non-Negotiable Constraints (Real-time Audio)

1. **No heap allocation on audio callback**
2. **No locks/mutex blocking on audio callback**
3. Use lock-free queues for UI->audio thread messages
4. Process graph in deterministic topological order
5. Handle sample-rate/block-size changes safely

Implementation safety rule:

1. Ship only behind milestone exit criteria; no milestone is "done" without tests passing

---

## 3) Product Scope (MVP)

### MVP Modules

- `Audio Output` (mandatory sink)
- `Synth` (simple poly synth)
- `Drum Machine` (step sequencer + sample playback)
- `Gain`
- `Filter` (LPF)
- `Delay`
- `Mixer` (2-4 channel)

### MVP Features

- Canvas pan/zoom
- Drag/drop module creation from palette
- Input/output ports per module
- Cable creation/removal
- Validate legal connections
- Real-time playback
- Start/Stop transport and BPM control
- Preset save/load (`.json`)
- Basic undo/redo

Out of scope for MVP:

- VST hosting
- GPU-heavy visuals
- Collaboration/cloud sync

---

## 4) High-Level Architecture

### 4.1 Core Layers

1. **UI Layer**
   - Module widgets, ports, cable rendering, selection
2. **Graph Model Layer**
   - `Node`, `Port`, `Edge`, `PatchGraph`
3. **Audio Engine Layer**
   - Compiled runtime graph, processing scheduler, DSP modules
4. **Persistence Layer**
   - JSON serialization/deserialization
5. **Command Layer**
   - Undo/redo command bus (`AddNode`, `ConnectEdge`, etc.)

### 4.2 Thread Model

- **Message/UI thread**: interactions, patch editing, rendering UI
- **Audio thread**: DSP execution only
- **Bridge**: lock-free message queue + immutable graph snapshots

---

## 5) Data Model Specification

### 5.1 Core Types

- `NodeId` (UUID)
- `PortId` (UUID)
- `EdgeId` (UUID)

```text
Node
- id
- type (Synth, Delay, ...)
- position (x,y)
- params map
- inputPorts[]
- outputPorts[]

Port
- id
- nodeId
- direction (in/out)
- signalType (audio/cv/gate/midi)
- channels

Edge
- id
- fromPortId
- toPortId
```

### 5.2 Validation Rules

- Direction must be `out -> in`
- Signal types must match or be explicitly convertible
- No duplicate identical edge
- No audio-cycle unless cycle-safe mode implemented (MVP: reject)
- Single input policy for audio input ports (configurable)

---

## 6) Runtime Audio Graph

### 6.1 Compile Step (on graph change)

1. Validate graph
2. Build adjacency lists
3. Topological sort
4. Instantiate/update DSP processors
5. Build execution plan buffers
6. Atomically swap active plan for audio thread

### 6.2 Processing Step (audio callback)

1. Read current immutable execution plan
2. Clear temp buffers
3. Execute processors in sorted order
4. Mix to output node
5. Return audio block

Failure strategy:

- If compile fails, keep previous plan active and notify UI

---

## 7) UI/UX Implementation Plan

### 7.1 Canvas

- Infinite-ish canvas with pan/zoom
- Snap-to-grid optional
- Marquee selection

### 7.2 Module Box

- Header (name + bypass/mute)
- Body (controls)
- Left ports=input, right ports=output
- Drag module to reposition

### 7.3 Cable UX

- Click-drag from output port to input port
- Preview cable while dragging
- Color by signal type
- Right-click cable to disconnect

### 7.4 Inspector Panel

- Shows selected module params
- Parameter automation hooks reserved

---

## 8) Milestone Breakdown (Build Order)

Dependency rule:

- Do not start a milestone until all predecessor exit criteria are met.

### Milestone -1 — Technical Decisions Freeze (0.5-1 day)

- Pin JUCE version and compiler toolchain (`MSVC` target)
- Choose test framework (Catch2 or JUCE UnitTest) and lock choice
- Select JSON library strategy (`juce::var`/`ValueTree` or `nlohmann::json`)
- Define target audio buffer sizes for perf testing (e.g., 64/128/256)

**Exit criteria**: Toolchain and library choices are fixed in repo docs; CI reproduces local build.

### Milestone 0 — Project Bootstrap (1-2 days)

- Create JUCE+CMake app
- Audio device manager init
- Basic window + render loop
- CI build on Windows

**Predecessor**: Milestone -1

**Exit criteria**: App opens, audio device initializes, silent callback stable.

### Milestone 1 — Graph Model + Serialization (2-3 days)

- Implement `Node/Port/Edge/PatchGraph`
- Add validation
- Add compile-ready graph snapshot format (immutable DTO)

**Predecessor**: Milestone 0

**Exit criteria**: Graph edits, validation, and snapshot conversion pass tests.

### Milestone 2 — Audio Runtime Skeleton (2-4 days)

- Implement graph compiler interface + topological scheduler
- Implement audio-thread plan swap mechanism
- Add minimal processors: `Output`, `Gain`, `ToneGenerator` (test-only)
- Add audio smoke tests (non-silent output, no NaN/Inf)

**Predecessor**: Milestone 1

**Exit criteria**: A hardcoded graph renders stable audio through runtime scheduler.

### Milestone 3 — Canvas + Node UI (3-5 days)

- Palette, add/remove nodes
- Drag nodes
- Draw ports
- Selection state

**Predecessor**: Milestone 1

**Exit criteria**: User can create and arrange module boxes.

### Milestone 4 — Cable Routing + Connection Rules (2-4 days)

- Cable interactions
- Connect/disconnect
- Enforce validation rules

**Predecessor**: Milestone 3

**Exit criteria**: Valid routes connect; invalid routes rejected with feedback.

### Milestone 5 — UI-to-Audio Integration (2-3 days)

- On patch edit, compile immutable runtime plan
- Atomically swap active plan without glitches
- Display compile errors in UI without breaking current playback

**Predecessors**: Milestones 2 and 4

**Exit criteria**: Drag/connect in UI changes audible routing in real time safely.

### Milestone 6 — Audio Engine MVP Processors (4-7 days)

- Add `Mixer`, `Filter`, `Delay` processors
- Add parameter smoothing where needed
- Add buffer reuse/pool for temporary audio buffers

**Predecessor**: Milestone 5

**Exit criteria**: Audible signal path via connected modules.

### Milestone 7 — Instruments & Transport (4-7 days)

- Basic Synth module
- Drum machine sequencer
- BPM/transport clocking

**Predecessor**: Milestone 6

**Exit criteria**: Drums + synth can be patched and played in time.

### Milestone 8 — Persistence + Undo/Redo + Polish (3-5 days)

- Patch save/load UI and compatibility checks
- Command stack
- Error toasts
- Keyboard shortcuts
- Basic performance tuning

**Predecessor**: Milestone 7

**Exit criteria**: Core UX feels reliable for everyday patching.

### Milestone 9 — Packaging (1-2 days)

- Installer/executable packaging for Windows
- Crash logging + diagnostics

**Predecessor**: Milestone 8

**Exit criteria**: Fresh machine install works.

### Milestone 10 — Stabilization Gate (1-2 days)

- Run full test suite + manual QA checklist
- Validate callback CPU budget on target machine
- Verify no regressions on save/load/undo/redo

**Predecessor**: Milestone 9

**Exit criteria**: MVP Definition of Done is objectively met.

---

## 9) Suggested Folder Structure

```text
/src
  /app
  /ui
    CanvasView.*
    ModuleView.*
    CableView.*
    InspectorView.*
  /graph
    PatchGraph.*
    GraphValidator.*
    GraphCompiler.*
  /audio
    AudioEngine.*
    processors/
      IProcessor.*
      OutputProcessor.*
      SynthProcessor.*
      DrumProcessor.*
      GainProcessor.*
      FilterProcessor.*
      DelayProcessor.*
      MixerProcessor.*
  /commands
    ICommand.*
    UndoRedoManager.*
  /persistence
    PatchSerializer.*
  /util
/tests
/docs
```

---

## 10) Definition of Done (MVP)

- User can add modules, connect cables, and hear expected results
- Graph edits never crash audio callback
- Save/load preserves full patch and parameters
- Average callback time < 50% of buffer duration on target machine
- No audible clicks/pops under normal use

---

## 11) Automated Test Plan

1. **Unit tests**
   - Graph validation rules
   - Topological sorting
   - Parameter clamping and defaults
2. **Serialization tests**
   - JSON roundtrip equality
3. **Audio smoke tests**
   - Non-silent output for known patch
   - No NaN/Inf samples
4. **UI integration tests** (if feasible)
   - Add node, connect edge, save/load flow

Gate policy:

- Every milestone requires green tests in CI before merge.
- Add at least one regression test for each bug fixed after Milestone 5.

---

## 12) Risks + Mitigations

- **Glitches from UI edits** -> immutable plan swap + lock-free messaging
- **Graph complexity growth** -> cap module count for MVP, optimize buffer reuse
- **Timing drift in sequencer** -> sample-accurate transport clock in audio thread
- **Scope creep** -> strict MVP gate before plugin hosting
- **Integration deadlock between UI and DSP** -> enforce single one-way command queue UI->audio and snapshot swap only
- **Late discovery of architecture flaws** -> early vertical slice in Milestones 2 + 5

---

## 13) AI Execution Backlog (Task Queue)

Use this exact order when implementing:

1. Freeze toolchain/library decisions and document them
2. Bootstrap JUCE app + CMake + audio callback + CI
3. Implement graph data model + validation + tests
4. Implement immutable graph snapshot DTO
5. Implement runtime compiler + topological scheduler skeleton
6. Add minimal processors (`Output`, `Gain`, test tone) + audio smoke tests
7. Build canvas UI with draggable modules
8. Build cable connect/disconnect interactions + validation feedback
9. Integrate UI edits -> compile -> atomic runtime plan swap
10. Add processors: Mixer, Filter, Delay (+ smoothing)
11. Add Synth processor + UI params
12. Add Drum machine processor + step sequencer UI
13. Add transport/BPM clocking
14. Add patch save/load UI + compatibility versioning
15. Add undo/redo command framework
16. Optimize performance and memory behavior
17. Package for Windows + stabilization gate

---

## 14) Prompt Contract for Coding AI (Paste into agent)

```text
You are building a JUCE-based modular audio patcher desktop app for Windows.
Follow this execution plan exactly. Implement one milestone at a time.
For each milestone:
1) create/modify files,
2) compile,
3) run tests,
4) summarize changes,
5) list next step.
Hard constraints:
- no allocations/locks in audio callback,
- graph execution must be topologically sorted,
- use immutable runtime plan swap for graph edits.
If blocked, provide minimal unblock options and continue with the best default.
```

---

## 15) AI Build Rules (Anti-Stall Defaults)

If a choice is unspecified, use these defaults (do not pause for approval):

- C++ standard: `C++20`
- JSON: `nlohmann::json`
- Tests: Catch2
- Plugin format support: none in MVP (standalone app only)
- Sample rate for tests: `48000`
- Buffer size test matrix: `64`, `128`, `256`
- Channel layout: stereo only in MVP
- Max voices for synth: `8`
- Drum sequencer steps: `16`
- Patch file version field: `schemaVersion = 1`

When blocked:

1. Pick the safest default above.
2. Continue implementation.
3. Add a short note in [docs/DECISIONS.md](docs/DECISIONS.md).

---

## 16) Required Deliverable for Each Milestone

For each milestone, AI must produce all items below before moving on:

1. **Code changes** (small, reviewable commits)
2. **Passing build** in CI and local
3. **Tests added/updated** for the changed behavior
4. **Milestone report** in [docs/MILESTONE_LOG.md](docs/MILESTONE_LOG.md):
   - Scope completed
   - Files changed
   - Test evidence
   - Known limitations
   - Next milestone start checklist

Hard stop rule:

- Do not begin next milestone if any required deliverable is missing.

---

## 17) File Creation Order (Strict)

Create these documentation/control files first, before feature code:

1. [docs/DECISIONS.md](docs/DECISIONS.md)
2. [docs/MILESTONE_LOG.md](docs/MILESTONE_LOG.md)
3. [docs/QA_CHECKLIST.md](docs/QA_CHECKLIST.md)
4. [docs/PATCH_SCHEMA.md](docs/PATCH_SCHEMA.md)

Then create core code in this order:

1. `graph` + tests
2. `audio` runtime skeleton + tests
3. `ui` canvas/nodes/cables
4. `integration` (UI->compile->swap)
5. processors/instruments/transport
6. persistence/undo/redo
7. packaging

---

## 18) Quality Gates (Must Pass)

### 18.1 Real-Time Safety Gate

- No `new`, `delete`, `malloc`, file IO, or locks on audio callback path
- No dynamic `std::vector` growth on callback path
- No logging from callback path

### 18.2 Audio Correctness Gate

- Output contains finite samples only (no NaN/Inf)
- Silence when transport is stopped (unless intentionally sustaining)
- No hard clipping by default on neutral test patch

### 18.3 UX Gate

- Invalid cable attempts show clear reason
- Compile failure keeps last-good audio plan active
- Save/load preserves module positions and params

---

## 19) Ready-to-Use Task Prompt (Stricter)

```text
Execute Milestone N only.
Inputs:
- Read EXECUTION_PLAN.md fully.
- Respect predecessor exit criteria.
Steps:
1) Implement only Milestone N scope.
2) Add/adjust tests for Milestone N.
3) Build and run tests.
4) Update docs/MILESTONE_LOG.md with evidence.
5) Stop.
Rules:
- Never violate real-time audio constraints.
- If ambiguous, apply defaults in section 15.
- Do not start Milestone N+1.
Output format:
- Completed items
- Files changed
- Test results
- Known issues
- Next recommended step
```
