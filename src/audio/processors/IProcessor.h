#pragma once

#include "graph/PatchGraph.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <algorithm>
#include <cmath>
#include <vector>

namespace audio {

class IProcessor {
 public:
  virtual ~IProcessor() = default;
  virtual void process(const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& output,
                       int numSamples, double sampleRate) = 0;
};

class PassthroughProcessor final : public IProcessor {
 public:
  void process(const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& output, int numSamples,
               double) override {
    output.clear();
    for (int ch = 0; ch < output.getNumChannels(); ++ch) {
      output.copyFrom(ch, 0, input, ch, 0, numSamples);
    }
  }
};

class OutputProcessor final : public IProcessor {
 public:
  void process(const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& output, int numSamples,
               double) override {
    output.clear();
    for (int ch = 0; ch < output.getNumChannels(); ++ch) {
      output.copyFrom(ch, 0, input, ch, 0, numSamples);
    }
  }
};

class GainProcessor final : public IProcessor {
 public:
  explicit GainProcessor(float gain) : gain_(gain) {}

  void process(const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& output, int numSamples,
               double) override {
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

  void process(const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& output, int numSamples,
               double sampleRate) override {
    output.clear();
    sampleRate_ = sampleRate;

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

  void process(const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& output, int numSamples,
               double sampleRate) override {
    if (std::abs(sampleRate - sampleRate_) > 1.0e-6) {
      sampleRate_ = sampleRate;
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

  void process(const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& output, int numSamples,
               double) override {
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

  void process(const juce::AudioBuffer<float>&, juce::AudioBuffer<float>& output, int numSamples,
               double sampleRate) override {
    sampleRate_ = sampleRate;
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

}  // namespace audio
