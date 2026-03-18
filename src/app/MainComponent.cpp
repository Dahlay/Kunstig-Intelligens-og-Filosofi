#include "MainComponent.h"

#include "persistence/PatchSerializer.h"

namespace {
constexpr int kToolbarHeight = 46;
constexpr int kInspectorWidth = 270;

constexpr juce::uint32 kBgTop = 0xffd9f1ff;
constexpr juce::uint32 kBgMid = 0xffb8e0ff;
constexpr juce::uint32 kBgBottom = 0xff8fc3f2;
constexpr juce::uint32 kPanelGlass = 0x66ffffff;

void configureInspectorSlider(juce::Slider& slider, double min, double max, double step) {
  slider.setRange(min, max, step);
  slider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 72, 20);
}

void configureY2KButton(juce::TextButton& button) {
  button.setColour(juce::TextButton::buttonColourId, juce::Colour(0x8af4fbff));
  button.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff9de8dc));
  button.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff1f4e73));
  button.setColour(juce::TextButton::textColourOnId, juce::Colour(0xff0d3b58));
}

void configureY2KSlider(juce::Slider& slider) {
  slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffffffff));
  slider.setColour(juce::Slider::trackColourId, juce::Colour(0xff7ec4f5));
  slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff8de3d1));
  slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xff1f5278));
  slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0x885faedc));
  slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xb7ffffff));
}
}

MainComponent::MainComponent() : canvas_(graph_) {
  setSize(1280, 800);
  setAudioChannels(0, 2);
  setWantsKeyboardFocus(true);
  setLookAndFeel(&y2kLookAndFeel_);

  y2kLookAndFeel_.setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(kBgBottom));
  y2kLookAndFeel_.setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xf0f7fdff));
  y2kLookAndFeel_.setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xffbde8ff));
  y2kLookAndFeel_.setColour(juce::PopupMenu::highlightedTextColourId, juce::Colour(0xff1c5178));
  y2kLookAndFeel_.setColour(juce::GroupComponent::textColourId, juce::Colour(0xff2c6f98));
  y2kLookAndFeel_.setColour(juce::GroupComponent::outlineColourId, juce::Colour(0x8868b8df));

  addAndMakeVisible(toolbar_);
  addAndMakeVisible(canvas_);
  addAndMakeVisible(status_);
  addAndMakeVisible(transportButton_);
  addAndMakeVisible(undoButton_);
  addAndMakeVisible(redoButton_);
  addAndMakeVisible(bpmSlider_);
  addAndMakeVisible(bpmLabel_);
  addAndMakeVisible(inspectorGroup_);
  addAndMakeVisible(inspectorNodeLabel_);
  addAndMakeVisible(gainSlider_);
  addAndMakeVisible(filterCutoffSlider_);
  addAndMakeVisible(delayMsSlider_);
  addAndMakeVisible(delayFeedbackSlider_);
  addAndMakeVisible(delayMixSlider_);
  addAndMakeVisible(synthWaveformLabel_);
  addAndMakeVisible(synthWaveformBox_);
  addAndMakeVisible(synthChordLabel_);
  addAndMakeVisible(synthChordBox_);
  addAndMakeVisible(synthRateLabel_);
  addAndMakeVisible(synthRateBox_);
  addAndMakeVisible(synthTemplateLabel_);
  addAndMakeVisible(synthTemplateBox_);
  addAndMakeVisible(synthMidiLabel_);
  addAndMakeVisible(synthMidiDrawToggle_);
  addAndMakeVisible(synthMidiGrid_);

  addAndMakeVisible(bassWaveformLabel_);
  addAndMakeVisible(bassWaveformBox_);
  addAndMakeVisible(bassOctaveLabel_);
  addAndMakeVisible(bassOctaveBox_);
  addAndMakeVisible(bassRateLabel_);
  addAndMakeVisible(bassRateBox_);
  addAndMakeVisible(bassMidiLabel_);
  addAndMakeVisible(bassMidiDrawToggle_);
  addAndMakeVisible(bassMidiGrid_);

  status_.setColour(juce::Label::textColourId, juce::Colour(0xff2b5f86));
  status_.setColour(juce::Label::backgroundColourId, juce::Colour(0x82ffffff));
  status_.setText("Ready", juce::dontSendNotification);

  transportButton_.setButtonText("Stop");
  transportButton_.onClick = [this] { toggleTransport(); };
  configureY2KButton(transportButton_);

  undoButton_.setButtonText("Undo");
  undoButton_.onClick = [this] { undo(); };
  configureY2KButton(undoButton_);

  redoButton_.setButtonText("Redo");
  redoButton_.onClick = [this] { redo(); };
  configureY2KButton(redoButton_);

  bpmLabel_.setText("BPM", juce::dontSendNotification);
  bpmLabel_.setColour(juce::Label::textColourId, juce::Colour(0xff3a7fa9));

  bpmSlider_.setRange(40.0, 240.0, 1.0);
  bpmSlider_.setValue(120.0);
  bpmSlider_.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 54, 20);
  configureY2KSlider(bpmSlider_);
  bpmSlider_.onValueChange = [this] {
    engine_.setBpm(bpmSlider_.getValue());
    status_.setText("BPM: " + juce::String(static_cast<int>(bpmSlider_.getValue())),
                    juce::dontSendNotification);
  };

  inspectorGroup_.setText("Inspector");
  inspectorNodeLabel_.setText("No node selected", juce::dontSendNotification);
  inspectorNodeLabel_.setColour(juce::Label::textColourId, juce::Colour(0xff3c7395));

  synthWaveformLabel_.setText("Wave", juce::dontSendNotification);
  synthWaveformLabel_.setColour(juce::Label::textColourId, juce::Colour(0xff3a7fa9));
  synthWaveformBox_.addItem("Sine", 1);
  synthWaveformBox_.addItem("Saw", 2);
  synthWaveformBox_.addItem("Square", 3);

  synthChordLabel_.setText("Chord", juce::dontSendNotification);
  synthChordLabel_.setColour(juce::Label::textColourId, juce::Colour(0xff3a7fa9));
  synthChordBox_.addItem("Lead (single)", 1);
  synthChordBox_.addItem("C Major", 2);
  synthChordBox_.addItem("A Minor", 3);
  synthChordBox_.addItem("F Maj7", 4);
  synthChordBox_.addItem("G7", 5);
  synthChordBox_.addItem("D Minor", 6);
  synthChordBox_.addItem("E Min7", 7);

  synthRateLabel_.setText("Rate", juce::dontSendNotification);
  synthRateLabel_.setColour(juce::Label::textColourId, juce::Colour(0xff3a7fa9));
  synthRateBox_.addItem("1/4", 1);
  synthRateBox_.addItem("1/8", 2);
  synthRateBox_.addItem("1/16", 3);

  synthTemplateLabel_.setText("Shape", juce::dontSendNotification);
  synthTemplateLabel_.setColour(juce::Label::textColourId, juce::Colour(0xff3a7fa9));
  synthTemplateBox_.addItem("Soft Pad", 1);
  synthTemplateBox_.addItem("Pluck", 2);
  synthTemplateBox_.addItem("Organ", 3);
  synthTemplateBox_.addItem("Wide Motion", 4);

  synthMidiLabel_.setText("MIDI", juce::dontSendNotification);
  synthMidiLabel_.setColour(juce::Label::textColourId, juce::Colour(0xff3a7fa9));
  synthMidiDrawToggle_.setButtonText("Draw");
  synthMidiDrawToggle_.setColour(juce::ToggleButton::textColourId, juce::Colour(0xff2d638a));

  synthWaveformBox_.onChange = [this] { applyInspectorToSelectedNode(); };
  synthChordBox_.onChange = [this] { applyInspectorToSelectedNode(); };
  synthRateBox_.onChange = [this] { applyInspectorToSelectedNode(); };
  synthTemplateBox_.onChange = [this] { applyInspectorToSelectedNode(); };
  synthMidiDrawToggle_.onClick = [this] { applyInspectorToSelectedNode(); };
  synthMidiGrid_.setNotesChangedCallback([this](const ui::SynthMidiGrid::StepMasks&) {
    applyInspectorToSelectedNode();
  });

  bassWaveformLabel_.setText("Wave", juce::dontSendNotification);
  bassWaveformLabel_.setColour(juce::Label::textColourId, juce::Colour(0xff3a7fa9));
  bassWaveformBox_.addItem("Sine", 1);
  bassWaveformBox_.addItem("Saw", 2);
  bassWaveformBox_.addItem("Square", 3);
  bassWaveformBox_.setSelectedId(2, juce::dontSendNotification);

  bassOctaveLabel_.setText("Octave", juce::dontSendNotification);
  bassOctaveLabel_.setColour(juce::Label::textColourId, juce::Colour(0xff3a7fa9));
  bassOctaveBox_.addItem("C1 \xe2\x80\x93 B1", 1);
  bassOctaveBox_.addItem("C2 \xe2\x80\x93 B2", 2);
  bassOctaveBox_.setSelectedId(1, juce::dontSendNotification);

  bassRateLabel_.setText("Rate", juce::dontSendNotification);
  bassRateLabel_.setColour(juce::Label::textColourId, juce::Colour(0xff3a7fa9));
  bassRateBox_.addItem("1/4", 1);
  bassRateBox_.addItem("1/8", 2);
  bassRateBox_.addItem("1/16", 3);
  bassRateBox_.setSelectedId(2, juce::dontSendNotification);

  bassMidiLabel_.setText("MIDI", juce::dontSendNotification);
  bassMidiLabel_.setColour(juce::Label::textColourId, juce::Colour(0xff3a7fa9));
  bassMidiDrawToggle_.setButtonText("Draw");
  bassMidiDrawToggle_.setColour(juce::ToggleButton::textColourId, juce::Colour(0xff2d638a));

  bassWaveformBox_.onChange = [this] { applyInspectorToSelectedNode(); };
  bassOctaveBox_.onChange   = [this] { applyInspectorToSelectedNode(); };
  bassRateBox_.onChange     = [this] { applyInspectorToSelectedNode(); };
  bassMidiDrawToggle_.onClick = [this] { applyInspectorToSelectedNode(); };
  bassMidiGrid_.setNotesChangedCallback([this](const ui::SynthMidiGrid::StepMasks&) {
    applyInspectorToSelectedNode();
  });
  bassMidiGrid_.setStepMasks({{0b000000000001u, 0u, 0b000000010000u, 0u,
                               0b000010000000u, 0u, 0b000000010000u, 0u,
                               0b000000000001u, 0u, 0b000000010000u, 0u,
                               0b000010000000u, 0u, 0b000001000000u, 0u}});

  configureInspectorSlider(gainSlider_, 0.0, 2.0, 0.01);
  configureInspectorSlider(filterCutoffSlider_, 50.0, 10000.0, 1.0);
  configureInspectorSlider(delayMsSlider_, 1.0, 1000.0, 1.0);
  configureInspectorSlider(delayFeedbackSlider_, 0.0, 0.95, 0.01);
  configureInspectorSlider(delayMixSlider_, 0.0, 1.0, 0.01);
  configureY2KSlider(gainSlider_);
  configureY2KSlider(filterCutoffSlider_);
  configureY2KSlider(delayMsSlider_);
  configureY2KSlider(delayFeedbackSlider_);
  configureY2KSlider(delayMixSlider_);

  gainSlider_.onValueChange = [this] { applyInspectorToSelectedNode(); };
  filterCutoffSlider_.onValueChange = [this] { applyInspectorToSelectedNode(); };
  delayMsSlider_.onValueChange = [this] { applyInspectorToSelectedNode(); };
  delayFeedbackSlider_.onValueChange = [this] { applyInspectorToSelectedNode(); };
  delayMixSlider_.onValueChange = [this] { applyInspectorToSelectedNode(); };

  const std::vector<std::pair<juce::String, graph::NodeType>> nodeButtons = {
      {"Output", graph::NodeType::Output}, {"Synth", graph::NodeType::Synth}, {"Drum", graph::NodeType::Drum},
      {"Gain", graph::NodeType::Gain},     {"Filter", graph::NodeType::Filter}, {"Delay", graph::NodeType::Delay},
      {"Mixer", graph::NodeType::Mixer},   {"Bass", graph::NodeType::Bass}};

  for (const auto& [label, type] : nodeButtons) {
    auto button = std::make_unique<juce::TextButton>(label);
    button->onClick = [this, type] { addNode(type); };
    configureY2KButton(*button);
    toolbar_.addAndMakeVisible(*button);
    buttons_.push_back(std::move(button));
  }

  auto saveButton = std::make_unique<juce::TextButton>("Save");
  saveButton->onClick = [this] { savePatch(); };
  configureY2KButton(*saveButton);
  toolbar_.addAndMakeVisible(*saveButton);
  buttons_.push_back(std::move(saveButton));

  auto loadButton = std::make_unique<juce::TextButton>("Load");
  loadButton->onClick = [this] { loadPatch(); };
  configureY2KButton(*loadButton);
  toolbar_.addAndMakeVisible(*loadButton);
  buttons_.push_back(std::move(loadButton));

  canvas_.setGraphEditCallback([this] {
    if (!applyingHistory_) {
      pushUndoState();
    }
  });
  canvas_.setGraphChangedCallback([this] { rebuildAudioPlan(); });
  canvas_.setStatusCallback([this](const std::string& text) { status_.setText(text, juce::dontSendNotification); });
  canvas_.setNodeSelectionCallback([this](const std::optional<std::string>& nodeId) { onNodeSelected(nodeId); });

  // Default patch
  addNode(graph::NodeType::Output);
  addNode(graph::NodeType::Synth);
  refreshInspector();

  // Initial undo baseline
  undoStack_.push_back(persistence::PatchSerializer::toJson(graph_));
}

MainComponent::~MainComponent() {
  shutdownAudio();
  setLookAndFeel(nullptr);
}

void MainComponent::paint(juce::Graphics& g) {
  juce::ColourGradient bg(juce::Colour(kBgTop), 0.0f, 0.0f,
                          juce::Colour(kBgBottom), 0.0f, static_cast<float>(getHeight()), false);
  bg.addColour(0.52, juce::Colour(kBgMid));
  g.setGradientFill(bg);
  g.fillAll();

  // glossy aero arc
  juce::Path ribbon;
  ribbon.startNewSubPath(0.0f, 58.0f);
  ribbon.cubicTo(getWidth() * 0.20f, 28.0f, getWidth() * 0.36f, 86.0f, getWidth() * 0.52f, 54.0f);
  ribbon.cubicTo(getWidth() * 0.70f, 26.0f, getWidth() * 0.84f, 84.0f, static_cast<float>(getWidth()), 46.0f);
  g.setColour(juce::Colour(0x55ffffff));
  g.strokePath(ribbon, juce::PathStrokeType(3.0f));

  // soft bubbles
  g.setColour(juce::Colour(0x30ffffff));
  g.fillEllipse(40.0f, 80.0f, 180.0f, 180.0f);
  g.fillEllipse(static_cast<float>(getWidth()) - 250.0f, 120.0f, 210.0f, 210.0f);
  g.fillEllipse(static_cast<float>(getWidth()) * 0.38f, static_cast<float>(getHeight()) * 0.72f, 160.0f, 160.0f);

  // frosted overlays
  g.setColour(juce::Colour(kPanelGlass));
  g.fillRoundedRectangle(toolbar_.getBounds().toFloat().reduced(2.0f, 2.0f), 10.0f);
  g.fillRoundedRectangle(inspectorGroup_.getBounds().toFloat().expanded(4.0f), 12.0f);

  g.setColour(juce::Colour(0x66ffffff));
  g.drawRoundedRectangle(toolbar_.getBounds().toFloat().reduced(2.0f, 2.0f), 10.0f, 1.2f);
  g.drawRoundedRectangle(inspectorGroup_.getBounds().toFloat().expanded(4.0f), 12.0f, 1.2f);
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
  engine_.prepare(sampleRate, samplesPerBlockExpected, 2);
  engine_.setTransportPlaying(true);
  engine_.setBpm(bpmSlider_.getValue());
  rebuildAudioPlan();
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) {
  juce::MidiBuffer midi;
  juce::AudioBuffer<float> temp(bufferToFill.buffer->getArrayOfWritePointers(),
                                bufferToFill.buffer->getNumChannels(), bufferToFill.startSample,
                                bufferToFill.numSamples);

  engine_.processBlock(temp, midi);
}

void MainComponent::releaseResources() {}

void MainComponent::resized() {
  auto area = getLocalBounds();
  auto toolbarArea = area.removeFromTop(kToolbarHeight);
  toolbar_.setBounds(toolbarArea);

  int x = 8;
  for (auto& button : buttons_) {
    button->setBounds(x, 8, 92, 30);
    x += 96;
  }

  transportButton_.setBounds(x + 8, 8, 74, 30);
  undoButton_.setBounds(x + 88, 8, 68, 30);
  redoButton_.setBounds(x + 160, 8, 68, 30);
  bpmLabel_.setBounds(x + 236, 10, 34, 24);
  bpmSlider_.setBounds(x + 270, 8, 170, 30);

  status_.setBounds(getWidth() - 360, 8, 350, 30);

  auto inspectorArea = area.removeFromRight(kInspectorWidth);
  inspectorGroup_.setBounds(inspectorArea.reduced(8, 8));

  auto content = inspectorGroup_.getBounds().reduced(12, 28);
  inspectorNodeLabel_.setBounds(content.removeFromTop(26));
  auto synthRow = content.removeFromTop(28);
  synthWaveformLabel_.setBounds(synthRow.removeFromLeft(52));
  synthWaveformBox_.setBounds(synthRow);
  auto chordRow = content.removeFromTop(28);
  synthChordLabel_.setBounds(chordRow.removeFromLeft(52));
  synthChordBox_.setBounds(chordRow);
  auto rateRow = content.removeFromTop(28);
  synthRateLabel_.setBounds(rateRow.removeFromLeft(52));
  synthRateBox_.setBounds(rateRow);
  auto shapeRow = content.removeFromTop(28);
  synthTemplateLabel_.setBounds(shapeRow.removeFromLeft(52));
  synthTemplateBox_.setBounds(shapeRow);
  auto midiRow = content.removeFromTop(24);
  synthMidiLabel_.setBounds(midiRow.removeFromLeft(52));
  synthMidiDrawToggle_.setBounds(midiRow.removeFromLeft(72));
  synthMidiGrid_.setBounds(content.removeFromTop(146));
  content.removeFromTop(8);
  auto bassWaveRow = content.removeFromTop(28);
  bassWaveformLabel_.setBounds(bassWaveRow.removeFromLeft(52));
  bassWaveformBox_.setBounds(bassWaveRow);
  auto bassOctRow = content.removeFromTop(28);
  bassOctaveLabel_.setBounds(bassOctRow.removeFromLeft(52));
  bassOctaveBox_.setBounds(bassOctRow);
  auto bassRateRow = content.removeFromTop(28);
  bassRateLabel_.setBounds(bassRateRow.removeFromLeft(52));
  bassRateBox_.setBounds(bassRateRow);
  auto bassMidiRow = content.removeFromTop(24);
  bassMidiLabel_.setBounds(bassMidiRow.removeFromLeft(52));
  bassMidiDrawToggle_.setBounds(bassMidiRow.removeFromLeft(72));
  bassMidiGrid_.setBounds(content.removeFromTop(110));
  content.removeFromTop(6);
  gainSlider_.setBounds(content.removeFromTop(34));
  filterCutoffSlider_.setBounds(content.removeFromTop(34));
  delayMsSlider_.setBounds(content.removeFromTop(34));
  delayFeedbackSlider_.setBounds(content.removeFromTop(34));
  delayMixSlider_.setBounds(content.removeFromTop(34));

  canvas_.setBounds(area);
}

void MainComponent::addNode(graph::NodeType type) {
  if (type == graph::NodeType::Output) {
    for (const auto& [nodeId, node] : graph_.getNodes()) {
      if (node.type == graph::NodeType::Output) {
        selectedNodeId_ = nodeId;
        refreshInspector();
        canvas_.repaint();
        status_.setText("Output is fixed in the center", juce::dontSendNotification);
        return;
      }
    }
  }

  if (!applyingHistory_) {
    pushUndoState();
  }
  const float x = 80.0f + static_cast<float>(graph_.getNodes().size() * 24);
  const float y = 100.0f + static_cast<float>(graph_.getNodes().size() * 24);
  const auto nodeId = graph_.addNode(type, {x, y});
  selectedNodeId_ = nodeId;
  refreshInspector();
  canvas_.repaint();
  rebuildAudioPlan();
}

void MainComponent::savePatch() {
  fileChooser_ = std::make_unique<juce::FileChooser>(
      "Save patch", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.json");

  fileChooser_->launchAsync(juce::FileBrowserComponent::saveMode |
                                juce::FileBrowserComponent::canSelectFiles,
                            [this](const juce::FileChooser& chooser) {
                              const auto file = chooser.getResult();
                              if (file == juce::File()) {
                                return;
                              }

                              if (!file.replaceWithText(persistence::PatchSerializer::toJson(graph_))) {
                                status_.setText("Failed to save patch", juce::dontSendNotification);
                                return;
                              }

                              status_.setText("Patch saved", juce::dontSendNotification);
                            });
}

void MainComponent::loadPatch() {
  fileChooser_ = std::make_unique<juce::FileChooser>(
      "Load patch", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.json");

  fileChooser_->launchAsync(juce::FileBrowserComponent::openMode |
                                juce::FileBrowserComponent::canSelectFiles,
                            [this](const juce::FileChooser& chooser) {
                              const auto file = chooser.getResult();
                              if (file == juce::File()) {
                                return;
                              }

                              const auto jsonText = file.loadFileAsString().toStdString();

                              std::string error;
                              graph::PatchGraph loaded;
                              if (!persistence::PatchSerializer::fromJson(jsonText, loaded, error)) {
                                status_.setText(error, juce::dontSendNotification);
                                return;
                              }

                              if (!applyingHistory_) {
                                pushUndoState();
                              }
                              graph_ = std::move(loaded);
                              selectedNodeId_.reset();
                              refreshInspector();
                              canvas_.repaint();
                              rebuildAudioPlan();
                              status_.setText("Patch loaded", juce::dontSendNotification);
                            });
}

bool MainComponent::keyPressed(const juce::KeyPress& key) {
  const auto mods = key.getModifiers();
  if (mods.isCommandDown() && key.getTextCharacter() == 'z' && mods.isShiftDown()) {
    redo();
    return true;
  }

  if (mods.isCommandDown() && key.getTextCharacter() == 'z') {
    undo();
    return true;
  }

  if (mods.isCommandDown() && key.getTextCharacter() == 's') {
    savePatch();
    return true;
  }

  if (mods.isCommandDown() && key.getTextCharacter() == 'o') {
    loadPatch();
    return true;
  }

  return false;
}

void MainComponent::pushUndoState() {
  undoStack_.push_back(persistence::PatchSerializer::toJson(graph_));
  if (undoStack_.size() > 200) {
    undoStack_.erase(undoStack_.begin());
  }
  redoStack_.clear();
}

void MainComponent::undo() {
  if (undoStack_.empty()) {
    status_.setText("Nothing to undo", juce::dontSendNotification);
    return;
  }

  applyingHistory_ = true;
  const auto previous = undoStack_.back();
  undoStack_.pop_back();
  redoStack_.push_back(persistence::PatchSerializer::toJson(graph_));

  std::string error;
  graph::PatchGraph loaded;
  if (persistence::PatchSerializer::fromJson(previous, loaded, error)) {
    graph_ = std::move(loaded);
    selectedNodeId_.reset();
    refreshInspector();
    canvas_.repaint();
    rebuildAudioPlan();
    status_.setText("Undo", juce::dontSendNotification);
  } else {
    status_.setText("Undo failed: " + error, juce::dontSendNotification);
  }
  applyingHistory_ = false;
}

void MainComponent::redo() {
  if (redoStack_.empty()) {
    status_.setText("Nothing to redo", juce::dontSendNotification);
    return;
  }

  applyingHistory_ = true;
  const auto next = redoStack_.back();
  redoStack_.pop_back();
  undoStack_.push_back(persistence::PatchSerializer::toJson(graph_));

  std::string error;
  graph::PatchGraph loaded;
  if (persistence::PatchSerializer::fromJson(next, loaded, error)) {
    graph_ = std::move(loaded);
    selectedNodeId_.reset();
    refreshInspector();
    canvas_.repaint();
    rebuildAudioPlan();
    status_.setText("Redo", juce::dontSendNotification);
  } else {
    status_.setText("Redo failed: " + error, juce::dontSendNotification);
  }
  applyingHistory_ = false;
}

void MainComponent::onNodeSelected(const std::optional<std::string>& nodeId) {
  selectedNodeId_ = nodeId;
  refreshInspector();
}

void MainComponent::refreshInspector() {
  updatingInspector_ = true;

  gainSlider_.setEnabled(false);
  filterCutoffSlider_.setEnabled(false);
  delayMsSlider_.setEnabled(false);
  delayFeedbackSlider_.setEnabled(false);
  delayMixSlider_.setEnabled(false);
  synthWaveformBox_.setEnabled(false);
  synthChordBox_.setEnabled(false);
  synthRateBox_.setEnabled(false);
  synthTemplateBox_.setEnabled(false);
  synthMidiDrawToggle_.setEnabled(false);
  synthMidiGrid_.setEnabled(false);
  bassWaveformBox_.setEnabled(false);
  bassOctaveBox_.setEnabled(false);
  bassRateBox_.setEnabled(false);
  bassMidiDrawToggle_.setEnabled(false);
  bassMidiGrid_.setEnabled(false);
  synthTemplateLabel_.setVisible(false);
  synthTemplateBox_.setVisible(false);
  synthMidiLabel_.setVisible(false);
  synthMidiDrawToggle_.setVisible(false);
  synthMidiGrid_.setVisible(false);
  bassWaveformLabel_.setVisible(false);
  bassWaveformBox_.setVisible(false);
  bassOctaveLabel_.setVisible(false);
  bassOctaveBox_.setVisible(false);
  bassRateLabel_.setVisible(false);
  bassRateBox_.setVisible(false);
  bassMidiLabel_.setVisible(false);
  bassMidiDrawToggle_.setVisible(false);
  bassMidiGrid_.setVisible(false);

  if (!selectedNodeId_) {
    inspectorNodeLabel_.setText("No node selected", juce::dontSendNotification);
    updatingInspector_ = false;
    return;
  }

  const auto* node = graph_.findNode(*selectedNodeId_);
  if (!node) {
    inspectorNodeLabel_.setText("Selection invalid", juce::dontSendNotification);
    updatingInspector_ = false;
    return;
  }

  const auto nodeName = [node]() {
    switch (node->type) {
      case graph::NodeType::Output:
        return juce::String("Output");
      case graph::NodeType::Synth:
        return juce::String("Synth");
      case graph::NodeType::Drum:
        return juce::String("Drum");
      case graph::NodeType::Gain:
        return juce::String("Gain");
      case graph::NodeType::Filter:
        return juce::String("Filter");
      case graph::NodeType::Delay:
        return juce::String("Delay");
      case graph::NodeType::Mixer:
        return juce::String("Mixer");
      case graph::NodeType::Bass:
        return juce::String("Bass");
    }
    return juce::String("Node");
  }();

  inspectorNodeLabel_.setText("Selected: " + nodeName, juce::dontSendNotification);

  gainSlider_.setValue(node->gain, juce::dontSendNotification);
  filterCutoffSlider_.setValue(node->filterCutoffHz, juce::dontSendNotification);
  delayMsSlider_.setValue(node->delayMs, juce::dontSendNotification);
  delayFeedbackSlider_.setValue(node->delayFeedback, juce::dontSendNotification);
  delayMixSlider_.setValue(node->delayMix, juce::dontSendNotification);
  synthWaveformBox_.setSelectedId(node->synthWaveform + 1, juce::dontSendNotification);
  synthChordBox_.setSelectedId(node->synthChord + 1, juce::dontSendNotification);
  if (node->synthRateDivision <= 1) {
    synthRateBox_.setSelectedId(1, juce::dontSendNotification);
  } else if (node->synthRateDivision <= 2) {
    synthRateBox_.setSelectedId(2, juce::dontSendNotification);
  } else {
    synthRateBox_.setSelectedId(3, juce::dontSendNotification);
  }
  synthTemplateBox_.setSelectedId(node->synthTemplate + 1, juce::dontSendNotification);
  synthMidiDrawToggle_.setToggleState(node->synthUseMidiDraw, juce::dontSendNotification);
  synthMidiGrid_.setStepMasks(node->synthStepMasks);
  bassWaveformBox_.setSelectedId(node->bassWaveform + 1, juce::dontSendNotification);
  bassOctaveBox_.setSelectedId(node->bassOctave + 1, juce::dontSendNotification);
  if (node->bassRateDivision <= 1) {
    bassRateBox_.setSelectedId(1, juce::dontSendNotification);
  } else if (node->bassRateDivision <= 2) {
    bassRateBox_.setSelectedId(2, juce::dontSendNotification);
  } else {
    bassRateBox_.setSelectedId(3, juce::dontSendNotification);
  }
  bassMidiDrawToggle_.setToggleState(node->bassUseMidiDraw, juce::dontSendNotification);
  bassMidiGrid_.setStepMasks(node->bassStepMasks);

  gainSlider_.setEnabled(node->type == graph::NodeType::Gain);
  filterCutoffSlider_.setEnabled(node->type == graph::NodeType::Filter);
  delayMsSlider_.setEnabled(node->type == graph::NodeType::Delay);
  delayFeedbackSlider_.setEnabled(node->type == graph::NodeType::Delay);
  delayMixSlider_.setEnabled(node->type == graph::NodeType::Delay);
  const bool isSynth = node->type == graph::NodeType::Synth;
  synthWaveformBox_.setEnabled(isSynth);
  synthChordBox_.setEnabled(isSynth && !synthMidiDrawToggle_.getToggleState());
  synthRateBox_.setEnabled(isSynth);
  synthTemplateBox_.setEnabled(isSynth);
  synthMidiDrawToggle_.setEnabled(isSynth);
  synthMidiGrid_.setEnabled(isSynth && synthMidiDrawToggle_.getToggleState());
  synthTemplateLabel_.setVisible(isSynth);
  synthTemplateBox_.setVisible(isSynth);
  synthMidiLabel_.setVisible(isSynth);
  synthMidiDrawToggle_.setVisible(isSynth);
  synthMidiGrid_.setVisible(isSynth);

  const bool isBass = node->type == graph::NodeType::Bass;
  bassWaveformBox_.setEnabled(isBass);
  bassOctaveBox_.setEnabled(isBass);
  bassRateBox_.setEnabled(isBass);
  bassMidiDrawToggle_.setEnabled(isBass);
  bassMidiGrid_.setEnabled(isBass && bassMidiDrawToggle_.getToggleState());
  bassWaveformLabel_.setVisible(isBass);
  bassWaveformBox_.setVisible(isBass);
  bassOctaveLabel_.setVisible(isBass);
  bassOctaveBox_.setVisible(isBass);
  bassRateLabel_.setVisible(isBass);
  bassRateBox_.setVisible(isBass);
  bassMidiLabel_.setVisible(isBass);
  bassMidiDrawToggle_.setVisible(isBass);
  bassMidiGrid_.setVisible(isBass);

  updatingInspector_ = false;
}

void MainComponent::applyInspectorToSelectedNode() {
  if (updatingInspector_ || applyingHistory_ || !selectedNodeId_) {
    return;
  }

  auto* node = graph_.findNode(*selectedNodeId_);
  if (!node) {
    return;
  }

  pushUndoState();

  node->gain = static_cast<float>(gainSlider_.getValue());
  node->filterCutoffHz = static_cast<float>(filterCutoffSlider_.getValue());
  node->delayMs = static_cast<float>(delayMsSlider_.getValue());
  node->delayFeedback = static_cast<float>(delayFeedbackSlider_.getValue());
  node->delayMix = static_cast<float>(delayMixSlider_.getValue());
  node->synthWaveform = std::max(0, synthWaveformBox_.getSelectedId() - 1);
  node->synthChord = synthMidiDrawToggle_.getToggleState() ? 0 : std::max(0, synthChordBox_.getSelectedId() - 1);
  const int rateId = synthRateBox_.getSelectedId();
  node->synthRateDivision = (rateId == 3 ? 4 : (rateId == 2 ? 2 : 1));
  node->synthTemplate = std::max(0, synthTemplateBox_.getSelectedId() - 1);
  node->synthUseMidiDraw = synthMidiDrawToggle_.getToggleState();
  node->synthStepMasks = synthMidiGrid_.getStepMasks();
  node->bassWaveform = std::max(0, bassWaveformBox_.getSelectedId() - 1);
  node->bassOctave = std::max(0, bassOctaveBox_.getSelectedId() - 1);
  const int bassRateId = bassRateBox_.getSelectedId();
  node->bassRateDivision = (bassRateId == 3 ? 4 : (bassRateId == 2 ? 2 : 1));
  node->bassUseMidiDraw = bassMidiDrawToggle_.getToggleState();
  node->bassStepMasks = bassMidiGrid_.getStepMasks();
  if (node->synthUseMidiDraw) {
    synthChordBox_.setSelectedId(1, juce::dontSendNotification);
  }
  synthChordBox_.setEnabled(!node->synthUseMidiDraw);
  synthMidiGrid_.setEnabled(node->synthUseMidiDraw);
  bassMidiGrid_.setEnabled(node->bassUseMidiDraw);

  rebuildAudioPlan();
}

void MainComponent::toggleTransport() {
  const bool newState = !engine_.isTransportPlaying();
  engine_.setTransportPlaying(newState);
  transportButton_.setButtonText(newState ? "Stop" : "Start");
  status_.setText(newState ? "Transport running" : "Transport stopped", juce::dontSendNotification);
}

void MainComponent::rebuildAudioPlan() {
  std::string error;
  if (!engine_.setGraph(graph_, error)) {
    status_.setText(error, juce::dontSendNotification);
  }
}
