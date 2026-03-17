#pragma once

#include "audio/processors/IProcessor.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <memory>
#include <string>
#include <vector>

namespace audio {

struct RuntimeNode {
  std::string nodeId;
  graph::NodeType type{graph::NodeType::Gain};
  float gain{1.0f};
  std::vector<int> inputNodeIndices;
  std::unique_ptr<IProcessor> processor;
  juce::AudioBuffer<float> inputMix;
  juce::AudioBuffer<float> buffer;
};

struct RuntimePlan {
  double sampleRate{48000.0};
  int blockSize{512};
  int channels{2};
  int outputNodeIndex{-1};
  std::vector<RuntimeNode> nodes;
};

}  // namespace audio
