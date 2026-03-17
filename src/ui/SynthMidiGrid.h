#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <array>
#include <functional>

namespace ui {

class SynthMidiGrid : public juce::Component {
 public:
  using StepMasks = std::array<uint16_t, 16>;
  using NotesChangedCallback = std::function<void(const StepMasks&)>;

  SynthMidiGrid();

  void setStepMasks(const StepMasks& masks);
  const StepMasks& getStepMasks() const noexcept { return stepMasks_; }
  void setNotesChangedCallback(NotesChangedCallback cb) { onNotesChanged_ = std::move(cb); }

  void paint(juce::Graphics& g) override;
  void mouseDown(const juce::MouseEvent& event) override;
  void mouseDrag(const juce::MouseEvent& event) override;

 private:
  void setCellAt(juce::Point<float> p, bool clearCell);

  static constexpr int kSteps = 16;
  static constexpr int kRows = 12;
  static constexpr int kMidiMin = 60;  // C4

  StepMasks stepMasks_{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
  NotesChangedCallback onNotesChanged_;
};

}  // namespace ui
