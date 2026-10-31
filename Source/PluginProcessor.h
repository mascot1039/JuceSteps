#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class JuceStepsAudioProcessor : public juce::AudioProcessor
{
public:
    JuceStepsAudioProcessor();
    ~JuceStepsAudioProcessor() override;

    // プラグインの基本情報
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    // オーディオ処理
    void processBlock (
        juce::AudioBuffer<float>& buffer,
        juce::MidiBuffer& midiMessages) override;

    // エディター
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    // プラグイン情報
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;

    double getTailLengthSeconds() const override;

    // プログラム管理
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    // 状態の保存・復元
    void getStateInformation (
        juce::MemoryBlock& destData) override;

    void setStateInformation (
        const void* data,
        int sizeInBytes) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (
        JuceStepsAudioProcessor)
};

// JUCEがプラグインを生成するときに呼び出す関数
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();
