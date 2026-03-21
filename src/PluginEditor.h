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
#include "ui/DuckMeter.h"

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
    void showModRateMenu();
    void showModModeMenu();
    void showModTabMenu();
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
    TextButton reverseBtn;
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
    std::unique_ptr<Rotary> dry;
    std::unique_ptr<Rotary> wet;
    std::unique_ptr<Rotary> feedback;
    TextButton panDrySumBtn;
    TextButton panWetSumBtn;
    std::unique_ptr<Rotary> panDry;
    std::unique_ptr<Rotary> panWet;
    std::unique_ptr<Rotary> stereo;

    std::unique_ptr<Rotary> lowcut;
    std::unique_ptr<Rotary> highcut;
    std::unique_ptr<Rotary> distDrive;
    std::unique_ptr<Rotary> distColor;
    std::unique_ptr<Rotary> modRate;
    std::unique_ptr<Rotary> modRateSync;
    std::unique_ptr<Rotary> modDepth;
    std::unique_ptr<Rotary> diffAmt;
    std::unique_ptr<Rotary> diffSize;
    TextButton modTabBtn;
    TextButton modModeBtn;
    TextButton modRateModeBtn;

    std::unique_ptr<Rotary> duckThres;
    std::unique_ptr<Rotary> duckAmt;
    std::unique_ptr<Rotary> duckAtk;
    std::unique_ptr<Rotary> duckRel;
    std::unique_ptr<DuckMeter> duckMeter;

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
