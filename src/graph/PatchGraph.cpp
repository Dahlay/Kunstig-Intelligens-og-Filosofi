#include "PatchGraph.h"

#include "GraphValidator.h"
#include <juce_core/juce_core.h>

namespace graph {

void PatchGraph::clear() {
  nodes_.clear();
  ports_.clear();
  edges_.clear();
}

std::string PatchGraph::makeId() {
  return juce::Uuid().toString().toStdString();
}

std::vector<Port> PatchGraph::makeDefaultPorts(NodeType type, const std::string& nodeId) {
  std::vector<Port> ports;
  auto makePort = [&](PortDirection direction) {
    Port p;
    p.id = makeId();
    p.nodeId = nodeId;
    p.direction = direction;
    p.signalType = SignalType::Audio;
    return p;
  };

  switch (type) {
    case NodeType::Output:
      for (int i = 0; i < 12; ++i) {
        ports.push_back(makePort(PortDirection::In));
      }
      break;
    case NodeType::Synth:
    case NodeType::Drum:
    case NodeType::Bass:
      ports.push_back(makePort(PortDirection::Out));
      break;
    case NodeType::Gain:
    case NodeType::Filter:
    case NodeType::Delay:
      ports.push_back(makePort(PortDirection::In));
      ports.push_back(makePort(PortDirection::Out));
      break;
    case NodeType::Mixer:
      ports.push_back(makePort(PortDirection::In));
      ports.push_back(makePort(PortDirection::In));
      ports.push_back(makePort(PortDirection::Out));
      break;
  }

  return ports;
}

std::string PatchGraph::addNode(NodeType type, Node::Position position) {
  Node node;
  node.id = makeId();
  node.type = type;
  node.position = position;

  auto defaultPorts = makeDefaultPorts(type, node.id);
  for (const auto& port : defaultPorts) {
    ports_[port.id] = port;
    if (port.direction == PortDirection::In) {
      node.inputPortIds.push_back(port.id);
    } else {
      node.outputPortIds.push_back(port.id);
    }
  }

  nodes_[node.id] = node;
  return node.id;
}

bool PatchGraph::removeNode(const std::string& nodeId) {
  auto it = nodes_.find(nodeId);
  if (it == nodes_.end()) {
    return false;
  }

  std::vector<std::string> edgeIdsToDelete;
  for (const auto& [edgeId, edge] : edges_) {
    auto fromPortIt = ports_.find(edge.fromPortId);
    auto toPortIt = ports_.find(edge.toPortId);
    if (fromPortIt != ports_.end() && fromPortIt->second.nodeId == nodeId) {
      edgeIdsToDelete.push_back(edgeId);
    } else if (toPortIt != ports_.end() && toPortIt->second.nodeId == nodeId) {
      edgeIdsToDelete.push_back(edgeId);
    }
  }

  for (const auto& edgeId : edgeIdsToDelete) {
    edges_.erase(edgeId);
  }

  for (const auto& portId : it->second.inputPortIds) {
    ports_.erase(portId);
  }
  for (const auto& portId : it->second.outputPortIds) {
    ports_.erase(portId);
  }

  nodes_.erase(it);
  return true;
}

bool PatchGraph::connect(const std::string& fromPortId, const std::string& toPortId, std::string& error) {
  if (!GraphValidator::canConnect(*this, fromPortId, toPortId, error)) {
    return false;
  }

  Edge edge;
  edge.id = makeId();
  edge.fromPortId = fromPortId;
  edge.toPortId = toPortId;
  edges_[edge.id] = edge;
  return true;
}

bool PatchGraph::disconnect(const std::string& edgeId) {
  return edges_.erase(edgeId) > 0;
}

Node* PatchGraph::findNode(const std::string& id) {
  auto it = nodes_.find(id);
  return it == nodes_.end() ? nullptr : &it->second;
}

const Node* PatchGraph::findNode(const std::string& id) const {
  auto it = nodes_.find(id);
  return it == nodes_.end() ? nullptr : &it->second;
}

Port* PatchGraph::findPort(const std::string& id) {
  auto it = ports_.find(id);
  return it == ports_.end() ? nullptr : &it->second;
}

const Port* PatchGraph::findPort(const std::string& id) const {
  auto it = ports_.find(id);
  return it == ports_.end() ? nullptr : &it->second;
}

}  // namespace graph
