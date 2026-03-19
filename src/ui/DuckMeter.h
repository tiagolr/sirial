#pragma once

#include <JuceHeader.h>
#include "../Globals.h"

using namespace globals;
class SirialAudioProcessor;

class DuckMeter : public juce::Component, private juce::Timer
{
public:
    DuckMeter(SirialAudioProcessor& p);
    ~DuckMeter() override;
    void timerCallback() override;

    void paint(juce::Graphics& g) override;

private:
    SirialAudioProcessor& audioProcessor;
};