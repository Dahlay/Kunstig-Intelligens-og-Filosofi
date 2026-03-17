#include "graph/GraphCompiler.h"
#include "graph/PatchGraph.h"
#include "persistence/PatchSerializer.h"

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

TEST_CASE("Validator rejects cycle-producing connection") {
  graph::PatchGraph graph;
  const auto gainAId = graph.addNode(graph::NodeType::Gain, {120.0f, 10.0f});
  const auto gainBId = graph.addNode(graph::NodeType::Gain, {240.0f, 10.0f});

  const auto* gainA = graph.findNode(gainAId);
  const auto* gainB = graph.findNode(gainBId);

  REQUIRE(gainA != nullptr);
  REQUIRE(gainB != nullptr);

  std::string error;
  REQUIRE(graph.connect(gainA->outputPortIds.front(), gainB->inputPortIds.front(), error));

  REQUIRE_FALSE(graph.connect(gainB->outputPortIds.front(), gainA->inputPortIds.front(), error));
  REQUIRE(error == "Connection would create a cycle");
}

TEST_CASE("Patch serializer roundtrip preserves graph shape") {
  graph::PatchGraph graph;
  const auto synthId = graph.addNode(graph::NodeType::Synth, {10.0f, 12.0f});
  const auto gainId = graph.addNode(graph::NodeType::Gain, {100.0f, 120.0f});
  const auto outId = graph.addNode(graph::NodeType::Output, {240.0f, 60.0f});

  const auto* synth = graph.findNode(synthId);
  const auto* gain = graph.findNode(gainId);
  const auto* out = graph.findNode(outId);
  REQUIRE(synth != nullptr);
  REQUIRE(gain != nullptr);
  REQUIRE(out != nullptr);

  std::string error;
  REQUIRE(graph.connect(synth->outputPortIds.front(), gain->inputPortIds.front(), error));
  REQUIRE(graph.connect(gain->outputPortIds.front(), out->inputPortIds.front(), error));

  const auto json = persistence::PatchSerializer::toJson(graph);

  graph::PatchGraph loaded;
  REQUIRE(persistence::PatchSerializer::fromJson(json, loaded, error));
  REQUIRE(error.empty());
  REQUIRE(loaded.getNodes().size() == graph.getNodes().size());
  REQUIRE(loaded.getEdges().size() == graph.getEdges().size());
}
