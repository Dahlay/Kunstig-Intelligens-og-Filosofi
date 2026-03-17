#include "PatchSerializer.h"

#include <nlohmann/json.hpp>

namespace persistence {

using nlohmann::json;

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
    j["edges"].push_back({{"id", edgeId}, {"fromPortId", edge.fromPortId}, {"toPortId", edge.toPortId}});
  }

  return j.dump(2);
}

bool PatchSerializer::fromJson(const std::string&, graph::PatchGraph&, std::string& error) {
  error = "Deserialization not implemented yet (MVP scaffold)";
  return false;
}

}  // namespace persistence
