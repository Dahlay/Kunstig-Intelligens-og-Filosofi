#include "MainComponent.h"

#include "persistence/PatchSerializer.h"

namespace {
constexpr int kToolbarHeight = 46;
}

MainComponent::MainComponent() : canvas_(graph_) {
  setSize(1280, 800);
  setAudioChannels(0, 2);

  addAndMakeVisible(toolbar_);
  addAndMakeVisible(canvas_);
  addAndMakeVisible(status_);
  addAndMakeVisible(transportButton_);
  addAndMakeVisible(bpmSlider_);
  addAndMakeVisible(bpmLabel_);

  status_.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
  status_.setText("Ready", juce::dontSendNotification);

  transportButton_.setButtonText("Stop");
  transportButton_.onClick = [this] { toggleTransport(); };

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

  canvas_.setGraphChangedCallback([this] { rebuildAudioPlan(); });
  canvas_.setStatusCallback([this](const std::string& text) { status_.setText(text, juce::dontSendNotification); });

  // Default patch
  addNode(graph::NodeType::Output);
  addNode(graph::NodeType::Synth);
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
  bpmLabel_.setBounds(x + 90, 10, 34, 24);
  bpmSlider_.setBounds(x + 124, 8, 170, 30);

  status_.setBounds(getWidth() - 360, 8, 350, 30);
  canvas_.setBounds(area);
}

void MainComponent::addNode(graph::NodeType type) {
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

                              graph_ = std::move(loaded);
                              canvas_.repaint();
                              rebuildAudioPlan();
                              status_.setText("Patch loaded", juce::dontSendNotification);
                            });
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
