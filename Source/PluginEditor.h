// ============================================================================
// Source/PluginEditor.h
// ============================================================================
#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class ChannelAlpha2Editor : public juce::AudioProcessorEditor,
    private juce::Timer {
public:
    ChannelAlpha2Editor(ChannelAlpha2Processor&);
    ~ChannelAlpha2Editor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void updateButtonStates();

    ChannelAlpha2Processor& processor;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> faderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> channelIDAttachment;

    juce::Label channelLabel;
    juce::ComboBox channelIDSelector;
    juce::Slider panKnob;
    juce::Slider fader;
    juce::Label levelDisplay;
    juce::TextButton selectButton;
    juce::TextButton autoRecButton;
    juce::TextButton soloButton;
    juce::TextButton muteButton;
    juce::TextButton emuButton;  // NEW: DDX3216 Emulation

    static constexpr int BACKGROUND_COLOR = 0xff2a2a2a;
    static constexpr int PANEL_COLOR = 0xff1a1a1a;
    static constexpr int BORDER_COLOR = 0xff404040;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelAlpha2Editor)
};
