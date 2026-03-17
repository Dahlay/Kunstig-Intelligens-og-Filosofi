#include "GraphValidator.h"

#include <unordered_set>

namespace graph {

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

  error.clear();
  return true;
}

}  // namespace graph
