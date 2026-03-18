// Copyright 2025 tilr

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Globals.h"

SirialAudioProcessorEditor::SirialAudioProcessorEditor (SirialAudioProcessor& p)
    : AudioProcessorEditor (&p)
    , audioProcessor (p)
{
    setResizable(false, false);
    setSize (PLUG_WIDTH, PLUG_HEIGHT);
    Desktop::getInstance().setGlobalScaleFactor(audioProcessor.scale);

    audioProcessor.addChangeListener(this);
    audioProcessor.params.addParameterListener("mode", this);
    audioProcessor.params.addParameterListener("reverse", this);
    audioProcessor.params.addParameterListener("pan_dry_sum", this);
    audioProcessor.params.addParameterListener("pan_wet_sum", this);

    // NAVBAR
    int col = PLUG_PADDING;
    int row = 0;

    addAndMakeVisible(logo);
    logo.setAlpha(0.f);
    logo.setBounds(col, row, 100, NAV_HEIGHT);
    logo.onClick = [this]
        {
            about->setVisible(true);
        };

    addAndMakeVisible(settingsBtn);
    settingsBtn.setAlpha(0.f);
    settingsBtn.setBounds(col + 100 - 5, NAV_HEIGHT / 2 - 25 / 2, 25, 25);
    settingsBtn.onClick = [this]
        {
            showSettings();
        };

    addAndMakeVisible(presetBtn);
    presetBtn.setAlpha(0.f);
    presetBtn.setBounds(getWidth() / 2 - KNOB_WIDTH * 3 / 2, NAV_HEIGHT / 2 - 25 / 2 + 1, KNOB_WIDTH * 3, 25);
    presetBtn.onClick = [this]
        {
            showPresetsMenu();
        };

    addAndMakeVisible(nextPresetBtn);
    nextPresetBtn.setAlpha(0.f);
    nextPresetBtn.setBounds(Rectangle<int>(25, 25).withY(presetBtn.getY()).withX(presetBtn.getRight()));
    nextPresetBtn.onClick = [this]
        {
            //audioProcessor.presetmgr->loadNext();
        };

    addAndMakeVisible(prevPresetBtn);
    prevPresetBtn.setAlpha(0.f);
    prevPresetBtn.setBounds(Rectangle<int>(25, 25).withY(presetBtn.getY()).withRightX(presetBtn.getX()));
    prevPresetBtn.onClick = [this]
        {
            //audioProcessor.presetmgr->loadPrev();
        };

    addAndMakeVisible(saveBtn);
    saveBtn.setAlpha(0.f);
    saveBtn.setBounds(Rectangle<int>(25, 25).withY(presetBtn.getY()).withX(nextPresetBtn.getRight()));
    saveBtn.onClick = [this]
        {
            savePreset();
        };

    row += NAV_HEIGHT + 10;

    delayView = std::make_unique<DelayView>(*this);
    addAndMakeVisible(delayView.get());
    delayView->setBounds(col, row + 25, KNOB_WIDTH * 7, 175);

    addAndMakeVisible(outGain);
    outGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.params, "out_gain", outGain);
    outGain.setComponentID("out_gain");
    outGain.setSliderStyle(Slider::LinearBar);
    outGain.setBounds(Rectangle<int>(75, 25).withY(NAV_HEIGHT / 2 - 25/2 + 1).withRightX(getWidth() - PLUG_PADDING));
    outGain.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    outGain.setTextBoxStyle(Slider::NoTextBox, true, 10, 10);
    outGain.setVelocityBasedMode(true);
    outGain.onDragStart = [this] { draggingOutGain = true; refreshOutGainLabel(); };
    outGain.onDragEnd = [this] { draggingOutGain = false; refreshOutGainLabel(); };
    outGain.onValueChange = [this] { refreshOutGainLabel(); };

    addAndMakeVisible(outGainLabel);
    outGainLabel.setFont(FontOptions(16.f));
    outGainLabel.setText("Out", dontSendNotification);
    outGainLabel.setJustificationType(Justification::centredRight);
    outGainLabel.setColour(Label::ColourIds::textColourId, Colours::white);
    outGainLabel.setBounds(Rectangle<int>(60, 20).withY(NAV_HEIGHT / 2 - 20 / 2 + 1).withRightX(outGain.getX() - 5));

    meter = std::make_unique<Meter>(audioProcessor);
    addAndMakeVisible(meter.get());
    meter->setBounds(Rectangle<int>(0, row, METER_WIDTH, KNOB_HEIGHT * 3 + VSEPARATOR + 30 + HEADER_HEIGHT + 5)
        .withRightX(getWidth() - PLUG_PADDING));

    // ABOUT
    about = std::make_unique<About>();
    addChildComponent(*about);
    about->setBounds(getBounds());

    customLookAndFeel = new CustomLookAndFeel();
    setLookAndFeel(customLookAndFeel);

    init = true;
    resized();
    toggleUIComponents();
}

SirialAudioProcessorEditor::~SirialAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    delete customLookAndFeel;
    audioProcessor.removeChangeListener(this);
    audioProcessor.params.removeParameterListener("mode", this);
    audioProcessor.params.removeParameterListener("reverse", this);
    audioProcessor.params.removeParameterListener("pan_dry_sum", this);
    audioProcessor.params.removeParameterListener("pan_wet_sum", this);
}

void SirialAudioProcessorEditor::changeListenerCallback(ChangeBroadcaster* source)
{
    (void)source;
    MessageManager::callAsync([this] { toggleUIComponents(); });
}

void SirialAudioProcessorEditor::parameterChanged (const juce::String& parameterID, float newValue)
{
    (void)parameterID;
    (void)newValue;
    MessageManager::callAsync([this]() { toggleUIComponents(); });
};

void SirialAudioProcessorEditor::toggleUIComponents()
{
    MessageManager::callAsync([this] { repaint(); });
}

//==============================================================================

void SirialAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(Colour(COLOR_BACKGROUND));

    g.setColour(Colour(COLOR_ACTIVE).withAlpha(0.15f));
    g.fillRect(0, 0, getWidth(), NAV_HEIGHT);
    auto navsep = getLocalBounds().withY(NAV_HEIGHT).withHeight(3).toFloat();
    auto grad = ColourGradient(
        Colours::black.withAlpha(0.25f),
        navsep.getTopLeft(),
        Colours::transparentBlack,
        navsep.getBottomLeft(),
        false
    );
    if (getHeight() >= PLUG_HEIGHT) {
        g.setGradientFill(grad);
        g.fillRect(navsep);
    }

    //g.setColour(Colour(COLOR_BEVEL));
    //g.fillRoundedRectangle(presetBtn.getBounds().toFloat().reduced(0.5f), BEVEL_CORNER);
    UIUtils::drawBevel(g, presetBtn.getBounds().toFloat().reduced(0.5f), BEVEL_CORNER, Colour(COLOR_BEVEL));
    g.setColour(Colours::white);
    g.setFont(FontOptions(16.f));
    g.drawText(audioProcessor.presetName, presetBtn.getBounds().toFloat(), Justification::centred);
    UIUtils::drawTriangle(g, nextPresetBtn.getBounds().toFloat().reduced(8.f), 1, Colours::white);
    UIUtils::drawTriangle(g, prevPresetBtn.getBounds().toFloat().reduced(8.f), 3, Colours::white);
    UIUtils::drawSave(g, saveBtn.getBounds().toFloat().translated(3.5, 3.5), Colours::white);

    g.setFont(FontOptions(26.f));
    g.setColour(Colours::white);
    g.drawText("Sirial", logo.getBounds().expanded(0, 10), Justification::centredLeft);
    UIUtils::drawGear(g, settingsBtn.getBounds(), 9, 6, Colours::white, Colour(0xff3A2727));
    g.setFont(FontOptions(16.f));
}

void SirialAudioProcessorEditor::resized()
{
    if (!init) return; // defer resized() call during constructor
}

void SirialAudioProcessorEditor::showSettings()
{
    PopupMenu menu;

    auto menuPos = localPointToGlobal(settingsBtn.getBounds().getBottomLeft());
    menu.showMenuAsync(PopupMenu::Options()
        .withTargetComponent(*this)
        .withTargetScreenArea({ menuPos.getX(), menuPos.getY(), 1, 1 }),
        [this](int result)
        {
            if (result == 0) return;
        }
    );
}

void SirialAudioProcessorEditor::showPresetsMenu()
{
    /*
    PopupMenu menu;
    PopupMenu basicPresets;
    PopupMenu drumsPresets;
    PopupMenu fxPresets;
    for (int i = 0; i < PresetMgr::factoryPresets.size(); ++i)
    {
        auto& preset = PresetMgr::factoryPresets[i];
        if (preset.category == "")
            menu.addItem(i + 1, preset.name, true, audioProcessor.presetName == preset.name);
        else if (preset.category == "Basic")
            basicPresets.addItem(i + 1, preset.name, true, audioProcessor.presetName == preset.name);
        else if (preset.category == "Drums")
            drumsPresets.addItem(i + 1, preset.name, true, audioProcessor.presetName == preset.name);
        else if (preset.category == "FX")
            fxPresets.addItem(i + 1, preset.name, true, audioProcessor.presetName == preset.name);
    }

    menu.addSubMenu("Basic", basicPresets);
    menu.addSubMenu("Drums", drumsPresets);
    menu.addSubMenu("FX", fxPresets);

    PopupMenu userMenu;
    File parent = File(audioProcessor.presetmgr->dir);
	juce::Array<juce::File> userPresets;
	parent.findChildFiles(userPresets, File::findFiles, false, "*.xml");
    for (int i = 0; i < userPresets.size(); ++i)
    {
        String name = userPresets[i].getFileNameWithoutExtension();
        userMenu.addItem(1000 + i, name, true, audioProcessor.presetName == name);
    }

    menu.addSubMenu("User", userMenu);

    auto menuPos = localPointToGlobal(presetBtn.getBounds().getBottomLeft());
    menu.showMenuAsync(PopupMenu::Options()
        .withTargetComponent(*this)
        .withTargetScreenArea({ menuPos.getX() + 50, menuPos.getY(), 1, 1 }),
        [this, userPresets](int result)
        {
            if (result == 0) return;
            if (result < 1000)
            {
                audioProcessor.presetmgr->loadFactory(result - 1);
            }
            if (result >= 1000)
            {
                audioProcessor.presetmgr->load(userPresets[result - 1000].getFileNameWithoutExtension(), 0);
            }
        }
    );
    */
}

void SirialAudioProcessorEditor::savePreset()
{
    /*
    auto dir = File(audioProcessor.presetmgr->dir);
    fileChooser.reset(new juce::FileChooser("Save Preset", dir, "*.xml"));
    fileChooser->launchAsync(juce::FileBrowserComponent::saveMode | FileBrowserComponent::warnAboutOverwriting,
        [this](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file.isDirectory()) return;
            if (file == juce::File()) return;

            audioProcessor.presetName = file.getFileNameWithoutExtension();
            String filestr = audioProcessor.presetmgr->exportPreset();
            if (filestr == "") return;
            file.replaceWithText(filestr);
            MessageManager::callAsync([this] { repaint(); });
        });

    */
}

void SirialAudioProcessorEditor::refreshOutGainLabel()
{
    float gain = audioProcessor.params.getRawParameterValue("out_gain")->load();
    String text = draggingOutGain
        ? String(gain, 1) + " dB"
        : "Out";
    outGainLabel.setText(text, dontSendNotification);
}