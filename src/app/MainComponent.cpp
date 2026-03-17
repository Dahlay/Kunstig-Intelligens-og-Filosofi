#include "MainComponent.h"

namespace {
constexpr int kToolbarHeight = 46;
}

MainComponent::MainComponent() : canvas_(graph_) {
  setSize(1280, 800);
  setAudioChannels(0, 2);

  addAndMakeVisible(toolbar_);
  addAndMakeVisible(canvas_);
  addAndMakeVisible(status_);

  status_.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
  status_.setText("Ready", juce::dontSendNotification);

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
    button->setBounds(x, 8, 86, 30);
    x += 90;
  }

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

void MainComponent::rebuildAudioPlan() {
  std::string error;
  if (!engine_.setGraph(graph_, error)) {
    status_.setText(error, juce::dontSendNotification);
  }
}
