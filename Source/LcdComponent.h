#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/midi/juce_MidiMessage.h>

// MC-50風 液晶画面コンポーネント
class LcdComponent : public juce::Component {
public:
    LcdComponent();
    ~LcdComponent() override = default;

    void paint(juce::Graphics& g) override;

    enum class EditMode
    {
        None,
        StepTime,
        Note,
        Velocity,
        GateTime
    };
    void setEditMode (EditMode mode);

    // 外部（テンキーなど）から値を書き換えるための関数群
    void setTimePosition(int measure, int beat, int clock);
    void setNoteInfo(const juce::String& name, int number);
    void setMeasure(int measure);
    void setBeat(int beat);
    void setClock(int clock);
    void setStepTime(int step);
    void setNoteName(const juce::String& name);
    void setNoteNumber(int number);
    void setVelocity(int vel);
    void setGateTime(int gate);
    // 現在のパラメータ値を取得するためのゲッター関数群
    int getMeasure() const { return mMeasure; }
    int getBeat() const { return mBeat; }
    int getClock() const { return mClock; }
    int getStepTime() const { return mStepTime; }
    juce::String getNoteName() const { return mNoteName; }
    int getNoteNumber() const { return mNoteNumber; }
    int getVelocity() const { return mVelocity; }
    int getGateTime() const { return mGateTime; }
    EditMode getEditMode() const { return mCurrentMode; }

private:
    // 液晶に表示するデータ（状態）
    int mMeasure = 1;
    int mBeat = 1;
    int mClock = 0;
    juce::String mNoteName = "C#-1";
    int mNoteNumber = 60;
    int mVelocity = 64;
    int mGateTime = 96;
    int mStepTime = 96;
    EditMode mCurrentMode = EditMode::StepTime; // 初期状態はStepTime選択状態

    juce::String getMc500StyleNoteName(int noteNumber);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LcdComponent)
};
