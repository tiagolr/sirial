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

    Rectangle<int> getTooltipBounds(const String& tipText,
        Point<int> screenPos,
        Rectangle<int> parentArea) override
    {
        Font font(FontOptions(18.0f));
        AttributedString attributedText;
        attributedText.append(tipText, font);

        TextLayout layout;
        layout.createLayout(attributedText, 1000.0f);

        int width = (int)layout.getWidth() + 20;
        int height = (int)layout.getHeight() + 10;

        int x = screenPos.x;
        int y = screenPos.y - height / 2;

        Rectangle<int> bounds(x, y, width, height);
        bounds = bounds.constrainedWithin(parentArea);

        return bounds;
    }

    void drawTooltip(Graphics& g,
        const String& text,
        int width, int height) override
    {
        g.setColour(Colours::black);
        g.fillRoundedRectangle(0, 0, (float)width, (float)height, 6.0f);
        g.setColour(Colours::white);
        g.setFont(Font(FontOptions(18.0f)));
        g.drawFittedText(text, 0, 0, width, height, Justification::centred, 1);
    }

    void drawTreeviewPlusMinusBox(Graphics& g, const Rectangle<float>& area,
        juce::Colour backgroundColour, bool isOpen, bool isMouseOver) override;

private:
    juce::Typeface::Ptr typeface;
};