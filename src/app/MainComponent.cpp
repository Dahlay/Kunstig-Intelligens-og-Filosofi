#include "MainComponent.h"

#include "persistence/PatchSerializer.h"

namespace {
constexpr int kToolbarHeight = 46;
}

MainComponent::MainComponent() : canvas_(graph_) {
  setSize(1280, 800);
  setAudioChannels(0, 2);
  setWantsKeyboardFocus(true);

  addAndMakeVisible(toolbar_);
  addAndMakeVisible(canvas_);
  addAndMakeVisible(status_);
  addAndMakeVisible(transportButton_);
  addAndMakeVisible(undoButton_);
  addAndMakeVisible(redoButton_);
  addAndMakeVisible(bpmSlider_);
  addAndMakeVisible(bpmLabel_);

  status_.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
  status_.setText("Ready", juce::dontSendNotification);

  transportButton_.setButtonText("Stop");
  transportButton_.onClick = [this] { toggleTransport(); };

  undoButton_.setButtonText("Undo");
  undoButton_.onClick = [this] { undo(); };

  redoButton_.setButtonText("Redo");
  redoButton_.onClick = [this] { redo(); };

  bpmLabel_.setText("BPM", juce::dontSendNotification);
  bpmLabel_.setColour(juce::Label::textColourId, juce::Colours::lightgrey);

  bpmSlider_.setRange(40.0, 240.0, 1.0);
  bpmSlider_.setValue(120.0);
  bpmSlider_.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 54, 20);
  bpmSlider_.onValueChange = [this] {
    engine_.setBpm(bpmSlider_.getValue());
    status_.setText("BPM: " + juce::String(static_cast<int>(bpmSlider_.getValue())),
                    juce::dontSendNotification);
  };

  const std::vector<std::pair<juce::String, graph::NodeType>> nodeButtons = {
      {"Output", graph::NodeType::Output}, {"Synth", graph::NodeType::Synth}, {"Drum", graph::NodeType::Drum},
      {"Gain", graph::NodeType::Gain},     {"Filter", graph::NodeType::Filter}, {"Delay", graph::NodeType::Delay},
      {"Mixer", graph::NodeType::Mixer}};

  for (const auto& [label, type] : nodeButtons) {
    auto button = std::make_unique<juce::TextButton>(label);
    button->onClick = [this, type] { addNode(type); };
    toolbar_.addAndMakeVisible(*button);
    buttons_.push_back(std::move(button));
  }

  auto saveButton = std::make_unique<juce::TextButton>("Save");
  saveButton->onClick = [this] { savePatch(); };
  toolbar_.addAndMakeVisible(*saveButton);
  buttons_.push_back(std::move(saveButton));

  auto loadButton = std::make_unique<juce::TextButton>("Load");
  loadButton->onClick = [this] { loadPatch(); };
  toolbar_.addAndMakeVisible(*loadButton);
  buttons_.push_back(std::move(loadButton));

  canvas_.setGraphEditCallback([this] {
    if (!applyingHistory_) {
      pushUndoState();
    }
  });
  canvas_.setGraphChangedCallback([this] { rebuildAudioPlan(); });
  canvas_.setStatusCallback([this](const std::string& text) { status_.setText(text, juce::dontSendNotification); });

  // Default patch
  addNode(graph::NodeType::Output);
  addNode(graph::NodeType::Synth);

  // Initial undo baseline
  undoStack_.push_back(persistence::PatchSerializer::toJson(graph_));
}

MainComponent::~MainComponent() {
  shutdownAudio();
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
  canvas_.setBounds(area);
}

void MainComponent::addNode(graph::NodeType type) {
  if (!applyingHistory_) {
    pushUndoState();
  }
  const float x = 80.0f + static_cast<float>(graph_.getNodes().size() * 24);
  const float y = 100.0f + static_cast<float>(graph_.getNodes().size() * 24);
  graph_.addNode(type, {x, y});
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
    canvas_.repaint();
    rebuildAudioPlan();
    status_.setText("Redo", juce::dontSendNotification);
  } else {
    status_.setText("Redo failed: " + error, juce::dontSendNotification);
  }
  applyingHistory_ = false;
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
