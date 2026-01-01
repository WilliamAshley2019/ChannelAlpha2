// ============================================================================
// Source/PluginProcessor.h
// ============================================================================
#pragma once
#include <JuceHeader.h>
#include "DDX3216Emulator.h"

class ChannelAlpha2Processor : public juce::AudioProcessor {
public:
    ChannelAlpha2Processor();
    ~ChannelAlpha2Processor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Channel Alpha 2"; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    bool isMuted() const { return muted.load(); }
    bool isSoloed() const { return soloed.load(); }
    bool isSelected() const { return selected.load(); }
    bool isAutoRecEnabled() const { return autoRec.load(); }
    bool isDDXEmulationEnabled() const { return ddxEmulation.load(); }

    void setMuted(bool shouldMute);
    void setSoloed(bool shouldSolo);
    void setSelected(bool shouldSelect);
    void setAutoRec(bool shouldAutoRec);
    void setDDXEmulation(bool shouldEnable);

    int getChannelID() const;

private:
    juce::AudioProcessorValueTreeState apvts;

    std::atomic<bool> muted{ false };
    std::atomic<bool> soloed{ false };
    std::atomic<bool> selected{ false };
    std::atomic<bool> autoRec{ false };
    std::atomic<bool> ddxEmulation{ false };

    // Smooth parameter changes (NO MORE ZIPPER!)
    juce::LinearSmoothedValue<float> muteGain;
    juce::LinearSmoothedValue<float> faderGain;
    juce::LinearSmoothedValue<float> panValue;

    DDX3216Emulator ddxEmulator;

    static constexpr const char* PARAM_CHANNEL_ID = "channelID";
    static constexpr const char* PARAM_FADER = "fader";
    static constexpr const char* PARAM_PAN = "pan";

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelAlpha2Processor)
};
