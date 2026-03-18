#pragma once

#include "graph/PatchGraph.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cmath>
#include <vector>

namespace audio {

struct ProcessContext {
  int numSamples{0};
  double sampleRate{48000.0};
  double bpm{120.0};
  bool playing{true};
  long long samplePosition{0};
};

class IProcessor {
 public:
  virtual ~IProcessor() = default;
  virtual void process(const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& output,
                       const ProcessContext& context) = 0;
};

class PassthroughProcessor final : public IProcessor {
 public:
  void process(const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& output,
               const ProcessContext& context) override {
    const int numSamples = context.numSamples;
    output.clear();
    for (int ch = 0; ch < output.getNumChannels(); ++ch) {
      output.copyFrom(ch, 0, input, ch, 0, numSamples);
    }
  }
};

class OutputProcessor final : public IProcessor {
 public:
  void process(const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& output,
               const ProcessContext& context) override {
    const int numSamples = context.numSamples;
    output.clear();
    for (int ch = 0; ch < output.getNumChannels(); ++ch) {
      output.copyFrom(ch, 0, input, ch, 0, numSamples);
    }
  }
};

class GainProcessor final : public IProcessor {
 public:
  explicit GainProcessor(float gain) : gain_(gain) {}

  void process(const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& output,
               const ProcessContext& context) override {
    const int numSamples = context.numSamples;
    output.clear();
    for (int ch = 0; ch < output.getNumChannels(); ++ch) {
      output.copyFrom(ch, 0, input, ch, 0, numSamples);
      output.applyGain(ch, 0, numSamples, gain_);
    }
  }

 private:
  float gain_{1.0f};
};

class FilterProcessor final : public IProcessor {
 public:
  FilterProcessor(float cutoffHz, double sampleRate)
      : cutoffHz_(cutoffHz), sampleRate_(sampleRate), state_(2, 0.0f) {}

  void process(const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& output,
               const ProcessContext& context) override {
    const int numSamples = context.numSamples;
    output.clear();
    sampleRate_ = context.sampleRate;

    const auto alpha = calculateAlpha();
    const int channels = output.getNumChannels();
    if (static_cast<int>(state_.size()) < channels) {
      state_.resize(static_cast<size_t>(channels), 0.0f);
    }

    for (int ch = 0; ch < channels; ++ch) {
      auto* out = output.getWritePointer(ch);
      const auto* in = input.getReadPointer(ch);
      float z = state_[static_cast<size_t>(ch)];

      for (int s = 0; s < numSamples; ++s) {
        z += alpha * (in[s] - z);
        out[s] = z;
      }

      state_[static_cast<size_t>(ch)] = z;
    }
  }

 private:
  float calculateAlpha() const {
    const float wc = 2.0f * juce::MathConstants<float>::pi * cutoffHz_;
    const float dt = 1.0f / static_cast<float>(sampleRate_);
    const float x = wc * dt;
    return std::clamp(x / (x + 1.0f), 0.0f, 1.0f);
  }

  float cutoffHz_{1200.0f};
  double sampleRate_{48000.0};
  std::vector<float> state_;
};

class DelayProcessor final : public IProcessor {
 public:
  DelayProcessor(double sampleRate, float delayMs = 250.0f, float feedback = 0.25f, float mix = 0.35f)
      : sampleRate_(sampleRate), delayMs_(delayMs), feedback_(feedback), mix_(mix) {
    allocateDelayBuffers();
  }

  void process(const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& output,
               const ProcessContext& context) override {
    const int numSamples = context.numSamples;
    if (std::abs(context.sampleRate - sampleRate_) > 1.0e-6) {
      sampleRate_ = context.sampleRate;
      allocateDelayBuffers();
    }

    output.clear();

    const int channels = output.getNumChannels();
    if (static_cast<int>(delayLine_.size()) < channels) {
      delayLine_.resize(static_cast<size_t>(channels), std::vector<float>(static_cast<size_t>(maxDelaySamples_), 0.0f));
      writePos_.resize(static_cast<size_t>(channels), 0);
    }

    const int delaySamples = std::clamp(static_cast<int>(delayMs_ * 0.001f * static_cast<float>(sampleRate_)), 1,
                                        maxDelaySamples_ - 1);

    for (int ch = 0; ch < channels; ++ch) {
      auto* out = output.getWritePointer(ch);
      const auto* in = input.getReadPointer(ch);
      auto& line = delayLine_[static_cast<size_t>(ch)];
      int write = writePos_[static_cast<size_t>(ch)];

      for (int s = 0; s < numSamples; ++s) {
        int read = write - delaySamples;
        if (read < 0) {
          read += maxDelaySamples_;
        }

        const float delayed = line[static_cast<size_t>(read)];
        const float dry = in[s];
        line[static_cast<size_t>(write)] = dry + delayed * feedback_;
        out[s] = dry * (1.0f - mix_) + delayed * mix_;

        ++write;
        if (write >= maxDelaySamples_) {
          write = 0;
        }
      }

      writePos_[static_cast<size_t>(ch)] = write;
    }
  }

 private:
  void allocateDelayBuffers() {
    maxDelaySamples_ = std::max(2, static_cast<int>(sampleRate_ * 2.0));
    delayLine_.assign(2, std::vector<float>(static_cast<size_t>(maxDelaySamples_), 0.0f));
    writePos_.assign(2, 0);
  }

  double sampleRate_{48000.0};
  float delayMs_{250.0f};
  float feedback_{0.25f};
  float mix_{0.35f};

  int maxDelaySamples_{96000};
  std::vector<std::vector<float>> delayLine_;
  std::vector<int> writePos_;
};

class MixerProcessor final : public IProcessor {
 public:
  explicit MixerProcessor(int numInputs) {
    const float n = static_cast<float>(std::max(1, numInputs));
    normaliseGain_ = 1.0f / std::sqrt(n);
  }

  void process(const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& output,
               const ProcessContext& context) override {
    const int numSamples = context.numSamples;
    output.clear();
    for (int ch = 0; ch < output.getNumChannels(); ++ch) {
      output.copyFrom(ch, 0, input, ch, 0, numSamples);
      output.applyGain(ch, 0, numSamples, normaliseGain_);
    }
  }

 private:
  float normaliseGain_{1.0f};
};

class ToneProcessor final : public IProcessor {
 public:
  ToneProcessor(float frequency, double sampleRate) : frequency_(frequency), sampleRate_(sampleRate) {}

  void process(const juce::AudioBuffer<float>&, juce::AudioBuffer<float>& output,
               const ProcessContext& context) override {
    const int numSamples = context.numSamples;
    sampleRate_ = context.sampleRate;
    output.clear();
    const float phaseInc = static_cast<float>((2.0 * juce::MathConstants<double>::pi * frequency_) / sampleRate_);

    for (int s = 0; s < numSamples; ++s) {
      const float value = 0.15f * std::sin(phase_);
      for (int ch = 0; ch < output.getNumChannels(); ++ch) {
        output.setSample(ch, s, value);
      }
      phase_ += phaseInc;
      if (phase_ > 2.0f * juce::MathConstants<float>::pi) {
        phase_ -= 2.0f * juce::MathConstants<float>::pi;
      }
    }
  }

 private:
  float frequency_{220.0f};
  double sampleRate_{48000.0};
  float phase_{0.0f};
};

class SynthProcessor final : public IProcessor {
 public:
  SynthProcessor(int maxVoices = 8, int waveform = 0, int chord = 0, int rateDivision = 1,
                 int synthTemplate = 0,
                 bool useMidiDraw = false,
                 const std::array<uint16_t, 16>& stepMasks = {{0, 0, 0, 0, 0, 0, 0, 0,
                                                               0, 0, 0, 0, 0, 0, 0, 0}})
      : voices_(static_cast<size_t>(std::max(1, maxVoices))),
        waveform_(std::clamp(waveform, 0, 2)),
        chord_(std::clamp(chord, 0, 6)),
        rateDivision_(rateDivision <= 0 ? 1 : rateDivision),
        template_(std::clamp(synthTemplate, 0, 3)),
        useMidiDraw_(useMidiDraw),
        stepMasks_(stepMasks) {}

  void process(const juce::AudioBuffer<float>&, juce::AudioBuffer<float>& output,
               const ProcessContext& context) override {
    output.clear();

    if (!context.playing) {
      releaseAll();
    }

    const int samplesPerBeat =
        std::max(1, static_cast<int>(context.sampleRate * (60.0 / std::max(1.0, context.bpm))));
    const int samplesPerEvent = std::max(1, samplesPerBeat / std::max(1, rateDivision_));

    for (int s = 0; s < context.numSamples; ++s) {
      if (context.playing && ((context.samplePosition + s) % samplesPerEvent == 0)) {
        if (useMidiDraw_) {
          triggerDrawStep();
        } else if (chord_ == 0) {
          triggerNextMelodyNote();
        } else {
          triggerChord(chord_);
        }
      }

      float sampleValue = 0.0f;
      for (auto& v : voices_) {
        if (!v.active && v.env <= 1.0e-5f) {
          continue;
        }

        const auto cfg = templateConfig(template_);
        if (v.releasing) {
          v.env -= cfg.release;
          if (v.env <= 0.0f) {
            v.env = 0.0f;
            v.active = false;
            v.releasing = false;
            continue;
          }
        } else {
          v.env = std::min(1.0f, v.env + cfg.attack);
        }

        const float inc = static_cast<float>((2.0 * juce::MathConstants<double>::pi * v.freq) / context.sampleRate);
        const float amp = cfg.level * v.env;
        sampleValue += renderWave(v.phase) * amp;
        v.phase += inc;
        if (v.phase > 2.0f * juce::MathConstants<float>::pi) {
          v.phase -= 2.0f * juce::MathConstants<float>::pi;
        }
      }

      for (int ch = 0; ch < output.getNumChannels(); ++ch) {
        output.setSample(ch, s, sampleValue);
      }
    }
  }

 private:
  struct Voice {
    float freq{220.0f};
    float phase{0.0f};
    float env{0.0f};
    bool active{false};
    bool releasing{false};
  };

  struct TemplateConfig {
    float attack;
    float release;
    float level;
  };

  static TemplateConfig templateConfig(int t) {
    switch (t) {
      case 1:  // Pluck
        return {0.010f, 0.0105f, 0.070f};
      case 2:  // Organ
        return {0.006f, 0.0014f, 0.055f};
      case 3:  // Wide Motion
        return {0.0023f, 0.0021f, 0.062f};
      case 0:
      default:  // Soft Pad
        return {0.0014f, 0.0012f, 0.058f};
    }
  }

  void releaseAll() {
    for (auto& v : voices_) {
      if (v.active) {
        v.releasing = true;
      }
    }
  }

  void triggerVoice(float frequency) {
    auto& v = voices_[voiceCursor_ % voices_.size()];
    v.freq = frequency;
    v.phase = 0.0f;
    v.env = 0.0f;
    v.active = true;
    v.releasing = false;
    ++voiceCursor_;
  }

  float renderWave(float phase) const {
    switch (waveform_) {
      case 1: {  // saw
        const float norm = phase / juce::MathConstants<float>::twoPi;
        return (norm * 2.0f) - 1.0f;
      }
      case 2:  // square
        return phase < juce::MathConstants<float>::pi ? 1.0f : -1.0f;
      case 0:
      default:
        return std::sin(phase);
    }
  }

  void triggerNextMelodyNote() {
    static constexpr std::array<float, 8> notes = {110.0f, 138.59f, 164.81f, 220.0f,
                                                    261.63f, 329.63f, 392.0f, 440.0f};

    triggerVoice(notes[noteCursor_ % notes.size()]);
    ++noteCursor_;

    releaseSustainedExceptNewest(1);
  }

  void triggerDrawStep() {
    const int step = stepIndex_ % static_cast<int>(stepMasks_.size());
    const uint16_t mask = stepMasks_[static_cast<size_t>(step)];
    ++stepIndex_;

    if (mask == 0u) {
      releaseAll();
      return;
    }

    int notesTriggered = 0;
    for (int semitone = 0; semitone < 12; ++semitone) {
      if ((mask & static_cast<uint16_t>(1u << semitone)) == 0u) {
        continue;
      }
      const int midiNote = 60 + semitone;
      const float hz = 440.0f * std::pow(2.0f, (static_cast<float>(midiNote) - 69.0f) / 12.0f);
      triggerVoice(hz);
      ++notesTriggered;
    }
    releaseSustainedExceptNewest(notesTriggered);
  }

  void releaseSustainedExceptNewest(int keepNewestVoices) {
    if (keepNewestVoices <= 0 || voices_.empty()) {
      return;
    }
    const size_t keep = static_cast<size_t>(keepNewestVoices);
    const size_t total = voices_.size();
    for (size_t i = 0; i < total; ++i) {
      const size_t newestIndex = (voiceCursor_ + total - 1 - (i % total)) % total;
      const bool shouldKeep = i < keep;
      auto& v = voices_[newestIndex];
      if (!shouldKeep && v.active && !v.releasing) {
        v.releasing = true;
      }
    }
  }

  void triggerChord(int chordIndex) {
    // 1:Cmaj 2:Amin 3:Fmaj7 4:G7 5:Dmin 6:Emin7
    static constexpr std::array<std::array<float, 4>, 6> chords = {
        std::array<float, 4>{261.63f, 329.63f, 392.00f, 0.0f},   // C E G
        std::array<float, 4>{220.00f, 261.63f, 329.63f, 0.0f},   // A C E
        std::array<float, 4>{174.61f, 220.00f, 261.63f, 329.63f},// F A C E
        std::array<float, 4>{196.00f, 246.94f, 293.66f, 349.23f},// G B D F
        std::array<float, 4>{146.83f, 174.61f, 220.00f, 0.0f},   // D F A
        std::array<float, 4>{164.81f, 196.00f, 246.94f, 293.66f} // E G B D
    };

    const int idx = std::clamp(chordIndex, 1, 6) - 1;
    int notesTriggered = 0;
    for (float f : chords[static_cast<size_t>(idx)]) {
      if (f > 1.0f) {
        triggerVoice(f);
        ++notesTriggered;
      }
    }
    releaseSustainedExceptNewest(notesTriggered);
  }

  std::vector<Voice> voices_;
  int waveform_{0};
  int chord_{0};
  int rateDivision_{1};
  int template_{0};
  bool useMidiDraw_{false};
  std::array<uint16_t, 16> stepMasks_{{0, 0, 0, 0, 0, 0, 0, 0,
                                       0, 0, 0, 0, 0, 0, 0, 0}};
  int stepIndex_{0};
  size_t voiceCursor_{0};
  size_t noteCursor_{0};
};

// ─── Bass Processor ───────────────────────────────────────────────────────────
class BassProcessor final : public IProcessor {
 public:
  BassProcessor(int waveform = 1, int octave = 0, int rateDivision = 2,
                bool useMidiDraw = false,
                const std::array<uint16_t, 16>& stepMasks = {{0, 0, 0, 0, 0, 0, 0, 0,
                                                              0, 0, 0, 0, 0, 0, 0, 0}})
      : waveform_(std::clamp(waveform, 0, 2)),
        octave_(std::clamp(octave, 0, 1)),
        rateDivision_(rateDivision <= 0 ? 1 : rateDivision),
        useMidiDraw_(useMidiDraw),
        stepMasks_(stepMasks) {}

  void process(const juce::AudioBuffer<float>&, juce::AudioBuffer<float>& output,
               const ProcessContext& context) override {
    output.clear();

    if (!context.playing) {
      voices_[0].active = false; voices_[0].env = 0.0f;
      voices_[1].active = false; voices_[1].env = 0.0f;
      return;
    }

    const int samplesPerBeat =
        std::max(1, static_cast<int>(context.sampleRate * (60.0 / std::max(1.0, context.bpm))));
    const int samplesPerStep = std::max(1, samplesPerBeat / std::max(1, rateDivision_));

    for (int s = 0; s < context.numSamples; ++s) {
      if ((context.samplePosition + s) % samplesPerStep == 0) {
        if (useMidiDraw_) {
          triggerDrawStep();
        } else {
          triggerGrooveStep();
        }
      }

      float sampleVal = 0.0f;
      for (auto& v : voices_) {
        if (!v.active && v.env <= 1.0e-5f) continue;
        if (v.releasing) {
          v.env -= kRelease;
          if (v.env <= 0.0f) { v.env = 0.0f; v.active = false; v.releasing = false; continue; }
        } else {
          v.env = std::min(1.0f, v.env + kAttack);
        }
        const float inc = static_cast<float>(
            (2.0 * juce::MathConstants<double>::pi * v.freq) / context.sampleRate);
        sampleVal += renderWave(v.phase) * (kLevel * v.env);
        v.phase += inc;
        if (v.phase > juce::MathConstants<float>::twoPi)
          v.phase -= juce::MathConstants<float>::twoPi;
      }

      for (int ch = 0; ch < output.getNumChannels(); ++ch)
        output.setSample(ch, s, sampleVal);
    }
  }

 private:
  static constexpr float kAttack  = 0.040f;  // punchy ~0.5 ms
  static constexpr float kRelease = 0.0015f; // ~13 ms tail
  static constexpr float kLevel   = 0.072f;

  struct Voice {
    float freq{80.0f};
    float phase{0.0f};
    float env{0.0f};
    bool active{false};
    bool releasing{false};
  };

  float bassNoteHz(int semitone) const {
    const int midiNote = 24 + octave_ * 12 + semitone;
    return 440.0f * std::pow(2.0f, (static_cast<float>(midiNote) - 69.0f) / 12.0f);
  }

  void triggerVoice(float hz) {
    auto& v  = voices_[cursor_ & 1u];
    auto& ov = voices_[(cursor_ + 1u) & 1u];
    v.freq = hz; v.phase = 0.0f; v.env = 0.0f;
    v.active = true; v.releasing = false;
    if (ov.active && !ov.releasing) ov.releasing = true;
    ++cursor_;
  }

  void triggerGrooveStep() {
    // 16-step walking groove: C root, G fifth, F fourth, with rests
    static constexpr std::array<int, 16> kGroove = {
        0, -1, 7, -1, 0, -1, 5, -1, 0, -1, 7, -1, 0, -1, 7, 4};
    const int semi = kGroove[static_cast<size_t>(grooveStep_ % 16)];
    ++grooveStep_;
    if (semi < 0) {
      for (auto& v : voices_) { if (v.active && !v.releasing) v.releasing = true; }
      return;
    }
    triggerVoice(bassNoteHz(semi));
  }

  void triggerDrawStep() {
    const int step = drawStep_ % static_cast<int>(stepMasks_.size());
    const uint16_t mask = stepMasks_[static_cast<size_t>(step)];
    ++drawStep_;
    if (mask == 0u) {
      for (auto& v : voices_) { if (v.active && !v.releasing) v.releasing = true; }
      return;
    }
    for (int semi = 0; semi < 12; ++semi) {
      if (mask & static_cast<uint16_t>(1u << semi))
        triggerVoice(bassNoteHz(semi));
    }
  }

  float renderWave(float phase) const {
    switch (waveform_) {
      case 1: return (phase / juce::MathConstants<float>::twoPi) * 2.0f - 1.0f;  // saw
      case 2: return phase < juce::MathConstants<float>::pi ? 1.0f : -1.0f;       // square
      default: return std::sin(phase);
    }
  }

  int waveform_{1};
  int octave_{0};
  int rateDivision_{2};
  bool useMidiDraw_{false};
  std::array<uint16_t, 16> stepMasks_{{0, 0, 0, 0, 0, 0, 0, 0,
                                       0, 0, 0, 0, 0, 0, 0, 0}};
  Voice voices_[2]{};
  unsigned int cursor_{0};
  int grooveStep_{0};
  int drawStep_{0};
};

class DrumProcessor final : public IProcessor {
 public:
  void process(const juce::AudioBuffer<float>&, juce::AudioBuffer<float>& output,
               const ProcessContext& context) override {
    output.clear();

    if (!context.playing) {
      kickEnv_ = 0.0f;
      snareEnv_ = 0.0f;
      hatEnv_ = 0.0f;
      return;
    }

    const int samplesPerStep =
        std::max(1, static_cast<int>(context.sampleRate * (60.0 / std::max(1.0, context.bpm)) / 4.0));

    for (int s = 0; s < context.numSamples; ++s) {
      const auto absoluteSample = context.samplePosition + s;
      if (absoluteSample % samplesPerStep == 0) {
        const int step = static_cast<int>((absoluteSample / samplesPerStep) % 16);
        triggerStep(step);
      }

      const float kick = processKick(context.sampleRate);
      const float snare = processSnare();
      const float hat = processHat();
      const float mix = kick + snare + hat;

      for (int ch = 0; ch < output.getNumChannels(); ++ch) {
        output.setSample(ch, s, mix);
      }
    }
  }

 private:
  void triggerStep(int step) {
    if (step == 0 || step == 4 || step == 8 || step == 12) {
      kickEnv_ = 1.0f;
      kickPhase_ = 0.0f;
    }
    if (step == 4 || step == 12) {
      snareEnv_ = 1.0f;
    }
    if ((step % 2) == 0) {
      hatEnv_ = 0.8f;
    }
  }

  float processKick(double sampleRate) {
    if (kickEnv_ <= 1.0e-5f) {
      return 0.0f;
    }

    const float freq = 42.0f + kickEnv_ * 130.0f;
    const float inc = static_cast<float>((2.0 * juce::MathConstants<double>::pi * freq) / sampleRate);
    const float out = std::sin(kickPhase_) * (0.45f * kickEnv_);
    kickPhase_ += inc;
    if (kickPhase_ > 2.0f * juce::MathConstants<float>::pi) {
      kickPhase_ -= 2.0f * juce::MathConstants<float>::pi;
    }
    kickEnv_ *= 0.9965f;
    return out;
  }

  float processSnare() {
    if (snareEnv_ <= 1.0e-5f) {
      return 0.0f;
    }
    snareEnv_ *= 0.989f;
    return (nextNoise() * 2.0f - 1.0f) * (0.23f * snareEnv_);
  }

  float processHat() {
    if (hatEnv_ <= 1.0e-5f) {
      return 0.0f;
    }
    hatEnv_ *= 0.962f;
    return (nextNoise() * 2.0f - 1.0f) * (0.12f * hatEnv_);
  }

  float nextNoise() {
    rng_ = rng_ * 1664525u + 1013904223u;
    return static_cast<float>((rng_ >> 8) & 0x00FFFFFFu) / static_cast<float>(0x00FFFFFFu);
  }

  float kickPhase_{0.0f};
  float kickEnv_{0.0f};
  float snareEnv_{0.0f};
  float hatEnv_{0.0f};
  unsigned int rng_{22222u};
};

}  // namespace audio
