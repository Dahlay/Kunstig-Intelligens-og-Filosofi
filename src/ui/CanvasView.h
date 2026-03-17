#pragma once

#include "graph/PatchGraph.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <optional>
#include <string>

namespace ui {

class CanvasView : public juce::Component {
 public:
  using GraphChangedCallback = std::function<void()>;
  using StatusCallback = std::function<void(const std::string&)>;

  explicit CanvasView(graph::PatchGraph& graph);

  void setGraphChangedCallback(GraphChangedCallback cb) { onGraphChanged_ = std::move(cb); }
  void setStatusCallback(StatusCallback cb) { onStatus_ = std::move(cb); }

  void paint(juce::Graphics& g) override;
  void mouseDown(const juce::MouseEvent& event) override;
  void mouseDrag(const juce::MouseEvent& event) override;
  void mouseUp(const juce::MouseEvent& event) override;

 private:
  struct PortHit {
    std::string portId;
    juce::Point<float> point;
    graph::PortDirection direction{graph::PortDirection::In};
  };

  juce::Rectangle<float> nodeRect(const graph::Node& node) const;
  std::optional<std::string> hitNode(juce::Point<float> p) const;
  std::optional<PortHit> hitPort(juce::Point<float> p) const;
  juce::Point<float> getPortPoint(const graph::Port& port) const;

  graph::PatchGraph& graph_;
  GraphChangedCallback onGraphChanged_;
  StatusCallback onStatus_;

  std::optional<std::string> draggingNodeId_;
  juce::Point<float> dragOffset_;

  std::optional<std::string> cableFromPortId_;
  juce::Point<float> cableMousePoint_;
};

}  // namespace ui
