#pragma once

#include "PatchGraph.h"
#include "audio/RuntimePlan.h"

#include <memory>
#include <string>

namespace graph {

class GraphCompiler {
 public:
  static std::shared_ptr<audio::RuntimePlan> compile(const PatchGraph& graph, double sampleRate,
                                                     int blockSize, int channels,
                                                     std::string& error);
};

}  // namespace graph
