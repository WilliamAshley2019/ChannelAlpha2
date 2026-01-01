// ============================================================================
// Source/PluginProcessor.cpp
// ============================================================================
#include "PluginProcessor.h"
#include "PluginEditor.h"

ChannelAlpha2Processor::ChannelAlpha2Processor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    muteGain.setCurrentAndTargetValue(1.0f);
    faderGain.setCurrentAndTargetValue(1.0f);
    panValue.setCurrentAndTargetValue(0.0f);
}

ChannelAlpha2Processor::~ChannelAlpha2Processor() {}

juce::AudioProcessorValueTreeState::ParameterLayout ChannelAlpha2Processor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{ PARAM_CHANNEL_ID, 1 },
        "Channel ID",
        1, 32, 1));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ PARAM_FADER, 1 },
        "Fader",
        juce::NormalisableRange<float>(-60.0f, 10.0f, 0.1f, 2.0f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ PARAM_PAN, 1 },
        "Pan",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f),
        0.0f));

    return { params.begin(), params.end() };
}

void ChannelAlpha2Processor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    juce::ignoreUnused(samplesPerBlock);

    // Smooth parameter changes over 50ms (NO MORE ZIPPER!)
    muteGain.reset(sampleRate, 0.05);
    faderGain.reset(sampleRate, 0.05);
    panValue.reset(sampleRate, 0.05);

    muteGain.setCurrentAndTargetValue(muted ? 0.0f : 1.0f);

    // Prepare DDX3216 emulation
    ddxEmulator.prepare(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
}

void ChannelAlpha2Processor::releaseResources() {
    ddxEmulator.reset();
}

void ChannelAlpha2Processor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Update smoothed parameter targets
    float faderDB = apvts.getRawParameterValue(PARAM_FADER)->load();
    float pan = apvts.getRawParameterValue(PARAM_PAN)->load();

    faderGain.setTargetValue(juce::Decibels::decibelsToGain(faderDB));
    panValue.setTargetValue(pan);
    muteGain.setTargetValue(muted ? 0.0f : 1.0f);

    // Process audio with SMOOTH parameter changes
    if (buffer.getNumChannels() >= 2) {
        auto* leftChannel = buffer.getWritePointer(0);
        auto* rightChannel = buffer.getWritePointer(1);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            // Get smoothed values for THIS sample
            float currentFader = faderGain.getNextValue();
            float currentPan = panValue.getNextValue();
            float currentMute = muteGain.getNextValue();

            // Constant power panning
            float panAngle = (currentPan + 1.0f) * 0.25f * juce::MathConstants<float>::pi;
            float leftPanGain = std::cos(panAngle);
            float rightPanGain = std::sin(panAngle);

            // Apply all gains smoothly
            float totalGain = currentFader * currentMute;
            leftChannel[sample] *= totalGain * leftPanGain;
            rightChannel[sample] *= totalGain * rightPanGain;
        }
    }
    else if (buffer.getNumChannels() == 1) {
        auto* channel = buffer.getWritePointer(0);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            float currentFader = faderGain.getNextValue();
            float currentMute = muteGain.getNextValue();
            channel[sample] *= currentFader * currentMute;
        }
    }

    // Apply DDX3216 emulation if enabled
    ddxEmulator.processBlock(buffer);
}

void ChannelAlpha2Processor::setMuted(bool shouldMute) {
    muted = shouldMute;
}

void ChannelAlpha2Processor::setSoloed(bool shouldSolo) {
    soloed = shouldSolo;
}

void ChannelAlpha2Processor::setSelected(bool shouldSelect) {
    selected = shouldSelect;
}

void ChannelAlpha2Processor::setAutoRec(bool shouldAutoRec) {
    autoRec = shouldAutoRec;
}

void ChannelAlpha2Processor::setDDXEmulation(bool shouldEnable) {
    ddxEmulation = shouldEnable;
    ddxEmulator.setEnabled(shouldEnable);
}

int ChannelAlpha2Processor::getChannelID() const {
    return static_cast<int>(apvts.getRawParameterValue(PARAM_CHANNEL_ID)->load());
}

juce::AudioProcessorEditor* ChannelAlpha2Processor::createEditor() {
    return new ChannelAlpha2Editor(*this);
}

void ChannelAlpha2Processor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState();

    state.setProperty("muted", muted.load(), nullptr);
    state.setProperty("soloed", soloed.load(), nullptr);
    state.setProperty("selected", selected.load(), nullptr);
    state.setProperty("autoRec", autoRec.load(), nullptr);
    state.setProperty("ddxEmulation", ddxEmulation.load(), nullptr);

    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ChannelAlpha2Processor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr) {
        if (xmlState->hasTagName(apvts.state.getType())) {
            auto state = juce::ValueTree::fromXml(*xmlState);
            apvts.replaceState(state);

            muted = state.getProperty("muted", false);
            soloed = state.getProperty("soloed", false);
            selected = state.getProperty("selected", false);
            autoRec = state.getProperty("autoRec", false);

            bool ddxState = state.getProperty("ddxEmulation", false);
            setDDXEmulation(ddxState);
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new ChannelAlpha2Processor();
}
