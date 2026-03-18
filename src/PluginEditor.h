/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Globals.h"
#include "ui/Rotary.h"
#include "ui/CustomLookAndFeel.h"
#include "ui/About.h"
#include "ui/Meter.h"
#include "ui/DelayView.h"
#include "ui/TimePicker.h"
#include "ui/ValuePicker.h"

using namespace globals;

class SirialAudioProcessorEditor
	: public juce::AudioProcessorEditor
	, private juce::AudioProcessorValueTreeState::Listener
	, public juce::ChangeListener
{
public:

    SirialAudioProcessorEditor (SirialAudioProcessor&);
    ~SirialAudioProcessorEditor() override;

    //==============================================================================
    void parameterChanged (const juce::String& parameterID, float newValue) override;
    void toggleUIComponents ();
    void paint (juce::Graphics&) override;
    void resized() override;
    void changeListenerCallback(ChangeBroadcaster* source) override;
    void showSettings();
    void showPresetsMenu();
    void showModeMenu();
    void savePreset();
    void refreshOutGainLabel();

    SirialAudioProcessor& audioProcessor;
private:
    TextButton logo;
    TextButton settingsBtn;
    TextButton presetBtn;
    TextButton nextPresetBtn;
    TextButton prevPresetBtn;
    TextButton saveBtn;
    std::unique_ptr<Meter> meter;

    TextButton modeBtn;
    Label tapsLabel;
    std::unique_ptr<ValuePicker> tapsPicker;
    Label timeLabel;
    std::unique_ptr<TimePicker> tapsTimeSync;
    std::unique_ptr<TimePicker> tapsTimeMillis;
    TextButton millisBtn;
    TextButton straightBtn;
    TextButton tripletBtn;
    TextButton dottedBtn;
    TextButton linkBtn;

    std::unique_ptr<DelayView> delayView;
    std::unique_ptr<Rotary> mix;

    Slider outGain;
    Label outGainLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outGainAttachment;
    bool draggingOutGain;

    CustomLookAndFeel* customLookAndFeel = nullptr;
    std::unique_ptr<About> about;
    TooltipWindow tooltipWindow;
    std::unique_ptr<juce::FileChooser> fileChooser;
    bool init = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SirialAudioProcessorEditor)
};
