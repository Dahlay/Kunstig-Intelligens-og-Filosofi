#pragma once

#include "PatchGraph.h"

#include <string>

namespace graph {

class GraphValidator {
 public:
  static bool canConnect(const PatchGraph& graph, const std::string& fromPortId, const std::string& toPortId,
                         std::string& error);
};

}  // namespace graph
