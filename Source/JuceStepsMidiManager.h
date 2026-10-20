#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_basics/midi/juce_MidiMessage.h>
#include <juce_audio_basics/midi/juce_MidiMessageSequence.h>
#include <juce_audio_basics/midi/juce_MidiFile.h>
#include <vector>

// --- MC-50のディスプレイと1対1で対応するステップデータ構造体 ---
struct JuceStepEvent
{
    int bar = 1;            // 小節 (1始まり) [例: 0001]
    int beat = 1;           // 拍 (1始まり)   [例: 01]
    int tick = 0;           // 拍内クロック (0始まり) [例: 000]

    int stepTime = 24;      // ステップタイム
    int noteNumber = 60;    // ノート番号 (0 - 127) [例: C 4 (060)]
    int velocity = 64;      // ベロシティ (0 - 127) [例: V:064]
    int gateTime = 96;      // ゲートタイム (発音クロック数) [例: G:0096]

    // MC-50仕様（1拍=96クロック）で通算の絶対ティックを計算
    int getAbsoluteTick() const;
};

// --- 独自フォーマットで管理し、SMFへ一括エクスポートするクラス ---
class JuceStepsMidiManager
{
public:
    JuceStepsMidiManager();
    ~JuceStepsMidiManager() = default;

    void addStepEvent(const JuceStepEvent& newStep);    // ステップデータを追加
    void clearAll();                                    // データの全消去
    //const std::vector<JuceStepEvent>& getStepEvents() const;    // イベントリストの取得
    std::vector<JuceStepEvent>& getStepEvents();    // イベントリストの取得

    void setNumerator(int numerator);       // 拍子（分子）を設定する
    void setDenominator(int denominator);   // 拍子（分母）を設定する
    void setTrack(int track);               // トラックを設定する
    void setMidiChannel(int channel);       // MIDIチャンネルを設定する
    int getNumerator() const { return mNumerator; }             // 拍子（分子）を取得する
    int getDenominator() const { return mDenominator; }         // 拍子（分母）を取得する
    int getTrack() const { return mTrack; }                     // トラックを取得する
    int getMidiChannel() const { return mChannel; }             // MIDIチャンネルを取得する

    // SMF（.midファイル）へのエクスポート
    bool exportToStandardMidiFile(const juce::File& targetFile);

private:
    std::vector<JuceStepEvent> stepEvents;
    int ticksPerQuarterNote;

    int mNumerator = 4;
    int mDenominator = 4;
    int mTrack = 1;
    int mChannel = 1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JuceStepsMidiManager)
};
