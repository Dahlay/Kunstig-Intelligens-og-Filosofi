#include "SynthMidiGrid.h"

#include <algorithm>

namespace ui {

SynthMidiGrid::SynthMidiGrid() {
  setInterceptsMouseClicks(true, false);
}

void SynthMidiGrid::setNotes(const Notes& notes) {
  notes_ = notes;
  repaint();
}

void SynthMidiGrid::paint(juce::Graphics& g) {
  auto r = getLocalBounds().toFloat();
  g.setColour(juce::Colour(0x7affffff));
  g.fillRoundedRectangle(r, 8.0f);
  g.setColour(juce::Colour(0x8870b7dc));
  g.drawRoundedRectangle(r, 8.0f, 1.0f);

  const float cw = r.getWidth() / static_cast<float>(kSteps);
  const float ch = r.getHeight() / static_cast<float>(kRows);

  for (int step = 0; step < kSteps; ++step) {
    for (int row = 0; row < kRows; ++row) {
      const float x = r.getX() + static_cast<float>(step) * cw;
      const float y = r.getY() + static_cast<float>(row) * ch;
      juce::Rectangle<float> cell{x, y, cw, ch};

      const int midiNote = kMidiMin + (kRows - 1 - row);
      const bool active = notes_[static_cast<size_t>(step)] == midiNote;

      g.setColour(active ? juce::Colour(0xff8de3d1) : juce::Colour(0x24ffffff));
      g.fillRect(cell.reduced(1.0f));

      g.setColour(juce::Colour(0x3375add5));
      g.drawRect(cell.toNearestInt(), 1);
    }
  }
}

void SynthMidiGrid::mouseDown(const juce::MouseEvent& event) {
  setCellAt(event.position, event.mods.isRightButtonDown());
}

void SynthMidiGrid::mouseDrag(const juce::MouseEvent& event) {
  setCellAt(event.position, event.mods.isRightButtonDown());
}

void SynthMidiGrid::setCellAt(juce::Point<float> p, bool clearCell) {
  if (!getLocalBounds().toFloat().contains(p)) {
    return;
  }

  auto r = getLocalBounds().toFloat();
  const float cw = r.getWidth() / static_cast<float>(kSteps);
  const float ch = r.getHeight() / static_cast<float>(kRows);

  const int step = std::clamp(static_cast<int>((p.x - r.getX()) / cw), 0, kSteps - 1);
  const int row = std::clamp(static_cast<int>((p.y - r.getY()) / ch), 0, kRows - 1);

  const int midiNote = kMidiMin + (kRows - 1 - row);
  const int newValue = clearCell ? -1 : midiNote;

  if (notes_[static_cast<size_t>(step)] == newValue) {
    return;
  }

  notes_[static_cast<size_t>(step)] = newValue;
  repaint();

  if (onNotesChanged_) {
    onNotesChanged_(notes_);
  }
}

}  // namespace ui
