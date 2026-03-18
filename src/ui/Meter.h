#pragma once

#include <JuceHeader.h>
#include "../Globals.h"
#include "UIUtils.h"

using namespace globals;
class SirialAudioProcessor;

class Meter : public juce::Component, private juce::Timer
{
public:
    Meter(SirialAudioProcessor& p);
    ~Meter() override;
    void timerCallback() override;

    void paint(juce::Graphics& g) override;

private:
    SirialAudioProcessor& audioProcessor;
    float db6 = 0.f;
    float db18 = 0.f;
    float db30 = 0.f;
    float db42 = 0.f;
    float rmsSmoothedL = 0.f;
    float rmsSmoothedR = 0.f;
};