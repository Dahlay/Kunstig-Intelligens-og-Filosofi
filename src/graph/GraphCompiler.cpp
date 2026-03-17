#include "GraphCompiler.h"

#include "audio/processors/IProcessor.h"

#include <algorithm>
#include <memory>
#include <queue>
#include <unordered_map>

namespace graph {
namespace {

using audio::GainProcessor;
using audio::IProcessor;
using audio::FilterProcessor;
using audio::DelayProcessor;
using audio::MixerProcessor;
using audio::OutputProcessor;
using audio::PassthroughProcessor;
using audio::DrumProcessor;
using audio::SynthProcessor;
using audio::ToneProcessor;

std::unique_ptr<IProcessor> makeProcessor(const Node& node, double sampleRate) {
  switch (node.type) {
    case NodeType::Synth:
      return std::make_unique<SynthProcessor>(8, node.synthWaveform, node.synthChord, node.synthRateDivision);
    case NodeType::Drum:
      return std::make_unique<DrumProcessor>();
    case NodeType::Gain:
      return std::make_unique<GainProcessor>(node.gain);
    case NodeType::Output:
      return std::make_unique<OutputProcessor>();
    case NodeType::Filter:
      return std::make_unique<FilterProcessor>(node.filterCutoffHz, sampleRate);
    case NodeType::Delay:
      return std::make_unique<DelayProcessor>(sampleRate, node.delayMs, node.delayFeedback, node.delayMix);
    case NodeType::Mixer:
      return std::make_unique<MixerProcessor>(2);
  }

  return std::make_unique<ToneProcessor>(220.0f, sampleRate);
}

}  // namespace

std::shared_ptr<audio::RuntimePlan> GraphCompiler::compile(const PatchGraph& graph, double sampleRate,
                                                           int blockSize, int channels,
                                                           std::string& error) {
  std::unordered_map<std::string, int> indegree;
  std::unordered_map<std::string, std::vector<std::string>> adjacency;

  for (const auto& [nodeId, _] : graph.getNodes()) {
    indegree[nodeId] = 0;
  }

  for (const auto& [_, edge] : graph.getEdges()) {
    const auto* fromPort = graph.findPort(edge.fromPortId);
    const auto* toPort = graph.findPort(edge.toPortId);
    if (fromPort == nullptr || toPort == nullptr) {
      continue;
    }
    adjacency[fromPort->nodeId].push_back(toPort->nodeId);
    indegree[toPort->nodeId] += 1;
  }

  std::queue<std::string> q;
  for (const auto& [nodeId, in] : indegree) {
    if (in == 0) {
      q.push(nodeId);
    }
  }

  std::vector<std::string> topo;
  while (!q.empty()) {
    auto current = q.front();
    q.pop();
    topo.push_back(current);

    for (const auto& next : adjacency[current]) {
      indegree[next] -= 1;
      if (indegree[next] == 0) {
        q.push(next);
      }
    }
  }

  if (topo.size() != graph.getNodes().size()) {
    error = "Graph contains an audio cycle";
    return nullptr;
  }

  auto plan = std::make_shared<audio::RuntimePlan>();
  plan->sampleRate = sampleRate;
  plan->blockSize = blockSize;
  plan->channels = channels;

  std::unordered_map<std::string, int> nodeIndex;
  int index = 0;
  for (const auto& nodeId : topo) {
    const auto* node = graph.findNode(nodeId);
    if (node == nullptr) {
      continue;
    }

    audio::RuntimeNode runtimeNode;
    runtimeNode.nodeId = node->id;
    runtimeNode.type = node->type;
    runtimeNode.gain = node->gain;
    runtimeNode.processor = makeProcessor(*node, sampleRate);
    runtimeNode.buffer.setSize(channels, blockSize, false, true, true);
    runtimeNode.inputMix.setSize(channels, blockSize, false, true, true);

    if (node->type == NodeType::Output) {
      plan->outputNodeIndex = index;
    }

    nodeIndex[node->id] = index;
    plan->nodes.push_back(std::move(runtimeNode));
    ++index;
  }

  for (auto& runtimeNode : plan->nodes) {
    const auto* node = graph.findNode(runtimeNode.nodeId);
    if (node == nullptr) {
      continue;
    }

    for (const auto& inputPortId : node->inputPortIds) {
      for (const auto& [_, edge] : graph.getEdges()) {
        if (edge.toPortId != inputPortId) {
          continue;
        }

        const auto* from = graph.findPort(edge.fromPortId);
        if (from == nullptr) {
          continue;
        }

        const auto it = nodeIndex.find(from->nodeId);
        if (it != nodeIndex.end()) {
          runtimeNode.inputNodeIndices.push_back(it->second);
        }
      }
    }
  }

  if (plan->outputNodeIndex < 0) {
    error = "No Output module in patch";
    return nullptr;
  }

  error.clear();
  return plan;
}

}  // namespace graph
