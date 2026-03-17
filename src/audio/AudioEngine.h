#pragma once

#include "audio/RuntimePlan.h"
#include "graph/PatchGraph.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <memory>
#include <string>

namespace audio {

class AudioEngine {
 public:
  void prepare(double sampleRate, int blockSize, int channels);
  bool setGraph(const graph::PatchGraph& graph, std::string& error);

  void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi);

 private:
  double sampleRate_{48000.0};
  int blockSize_{512};
  int channels_{2};

  std::shared_ptr<RuntimePlan> activePlan_;
};

}  // namespace audio
