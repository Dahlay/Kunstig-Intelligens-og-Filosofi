#pragma once

#include <optional>
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

namespace graph {

enum class NodeType {
  Output,
  Synth,
  Drum,
  Gain,
  Filter,
  Delay,
  Mixer,
  Bass
};

enum class PortDirection { In, Out };
enum class SignalType { Audio };

struct Port {
  std::string id;
  std::string nodeId;
  PortDirection direction{};
  SignalType signalType{SignalType::Audio};
  int channels{2};
};

struct Node {
  struct Position {
    float x{0.0f};
    float y{0.0f};
  };

  std::string id;
  NodeType type{NodeType::Gain};
  Position position{};
  float gain{1.0f};
  float filterCutoffHz{1200.0f};
  float delayMs{250.0f};
  float delayFeedback{0.25f};
  float delayMix{0.35f};
  int synthWaveform{0};      // 0=sine, 1=saw, 2=square
  int synthChord{0};         // 0=lead/melody, >0 chord preset
  int synthRateDivision{1};  // 1=quarter, 2=eighth, 4=sixteenth
  int synthTemplate{0};      // 0=Soft Pad, 1=Pluck, 2=Organ, 3=Wide Motion
  bool synthUseMidiDraw{false};
  std::array<uint16_t, 16> synthStepMasks{{0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0}};
  int bassWaveform{1};     // 0=sine, 1=saw, 2=square
  int bassOctave{0};       // 0=C1-B1 range, 1=C2-B2 range
  int bassRateDivision{2}; // 1=quarter, 2=eighth, 4=sixteenth
  bool bassUseMidiDraw{false};
  std::array<uint16_t, 16> bassStepMasks{{0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0}};
  std::vector<std::string> inputPortIds;
  std::vector<std::string> outputPortIds;
};

struct Edge {
  std::string id;
  std::string fromPortId;
  std::string toPortId;
};

class PatchGraph {
 public:
  void clear();
  std::string addNode(NodeType type, Node::Position position);
  bool removeNode(const std::string& nodeId);

  bool connect(const std::string& fromPortId, const std::string& toPortId, std::string& error);
  bool disconnect(const std::string& edgeId);

  const std::unordered_map<std::string, Node>& getNodes() const noexcept { return nodes_; }
  const std::unordered_map<std::string, Port>& getPorts() const noexcept { return ports_; }
  const std::unordered_map<std::string, Edge>& getEdges() const noexcept { return edges_; }

  Node* findNode(const std::string& id);
  const Node* findNode(const std::string& id) const;
  Port* findPort(const std::string& id);
  const Port* findPort(const std::string& id) const;

 private:
  static std::string makeId();
  static std::vector<Port> makeDefaultPorts(NodeType type, const std::string& nodeId);

  std::unordered_map<std::string, Node> nodes_;
  std::unordered_map<std::string, Port> ports_;
  std::unordered_map<std::string, Edge> edges_;
};

}  // namespace graph
