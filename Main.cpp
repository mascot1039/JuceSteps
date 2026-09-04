#include <juce_gui_basics/juce_gui_basics.h>

class MainComponent : public juce::Component {
public:
    MainComponent() { setSize(800, 600); }
    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::darkgrey);
        g.setColour(juce::Colours::white);
        g.setFont(16.0f);
        g.drawText("Hello JUCE with CMake!", getLocalBounds(), juce::Justification::centred, true);
    }
    void resized() override {}
};

class Application : public juce::JUCEApplication {
public:
    //const juce::String getApplicationName() override { return "JuceMinimum"; }
    //const juce::String getApplicationVersion() override { return "1.0.0"; }
    const juce::String getApplicationName() override       { return JUCE_APPLICATION_NAME; }
    const juce::String getApplicationVersion() override    { return JUCE_APPLICATION_VERSION; }
    void initialise(const juce::String&) override {
        window = std::make_unique<MainWindow>(getApplicationName());
    }
    void shutdown() override { window = nullptr; }

private:
    class MainWindow : public juce::DocumentWindow {
    public:
        MainWindow(const juce::String& name)
            : DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), allButtons) {
            setContentOwned(new MainComponent(), true);
            centreWithSize(getWidth(), getHeight());
            setVisible(true);
        }
        void closeButtonPressed() override { juce::JUCEApplication::getInstance()->systemRequestedQuit(); }
    };
    std::unique_ptr<MainWindow> window;
};

START_JUCE_APPLICATION(Application)
