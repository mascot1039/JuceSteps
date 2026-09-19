#include <juce_gui_basics/juce_gui_basics.h>
#include "MC500_InputComponent.h"

// ==============================================================================
// 1. アプリケーションのウィンドウを管理するクラス
// ==============================================================================
class MainWindow    : public juce::DocumentWindow
{
public:
    MainWindow (juce::String name)
        : DocumentWindow (name,
                          juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                      .findColour (juce::ResizableWindow::backgroundColourId),
                          DocumentWindow::closeButton | DocumentWindow::minimiseButton)
    {
        setUsingNativeTitleBar (false);

        // 1. 先にメインとなるコンポーネントのインスタンスを生成
        auto* mainComponent = new MC500_InputComponent();

        // 2. ウィンドウにセットする前に、コンポーネント自体の初期サイズを必ず指定する（★最重要修正ポイント）
        mainComponent->setSize (590, 320);

        // 3. ウィンドウの所有権をセット（第2引数を true にするとウィンドウが自動的にアスペクト比などを維持・追従します）
        setContentOwned (mainComponent, true);

        #if JUCE_IOS || JUCE_ANDROID
         setFullScreen (true);
        #else
         setResizable (false, false);
        #endif

        setVisible (true);
    }

    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
};

// ==============================================================================
// 2. アプリケーション自体のライフサイクルを管理するクラス
// ==============================================================================
class JuceGuiApplication  : public juce::JUCEApplication
{
public:
    JuceGuiApplication() {}

    const juce::String getApplicationName() override       { return JUCE_APPLICATION_NAME; }
    const juce::String getApplicationVersion() override    { return JUCE_APPLICATION_VERSION; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    void initialise (const juce::String& commandLine) override
    {
        mainWindow.reset (new MainWindow (getApplicationName()));
    }

    void shutdown() override
    {
        mainWindow.reset();
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted (const juce::String& commandLine) override
    {
    }

private:
    std::unique_ptr<MainWindow> mainWindow;
};

// ==============================================================================
// アプリケーションのエントリーポイント（main関数）を生成するマクロ
// ==============================================================================
START_JUCE_APPLICATION (JuceGuiApplication)
