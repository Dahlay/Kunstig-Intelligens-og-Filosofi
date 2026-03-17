#include "graph/GraphCompiler.h"
#include "graph/PatchGraph.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Graph can connect output to input") {
  graph::PatchGraph graph;
  const auto outNode = graph.addNode(graph::NodeType::Synth, {10.0f, 10.0f});
  const auto inNode = graph.addNode(graph::NodeType::Output, {200.0f, 10.0f});

  const auto* fromNode = graph.findNode(outNode);
  const auto* toNode = graph.findNode(inNode);
  REQUIRE(fromNode != nullptr);
  REQUIRE(toNode != nullptr);
  REQUIRE_FALSE(fromNode->outputPortIds.empty());
  REQUIRE_FALSE(toNode->inputPortIds.empty());

  std::string error;
  REQUIRE(graph.connect(fromNode->outputPortIds.front(), toNode->inputPortIds.front(), error));
  REQUIRE(error.empty());
}

TEST_CASE("Compiler rejects graph without output") {
  graph::PatchGraph graph;
  graph.addNode(graph::NodeType::Synth, {10.0f, 10.0f});

  std::string error;
  auto plan = graph::GraphCompiler::compile(graph, 48000.0, 256, 2, error);
  REQUIRE(plan == nullptr);
  REQUIRE_FALSE(error.empty());
}
