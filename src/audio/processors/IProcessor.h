#pragma once

#include "graph/PatchGraph.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <algorithm>
#include <array>
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
  explicit SynthProcessor(int maxVoices = 8) : voices_(static_cast<size_t>(std::max(1, maxVoices))) {}

  void process(const juce::AudioBuffer<float>&, juce::AudioBuffer<float>& output,
               const ProcessContext& context) override {
    output.clear();

    if (!context.playing) {
      releaseAll();
    }

    const int samplesPerBeat =
        std::max(1, static_cast<int>(context.sampleRate * (60.0 / std::max(1.0, context.bpm))));

    for (int s = 0; s < context.numSamples; ++s) {
      if (context.playing && ((context.samplePosition + s) % samplesPerBeat == 0)) {
        triggerNextNote();
      }

      float sampleValue = 0.0f;
      for (auto& v : voices_) {
        if (!v.active && v.env <= 1.0e-5f) {
          continue;
        }

        if (v.releasing) {
          v.env -= 0.0016f;
          if (v.env <= 0.0f) {
            v.env = 0.0f;
            v.active = false;
            v.releasing = false;
            continue;
          }
        } else {
          v.env = std::min(1.0f, v.env + 0.0025f);
        }

        const float inc = static_cast<float>((2.0 * juce::MathConstants<double>::pi * v.freq) / context.sampleRate);
        sampleValue += std::sin(v.phase) * (0.07f * v.env);
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

  void releaseAll() {
    for (auto& v : voices_) {
      if (v.active) {
        v.releasing = true;
      }
    }
  }

  void triggerNextNote() {
    static constexpr std::array<float, 8> notes = {110.0f, 138.59f, 164.81f, 220.0f,
                                                    261.63f, 329.63f, 392.0f, 440.0f};

    auto& v = voices_[voiceCursor_ % voices_.size()];
    v.freq = notes[noteCursor_ % notes.size()];
    v.phase = 0.0f;
    v.env = 0.0f;
    v.active = true;
    v.releasing = false;

    ++voiceCursor_;
    ++noteCursor_;

    for (auto& other : voices_) {
      if (&other != &v && other.active && !other.releasing) {
        other.releasing = true;
      }
    }
  }

  std::vector<Voice> voices_;
  size_t voiceCursor_{0};
  size_t noteCursor_{0};
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
