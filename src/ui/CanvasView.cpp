#include "CanvasView.h"

#include <algorithm>

namespace ui {
namespace {
constexpr float kNodeWidth = 170.0f;
constexpr float kNodeHeight = 96.0f;
constexpr float kPortRadius = 6.0f;

juce::String nodeName(graph::NodeType t) {
  switch (t) {
    case graph::NodeType::Output:
      return "Output";
    case graph::NodeType::Synth:
      return "Synth";
    case graph::NodeType::Drum:
      return "Drum";
    case graph::NodeType::Gain:
      return "Gain";
    case graph::NodeType::Filter:
      return "Filter";
    case graph::NodeType::Delay:
      return "Delay";
    case graph::NodeType::Mixer:
      return "Mixer";
  }
  return "Node";
}
}  // namespace

CanvasView::CanvasView(graph::PatchGraph& graph) : graph_(graph) {}

juce::Rectangle<float> CanvasView::nodeRect(const graph::Node& node) const {
  return {node.position.x, node.position.y, kNodeWidth, kNodeHeight};
}

juce::Point<float> CanvasView::getPortPoint(const graph::Port& port) const {
  const auto* node = graph_.findNode(port.nodeId);
  if (!node) {
    return {};
  }

  const auto rect = nodeRect(*node);
  if (port.direction == graph::PortDirection::In) {
    auto idx = std::find(node->inputPortIds.begin(), node->inputPortIds.end(), port.id);
    const int i = idx == node->inputPortIds.end() ? 0 : static_cast<int>(std::distance(node->inputPortIds.begin(), idx));
    return {rect.getX(), rect.getY() + 32.0f + i * 18.0f};
  }

  auto idx = std::find(node->outputPortIds.begin(), node->outputPortIds.end(), port.id);
  const int i = idx == node->outputPortIds.end() ? 0 : static_cast<int>(std::distance(node->outputPortIds.begin(), idx));
  return {rect.getRight(), rect.getY() + 32.0f + i * 18.0f};
}

std::optional<std::string> CanvasView::hitNode(juce::Point<float> p) const {
  for (const auto& [nodeId, node] : graph_.getNodes()) {
    if (nodeRect(node).contains(p)) {
      return nodeId;
    }
  }
  return std::nullopt;
}

std::optional<CanvasView::PortHit> CanvasView::hitPort(juce::Point<float> p) const {
  for (const auto& [portId, port] : graph_.getPorts()) {
    const auto point = getPortPoint(port);
    if (point.getDistanceFrom(p) <= kPortRadius * 1.8f) {
      return PortHit{portId, point, port.direction};
    }
  }
  return std::nullopt;
}

std::optional<std::string> CanvasView::hitEdge(juce::Point<float> p) const {
  constexpr float kHitDistance = 8.0f;

  for (const auto& [edgeId, edge] : graph_.getEdges()) {
    const auto* from = graph_.findPort(edge.fromPortId);
    const auto* to = graph_.findPort(edge.toPortId);
    if (!from || !to) {
      continue;
    }

    const auto a = getPortPoint(*from);
    const auto b = getPortPoint(*to);
    const auto mid = juce::Point<float>((a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f);

    if (p.getDistanceFrom(a) <= kHitDistance || p.getDistanceFrom(mid) <= kHitDistance ||
        p.getDistanceFrom(b) <= kHitDistance) {
      return edgeId;
    }
  }

  return std::nullopt;
}

void CanvasView::paint(juce::Graphics& g) {
  g.fillAll(juce::Colour(0xff11151d));

  // Cables
  g.setColour(juce::Colours::orange.withAlpha(0.9f));
  for (const auto& [_, edge] : graph_.getEdges()) {
    const auto* from = graph_.findPort(edge.fromPortId);
    const auto* to = graph_.findPort(edge.toPortId);
    if (!from || !to) {
      continue;
    }

    const auto a = getPortPoint(*from);
    const auto b = getPortPoint(*to);

    juce::Path path;
    path.startNewSubPath(a);
    path.cubicTo({a.x + 60.0f, a.y}, {b.x - 60.0f, b.y}, b);
    g.strokePath(path, juce::PathStrokeType(2.0f));
  }

  if (cableFromPortId_) {
    const auto* from = graph_.findPort(*cableFromPortId_);
    if (from) {
      const auto a = getPortPoint(*from);
      juce::Path preview;
      preview.startNewSubPath(a);
      preview.cubicTo({a.x + 60.0f, a.y}, {cableMousePoint_.x - 60.0f, cableMousePoint_.y}, cableMousePoint_);
      g.setColour(juce::Colours::lightskyblue);
      g.strokePath(preview, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved));
    }
  }

  // Nodes
  for (const auto& [_, node] : graph_.getNodes()) {
    const auto r = nodeRect(node);

    g.setColour(juce::Colour(0xff242b38));
    g.fillRoundedRectangle(r, 10.0f);
    g.setColour(juce::Colour(0xff4c5d7a));
    g.drawRoundedRectangle(r, 10.0f, 1.2f);

    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    auto header = r;
    g.drawText(nodeName(node.type), header.removeFromTop(26.0f).toNearestInt(),
           juce::Justification::centredLeft, false);

    for (const auto& portId : node.inputPortIds) {
      if (const auto* port = graph_.findPort(portId)) {
        const auto p = getPortPoint(*port);
        g.setColour(juce::Colours::lightgreen);
        g.fillEllipse(p.x - kPortRadius, p.y - kPortRadius, kPortRadius * 2.0f, kPortRadius * 2.0f);
      }
    }

    for (const auto& portId : node.outputPortIds) {
      if (const auto* port = graph_.findPort(portId)) {
        const auto p = getPortPoint(*port);
        g.setColour(juce::Colours::goldenrod);
        g.fillEllipse(p.x - kPortRadius, p.y - kPortRadius, kPortRadius * 2.0f, kPortRadius * 2.0f);
      }
    }
  }
}

void CanvasView::mouseDown(const juce::MouseEvent& event) {
  auto p = event.position;

  if (event.mods.isRightButtonDown()) {
    if (auto edgeId = hitEdge(p)) {
      graph_.disconnect(*edgeId);
      if (onGraphChanged_) {
        onGraphChanged_();
      }
      if (onStatus_) {
        onStatus_("Disconnected");
      }
      repaint();
      return;
    }
  }

  if (auto portHit = hitPort(p); portHit && portHit->direction == graph::PortDirection::Out) {
    cableFromPortId_ = portHit->portId;
    cableMousePoint_ = p;
    repaint();
    return;
  }

  if (auto nodeId = hitNode(p)) {
    draggingNodeId_ = *nodeId;
    if (const auto* node = graph_.findNode(*nodeId)) {
      dragOffset_ = {p.x - node->position.x, p.y - node->position.y};
    }
  }
}

void CanvasView::mouseDrag(const juce::MouseEvent& event) {
  auto p = event.position;

  if (cableFromPortId_) {
    cableMousePoint_ = p;
    repaint();
    return;
  }

  if (!draggingNodeId_) {
    return;
  }

  if (auto* node = graph_.findNode(*draggingNodeId_)) {
    node->position = {p.x - dragOffset_.x, p.y - dragOffset_.y};
    repaint();
    if (onGraphChanged_) {
      onGraphChanged_();
    }
  }
}

void CanvasView::mouseUp(const juce::MouseEvent& event) {
  draggingNodeId_.reset();

  if (!cableFromPortId_) {
    return;
  }

  const auto fromPortId = *cableFromPortId_;
  cableFromPortId_.reset();

  std::string error;
  if (auto target = hitPort(event.position); target && target->direction == graph::PortDirection::In) {
    if (graph_.connect(fromPortId, target->portId, error)) {
      if (onGraphChanged_) {
        onGraphChanged_();
      }
      if (onStatus_) {
        onStatus_("Connected");
      }
    } else if (onStatus_) {
      onStatus_(error);
    }
  }

  repaint();
}

}  // namespace ui
