#include "PluginEditor.h"

JuceStepsAudioProcessorEditor::JuceStepsAudioProcessorEditor (
    JuceStepsAudioProcessor& processor)
    : juce::AudioProcessorEditor (&processor)
{
    // 既存のMC-500風操作画面を表示する
    addAndMakeVisible (mc500Component);

    // 既存の画面と同じサイズに設定する
    setSize (590, 320);
}

void JuceStepsAudioProcessorEditor::paint (juce::Graphics& g)
{
    // エディターの背景を黒にする
    g.fillAll (juce::Colours::black);
}

void JuceStepsAudioProcessorEditor::resized()
{
    // エディター全体に操作画面を配置する
    mc500Component.setBounds (getLocalBounds());
}
