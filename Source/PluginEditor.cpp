// ============================================================================
// Source/PluginEditor.cpp
// ============================================================================
#include "PluginEditor.h"

ChannelAlpha2Editor::ChannelAlpha2Editor(ChannelAlpha2Processor& p)
    : AudioProcessorEditor(&p), processor(p) {

    setSize(80, 530); // Slightly taller for EMU button

    channelLabel.setText("CHANNEL", juce::dontSendNotification);
    channelLabel.setJustificationType(juce::Justification::centred);
    channelLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    channelLabel.setFont(juce::Font(juce::FontOptions(12.0f).withStyle("Bold")));
    addAndMakeVisible(channelLabel);

    for (int i = 1; i <= 32; ++i)
        channelIDSelector.addItem(juce::String(i), i);
    channelIDSelector.setSelectedId(1, juce::dontSendNotification);
    addAndMakeVisible(channelIDSelector);

    channelIDAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        processor.getAPVTS(), "channelID", channelIDSelector);

    panKnob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    panKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    panKnob.setRange(-1.0, 1.0, 0.01);
    panKnob.setValue(0.0);
    panKnob.setDoubleClickReturnValue(true, 0.0);
    addAndMakeVisible(panKnob);

    panAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.getAPVTS(), "pan", panKnob);

    fader.setSliderStyle(juce::Slider::LinearVertical);
    fader.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    fader.setRange(-60.0, 10.0, 0.1);
    fader.setValue(0.0);
    fader.setDoubleClickReturnValue(true, 0.0);
    addAndMakeVisible(fader);

    faderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.getAPVTS(), "fader", fader);

    levelDisplay.setText("0.0 dB", juce::dontSendNotification);
    levelDisplay.setJustificationType(juce::Justification::centred);
    levelDisplay.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
    levelDisplay.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
    addAndMakeVisible(levelDisplay);

    selectButton.setButtonText("SEL");
    selectButton.setClickingTogglesState(true);
    selectButton.onClick = [this] {
        processor.setSelected(selectButton.getToggleState());
        };
    selectButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::yellow);
    addAndMakeVisible(selectButton);

    autoRecButton.setButtonText("AUTO");
    autoRecButton.setClickingTogglesState(true);
    autoRecButton.onClick = [this] {
        processor.setAutoRec(autoRecButton.getToggleState());
        };
    autoRecButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red);
    addAndMakeVisible(autoRecButton);

    soloButton.setButtonText("SOLO");
    soloButton.setClickingTogglesState(true);
    soloButton.onClick = [this] {
        processor.setSoloed(soloButton.getToggleState());
        };
    soloButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::yellow);
    addAndMakeVisible(soloButton);

    muteButton.setButtonText("MUTE");
    muteButton.setClickingTogglesState(true);
    muteButton.onClick = [this] {
        processor.setMuted(muteButton.getToggleState());
        };
    muteButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red);
    addAndMakeVisible(muteButton);

    // NEW: DDX3216 Emulation button
    emuButton.setButtonText("EMU");
    emuButton.setClickingTogglesState(true);
    emuButton.onClick = [this] {
        processor.setDDXEmulation(emuButton.getToggleState());
        };
    emuButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::orange);
    addAndMakeVisible(emuButton);

    updateButtonStates();
    startTimerHz(30);
}

ChannelAlpha2Editor::~ChannelAlpha2Editor() {
    stopTimer();
}

void ChannelAlpha2Editor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour((juce::uint32)BACKGROUND_COLOR));

    auto area = getLocalBounds();
    g.setColour(juce::Colour((juce::uint32)PANEL_COLOR));
    g.fillRect(area.removeFromTop(60));

    g.setColour(juce::Colour((juce::uint32)BORDER_COLOR));
    g.drawRect(getLocalBounds(), 2);
}

void ChannelAlpha2Editor::resized() {
    auto area = getLocalBounds().reduced(5);

    channelLabel.setBounds(area.removeFromTop(18));
    area.removeFromTop(2);
    channelIDSelector.setBounds(area.removeFromTop(24));
    area.removeFromTop(5);

    panKnob.setBounds(area.removeFromTop(60).reduced(5));
    area.removeFromTop(5);

    auto faderHeight = area.getHeight() - 142; // More buttons now
    fader.setBounds(area.removeFromTop(faderHeight).withSizeKeepingCentre(40, faderHeight));

    levelDisplay.setBounds(area.removeFromTop(18));
    area.removeFromTop(2);

    selectButton.setBounds(area.removeFromTop(22));
    area.removeFromTop(2);
    autoRecButton.setBounds(area.removeFromTop(22));
    area.removeFromTop(2);
    soloButton.setBounds(area.removeFromTop(22));
    area.removeFromTop(2);
    muteButton.setBounds(area.removeFromTop(22));
    area.removeFromTop(2);
    emuButton.setBounds(area.removeFromTop(22)); // NEW
}

void ChannelAlpha2Editor::timerCallback() {
    float faderDB = processor.getAPVTS().getRawParameterValue("fader")->load();
    levelDisplay.setText(juce::String(faderDB, 1) + " dB", juce::dontSendNotification);
    updateButtonStates();
}

void ChannelAlpha2Editor::updateButtonStates() {
    selectButton.setToggleState(processor.isSelected(), juce::dontSendNotification);
    autoRecButton.setToggleState(processor.isAutoRecEnabled(), juce::dontSendNotification);
    soloButton.setToggleState(processor.isSoloed(), juce::dontSendNotification);
    muteButton.setToggleState(processor.isMuted(), juce::dontSendNotification);
    emuButton.setToggleState(processor.isDDXEmulationEnabled(), juce::dontSendNotification);
}