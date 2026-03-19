#pragma once

#include <JuceHeader.h>
#include "../Globals.h"
#include "UIUtils.h"

using namespace globals;

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel();
    juce::Typeface::Ptr getTypefaceForFont(const juce::Font&) override;
    int getPopupMenuBorderSize() override;
    void drawButtonBackground (Graphics& g, Button& button, const Colour& backgroundColour, bool isMouseOverButton, bool isButtonDown) override;
    void drawComboBox (Graphics&, int width, int height, bool isButtonDown,int buttonX, int buttonY, int buttonW, int buttonH, ComboBox&) override;
    void positionComboBoxText (ComboBox& box, Label& label) override;
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const juce::Slider::SliderStyle style, juce::Slider& slider) override;

    juce::Font getTextButtonFont(juce::TextButton& button, int buttonHeight) override {
        (void)buttonHeight;
        return button.getComponentID() == "small" ? FontOptions(12.0f) : FontOptions(16.0f);  
    }

    juce::Font getComboBoxFont (juce::ComboBox& box) override
    {
        return box.getComponentID() == "small" ? FontOptions(12.0f) : FontOptions(16.0f);
    }

    juce::Font getPopupMenuFont() override
    {
        return juce::Font(FontOptions(19.0f));
    }

    void drawTooltip(juce::Graphics& g, const juce::String& text, int width, int height) override
    {
        const int padding = 10;
        juce::Rectangle<float> area(0, 0, (float)width, (float)height);

        g.setColour(findColour(juce::TooltipWindow::backgroundColourId));
        g.fillRoundedRectangle(area, 4.0f);

        juce::AttributedString s;
        s.setJustification(juce::Justification::centredLeft);
        s.append(text, juce::Font(FontOptions(16.f)),
            findColour(juce::TooltipWindow::textColourId));

        juce::TextLayout tl;
        tl.createLayout(s, (float)width - 2 * padding);
        tl.draw(g, area.reduced((float)padding));
    }

    void drawTreeviewPlusMinusBox(Graphics& g, const Rectangle<float>& area,
        juce::Colour backgroundColour, bool isOpen, bool isMouseOver) override;

private:
    juce::Typeface::Ptr typeface;
};