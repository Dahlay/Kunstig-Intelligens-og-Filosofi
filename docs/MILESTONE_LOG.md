# Milestone Log

## Milestone 0-2 (Bootstrap + Graph + Runtime Skeleton)

### Scope completed

- CMake project with JUCE + Catch2 + JSON
- Graph model (`Node`, `Port`, `Edge`, `PatchGraph`) and validation
- Runtime compiler with topological scheduling and no-cycle rejection
- Audio engine that swaps compiled runtime plans
- Minimal processors (`Output`, `Gain`, `ToneGenerator`, `Passthrough`)
- Basic node canvas with drag and cable connect

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
