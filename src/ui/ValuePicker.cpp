#include "ValuePicker.h"
#include "../PluginEditor.h"

ValuePicker::ValuePicker(SirialAudioProcessorEditor& p, String _paramId)
	: editor(p)
	, paramId(_paramId)
{
	editor.audioProcessor.params.addParameterListener(paramId, this);
}
ValuePicker::~ValuePicker()
{
	editor.audioProcessor.params.removeParameterListener(paramId, this);
}

void ValuePicker::parameterChanged(const juce::String&, float)
{
	MessageManager::callAsync([this]{ repaint(); });
}

void ValuePicker::setParam(String id)
{
    editor.audioProcessor.params.removeParameterListener(paramId, this);
    paramId = id;
    editor.audioProcessor.params.addParameterListener(paramId, this);
    repaint();
}

void ValuePicker::mouseDown(const MouseEvent& e)
{
	e.source.enableUnboundedMouseMovement(true);
    mouse_down = true;
    auto param = editor.audioProcessor.params.getParameter(paramId);
    auto cur_val = param->getValue();
    cur_normed_value = cur_val;
    last_mouse_position = e.getPosition();
    setMouseCursor(MouseCursor::NoCursor);
    start_mouse_pos = Desktop::getInstance().getMousePosition();
    repaint();
    param->beginChangeGesture();
}

void ValuePicker::mouseUp(const MouseEvent& e)
{
	if (!mouse_down) return;
    mouse_down = false;
    setMouseCursor(MouseCursor::NormalCursor);
    e.source.enableUnboundedMouseMovement(false);
    Desktop::getInstance().setMousePosition(start_mouse_pos);
    repaint();
    auto param = editor.audioProcessor.params.getParameter(paramId);
    param->endChangeGesture();
}

void ValuePicker::mouseDoubleClick(const juce::MouseEvent&)
{
    auto param = editor.audioProcessor.params.getParameter(paramId);
    param->setValueNotifyingHost(param->getDefaultValue());
}

void ValuePicker::mouseDrag(const MouseEvent& e)
{
	auto change = e.getPosition() - last_mouse_position;
    last_mouse_position = e.getPosition();
    auto speed = (e.mods.isShiftDown() ? 40.0f : 4.0f) * pixels_per_percent;
    auto slider_change = float(change.getX() - change.getY()) / speed;
    cur_normed_value += slider_change;
    auto param = editor.audioProcessor.params.getParameter(paramId);
    param->setValueNotifyingHost(cur_normed_value);
}

void ValuePicker::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
	if (event.mods.isLeftButtonDown() || event.mods.isRightButtonDown())
    {
        return; // prevent crash, param is already mutating
    }
    auto speed = (event.mods.isShiftDown() ? 0.01f : 0.05f);
    auto slider_change = wheel.deltaY > 0 ? speed : wheel.deltaY < 0 ? -speed : 0;
    auto param = editor.audioProcessor.params.getParameter(paramId);
    param->beginChangeGesture();

	if (isInteger)
	{
		auto val = param->convertFrom0to1(param->getValue());
		param->setValueNotifyingHost(param->convertTo0to1(val + (slider_change > 0.f ? 1.f : -1.f)));
	}
	else
	{
		param->setValueNotifyingHost(param->getValue() + slider_change);
	}

    param->endChangeGesture();
}


void ValuePicker::paint(Graphics& g)
{
	auto param = editor.audioProcessor.params.getParameter(paramId);
    auto normValue = param->getValue();
    auto value = param->convertFrom0to1(normValue);

    if (isPercentage)
        value = std::round(value * 100);

	g.setFont(FontOptions(fontSize));
    g.setColour(color);
	g.drawText(prefix, getLocalBounds(), Justification::centredLeft);
	g.drawText(String(value, precision) + suffix, getLocalBounds(), align);
}