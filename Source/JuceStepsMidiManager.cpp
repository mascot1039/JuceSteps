#include "JuceStepsMidiManager.h"

// ==============================================================================
// JuceStepEvent 構造体の実装
// ==============================================================================
int JuceStepEvent::getAbsoluteTick() const
{
    const int ticksPerQuarterNote = 96; // 1拍 = 96ティック
    const int ticksPerBar = ticksPerQuarterNote * 4; // 1小節 = 384ティック (4/4拍子の場合)

    int zeroBasedBar = bar - 1;
    int zeroBasedBeat = beat - 1;

    return (zeroBasedBar * ticksPerBar) + (zeroBasedBeat * ticksPerQuarterNote) + tick;
}

// ==============================================================================
// JuceStepsMidiManager クラスの実装
// ==============================================================================
JuceStepsMidiManager::JuceStepsMidiManager()
{
    // MIDIファイルの分解能（PPQ）をMC-50と同じ「96」に設定
    ticksPerQuarterNote = 96;
}

void JuceStepsMidiManager::addStepEvent(const JuceStepEvent& newStep)
{
    stepEvents.push_back(newStep);
}
void JuceStepsMidiManager::clearAll()
{
    stepEvents.clear();
}
//const std::vector<JuceStepEvent>& JuceStepsMidiManager::getStepEvents() const
std::vector<JuceStepEvent>& JuceStepsMidiManager::getStepEvents()
{
    return stepEvents;
}

// 拍子（分子）を設定する
void JuceStepsMidiManager::setNumerator(int numerator)
{
    mNumerator = numerator;
}
// 拍子（分母）を設定する
void JuceStepsMidiManager::setDenominator(int denominator)
{
    mDenominator = denominator;
}
// トラックを設定する
void JuceStepsMidiManager::setTrack(int track)
{
    mTrack = track;
}
// MIDIチャンネルを設定する
void JuceStepsMidiManager::setMidiChannel(int channel)
{
    mChannel = channel;
}

bool JuceStepsMidiManager::exportToStandardMidiFile(const juce::File& targetFile)
{
    juce::MidiFile midiFile;
    midiFile.setTicksPerQuarterNote(ticksPerQuarterNote);

    juce::MidiMessageSequence trackSequence;

    // 1. タイムシグネチャー（拍子情報）を先頭（0ティック）に追加
    auto timeSigMsg = juce::MidiMessage::timeSignatureMetaEvent(mNumerator, mDenominator);
    trackSequence.addEvent(timeSigMsg, 0.0);

    // 2. 蓄積したMC-50形式のステップデータをループ処理
    for (const auto& step : stepEvents)
    {
        double startTick = static_cast<double>(step.getAbsoluteTick());
        double endTick   = startTick + static_cast<double>(step.gateTime);

        // ベロシティを float (0.0 〜 1.0) に変換
        float floatVelocity = static_cast<float>(step.velocity) / 127.0f;

        // Note On / Off メッセージの作成
        auto noteOn  = juce::MidiMessage::noteOn(mChannel, step.noteNumber, floatVelocity);
        auto noteOff = juce::MidiMessage::noteOff(mChannel, step.noteNumber);

        // シーケンスへ追加
        trackSequence.addEvent(noteOn, startTick);
        trackSequence.addEvent(noteOff, endTick);
    }

    // 3. 書き出し直前にソートとNote On/Offの紐付け
    trackSequence.updateMatchedPairs();

    // 4. トラックをファイルに登録して保存
    midiFile.addTrack(trackSequence);

    targetFile.deleteFile();
    std::unique_ptr<juce::FileOutputStream> stream(targetFile.createOutputStream());

    if (stream != nullptr)
    {
        return midiFile.writeTo(*stream, 1);
    }
    return false;
}
