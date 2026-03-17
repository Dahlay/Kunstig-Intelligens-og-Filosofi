#pragma once

#include "audio/RuntimePlan.h"
#include "graph/PatchGraph.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <memory>
#include <string>
#include <algorithm>

namespace audio {

class AudioEngine {
 public:
  void prepare(double sampleRate, int blockSize, int channels);
  bool setGraph(const graph::PatchGraph& graph, std::string& error);

  void setTransportPlaying(bool playing) noexcept { playing_ = playing; }
  void setBpm(double bpm) noexcept { bpm_ = std::clamp(bpm, 40.0, 240.0); }
  bool isTransportPlaying() const noexcept { return playing_; }
  double getBpm() const noexcept { return bpm_; }

  void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi);

 private:
  double sampleRate_{48000.0};
  int blockSize_{512};
  int channels_{2};
  bool playing_{true};
  double bpm_{120.0};
  long long samplePosition_{0};

  std::shared_ptr<RuntimePlan> activePlan_;
};

}  // namespace audio
