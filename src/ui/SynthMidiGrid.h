#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <array>
#include <functional>

namespace ui {

class SynthMidiGrid : public juce::Component {
 public:
  using Notes = std::array<int, 16>;
  using NotesChangedCallback = std::function<void(const Notes&)>;

  SynthMidiGrid();

  void setNotes(const Notes& notes);
  const Notes& getNotes() const noexcept { return notes_; }
  void setNotesChangedCallback(NotesChangedCallback cb) { onNotesChanged_ = std::move(cb); }

  void paint(juce::Graphics& g) override;
  void mouseDown(const juce::MouseEvent& event) override;
  void mouseDrag(const juce::MouseEvent& event) override;

 private:
  void setCellAt(juce::Point<float> p, bool clearCell);

  static constexpr int kSteps = 16;
  static constexpr int kRows = 12;
  static constexpr int kMidiMin = 60;  // C4

  Notes notes_{{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}};
  NotesChangedCallback onNotesChanged_;
};

}  // namespace ui
