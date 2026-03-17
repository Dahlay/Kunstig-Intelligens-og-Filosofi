#pragma once

#include "graph/PatchGraph.h"

#include <juce_audio_basics/juce_audio_basics.h>

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
