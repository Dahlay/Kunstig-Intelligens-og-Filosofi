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
