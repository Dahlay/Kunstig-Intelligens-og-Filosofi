#include "GraphValidator.h"

#include <stack>
#include <unordered_map>
#include <unordered_set>

namespace graph {
namespace {

bool wouldCreateCycle(const PatchGraph& graph, const std::string& fromNodeId,
                      const std::string& toNodeId) {
  std::unordered_map<std::string, std::vector<std::string>> adjacency;
  for (const auto& [nodeId, _] : graph.getNodes()) {
    adjacency[nodeId] = {};
  }

  for (const auto& [_, edge] : graph.getEdges()) {
    const auto* fromPort = graph.findPort(edge.fromPortId);
    const auto* toPort = graph.findPort(edge.toPortId);
    if (!fromPort || !toPort) {
      continue;
    }
    adjacency[fromPort->nodeId].push_back(toPort->nodeId);
  }

  adjacency[fromNodeId].push_back(toNodeId);

  std::unordered_set<std::string> visited;
  std::stack<std::string> stack;
  stack.push(toNodeId);

  while (!stack.empty()) {
    auto current = stack.top();
    stack.pop();

    if (current == fromNodeId) {
      return true;
    }

    if (!visited.insert(current).second) {
      continue;
    }

    for (const auto& next : adjacency[current]) {
      stack.push(next);
    }
  }

  return false;
}

}  // namespace

bool GraphValidator::canConnect(const PatchGraph& graph, const std::string& fromPortId,
                                const std::string& toPortId, std::string& error) {
  const auto* from = graph.findPort(fromPortId);
  const auto* to = graph.findPort(toPortId);

  if (from == nullptr || to == nullptr) {
    error = "Port not found";
    return false;
  }

  if (from->direction != PortDirection::Out || to->direction != PortDirection::In) {
    error = "Connection must be output -> input";
    return false;
  }

  if (from->signalType != to->signalType) {
    error = "Signal type mismatch";
    return false;
  }

  if (from->nodeId == to->nodeId) {
    error = "Cannot connect module to itself";
    return false;
  }

  for (const auto& [_, edge] : graph.getEdges()) {
    if (edge.fromPortId == fromPortId && edge.toPortId == toPortId) {
      error = "Duplicate connection";
      return false;
    }
    if (edge.toPortId == toPortId) {
      error = "Input already connected";
      return false;
    }
  }

  if (wouldCreateCycle(graph, from->nodeId, to->nodeId)) {
    error = "Connection would create a cycle";
    return false;
  }

  error.clear();
  return true;
}

}  // namespace graph
