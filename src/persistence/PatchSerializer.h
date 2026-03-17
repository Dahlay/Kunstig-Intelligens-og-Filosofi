#pragma once

#include "graph/PatchGraph.h"

#include <string>

namespace persistence {

class PatchSerializer {
 public:
  static std::string toJson(const graph::PatchGraph& graph);
  static bool fromJson(const std::string& jsonText, graph::PatchGraph& outGraph, std::string& error);
};

}  // namespace persistence
