#include "AudioEngine.h"

#include "graph/GraphCompiler.h"

#include <algorithm>
#include <atomic>

namespace audio {

void AudioEngine::prepare(double sampleRate, int blockSize, int channels) {
  sampleRate_ = sampleRate;
  blockSize_ = blockSize;
  channels_ = channels;
}

bool AudioEngine::setGraph(const graph::PatchGraph& graph, std::string& error) {
  auto plan = graph::GraphCompiler::compile(graph, sampleRate_, blockSize_, channels_, error);
  if (!plan) {
    return false;
  }

  std::atomic_store(&activePlan_, std::move(plan));
  return true;
}

void AudioEngine::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
  auto plan = std::atomic_load(&activePlan_);
  buffer.clear();

  if (!plan) {
    return;
  }

  const int numSamples = std::min(buffer.getNumSamples(), plan->blockSize);

  for (auto& node : plan->nodes) {
    node.inputMix.clear();

    for (const auto inputNodeIndex : node.inputNodeIndices) {
      if (inputNodeIndex < 0 || inputNodeIndex >= static_cast<int>(plan->nodes.size())) {
        continue;
      }

      auto& source = plan->nodes[static_cast<size_t>(inputNodeIndex)].buffer;
      for (int ch = 0; ch < node.inputMix.getNumChannels(); ++ch) {
        node.inputMix.addFrom(ch, 0, source, ch, 0, numSamples);
      }
    }

    node.processor->process(node.inputMix, node.buffer, numSamples, plan->sampleRate);
  }

  if (plan->outputNodeIndex >= 0 && plan->outputNodeIndex < static_cast<int>(plan->nodes.size())) {
    auto& out = plan->nodes[static_cast<size_t>(plan->outputNodeIndex)].buffer;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
      buffer.copyFrom(ch, 0, out, ch, 0, numSamples);
    }
  }
}

}  // namespace audio
