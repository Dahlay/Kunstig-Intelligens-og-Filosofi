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

## Milestone 8 (Undo/Redo + Usability)

### Scope completed (M8)

- Added graph history stack with undo/redo snapshot restore
- Added toolbar Undo/Redo buttons
- Added keyboard shortcuts: Cmd+Z, Cmd+Shift+Z, Cmd+S, Cmd+O
- Added pre-edit callback plumbing from canvas to capture history before mutations

### Files changed (M8)

- `src/ui/CanvasView.h`
- `src/ui/CanvasView.cpp`
- `src/app/MainComponent.h`
- `src/app/MainComponent.cpp`

### Test evidence (M8)

- `ctest` passing: 6/6 tests
- Full app target builds successfully

### Known limitations (M8)

- Undo granularity for drag is coarse snapshot-per-edit-start
- Inspector parameter editing is still pending

### Next milestone checklist (M8.5)

- Add inspector panel with selected node parameter editing (start with `Gain`)
- Add per-module parameter bindings for filter/delay/mixer

## Milestone 8.5 (Inspector + Parameter Persistence)

### Scope completed (M8.5)

- Added node selection state (click-to-select with blue highlight on canvas)
- Added Inspector panel with enabled/disabled sliders per node type:
  - Gain knob (Gain node)
  - Filter cutoff (Filter node)
  - Delay ms / feedback / mix (Delay node)
- Inspector changes apply live to graph and rebuild runtime plan
- Parameter values (`filterCutoffHz`, `delayMs`, `delayFeedback`, `delayMix`) persisted in patch JSON
- Serialization roundtrip test expanded to validate parameter preservation

### Files changed (M8.5)

- `src/graph/PatchGraph.h`
- `src/graph/GraphCompiler.cpp`
- `src/persistence/PatchSerializer.cpp`
- `src/ui/CanvasView.h`
- `src/ui/CanvasView.cpp`
- `src/app/MainComponent.h`
- `src/app/MainComponent.cpp`
- `tests/GraphTests.cpp`

### Test evidence (M8.5)

- `ctest` passing: 6/6 tests
- Serializer test now validates gain, delayMs, delayFeedback, delayMix roundtrip

### Known limitations (M8.5)

- Synth has no editable parameters yet (note pattern and voice count are fixed)
- Mixer channel count and levels not yet inspector-bound

### Next milestone checklist (M9)

- Performance tuning + validate callback budget
- Packaging and distribution (macOS app bundle / Windows installer)

## Milestone 9 (Packaging + Distribution)

### Scope completed (M9)

- Bumped project version to `1.0.0`; version string propagated as compile-time macro `MAP_VERSION_STRING`
- `juce_add_gui_app` extended with full metadata: `BUNDLE_ID`, `COMPANY_NAME`, `VERSION`, microphone permission text
- CPack configured in `CMakeLists.txt`:
  - **macOS**: `DragNDrop` generator (`.dmg` via hdiutil)
  - **Windows**: `NSIS` generator (64-bit installer, registry entries, shortcuts)
  - **Linux fallback**: `TGZ`
- macOS packaging assets (`packaging/macos/`):
  - `Info.plist.in` — bundle plist template with category, copyright, HighDPI, microphone usage
  - `entitlements.plist` — hardened-runtime entitlements for notarization (audio-input)
  - `notarize.sh` — codesign → zip → `notarytool submit` → staple one-liner script
- Windows packaging assets (`packaging/windows/`):
  - `installer.nsi` — full NSIS Modern UI 2 script (welcome, license, dir, files, uninstaller, Add/Remove Programs registry keys)
  - `build_installer.bat` — one-shot CMake configure + build + makensis script
- Crash logger (`src/diagnostics/CrashLogger.h`):
  - POSIX signal handler (macOS/Linux): SIGSEGV, SIGABRT, SIGFPE, SIGILL, SIGBUS
  - Windows `SetUnhandledExceptionFilter` with exception code capture
  - Stack trace via `backtrace_symbols` on POSIX
  - Plain-text report written to platform log directory with timestamp
  - `CrashLogger::logWarning()` for non-fatal diagnostic notes
  - Installed early in `JUCEApplication::initialise()`
- Project documentation: added `README.md`, `LICENSE.txt` (MIT)

### Files changed (M9)

- `CMakeLists.txt`
- `src/main.cpp`
- `src/diagnostics/CrashLogger.h` *(new)*
- `packaging/macos/Info.plist.in` *(new)*
- `packaging/macos/entitlements.plist` *(new)*
- `packaging/macos/notarize.sh` *(new)*
- `packaging/windows/installer.nsi` *(new)*
- `packaging/windows/build_installer.bat` *(new)*
- `README.md` *(new)*
- `LICENSE.txt` *(new)*
- `docs/MILESTONE_LOG.md`

### Test evidence (M9)

- `cmake --build build -j` → `[100%] Built target ModularAudioPatcher`
- `ctest --output-on-failure` → 100% tests passed, 0 failed out of 6

### Known limitations (M9)

- Actual `.dmg` and `.exe` installer have not been produced from this machine (requires macOS packaging step `cmake --build build --target package` and a Windows host for NSIS)
- Installer icon placeholder (`installer.ico`) not yet bundled — NSIS will warn but still build
- Notarization requires valid Apple Developer ID credentials

### Next milestone checklist (M10)

- Full manual QA pass against `docs/QA_CHECKLIST.md`
- Validate callback CPU budget stays within 40% on target hardware
- Smoke-test save/load/undo/redo round-trip
- Check all quality gates from EXECUTION_PLAN sections 17–19
