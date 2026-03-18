#include "CanvasView.h"

#include <algorithm>
#include <cmath>

namespace ui {
namespace {
constexpr float kNodeWidth = 170.0f;
constexpr float kNodeHeight = 96.0f;
constexpr float kOutputDiameter = 180.0f;
constexpr float kPortRadius = 6.0f;
constexpr juce::uint32 kCanvasTop = 0xffdff3ff;
constexpr juce::uint32 kCanvasBottom = 0xff95c9ee;
constexpr juce::uint32 kCableGlass = 0xff62b7eb;
constexpr juce::uint32 kCableMint = 0xff9ae4d3;

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
    case graph::NodeType::Bass:
      return "Bass";
  }
  return "Node";
}
}  // namespace

CanvasView::CanvasView(graph::PatchGraph& graph) : graph_(graph) {
  setWantsKeyboardFocus(true);
}

juce::Rectangle<float> CanvasView::nodeRect(const graph::Node& node) const {
  if (node.type == graph::NodeType::Output) {
    const float x = (static_cast<float>(getWidth()) - kOutputDiameter) * 0.5f;
    const float y = (static_cast<float>(getHeight()) - kOutputDiameter) * 0.5f;
    return {x, y, kOutputDiameter, kOutputDiameter};
  }
  return {node.position.x, node.position.y, kNodeWidth, kNodeHeight};
}

juce::Point<float> CanvasView::getPortPoint(const graph::Port& port) const {
  const auto* node = graph_.findNode(port.nodeId);
  if (!node) {
    return {};
  }

  const auto rect = nodeRect(*node);
  if (node->type == graph::NodeType::Output && port.direction == graph::PortDirection::In) {
    auto idx = std::find(node->inputPortIds.begin(), node->inputPortIds.end(), port.id);
    const int i = idx == node->inputPortIds.end() ? 0 : static_cast<int>(std::distance(node->inputPortIds.begin(), idx));
    const int total = std::max(1, static_cast<int>(node->inputPortIds.size()));
    const float angle = -juce::MathConstants<float>::halfPi +
                        (juce::MathConstants<float>::twoPi * static_cast<float>(i)) / static_cast<float>(total);
    const auto c = rect.getCentre();
    const float r = rect.getWidth() * 0.5f + 2.0f;
    return {c.x + std::cos(angle) * r, c.y + std::sin(angle) * r};
  }

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
  constexpr float kHitDistance = 10.0f;
  constexpr int kSamples = 24;

  for (const auto& [edgeId, edge] : graph_.getEdges()) {
    const auto* from = graph_.findPort(edge.fromPortId);
    const auto* to = graph_.findPort(edge.toPortId);
    if (!from || !to) continue;

    const auto a = getPortPoint(*from);
    const auto b = getPortPoint(*to);
    const float c1x = a.x + 60.0f, c1y = a.y;
    const float c2x = b.x - 60.0f, c2y = b.y;

    for (int i = 0; i <= kSamples; ++i) {
      const float t = static_cast<float>(i) / kSamples;
      const float u = 1.0f - t;
      const float x = u*u*u*a.x + 3*u*u*t*c1x + 3*u*t*t*c2x + t*t*t*b.x;
      const float y = u*u*u*a.y + 3*u*u*t*c1y + 3*u*t*t*c2y + t*t*t*b.y;
      if (p.getDistanceFrom({x, y}) <= kHitDistance)
        return edgeId;
    }
  }

  return std::nullopt;
}

void CanvasView::paint(juce::Graphics& g) {
  juce::ColourGradient bg(juce::Colour(kCanvasTop), 0.0f, 0.0f,
                          juce::Colour(kCanvasBottom), 0.0f, static_cast<float>(getHeight()), false);
  bg.addColour(0.50, juce::Colour(0xffbde7ff));
  g.setGradientFill(bg);
  g.fillAll();

  // soft glass grid
  g.setColour(juce::Colour(0x227bb8de));
  for (int x = 0; x < getWidth(); x += 32)
    g.drawVerticalLine(x, 0.0f, static_cast<float>(getHeight()));
  for (int y = 0; y < getHeight(); y += 32)
    g.drawHorizontalLine(y, 0.0f, static_cast<float>(getWidth()));

  // airy highlights
  g.setColour(juce::Colour(0x28ffffff));
  g.fillEllipse(32.0f, 44.0f, 180.0f, 180.0f);
  g.fillEllipse(static_cast<float>(getWidth()) - 220.0f, 30.0f, 190.0f, 190.0f);

  // Cables
  g.setColour(juce::Colour(kCableGlass).withAlpha(0.24f));
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
    // soft bloom
    g.strokePath(path, juce::PathStrokeType(6.0f));
    // glossy core
    g.setColour(juce::Colour(kCableMint).withAlpha(0.95f));
    g.strokePath(path, juce::PathStrokeType(2.2f));
    g.setColour(juce::Colour(kCableGlass).withAlpha(0.24f));
  }

  if (cableFromPortId_) {
    const auto* from = graph_.findPort(*cableFromPortId_);
    if (from) {
      const auto a = getPortPoint(*from);
      juce::Path preview;
      preview.startNewSubPath(a);
      preview.cubicTo({a.x + 60.0f, a.y}, {cableMousePoint_.x - 60.0f, cableMousePoint_.y}, cableMousePoint_);
      g.setColour(juce::Colour(kCableGlass).withAlpha(0.34f));
      g.strokePath(preview, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved));
      g.setColour(juce::Colour(0xffffffff));
      g.strokePath(preview, juce::PathStrokeType(1.8f, juce::PathStrokeType::curved));
    }
  }

  // Nodes
  for (const auto& [nodeId, node] : graph_.getNodes()) {
    const auto r = nodeRect(node);

    if (node.type == graph::NodeType::Output) {
      juce::DropShadow(juce::Colour(0x8899c5df), 22, {0, 0})
          .drawForRectangle(g, r.toNearestInt().expanded(3));

      juce::ColourGradient outFill(juce::Colour(0xffecfbff), r.getCentreX(), r.getY(),
                                   juce::Colour(0xff6eb0dc), r.getCentreX(), r.getBottom(), false);
      g.setGradientFill(outFill);
      g.fillEllipse(r);

      g.setColour(juce::Colour(0xb8ffffff));
      g.drawEllipse(r, 2.4f);

      auto inner = r.reduced(20.0f);
      g.setColour(juce::Colour(0x55ffffff));
      g.fillEllipse(inner);
      g.setColour(juce::Colour(0xaa8fd2ef));
      g.drawEllipse(inner, 1.2f);

      if (selectedNodeId_ && *selectedNodeId_ == nodeId) {
        g.setColour(juce::Colour(0x66ffffff));
        g.drawEllipse(r.expanded(3.0f), 4.0f);
      }

      g.setColour(juce::Colour(0xff2a678e));
      g.setFont(juce::FontOptions(18.0f, juce::Font::bold));
      g.drawFittedText("OUTPUT", r.reduced(24.0f).toNearestInt(), juce::Justification::centred, 1);

      for (const auto& portId : node.inputPortIds) {
        if (const auto* port = graph_.findPort(portId)) {
          const auto p = getPortPoint(*port);
          g.setColour(juce::Colour(0xffbffff2));
          g.fillEllipse(p.x - kPortRadius, p.y - kPortRadius, kPortRadius * 2.0f, kPortRadius * 2.0f);
          g.setColour(juce::Colour(0xb0ffffff));
          g.drawEllipse(p.x - kPortRadius, p.y - kPortRadius, kPortRadius * 2.0f, kPortRadius * 2.0f, 1.0f);
        }
      }
      continue;
    }

    juce::DropShadow(juce::Colour(0x667eb8dd), 16, {0, 0})
        .drawForRectangle(g, r.toNearestInt().expanded(2));

    const auto fillTop = node.type == graph::NodeType::Bass
        ? juce::Colour(0xffe8fff4) : juce::Colour(0xfff4fdff);
    const auto fillBot = node.type == graph::NodeType::Bass
        ? juce::Colour(0xff4f9b7c) : juce::Colour(0xff91c7e8);
    juce::ColourGradient nodeFill(fillTop, r.getTopLeft(), fillBot, r.getBottomRight(), false);
    g.setGradientFill(nodeFill);
    g.fillRoundedRectangle(r, 10.0f);
    g.setColour(juce::Colour(0x9cffffff));
    g.drawRoundedRectangle(r, 10.0f, 1.4f);

    auto rTop = r;
    auto gloss = rTop.removeFromTop(22.0f);
    g.setColour(juce::Colour(0x55ffffff));
    g.fillRoundedRectangle(gloss, 10.0f);

    if (selectedNodeId_ && *selectedNodeId_ == nodeId) {
      g.setColour(juce::Colour(0x66ffffff));
      g.drawRoundedRectangle(r.expanded(4.0f), 12.0f, 4.5f);
      g.setColour(juce::Colour(0xffe5f8ff));
      g.drawRoundedRectangle(r.expanded(2.0f), 11.0f, 2.2f);
    }

    g.setColour(juce::Colour(0xff2b5e82));
    g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    auto header = r;
    g.drawText(nodeName(node.type), header.removeFromTop(26.0f).toNearestInt(),
           juce::Justification::centredLeft, false);

    for (const auto& portId : node.inputPortIds) {
      if (const auto* port = graph_.findPort(portId)) {
        const auto p = getPortPoint(*port);
        g.setColour(juce::Colour(0xffb9f7f1));
        g.fillEllipse(p.x - kPortRadius, p.y - kPortRadius, kPortRadius * 2.0f, kPortRadius * 2.0f);
        g.setColour(juce::Colour(0x99ffffff));
        g.drawEllipse(p.x - kPortRadius, p.y - kPortRadius, kPortRadius * 2.0f, kPortRadius * 2.0f, 1.0f);
      }
    }

    for (const auto& portId : node.outputPortIds) {
      if (const auto* port = graph_.findPort(portId)) {
        const auto p = getPortPoint(*port);
        g.setColour(juce::Colour(0xff9ad0ff));
        g.fillEllipse(p.x - kPortRadius, p.y - kPortRadius, kPortRadius * 2.0f, kPortRadius * 2.0f);
        g.setColour(juce::Colour(0x99ffffff));
        g.drawEllipse(p.x - kPortRadius, p.y - kPortRadius, kPortRadius * 2.0f, kPortRadius * 2.0f, 1.0f);
      }
    }
  }
}

void CanvasView::mouseDown(const juce::MouseEvent& event) {
  auto p = event.position;
  grabKeyboardFocus();

  if (event.mods.isRightButtonDown()) {
    // Right-click cable → disconnect
    if (auto edgeId = hitEdge(p)) {
      if (onGraphEdit_) onGraphEdit_();
      graph_.disconnect(*edgeId);
      if (onGraphChanged_) onGraphChanged_();
      if (onStatus_) onStatus_("Disconnected");
      repaint();
      return;
    }
    // Right-click node → popup menu
    if (auto nodeId = hitNode(p)) {
      selectedNodeId_ = *nodeId;
      if (onNodeSelection_) onNodeSelection_(selectedNodeId_);
      repaint();
      juce::PopupMenu menu;
      menu.addItem(1, "Delete Node");
      menu.showMenuAsync(juce::PopupMenu::Options{}.withTargetComponent(this),
        [this, nid = *nodeId](int result) {
          if (result == 1) {
            const auto* node = graph_.findNode(nid);
            if (node && node->type == graph::NodeType::Output) {
              if (onStatus_) onStatus_("Output is fixed and cannot be deleted");
              return;
            }
            if (onGraphEdit_) onGraphEdit_();
            graph_.removeNode(nid);
            if (selectedNodeId_ && *selectedNodeId_ == nid)
              selectedNodeId_.reset();
            if (onNodeSelection_) onNodeSelection_(selectedNodeId_);
            if (onGraphChanged_) onGraphChanged_();
            if (onStatus_) onStatus_("Node deleted");
            repaint();
          }
        });
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
    selectedNodeId_ = *nodeId;
    if (onNodeSelection_) {
      onNodeSelection_(selectedNodeId_);
    }

    draggingNodeId_ = *nodeId;
    if (onGraphEdit_) {
      onGraphEdit_();
    }
    if (const auto* node = graph_.findNode(*nodeId)) {
      dragOffset_ = {p.x - node->position.x, p.y - node->position.y};
    }
    repaint();
    return;
  }

  selectedNodeId_.reset();
  if (onNodeSelection_) {
    onNodeSelection_(selectedNodeId_);
  }
  repaint();
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
    if (node->type == graph::NodeType::Output) {
      return;
    }
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
    if (onGraphEdit_) {
      onGraphEdit_();
    }
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

bool CanvasView::keyPressed(const juce::KeyPress& key) {
  if (selectedNodeId_ &&
      (key == juce::KeyPress::deleteKey || key == juce::KeyPress::backspaceKey)) {
    const auto* node = graph_.findNode(*selectedNodeId_);
    if (node && node->type == graph::NodeType::Output) {
      if (onStatus_) onStatus_("Output is fixed and cannot be deleted");
      return true;
    }
    if (onGraphEdit_) onGraphEdit_();
    graph_.removeNode(*selectedNodeId_);
    selectedNodeId_.reset();
    if (onNodeSelection_) onNodeSelection_(selectedNodeId_);
    if (onGraphChanged_) onGraphChanged_();
    if (onStatus_) onStatus_("Node deleted");
    repaint();
    return true;
  }
  return false;
}

}  // namespace ui
