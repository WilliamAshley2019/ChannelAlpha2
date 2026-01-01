// ============================================================================
// Source/DDX3216Emulator.h
// ============================================================================
#pragma once
#include <JuceHeader.h>

class DDX3216Emulator {
public:
    DDX3216Emulator();

    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void reset();

    void setEnabled(bool shouldEnable);
    bool isEnabled() const { return enabled; }

    void processBlock(juce::AudioBuffer<float>& buffer);

private:
    bool enabled = false;

    // DDX3216 character emulation components
    juce::dsp::ProcessorChain<
        juce::dsp::IIR::Filter<float>,  // Subtle HPF (20Hz, like DDX)
        juce::dsp::IIR::Filter<float>,  // Analog warmth shelf
        juce::dsp::IIR::Filter<float>   // Gentle air presence
    > characterChain;

    juce::dsp::ProcessSpec spec;

    // Soft saturation for subtle harmonic coloration
    float applySoftSaturation(float sample);
};
