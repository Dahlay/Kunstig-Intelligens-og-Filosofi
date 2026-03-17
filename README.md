# Modular Audio Patcher

A node-based modular audio workstation built with JUCE 8.

## Features

- Visual canvas: drag-and-drop audio nodes, connect ports with cables
- Real DSP: Gain, Filter (one-pole LPF), Delay (feedback + wet/dry), Mixer
- Synthesis: polyphonic Synth node, BPM-locked 16-step Drum sequencer
- Transport: Start/Stop + BPM control (40–240)
- Inspector: live parameter editing per selected node
- Persistence: save/load patches as JSON
- Undo/Redo (200-step history, Cmd+Z / Cmd+Shift+Z)

## Building (macOS)

```bash
brew install cmake
cmake -B build
cmake --build build -j
open "build/ModularAudioPatcher_artefacts/Debug/Modular Audio Patcher.app"
```

## Building (Windows)

Run `packaging\windows\build_installer.bat` from a Developer Command Prompt.

## Running Tests

```bash
cd build && ctest --output-on-failure
```

## Packaging

### macOS DMG

```bash
cmake --build build --target package
```

### Windows NSIS Installer

See `packaging/windows/build_installer.bat`.

## License

MIT — see [LICENSE.txt](LICENSE.txt).
