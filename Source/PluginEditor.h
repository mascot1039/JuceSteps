#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "MC500_InputComponent.h"

class JuceStepsAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit JuceStepsAudioProcessorEditor (JuceStepsAudioProcessor&);

    ~JuceStepsAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // 既存のMC-500風操作画面
    MC500_InputComponent mc500Component;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceStepsAudioProcessorEditor)
};
