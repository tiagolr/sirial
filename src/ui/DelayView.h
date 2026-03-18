#pragma once

#include <JuceHeader.h>
#include "../Globals.h"
#include "UIUtils.h"
#include "../dsp/Delay.h"

using namespace globals;
class SirialAudioProcessorEditor;

class DelayView 
    : public juce::Component
    , private juce::AudioProcessorValueTreeState::Listener
{
public:
    struct Tap 
    {
        float ampL;
        float ampR;
        float timeL;
        float timeR;
        float feedback;
        float snapacc; // used for snapping to other same tap channel
    };

    DelayView(SirialAudioProcessorEditor& p);
    ~DelayView() override;
    void parameterChanged(const juce::String& parameterID, float newValue);

    void mouseMove(const MouseEvent& e) override;
    void mouseDown(const MouseEvent& e) override;
    void mouseUp(const MouseEvent& e) override;
    void mouseDrag(const MouseEvent& e) override;

    void updateAmplitudes(const MouseEvent& e);

    void paint(Graphics& g) override;
    void drawGrid(Graphics& g);
    void drawTaps(Graphics& g);

private:
    SirialAudioProcessorEditor& editor;

    Point<int> last_mouse_position{};
    int mouseOverBase = -1;
    int selectedTap = 0;
    bool link = false;
    Delay::DelayMode mode = Delay::DelayMode::Mono;
    int ntaps = 0;
    int userGrid = 0;
    std::array<Tap, MAX_TAPS> taps{};
    std::array<Rectangle<float>, MAX_TAPS> bases_mono{};
    std::array<Rectangle<float>, MAX_TAPS> bases_left{};
    std::array<Rectangle<float>, MAX_TAPS> bases_right{};
    Delay::TimeMode timeMode = Delay::Millis;
    Delay::TimeSync timeSync = Delay::k1o4;
    int timeMillis = 0;
    Tap dummyTap{};

    Rectangle<float> viewb{};
};