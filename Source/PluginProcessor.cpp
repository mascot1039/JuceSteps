#include "PluginProcessor.h"
#include "PluginEditor.h"

JuceStepsAudioProcessor::JuceStepsAudioProcessor()
    : juce::AudioProcessor (
          BusesProperties()
              .withInput (
                  "Input",
                  juce::AudioChannelSet::stereo(),
                  true)
              .withOutput (
                  "Output",
                  juce::AudioChannelSet::stereo(),
                  true))
{
}

JuceStepsAudioProcessor::~JuceStepsAudioProcessor() = default;

void JuceStepsAudioProcessor::prepareToPlay (
    double sampleRate,
    int samplesPerBlock)
{
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void JuceStepsAudioProcessor::releaseResources()
{
}

bool JuceStepsAudioProcessor::isBusesLayoutSupported (
    const BusesLayout& layouts) const
{
    const auto mainInput = layouts.getMainInputChannelSet();
    const auto mainOutput = layouts.getMainOutputChannelSet();

    // MIDI用途が中心なので、モノラルまたはステレオを許可する。
    if (mainInput != juce::AudioChannelSet::mono()
        && mainInput != juce::AudioChannelSet::stereo())
        return false;

    if (mainOutput != juce::AudioChannelSet::mono()
        && mainOutput != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void JuceStepsAudioProcessor::processBlock (
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    // 現段階ではオーディオ処理・MIDI処理を行わない。
    // オーディオ出力に不要な入力音が残らないようにする。
    buffer.clear();
}

juce::AudioProcessorEditor*
JuceStepsAudioProcessor::createEditor()
{
    return new JuceStepsAudioProcessorEditor (*this);
}

bool JuceStepsAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String JuceStepsAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool JuceStepsAudioProcessor::acceptsMidi() const
{
    return true;
}

bool JuceStepsAudioProcessor::producesMidi() const
{
    return true;
}

bool JuceStepsAudioProcessor::isMidiEffect() const
{
    return true;
}

double JuceStepsAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int JuceStepsAudioProcessor::getNumPrograms()
{
    return 1;
}

int JuceStepsAudioProcessor::getCurrentProgram()
{
    return 0;
}

void JuceStepsAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String
JuceStepsAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void JuceStepsAudioProcessor::changeProgramName (
    int index,
    const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

void JuceStepsAudioProcessor::getStateInformation (
    juce::MemoryBlock& destData)
{
    juce::ignoreUnused (destData);
}

void JuceStepsAudioProcessor::setStateInformation (
    const void* data,
    int sizeInBytes)
{
    juce::ignoreUnused (data, sizeInBytes);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JuceStepsAudioProcessor();
}
