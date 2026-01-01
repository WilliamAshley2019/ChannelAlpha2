// ============================================================================
// Source/DDX3216Emulator.cpp
// ============================================================================
#include "DDX3216Emulator.h"

DDX3216Emulator::DDX3216Emulator() {}

void DDX3216Emulator::prepare(double sampleRate, int samplesPerBlock, int numChannels) {
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32)samplesPerBlock;
    spec.numChannels = (juce::uint32)numChannels;

    characterChain.prepare(spec);

    // DDX3216 character:
    // 1. Subtle HPF at 20Hz (like DDX hardware)
    auto hpfCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(
        sampleRate, 20.0f, 0.707f);
    *characterChain.get<0>().coefficients = *hpfCoeffs;

    // 2. Gentle low-shelf boost (analog warmth, +0.5dB @ 200Hz)
    auto warmthCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        sampleRate, 200.0f, 0.7f, juce::Decibels::decibelsToGain(0.5f));
    *characterChain.get<1>().coefficients = *warmthCoeffs;

    // 3. Subtle air presence (+0.3dB @ 10kHz)
    auto airCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sampleRate, 10000.0f, 0.7f, juce::Decibels::decibelsToGain(0.3f));
    *characterChain.get<2>().coefficients = *airCoeffs;
}

void DDX3216Emulator::reset() {
    characterChain.reset();
}

void DDX3216Emulator::setEnabled(bool shouldEnable) {
    enabled = shouldEnable;
}

float DDX3216Emulator::applySoftSaturation(float sample) {
    // Subtle analog-style soft clipping (tanh-based)
    // Adds 2nd/3rd harmonics like analog circuits
    const float drive = 1.02f; // Very gentle
    return std::tanh(sample * drive) / std::tanh(drive);
}

void DDX3216Emulator::processBlock(juce::AudioBuffer<float>& buffer) {
    if (!enabled) return;

    // Process through character chain
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    characterChain.process(context);

    // Apply subtle saturation for analog warmth
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        auto* channelData = buffer.getWritePointer(ch);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            channelData[sample] = applySoftSaturation(channelData[sample]);
        }
    }
}
