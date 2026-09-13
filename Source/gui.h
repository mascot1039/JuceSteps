#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// 画面の見た目や動きを管理するクラス
class MainComponent : public juce::Component {
public:
    MainComponent();
    ~MainComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress& key) override;

private:
    // --- 【重要】コンパイルエラーを解消する位置計算・プリセット関数群 ---
    void advancePosition (int clocksToAdd);
    void rollbackPosition (int clocksToRemove);
    void setStepAndGate (int clockValue);

    // 画面の大枠
    juce::TextEditor stepDataDisplay;
    juce::GroupComponent buttonGroup;
    juce::GroupComponent inputArea;

    // ボタン類
    juce::TextButton myButton;
    juce::TextButton autoForwardButton;
    juce::TextButton skipButton;
    juce::TextButton tieButton;
    juce::TextButton deleteButton;

    // 音符プリセットボタン
    juce::TextButton btnNote2;
    juce::TextButton btnNote4;
    juce::TextButton btnNote8;
    juce::TextButton btnNote8T;
    juce::TextButton btnNote16;
    juce::TextButton btnNote16T;
    juce::TextButton btnNote32;
    juce::TextButton btnNote64;

    // 10キーボタン
    juce::OwnedArray<juce::Component> btn10key;
    juce::TextButton btnEnter;

    // MC-50風の入力アイテム群
    juce::Slider measureSlider;   juce::Slider beatSlider;      juce::Slider clockSlider;
    juce::Slider noteSlider;      juce::Slider velocitySlider;  juce::Slider gateTimeSlider;
    juce::Slider stepTimeSlider;

    // 各項目の名前（ラベル）
    juce::Label measureLabel;    juce::Label beatLabel;       juce::Label clockLabel;
    juce::Label noteLabel;       juce::Label velocityLabel;   juce::Label gateTimeLabel;
    juce::Label stepTimeLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
