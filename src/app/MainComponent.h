#pragma once

#include "audio/AudioEngine.h"
#include "graph/PatchGraph.h"
#include "ui/CanvasView.h"

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

class MainComponent : public juce::AudioAppComponent {
 public:
  MainComponent();
  ~MainComponent() override;

  void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
  void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
  void releaseResources() override;

  void resized() override;

 private:
  void addNode(graph::NodeType type);
  void savePatch();
  void loadPatch();
  void rebuildAudioPlan();

  graph::PatchGraph graph_;
  audio::AudioEngine engine_;

  juce::Component toolbar_;
  ui::CanvasView canvas_;
  juce::Label status_;
  std::unique_ptr<juce::FileChooser> fileChooser_;

  std::vector<std::unique_ptr<juce::TextButton>> buttons_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
