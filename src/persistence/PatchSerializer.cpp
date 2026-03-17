#include "PatchSerializer.h"

#include <nlohmann/json.hpp>

#include <array>
#include <optional>
#include <unordered_map>
#include <vector>

namespace persistence {

using nlohmann::json;

namespace {

std::optional<graph::NodeType> nodeTypeFromInt(int typeValue) {
  if (typeValue < static_cast<int>(graph::NodeType::Output) ||
      typeValue > static_cast<int>(graph::NodeType::Mixer)) {
    return std::nullopt;
  }

  return static_cast<graph::NodeType>(typeValue);
}

}  // namespace

std::string PatchSerializer::toJson(const graph::PatchGraph& graph) {
  json j;
  j["schemaVersion"] = 1;
  j["nodes"] = json::array();
  j["edges"] = json::array();

  for (const auto& [id, node] : graph.getNodes()) {
    j["nodes"].push_back({{"id", id},
                            {"type", static_cast<int>(node.type)},
                            {"x", node.position.x},
                            {"y", node.position.y},
                            {"gain", node.gain}});
  }

  for (const auto& [edgeId, edge] : graph.getEdges()) {
    const auto* fromPort = graph.findPort(edge.fromPortId);
    const auto* toPort = graph.findPort(edge.toPortId);
    if (!fromPort || !toPort) {
      continue;
    }

    const auto* fromNode = graph.findNode(fromPort->nodeId);
    const auto* toNode = graph.findNode(toPort->nodeId);
    if (!fromNode || !toNode) {
      continue;
    }

    int fromPortIndex = 0;
    int toPortIndex = 0;

    for (int i = 0; i < static_cast<int>(fromNode->outputPortIds.size()); ++i) {
      if (fromNode->outputPortIds[static_cast<size_t>(i)] == fromPort->id) {
        fromPortIndex = i;
        break;
      }
    }

    for (int i = 0; i < static_cast<int>(toNode->inputPortIds.size()); ++i) {
      if (toNode->inputPortIds[static_cast<size_t>(i)] == toPort->id) {
        toPortIndex = i;
        break;
      }
    }

    j["edges"].push_back({{"id", edgeId},
                           {"fromPortId", edge.fromPortId},
                           {"toPortId", edge.toPortId},
                           {"fromNodeId", fromNode->id},
                           {"toNodeId", toNode->id},
                           {"fromPortIndex", fromPortIndex},
                           {"toPortIndex", toPortIndex}});
  }

  return j.dump(2);
}

bool PatchSerializer::fromJson(const std::string& jsonText, graph::PatchGraph& outGraph, std::string& error) {
  json j;
  try {
    j = json::parse(jsonText);
  } catch (const std::exception& ex) {
    error = std::string("Invalid JSON: ") + ex.what();
    return false;
  }

  if (!j.contains("nodes") || !j["nodes"].is_array() || !j.contains("edges") ||
      !j["edges"].is_array()) {
    error = "Invalid patch schema";
    return false;
  }

  graph::PatchGraph graph;
  std::unordered_map<std::string, std::string> oldNodeIdToNewNodeId;

  for (const auto& n : j["nodes"]) {
    if (!n.contains("id") || !n.contains("type") || !n.contains("x") || !n.contains("y")) {
      error = "Invalid node entry";
      return false;
    }

    const auto typeOpt = nodeTypeFromInt(n["type"].get<int>());
    if (!typeOpt) {
      error = "Unknown node type in patch";
      return false;
    }

    const auto oldId = n["id"].get<std::string>();
    const auto newId = graph.addNode(*typeOpt, {n["x"].get<float>(), n["y"].get<float>()});

    if (auto* node = graph.findNode(newId); node && n.contains("gain")) {
      node->gain = n["gain"].get<float>();
    }

    oldNodeIdToNewNodeId[oldId] = newId;
  }

  for (const auto& e : j["edges"]) {
    if (!e.contains("fromNodeId") || !e.contains("toNodeId") || !e.contains("fromPortIndex") ||
        !e.contains("toPortIndex")) {
      continue;
    }

    const auto fromOld = e["fromNodeId"].get<std::string>();
    const auto toOld = e["toNodeId"].get<std::string>();
    const auto fromNodeIt = oldNodeIdToNewNodeId.find(fromOld);
    const auto toNodeIt = oldNodeIdToNewNodeId.find(toOld);
    if (fromNodeIt == oldNodeIdToNewNodeId.end() || toNodeIt == oldNodeIdToNewNodeId.end()) {
      continue;
    }

    const auto* fromNode = graph.findNode(fromNodeIt->second);
    const auto* toNode = graph.findNode(toNodeIt->second);
    if (!fromNode || !toNode) {
      continue;
    }

    const int fromIndex = e["fromPortIndex"].get<int>();
    const int toIndex = e["toPortIndex"].get<int>();
    if (fromIndex < 0 || toIndex < 0 || fromIndex >= static_cast<int>(fromNode->outputPortIds.size()) ||
        toIndex >= static_cast<int>(toNode->inputPortIds.size())) {
      continue;
    }

    std::string connectError;
    graph.connect(fromNode->outputPortIds[static_cast<size_t>(fromIndex)],
                  toNode->inputPortIds[static_cast<size_t>(toIndex)], connectError);
  }

  outGraph = std::move(graph);
  error.clear();
  return true;
}

}  // namespace persistence
