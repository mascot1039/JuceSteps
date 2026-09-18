#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// MC-50風 液晶画面コンポーネント
class LcdComponent : public juce::Component {
public:
    LcdComponent();
    ~LcdComponent() override = default;

    void paint(juce::Graphics& g) override;

    // 外部（テンキーなど）から値を書き換えるための関数群
    void setTimePosition(int measure, int beat, int clock);
    void setNoteInfo(const juce::String& name, int number);
    void setVelocity(int vel);
    void setGateTime(int gate);
    void setStepTime(int step);

private:
    // 液晶に表示するデータ（状態）
    int mMeasure = 1;
    int mBeat = 1;
    int mClock = 0;
    juce::String mNoteName = "C 3";
    int mNoteNumber = 60;
    int mVelocity = 64;
    int mGateTime = 96;
    int mStepTime = 96;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LcdComponent)
};
