#pragma once

#include "audio/AudioEngine.h"
#include "graph/PatchGraph.h"
#include "ui/CanvasView.h"

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <optional>

class MainComponent : public juce::AudioAppComponent {
 public:
  MainComponent();
  ~MainComponent() override;

  void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
  void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
  void releaseResources() override;

  void resized() override;
  bool keyPressed(const juce::KeyPress& key) override;

 private:
  void pushUndoState();
  void undo();
  void redo();
  void onNodeSelected(const std::optional<std::string>& nodeId);
  void refreshInspector();
  void applyInspectorToSelectedNode();
  void addNode(graph::NodeType type);
  void savePatch();
  void loadPatch();
  void toggleTransport();
  void rebuildAudioPlan();

  graph::PatchGraph graph_;
  audio::AudioEngine engine_;

  juce::Component toolbar_;
  ui::CanvasView canvas_;
  juce::Label status_;
  juce::TextButton transportButton_;
  juce::TextButton undoButton_;
  juce::TextButton redoButton_;
  juce::Slider bpmSlider_;
  juce::Label bpmLabel_;

  juce::GroupComponent inspectorGroup_;
  juce::Label inspectorNodeLabel_;
  juce::Slider gainSlider_;
  juce::Slider filterCutoffSlider_;
  juce::Slider delayMsSlider_;
  juce::Slider delayFeedbackSlider_;
  juce::Slider delayMixSlider_;

  std::unique_ptr<juce::FileChooser> fileChooser_;
  bool applyingHistory_{false};
  bool updatingInspector_{false};
  std::optional<std::string> selectedNodeId_;

  std::vector<std::string> undoStack_;
  std::vector<std::string> redoStack_;

  std::vector<std::unique_ptr<juce::TextButton>> buttons_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
