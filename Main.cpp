#include <juce_gui_basics/juce_gui_basics.h>

class MainComponent : public juce::Component {
public:
    MainComponent() {
        // 1. 【フォント対策】ホスト側の環境にある「IPAGothic」を全体に強制適用します
        juce::LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypefaceName("IPAGothic");

        // --- 1. テキスト入力欄の準備 ---
        // ★【修正】juce::String::fromUTF8 を使用して、安全に日本語を渡します
        myTextBox.setTextToShowWhenEmpty(juce::String::fromUTF8("ここに文字を入力してください..."), juce::Colours::grey);
        addAndMakeVisible(myTextBox);

        // --- 2. ボタンの準備 ---
        // ★【修正】こちらも同じく fromUTF8 を使用します
        myButton.setButtonText(juce::String::fromUTF8("文字を送信"));
        addAndMakeVisible(myButton);

        myButton.onClick = [this]() {
            juce::String currentText = myTextBox.getText();
            DBG("入力された文字: " << currentText);
            myTextBox.setText(currentText.toUpperCase());
        };

        setSize(800, 600);
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::darkgrey);

        g.setColour(juce::Colours::white);
        g.setFont (20.0f);

        // ★【修正】タイトル部分も fromUTF8 を使用して描画します
        g.drawText(juce::String::fromUTF8("JUCE コンポーネント追加のテスト"), 20, 20, 400, 30, juce::Justification::left);
    }

    void resized() override {
        myTextBox.setBounds(20, 70, 500, 40);
        myButton.setBounds(530, 70, 150, 40);
    }

private:
    juce::TextEditor myTextBox;
    juce::TextButton myButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

class Application : public juce::JUCEApplication {
public:
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
