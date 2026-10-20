#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// MC-50風 液晶画面コンポーネント
class LcdComponent : public juce::Component {
public:
    LcdComponent();
    ~LcdComponent() override = default;

    void paint(juce::Graphics& g) override;

    enum class EditMode
    {
        TimeSignature,
        Editing,
        Viewer
    };
    void setEditMode(EditMode mode);
    EditMode getEditMode() const { return mCurrentMode; }

    enum class EditParameter
    {
        None,
        Numerator,      // 分子
        Denominator,    // 分母
        Bar,
        Beat,
        Tick,
        StepTime,
        Note,
        Velocity,
        GateTime
    };
    void setEditParameter(EditParameter parameter);
    EditParameter getEditParameter() const { return mCurrentParameter; }

    // 外部（テンキーなど）から値を書き換えるための関数群
    void setTimePosition(int measure, int beat, int clock);
    void setNoteInfo(const juce::String& name, int number);
    void setMeasure(int measure);
    void setBeat(int beat);
    void setTick(int tick);
    void setStepTime(int step);
    void setNoteName(const juce::String& name);
    void setNoteNumber(int number);
    void setVelocity(int vel);
    void setGateTime(int gate);
    void setNumerator(int numrator);
    void setDenominator(int denominator);
    void setTrack(int track);
    void setMidiChannel(int midi);
    // 現在のパラメータ値を取得するためのゲッター関数群
    int getMeasure() const { return mMeasure; }
    int getBeat() const { return mBeat; }
    int getTick() const { return mTick; }
    int getStepTime() const { return mStepTime; }
    juce::String getNoteName() const { return mNoteName; }
    int getNoteNumber() const { return mNoteNumber; }
    int getVelocity() const { return mVelocity; }
    int getGateTime() const { return mGateTime; }
    int getNumerator() const { return mNumerator; }
    int getDenomirator() const { return mDenominator; }
    int getTrack() const { return mTrack; }
    int getMidiChannel() const { return mMidi; }

    void addTick(int tick);

private:
    // 液晶に表示するデータ（状態）
    int mNumerator = 4;
    int mDenominator = 4;
    int mTrack = 1;
    int mMidi = 1;
    int mMeasure = 1;
    int mBeat = 1;
    int mTick = 0;
    int mStepTime = 24;
    juce::String mNoteName = "    ";
    int mNoteNumber = -1;
    int mVelocity = -1;
    int mGateTime = -1;
    EditMode mCurrentMode = EditMode::TimeSignature;
    EditParameter mCurrentParameter = EditParameter::Numerator;

    juce::String getMc500StyleNoteName(int noteNumber);
    int getNoteNumberFromName(const juce::String& inputName);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LcdComponent)
};
