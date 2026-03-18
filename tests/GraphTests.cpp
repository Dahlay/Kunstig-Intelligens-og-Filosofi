#include "graph/GraphCompiler.h"
#include "graph/PatchGraph.h"
#include "persistence/PatchSerializer.h"
#include "audio/AudioEngine.h"

#include <catch2/catch_test_macros.hpp>

#include <cmath>

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
  const auto bassId = graph.addNode(graph::NodeType::Bass, {40.0f, 40.0f});
  const auto gainId = graph.addNode(graph::NodeType::Gain, {100.0f, 120.0f});
  const auto delayId = graph.addNode(graph::NodeType::Delay, {160.0f, 100.0f});
  const auto outId = graph.addNode(graph::NodeType::Output, {240.0f, 60.0f});

  const auto* synth = graph.findNode(synthId);
  auto* bass = graph.findNode(bassId);
  auto* gain = graph.findNode(gainId);
  auto* delay = graph.findNode(delayId);
  auto* synthMutable = graph.findNode(synthId);
  const auto* out = graph.findNode(outId);
  REQUIRE(synth != nullptr);
  REQUIRE(bass != nullptr);
  REQUIRE(synthMutable != nullptr);
  REQUIRE(gain != nullptr);
  REQUIRE(delay != nullptr);
  REQUIRE(out != nullptr);

  synthMutable->synthWaveform = 2;
  synthMutable->synthChord = 4;
  synthMutable->synthRateDivision = 4;
  synthMutable->synthTemplate = 3;
  synthMutable->synthUseMidiDraw = true;
  synthMutable->synthStepMasks = {{0b000000000001u,
                                   0u,
                                   0b000000010000u,
                                   0u,
                                   0b000010000000u,
                                   0u,
                                   0b001000000000u,
                                   0u,
                                   0b100000000000u,
                                   0u,
                                   0b010000000000u,
                                   0u,
                                   0b001000000000u,
                                   0u,
                                   0b000010000000u,
                                   0u}};
  bass->bassWaveform = 2;
  bass->bassOctave = 1;
  bass->bassRateDivision = 4;
  bass->bassUseMidiDraw = true;
  bass->bassStepMasks = {{0b000000000001u, 0u, 0b000000010000u, 0u,
                          0b000010000000u, 0u, 0b000001000000u, 0u,
                          0b000000000001u, 0u, 0b000000010000u, 0u,
                          0b000010000000u, 0u, 0b000001000000u, 0u}};
  gain->gain = 1.37f;
  delay->delayMs = 420.0f;
  delay->delayFeedback = 0.42f;
  delay->delayMix = 0.57f;

  std::string error;
  REQUIRE(graph.connect(synth->outputPortIds.front(), gain->inputPortIds.front(), error));
  REQUIRE(graph.connect(gain->outputPortIds.front(), delay->inputPortIds.front(), error));
  REQUIRE(graph.connect(delay->outputPortIds.front(), out->inputPortIds.front(), error));
  REQUIRE(graph.connect(bass->outputPortIds.front(), out->inputPortIds[1], error));

  const auto json = persistence::PatchSerializer::toJson(graph);

  graph::PatchGraph loaded;
  REQUIRE(persistence::PatchSerializer::fromJson(json, loaded, error));
  REQUIRE(error.empty());
  REQUIRE(loaded.getNodes().size() == graph.getNodes().size());
  REQUIRE(loaded.getEdges().size() == graph.getEdges().size());

  bool foundGain = false;
  bool foundDelay = false;
  bool foundSynth = false;
  bool foundBass = false;
  for (const auto& [_, node] : loaded.getNodes()) {
    if (node.type == graph::NodeType::Synth) {
      foundSynth = true;
      REQUIRE(node.synthWaveform == 2);
      REQUIRE(node.synthChord == 4);
      REQUIRE(node.synthRateDivision == 4);
      REQUIRE(node.synthTemplate == 3);
      REQUIRE(node.synthUseMidiDraw);
      REQUIRE(node.synthStepMasks[0] == 0b000000000001u);
      REQUIRE(node.synthStepMasks[1] == 0u);
      REQUIRE(node.synthStepMasks[8] == 0b100000000000u);
      REQUIRE(node.synthStepMasks[15] == 0u);
    }
    if (node.type == graph::NodeType::Bass) {
      foundBass = true;
      REQUIRE(node.bassWaveform == 2);
      REQUIRE(node.bassOctave == 1);
      REQUIRE(node.bassRateDivision == 4);
      REQUIRE(node.bassUseMidiDraw);
      REQUIRE(node.bassStepMasks[0] == 0b000000000001u);
      REQUIRE(node.bassStepMasks[6] == 0b000001000000u);
    }
    if (node.type == graph::NodeType::Gain) {
      foundGain = true;
      REQUIRE(std::abs(node.gain - 1.37f) < 1.0e-3f);
    }
    if (node.type == graph::NodeType::Delay) {
      foundDelay = true;
      REQUIRE(std::abs(node.delayMs - 420.0f) < 1.0e-3f);
      REQUIRE(std::abs(node.delayFeedback - 0.42f) < 1.0e-3f);
      REQUIRE(std::abs(node.delayMix - 0.57f) < 1.0e-3f);
    }
  }

  REQUIRE(foundSynth);
  REQUIRE(foundBass);
  REQUIRE(foundGain);
  REQUIRE(foundDelay);
}

TEST_CASE("Audio engine processes filter-delay-mixer graph") {
  graph::PatchGraph graph;
  const auto synthId = graph.addNode(graph::NodeType::Synth, {20.0f, 20.0f});
  const auto drumId = graph.addNode(graph::NodeType::Drum, {20.0f, 120.0f});
  const auto filterId = graph.addNode(graph::NodeType::Filter, {140.0f, 20.0f});
  const auto delayId = graph.addNode(graph::NodeType::Delay, {260.0f, 20.0f});
  const auto mixerId = graph.addNode(graph::NodeType::Mixer, {380.0f, 60.0f});
  const auto outId = graph.addNode(graph::NodeType::Output, {500.0f, 60.0f});

  const auto* synth = graph.findNode(synthId);
  const auto* drum = graph.findNode(drumId);
  const auto* filter = graph.findNode(filterId);
  const auto* delay = graph.findNode(delayId);
  const auto* mixer = graph.findNode(mixerId);
  const auto* out = graph.findNode(outId);

  REQUIRE(synth != nullptr);
  REQUIRE(drum != nullptr);
  REQUIRE(filter != nullptr);
  REQUIRE(delay != nullptr);
  REQUIRE(mixer != nullptr);
  REQUIRE(out != nullptr);

  std::string error;
  REQUIRE(graph.connect(synth->outputPortIds.front(), filter->inputPortIds.front(), error));
  REQUIRE(graph.connect(filter->outputPortIds.front(), delay->inputPortIds.front(), error));
  REQUIRE(graph.connect(delay->outputPortIds.front(), mixer->inputPortIds[0], error));
  REQUIRE(graph.connect(drum->outputPortIds.front(), mixer->inputPortIds[1], error));
  REQUIRE(graph.connect(mixer->outputPortIds.front(), out->inputPortIds.front(), error));

  audio::AudioEngine engine;
  engine.prepare(48000.0, 256, 2);
  REQUIRE(engine.setGraph(graph, error));
  REQUIRE(error.empty());

  juce::AudioBuffer<float> buffer(2, 256);
  juce::MidiBuffer midi;
  engine.processBlock(buffer, midi);

  bool anyFiniteNonZero = false;
  for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
    for (int s = 0; s < buffer.getNumSamples(); ++s) {
      const auto v = buffer.getSample(ch, s);
      REQUIRE(std::isfinite(v));
      if (std::abs(v) > 1.0e-6f) {
        anyFiniteNonZero = true;
      }
    }
  }

  REQUIRE(anyFiniteNonZero);
}

TEST_CASE("Synth MIDI draw mode overrides chord preset") {
  graph::PatchGraph graph;
  const auto synthId = graph.addNode(graph::NodeType::Synth, {20.0f, 20.0f});
  const auto outId = graph.addNode(graph::NodeType::Output, {220.0f, 20.0f});

  auto* synth = graph.findNode(synthId);
  const auto* out = graph.findNode(outId);
  REQUIRE(synth != nullptr);
  REQUIRE(out != nullptr);

  synth->synthChord = 4;  // should be ignored in MIDI draw mode
  synth->synthUseMidiDraw = true;
  synth->synthRateDivision = 4;
  synth->synthStepMasks = {{0b000000010001u,
                            0u,
                            0b000000110000u,
                            0u,
                            0b000011100000u,
                            0u,
                            0b001000000000u,
                            0u,
                            0b000011100000u,
                            0u,
                            0b000000110000u,
                            0u,
                            0b000000010001u,
                            0u,
                            0b000000000111u,
                            0u}};

  std::string error;
  REQUIRE(graph.connect(synth->outputPortIds.front(), out->inputPortIds.front(), error));

  audio::AudioEngine engine;
  engine.prepare(48000.0, 256, 2);
  engine.setBpm(120.0);
  engine.setTransportPlaying(true);
  REQUIRE(engine.setGraph(graph, error));

  juce::AudioBuffer<float> buffer(2, 256);
  juce::MidiBuffer midi;
  bool anyFiniteNonZero = false;

  for (int block = 0; block < 8; ++block) {
    buffer.clear();
    engine.processBlock(buffer, midi);
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
      for (int s = 0; s < buffer.getNumSamples(); ++s) {
        const auto v = buffer.getSample(ch, s);
        REQUIRE(std::isfinite(v));
        if (std::abs(v) > 1.0e-6f) {
          anyFiniteNonZero = true;
        }
      }
    }
  }

  REQUIRE(anyFiniteNonZero);
}

TEST_CASE("Transport stop produces silence") {
  graph::PatchGraph graph;
  const auto synthId = graph.addNode(graph::NodeType::Synth, {20.0f, 20.0f});
  const auto outId = graph.addNode(graph::NodeType::Output, {220.0f, 20.0f});

  const auto* synth = graph.findNode(synthId);
  const auto* out = graph.findNode(outId);
  REQUIRE(synth != nullptr);
  REQUIRE(out != nullptr);

  std::string error;
  REQUIRE(graph.connect(synth->outputPortIds.front(), out->inputPortIds.front(), error));

  audio::AudioEngine engine;
  engine.prepare(48000.0, 256, 2);
  engine.setBpm(120.0);
  engine.setTransportPlaying(false);
  REQUIRE(engine.setGraph(graph, error));

  juce::AudioBuffer<float> buffer(2, 256);
  juce::MidiBuffer midi;
  engine.processBlock(buffer, midi);

  for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
    for (int s = 0; s < buffer.getNumSamples(); ++s) {
      REQUIRE(std::abs(buffer.getSample(ch, s)) <= 1.0e-7f);
    }
  }
}
