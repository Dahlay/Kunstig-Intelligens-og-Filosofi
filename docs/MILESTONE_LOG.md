# Milestone Log

## Milestone 0-2 (Bootstrap + Graph + Runtime Skeleton)

### Scope completed (M3-M5)

- CMake project with JUCE + Catch2 + JSON
- Graph model (`Node`, `Port`, `Edge`, `PatchGraph`) and validation
- Runtime compiler with topological scheduling and no-cycle rejection
- Audio engine that swaps compiled runtime plans
- Minimal processors (`Output`, `Gain`, `ToneGenerator`, `Passthrough`)
- Basic node canvas with drag and cable connect

## Milestone 3-5 (Canvas Interactions + Integration)

### Scope completed

- Right-click cable disconnect support
- Cycle-safe connection validation
- Patch save/load UI actions
- JSON deserialization and roundtrip support

### Files changed (M3-M5)

- `src/ui/CanvasView.*`
- `src/app/MainComponent.*`
- `src/persistence/PatchSerializer.*`
- `src/graph/GraphValidator.cpp`
- `tests/GraphTests.cpp`

### Test evidence (M3-M5)

- `ctest` passing: 4/4 tests

### Known limitations (M3-M5)

- Edge hit-testing for disconnect is approximate
- DSP modules beyond tone/gain are still placeholders

### Next milestone checklist (M6+)

- Implement real Filter/Delay/Mixer DSP
- Add synth voice engine and drum sequencer timing
- Add undo/redo command stack

## Milestone 6 (Audio Engine MVP Processors)

### Scope completed (M6)

- Implemented `FilterProcessor` (one-pole LPF)
- Implemented `DelayProcessor` (feedback delay with wet/dry mix)
- Implemented `MixerProcessor` (input-normalized summing)
- Updated compiler wiring to instantiate real processors

### Files changed (M6)

- `src/audio/processors/IProcessor.h`
- `src/graph/GraphCompiler.cpp`
- `tests/GraphTests.cpp`

### Test evidence (M6)

- `ctest` passing: 5/5 tests
- Added runtime test for filter-delay-mixer signal chain

### Known limitations (M6)

- Processor parameters are currently fixed defaults (no inspector mapping yet)
- Drum and synth are still tone generators

### Next milestone checklist (M7)

- Add proper synth voice allocation + ADSR
- Add drum sequencer clocking from transport/BPM
- Expose processor params in UI inspector

## Milestone 7 (Instruments + Transport)

### Scope completed (M7)

- Added transport-aware processing context (`playing`, `bpm`, `samplePosition`)
- Implemented `SynthProcessor` with polyphonic voice cycling and ADSR-style envelope behavior
- Implemented `DrumProcessor` with BPM-locked 16-step sequencing (kick/snare/hat)
- Added transport controls in UI (Start/Stop + BPM slider)
- Wired engine transport state into audio callback execution

### Files changed (M7)

- `src/audio/processors/IProcessor.h`
- `src/audio/AudioEngine.h`
- `src/audio/AudioEngine.cpp`
- `src/graph/GraphCompiler.cpp`
- `src/app/MainComponent.h`
- `src/app/MainComponent.cpp`
- `tests/GraphTests.cpp`

### Test evidence (M7)

- `ctest` passing: 6/6 tests
- Added transport stop silence regression test

### Known limitations (M7)

- Synth/drum pattern and timbre are fixed defaults (not yet editable in inspector)
- No MIDI input routing yet

### Next milestone checklist (M8)

- Implement undo/redo command stack
- Expose processor and instrument params in inspector
- Expand patch compatibility/version migration handling

### Files changed

- `CMakeLists.txt`
- `src/**`
- `tests/GraphTests.cpp`

### Test evidence

- Added graph connection test
- Added compiler error-path test

### Known limitations

- Patch deserialization is placeholder
- No undo/redo yet
- Filter/Delay/Mixer are pass-through placeholders

### Next milestone checklist

- Add richer graph tests (cycle and single-input policy)
- Improve cable UX (disconnect, validation feedback)
- Add save/load roundtrip
