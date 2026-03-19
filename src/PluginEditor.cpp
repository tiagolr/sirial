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
    audioProcessor.params.addParameterListener("link", this);
    audioProcessor.params.addParameterListener("reverse", this);
    audioProcessor.params.addParameterListener("time_mode", this);
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
    settingsBtn.setBounds(col + 100 - 12, NAV_HEIGHT / 2 - 25 / 2, 25, 25);
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


    // DELAY VIEW TOOLBAR
    row += NAV_HEIGHT + 10;

    addAndMakeVisible(modeBtn);
    modeBtn.setAlpha(0.f);
    modeBtn.setBounds(col, row, 70, 25);
    modeBtn.onClick = [this] { showModeMenu(); };

    addAndMakeVisible(tapsLabel);
    tapsLabel.setText("Taps", dontSendNotification);
    tapsLabel.setColour(Label::ColourIds::textColourId, Colour(COLOR_NEUTRAL));
    tapsLabel.setFont(FontOptions(18.f));
    tapsLabel.setJustificationType(Justification::centredLeft);
    tapsLabel.setBounds(modeBtn.getBounds().withX(modeBtn.getRight() + 10).withWidth(40));

    tapsPicker = std::make_unique<ValuePicker>(*this, "ntaps");
    tapsPicker->setBounds(tapsLabel.getBounds().withX(tapsLabel.getRight() + 5).withWidth(35));
    tapsPicker->precision = 0;
    addAndMakeVisible(tapsPicker.get());

    addAndMakeVisible(timeLabel);
    timeLabel.setText("Step", dontSendNotification);
    timeLabel.setColour(Label::ColourIds::textColourId, Colour(COLOR_NEUTRAL));
    timeLabel.setFont(FontOptions(18.f));
    timeLabel.setJustificationType(Justification::centredLeft);
    timeLabel.setBounds(tapsPicker->getBounds().withX(tapsPicker->getRight() + 10).withWidth(45));
    
    tapsTimeMillis = std::make_unique<TimePicker>(*this, "time_millis");
    addAndMakeVisible(tapsTimeMillis.get());
    tapsTimeMillis->setBounds(timeLabel.getBounds().withX(timeLabel.getRight()).withWidth(70));

    tapsTimeSync = std::make_unique<TimePicker>(*this, "time_sync");
    tapsTimeSync->mode = 1;
    addAndMakeVisible(tapsTimeSync.get());
    tapsTimeSync->setBounds(timeLabel.getBounds().withX(timeLabel.getRight()).withWidth(70));

    addAndMakeVisible(millisBtn);
    millisBtn.setAlpha(0.f);
    millisBtn.setBounds(tapsTimeSync->getBounds().withX(tapsTimeSync->getRight() + 12).withWidth(30));
    millisBtn.onClick = [this]
        {
            auto param = audioProcessor.params.getParameter("time_mode");
            param->setValueNotifyingHost(param->convertTo0to1(0.f));
        };

    addAndMakeVisible(straightBtn);
    straightBtn.setAlpha(0.f);
    straightBtn.setBounds(millisBtn.getBounds().withX(millisBtn.getRight()));
    straightBtn.onClick = [this]
        {
            auto param = audioProcessor.params.getParameter("time_mode");
            param->setValueNotifyingHost(param->convertTo0to1(1.f));
        };

    addAndMakeVisible(tripletBtn);
    tripletBtn.setAlpha(0.f);
    tripletBtn.setBounds(straightBtn.getBounds().withX(straightBtn.getRight()));
    tripletBtn.onClick = [this]
        {
            auto param = audioProcessor.params.getParameter("time_mode");
            param->setValueNotifyingHost(param->convertTo0to1(2.f));
        };

    addAndMakeVisible(dottedBtn);
    dottedBtn.setAlpha(0.f);
    dottedBtn.setBounds(tripletBtn.getBounds().withX(tripletBtn.getRight()));
    dottedBtn.onClick = [this]
        {
            auto param = audioProcessor.params.getParameter("time_mode");
            param->setValueNotifyingHost(param->convertTo0to1(3.f));
        };


    // DELAY VIEW

    delayView = std::make_unique<DelayView>(*this);
    addAndMakeVisible(delayView.get());
    delayView->setBounds(col, row + 35, KNOB_WIDTH * 7, 185);

    addAndMakeVisible(linkBtn);
    linkBtn.setBounds(dottedBtn.getBounds().withWidth(25).withRightX(delayView->getRight()));
    linkBtn.setAlpha(0.f);
    linkBtn.onClick = [this]
        {
            auto param = audioProcessor.params.getParameter("link");
            param->setValueNotifyingHost(param->getValue() > 0.f ? 0.f : 1.f);
        };

    // BELOW DELAY VIEW
    col = PLUG_PADDING;

    mix = std::make_unique<Rotary>(audioProcessor, "mix", "Mix", Rotary::percx100, true);
    addAndMakeVisible(mix.get());
    mix->setBounds(col, getHeight() - PLUG_PADDING - KNOB_HEIGHT, KNOB_WIDTH, KNOB_HEIGHT);

    feedback = std::make_unique<Rotary>(audioProcessor, "feedback", "Feedback", Rotary::percx100);
    addAndMakeVisible(feedback.get());
    feedback->setBounds(mix->getBounds().translated(KNOB_WIDTH, 0));

    // RIGHT OF DELAY VIEW
    col = delayView->getRight() + 10;

    lowcut = std::make_unique<Rotary>(audioProcessor, "lowcut", "Lowcut", Rotary::filterHP);
    addAndMakeVisible(lowcut.get());
    lowcut->setBounds(col, row + 25, KNOB_WIDTH, KNOB_HEIGHT);

    highcut = std::make_unique<Rotary>(audioProcessor, "highcut", "Highcut", Rotary::filterLP);
    addAndMakeVisible(highcut.get());
    highcut->setBounds(lowcut->getBounds().translated(0, KNOB_HEIGHT + 5));

    //

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
    outGainLabel.setFont(FontOptions(18.f));
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
    audioProcessor.params.removeParameterListener("link", this);
    audioProcessor.params.removeParameterListener("reverse", this);
    audioProcessor.params.removeParameterListener("time_mode", this);
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
    auto mode = (int)audioProcessor.params.getRawParameterValue("mode")->load();
    auto link = (bool)audioProcessor.params.getRawParameterValue("link")->load();
    linkBtn.setToggleState(link, dontSendNotification);
    linkBtn.setVisible(mode != 0);

    auto timeMode = (int)audioProcessor.params.getRawParameterValue("time_mode")->load();
    tapsTimeMillis->setVisible(timeMode == 0);
    tapsTimeSync->setVisible(timeMode > 0);
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
    g.setFont(FontOptions(18.f));
    g.drawText(audioProcessor.presetName, presetBtn.getBounds().toFloat(), Justification::centred);
    UIUtils::drawTriangle(g, nextPresetBtn.getBounds().toFloat().reduced(8.f), 1, Colours::white);
    UIUtils::drawTriangle(g, prevPresetBtn.getBounds().toFloat().reduced(8.f), 3, Colours::white);
    UIUtils::drawSave(g, saveBtn.getBounds().toFloat().translated(3.5, 3.5), Colours::white);

    juce::Font f(FontOptions(28.f));
    g.setFont(FontOptions(28.f));
    auto color = Colours::white;
    g.setColour(color);

    String text = "SIRIAL";
    float x = (float)logo.getX();
    float y = (float)logo.getY();
    for (auto c : text)
    {
        juce::String s = juce::String::charToString(c);
        g.drawText(s, Rectangle<float>(x, y, (float)100, (float)NAV_HEIGHT), juce::Justification::left);

        x += f.getStringWidthFloat(s) + 4.f;
    }
    //g.drawText("SIRIAL", logo.getBounds().expanded(0, 10), Justification::centredLeft);
    UIUtils::drawGear(g, settingsBtn.getBounds(), 9, 6, color, Colour(0xff242D3A));
    g.setFont(FontOptions(18.f));

    // 
    auto mode = (int)audioProcessor.params.getRawParameterValue("mode")->load();
    UIUtils::drawBevel(g, modeBtn.getBounds().toFloat().translated(0.5f, 0.5f), BEVEL_CORNER, Colour(COLOR_BEVEL));
    String modeText = mode == 0 ? "Mono" : mode == 1 ? "Stereo" : "Pipo";
    g.setColour(Colours::white);
    g.drawText(modeText, modeBtn.getBounds(), Justification::centred);

    // 
    UIUtils::drawBevel(g, tapsPicker->getBounds().toFloat().translated(0.5f, 0.5f), 3.f, Colour(COLOR_BEVEL));

    // draw buttons
    auto link = audioProcessor.params.getRawParameterValue("link")->load();
    if (linkBtn.isVisible())
    {
        UIUtils::drawBevelLight(g, linkBtn.getBounds().toFloat().expanded(1.f), 3.f);
        if (link)
        {
            g.setColour(Colour(COLOR_ACTIVE).darker(0.5f).withAlpha(0.5f));
            g.fillRoundedRectangle(linkBtn.getBounds().toFloat().reduced(0.5f), 3.f);
            g.setColour(Colour(COLOR_ACTIVE));
            g.drawRoundedRectangle(linkBtn.getBounds().toFloat().reduced(0.5f), 3.f, 1.f);
        }

        UIUtils::drawChain(g, linkBtn.getBounds().toFloat(), link ? Colour(0xffffffff) : Colour(COLOR_NEUTRAL));

        g.setColour(Colour(COLOR_NEUTRAL));
        g.drawText("Link", linkBtn.getBounds().translated(-40,0).withWidth(35), Justification::centred);
    }

    auto timeMode = (int)audioProcessor.params.getRawParameterValue("time_mode")->load();
    UIUtils::drawBevelLight(g, millisBtn.getBounds().toFloat().withRight((float)dottedBtn.getRight()).expanded(1.f,1.f), 5.f);

    g.setColour(Colour(COLOR_BEVEL).withAlpha(.5f));
    g.drawVerticalLine((int)millisBtn.getRight(), (float)millisBtn.getY(), (float)millisBtn.getBottom());
    g.drawVerticalLine((int)straightBtn.getRight(), (float)millisBtn.getY(), (float)millisBtn.getBottom());
    g.drawVerticalLine((int)tripletBtn.getRight(), (float)millisBtn.getY(), (float)millisBtn.getBottom());
    if (timeMode == 0)
    {
        Path p;
        auto r = millisBtn.getBounds().toFloat().translated(0.5f, 0.5f);
        p.addRoundedRectangle(r.getX(), r.getY(), r.getWidth(), r.getHeight(), 5.f, 5.f, true, false, true, false);
        g.setColour(Colour(COLOR_ACTIVE).darker(0.5f).withAlpha(0.5f));
        g.fillPath(p);
        g.setColour(Colour(COLOR_ACTIVE));
        g.strokePath(p, PathStrokeType(1.f));
    }
    UIUtils::drawClock(g, millisBtn.getBounds().toFloat().reduced(7.f, 5.f), timeMode == 0 ? Colour(0xffffffff) : Colour(COLOR_NEUTRAL));
    if (timeMode == 1)
    {
        g.setColour(Colour(COLOR_ACTIVE).darker(0.5f).withAlpha(0.5f));
        g.fillRoundedRectangle(straightBtn.getBounds().toFloat().translated(0.5f, 0.5f), 0.f);
        g.setColour(Colour(COLOR_ACTIVE));
        g.drawRoundedRectangle(straightBtn.getBounds().toFloat().translated(0.5f, 0.5f), 0.f, 1.f);

    }
    UIUtils::drawNote(g, straightBtn.getBounds().toFloat(), 0, timeMode == 1 ? Colour(0xffffffff) : Colour(COLOR_NEUTRAL));
    if (timeMode == 2)
    {
        g.setColour(Colour(COLOR_ACTIVE).darker(0.5f).withAlpha(0.5f));
        g.fillRoundedRectangle(tripletBtn.getBounds().toFloat().translated(0.5f, 0.5f), 0.f);
        g.setColour(Colour(COLOR_ACTIVE));
        g.drawRoundedRectangle(tripletBtn.getBounds().toFloat().translated(0.5f, 0.5f), 0.f, 1.f);
    }
    UIUtils::drawNote(g, tripletBtn.getBounds().toFloat().translated(-2.f, 0), 1, timeMode == 2 ? Colour(0xffffffff) : Colour(COLOR_NEUTRAL));
    if (timeMode == 3)
    {
        Path p;
        auto r = dottedBtn.getBounds().toFloat().translated(0.5f, 0.5f);
        p.addRoundedRectangle(r.getX(), r.getY(), r.getWidth(), r.getHeight(), 5.f, 5.f, false, true, false, true);
        g.setColour(Colour(COLOR_ACTIVE).darker(0.5f).withAlpha(0.5f));
        g.fillPath(p);
        g.setColour(Colour(COLOR_ACTIVE));
        g.strokePath(p, PathStrokeType(1.f));
    }
    UIUtils::drawNote(g, dottedBtn.getBounds().toFloat(), 2, timeMode == 3 ? Colour(0xffffffff) : Colour(COLOR_NEUTRAL));

    g.setColour(Colour(COLOR_NEUTRAL));
    g.drawText("DAMP", Rectangle<int>(KNOB_WIDTH, 25).withX(lowcut->getX()).withY(lowcut->getY() - 25), Justification::centred);
    g.drawText("SAT", Rectangle<int>(KNOB_WIDTH, 25).withX(lowcut->getX() + KNOB_WIDTH).withY(lowcut->getY() - 25), Justification::centred);
    g.drawText("DIFF", Rectangle<int>(KNOB_WIDTH, 25).withX(lowcut->getX() + KNOB_WIDTH * 2).withY(lowcut->getY() - 25), Justification::centred);
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

void SirialAudioProcessorEditor::showModeMenu()
{
    auto mode = (int)audioProcessor.params.getRawParameterValue("mode")->load();
    PopupMenu menu;
    menu.addItem(1, "Mono", true, mode == 0);
    menu.addItem(2, "Stereo", true, mode == 1);
    menu.addItem(3, "Ping-Pong", true, mode == 2);

    auto menuPos = localPointToGlobal(modeBtn.getBounds().getBottomLeft());
    menu.showMenuAsync(PopupMenu::Options()
        .withTargetComponent(*this)
        .withTargetScreenArea({ menuPos.getX(), menuPos.getY(), 1, 1 }),
        [this](int result)
        {
            if (result == 0) return;
            auto param = audioProcessor.params.getParameter("mode");
            param->setValueNotifyingHost(param->convertTo0to1(result - 1.f));
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