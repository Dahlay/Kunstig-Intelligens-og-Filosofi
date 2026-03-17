#include "app/MainComponent.h"

#include <juce_gui_extra/juce_gui_extra.h>

class ModularPatcherApplication final : public juce::JUCEApplication {
 public:
  const juce::String getApplicationName() override { return "Modular Audio Patcher"; }
  const juce::String getApplicationVersion() override { return "0.1.0"; }

  void initialise(const juce::String&) override {
    mainWindow_ = std::make_unique<MainWindow>(getApplicationName());
  }

  void shutdown() override {
    mainWindow_.reset();
  }

 private:
  class MainWindow : public juce::DocumentWindow {
   public:
    explicit MainWindow(juce::String name)
        : juce::DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(
                                         juce::ResizableWindow::backgroundColourId),
                               juce::DocumentWindow::allButtons) {
      setUsingNativeTitleBar(true);
      setContentOwned(new MainComponent(), true);
      setResizable(true, true);
      centreWithSize(getWidth(), getHeight());
      setVisible(true);
    }

    void closeButtonPressed() override {
      juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
  };

  std::unique_ptr<MainWindow> mainWindow_;
};

START_JUCE_APPLICATION(ModularPatcherApplication)
