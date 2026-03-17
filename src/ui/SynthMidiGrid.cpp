#include "SynthMidiGrid.h"

#include <algorithm>

namespace ui {

SynthMidiGrid::SynthMidiGrid() {
  setInterceptsMouseClicks(true, false);
}

void SynthMidiGrid::setStepMasks(const StepMasks& masks) {
  stepMasks_ = masks;
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

      const int semitone = kRows - 1 - row;
      const uint16_t bit = static_cast<uint16_t>(1u << semitone);
      const bool active = (stepMasks_[static_cast<size_t>(step)] & bit) != 0u;

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

  const int semitone = kRows - 1 - row;
  const uint16_t bit = static_cast<uint16_t>(1u << semitone);
  const uint16_t oldMask = stepMasks_[static_cast<size_t>(step)];
  const uint16_t newMask = clearCell ? static_cast<uint16_t>(oldMask & ~bit)
                                     : static_cast<uint16_t>(oldMask | bit);

  if (newMask == oldMask) {
    return;
  }

  stepMasks_[static_cast<size_t>(step)] = newMask;
  repaint();

  if (onNotesChanged_) {
    onNotesChanged_(stepMasks_);
  }
}

}  // namespace ui
