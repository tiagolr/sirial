#include "DelayView.h"
#include "../PluginEditor.h"

DelayView::DelayView(SirialAudioProcessorEditor& p)
	: editor(p)
{
	editor.audioProcessor.params.addParameterListener("mode", this);
	editor.audioProcessor.params.addParameterListener("ntaps", this);
	editor.audioProcessor.params.addParameterListener("time_mode", this);
	editor.audioProcessor.params.addParameterListener("time_sync", this);
	editor.audioProcessor.params.addParameterListener("time_millis", this);

	for (int t = 0; t < MAX_TAPS; ++t)
	{
		String prefix = "tap" + String(t) + "_";
		editor.audioProcessor.params.addParameterListener(prefix + "amp_l", this);
		editor.audioProcessor.params.addParameterListener(prefix + "amp_r", this);
		editor.audioProcessor.params.addParameterListener(prefix + "time_l", this);
		editor.audioProcessor.params.addParameterListener(prefix + "time_r", this);
		editor.audioProcessor.params.addParameterListener(prefix + "feedback", this);
	}
}

DelayView::~DelayView()
{
	editor.audioProcessor.params.removeParameterListener("mode", this);
	editor.audioProcessor.params.removeParameterListener("ntaps", this);
	editor.audioProcessor.params.removeParameterListener("time_mode", this);
	editor.audioProcessor.params.removeParameterListener("time_sync", this);
	editor.audioProcessor.params.removeParameterListener("time_millis", this);

	for (int t = 0; t < MAX_TAPS; ++t)
	{
		String prefix = "tap" + String(t) + "_";
		editor.audioProcessor.params.removeParameterListener(prefix + "amp_l", this);
		editor.audioProcessor.params.removeParameterListener(prefix + "amp_r", this);
		editor.audioProcessor.params.removeParameterListener(prefix + "time_l", this);
		editor.audioProcessor.params.removeParameterListener(prefix + "time_r", this);
		editor.audioProcessor.params.removeParameterListener(prefix + "feedback", this);
	}
}

void DelayView::parameterChanged(const juce::String& parameterID, float)
{
	if (parameterID == "ntaps")
	{
		selectedTap = 0;
		mouseOverBase = -1;
	}
	juce::MessageManager::callAsync([this] { repaint(); });
}

void DelayView::mouseMove(const MouseEvent& e)
{
	mouseOverBase = -1;
	auto mousepos = e.getPosition().toFloat();
	for (int t = 0; t < ntaps; ++t)
	{
		if (mode == Delay::Mono)
		{
			if (bases_mono[t].contains(mousepos))
			{
				mouseOverBase = t;
				break;
			}
		}
		else
		{
			if (bases_left[t].contains(mousepos))
			{
				mouseOverBase = t;
				break;
			}
			else if (bases_right[t].contains(mousepos))
			{
				mouseOverBase = t + 1337;
				break;
			}
		}
	}

	repaint();
}

void DelayView::mouseDown(const MouseEvent& e)
{
	if (mouseOverBase == -1)
		return;

	selectedTap = mouseOverBase;
	last_mouse_position = e.getPosition();
	setMouseCursor(MouseCursor::NoCursor);
	e.source.enableUnboundedMouseMovement(true);
}

void DelayView::mouseUp(const MouseEvent& e)
{
	if (mouseOverBase == -1)
		return;

	setMouseCursor(MouseCursor::NormalCursor);
	e.source.enableUnboundedMouseMovement(false);
	bool isRightBase = mouseOverBase >= 1337;
	int baseidx = isRightBase ? mouseOverBase - 1337 : mouseOverBase;
	auto& base = mode == Delay::Mono ? bases_mono[baseidx]
		: isRightBase ? bases_right[baseidx] : bases_left[baseidx];
	
	Desktop::getInstance().setMousePosition(localPointToGlobal(base.getCentre().roundToInt()));
}
void DelayView::mouseDrag(const MouseEvent& e)
{
	// check for amp changes
	if (mouseOverBase == -1)
	{

		return;
	}

	// else move taps

	auto change = e.getPosition() - last_mouse_position;
	last_mouse_position = e.getPosition();
	auto speed = (e.mods.isShiftDown() ? 40.0f : 4.0f) * 50;
	auto slider_change = float(change.getX()) / speed;

	bool isRightTap = mouseOverBase >= 1337;
	int tapidx = isRightTap ? mouseOverBase - 1337 : mouseOverBase;
	int nextTapidx = tapidx + 1;
	auto& tap = taps[tapidx];
	auto& nextTap = nextTapidx < MAX_TAPS ? taps[nextTapidx] : dummyTap;
	dummyTap.timeL = dummyTap.timeR = 100.f;
	
	if (mode == Delay::Mono || link || !isRightTap)
	{
		float pos = std::clamp(tap.timeL + slider_change, 0.f, 10.f);
		float posnxt = std::clamp(nextTap.timeL - slider_change, 0.f, 10.f);

		// clamp
		if (pos == 0 && tap.timeL == 0)
		{
			posnxt = nextTap.timeL; // keep next pos at same place
		}
		if (posnxt == 0 && tap.timeL >= 1.f)
		{
			pos = tap.timeL; // keep pos at same place
		}

		tap.timeL = pos;
		nextTap.timeL = posnxt;
	}

	if (mode == Delay::Mono || link || isRightTap)
	{
		tap.timeR = std::clamp(tap.timeR + slider_change, 0.f, 10.f);
		nextTap.timeR = std::clamp(nextTap.timeR - slider_change, 0.f, 10.f);
	}

	String prefix = "tap" + String(tapidx) + "_";
	auto param = editor.audioProcessor.params.getParameter(prefix + "time_l");
	param->setValueNotifyingHost(param->convertTo0to1(tap.timeL));
	param = editor.audioProcessor.params.getParameter(prefix + "time_r");
	param->setValueNotifyingHost(param->convertTo0to1(tap.timeR));

	if (nextTapidx < MAX_TAPS)
	{
		prefix = "tap" + String(nextTapidx) + "_";
		param = editor.audioProcessor.params.getParameter(prefix + "time_l");
		param->setValueNotifyingHost(param->convertTo0to1(nextTap.timeL));
		param = editor.audioProcessor.params.getParameter(prefix + "time_r");
		param->setValueNotifyingHost(param->convertTo0to1(nextTap.timeR));
	}
}

void DelayView::paint(Graphics& g)
{
	auto b = getLocalBounds().toFloat();
	viewb = b.reduced(20.f);
	UIUtils::drawBevel(g, b.translated(0.5f, 0.5f).reduced(1.f), 6.f, Colour(COLOR_BEVEL));

	mode = (Delay::DelayMode)editor.audioProcessor.params.getRawParameterValue("mode")->load();
	link = (bool)editor.audioProcessor.params.getRawParameterValue("link")->load();
	ntaps = (int)editor.audioProcessor.params.getRawParameterValue("ntaps")->load();
	timeMode = (Delay::TimeMode)editor.audioProcessor.params.getRawParameterValue("time_mode")->load();
	timeSync = (Delay::TimeSync)editor.audioProcessor.params.getRawParameterValue("time_sync")->load();
	timeMillis = (int)editor.audioProcessor.params.getRawParameterValue("time_millis")->load();
	userGrid = (int)editor.audioProcessor.params.getRawParameterValue("grid")->load();

	for (int t = 0; t < MAX_TAPS; ++t)
	{
		String prefix = "tap" + String(t) + "_";
		auto& tap = taps[t];
		tap.ampL = editor.audioProcessor.params.getRawParameterValue(prefix + "amp_l")->load();
		tap.ampR = editor.audioProcessor.params.getRawParameterValue(prefix + "amp_r")->load();
		tap.timeL = editor.audioProcessor.params.getRawParameterValue(prefix + "time_l")->load();
		tap.timeR = editor.audioProcessor.params.getRawParameterValue(prefix + "time_r")->load();
		tap.feedback = editor.audioProcessor.params.getRawParameterValue(prefix + "feedback")->load();
	}

	float lastx = 0;
	for (int t = 0; t < ntaps; ++t)
	{
		auto& tap = taps[t];
		float gridsz = viewb.getWidth() / (ntaps + 1);
		float x = gridsz * tap.timeL + lastx;
		if (t == 0) x += viewb.getX();
		lastx = x;
		bases_mono[t] = Rectangle<float>(x - TAP_BASE_W / 2.f, viewb.getBottom() - TAP_BASE_H, TAP_BASE_W, TAP_BASE_H);
		bases_left[t] = bases_mono[t].withY(viewb.getCentreY() - TAP_BASE_H);
		x = viewb.getX() + gridsz * t + gridsz * tap.timeR;
		bases_right[t] = Rectangle<float>(x - TAP_BASE_W / 2.f, viewb.getCentreY(), TAP_BASE_W, TAP_BASE_H);
	}

	g.setColour(Colour(COLOR_NEUTRAL));
	g.drawHorizontalLine((int)viewb.getY(), viewb.getX(), viewb.getRight());
	g.drawHorizontalLine((int)viewb.getBottom(), viewb.getX(), viewb.getRight());

	drawGrid(g);
	drawTaps(g);
}

void DelayView::drawGrid(Graphics& g)
{
	bool isTriplet = timeMode == Delay::Triplet;
	int grid = userGrid > 0 ? userGrid : isTriplet ? 3 * (ntaps + 1) : 4 * (ntaps + 1);
	if (userGrid == 0)
	{
		if (isTriplet && grid > 3 * (8 + 1))
		{
			grid = 3 * (8 + 1); // TODO
		}
		else if (grid > 4 * (8 + 1))
		{
			grid = 4 * (8 + 1); // TODO
		}
	}

	float gridx = viewb.getWidth() / grid;
	float gridy = viewb.getHeight() / 8;

	g.setColour(Colours::white.withAlpha(0.1f)); // map score into min + score * (max - min)
	for (int i = 0; i < grid + 1; ++i) 
	{
		bool isBold = isTriplet ? i % 3 == 0 : i % 4 == 0;
		float x = (float)(viewb.getX() + std::round(gridx * i) + 0.5f);

		g.setColour(Colours::white.withAlpha(isBold ? 0.1f : 0.05f));
		g.drawLine(x, viewb.getY(), x, viewb.getY() + viewb.getHeight());
	}

	g.setColour(Colours::white.withAlpha(0.05f));
	for (int i = 0; i < 8 + 1; ++i) 
	{
		float y = (float)(viewb.getY() + std::round(gridy * i) + 0.5f);
		g.drawLine(viewb.getX(), y, viewb.getX() + viewb.getWidth(), y);
	}

	g.setColour(Colour(COLOR_NEUTRAL));
	g.setFont(FontOptions(12.f));
	for (int i = 1; i <= 5; ++i)
	{
		int num = i;
		int den = 0;
		if (timeSync == Delay::k1o64) den = 64;
		if (timeSync == Delay::k1o32) den = 32;
		if (timeSync == Delay::k1o16) den = 16;
		if (timeSync == Delay::k1o8) den = 8;
		if (timeSync == Delay::k1o4) den = 4;
		if (timeSync == Delay::k1o2) den = 2;
		if (timeSync == Delay::k1o2) den = 1;

		auto x = (int)(viewb.getX() + i * (viewb.getWidth() / 5));
		g.drawVerticalLine(x, viewb.getY() - 5, viewb.getY());

		String text = String(num) + "/" + String(den);
		g.drawText(text, Rectangle<int>({ x - 23, (int)viewb.getY() - 15, 20, 15 }), Justification::centredRight);
	}
}

void DelayView::drawTaps(Graphics& g)
{
	auto drawBase = [&](Rectangle<float> b)
		{
			Path p;
			p.startNewSubPath(b.getBottomLeft());
			p.lineTo(b.getCentre().translated(-TAP_LINE_W / 2.f, -b.getHeight() / 2.f));
			p.lineTo(b.getCentre().translated(TAP_LINE_W / 2.f, -b.getHeight() / 2.f));
			p.lineTo(b.getBottomRight());
			p.closeSubPath();
			g.fillPath(p);
		};

	auto drawBaseInverted = [&](Rectangle<float> b)
		{
			Path p;
			p.startNewSubPath(b.getTopLeft());
			p.lineTo(b.getCentre().translated(-TAP_LINE_W / 2.f, +b.getHeight() / 2.f));
			p.lineTo(b.getCentre().translated(TAP_LINE_W / 2.f, +b.getHeight() / 2.f));
			p.lineTo(b.getTopRight());
			p.closeSubPath();
			g.fillPath(p);
		};

	auto drawWickMono = [&](Rectangle<float> base, Tap& tap)
		{
			float h = (viewb.getHeight() - TAP_BASE_H) * tap.ampL;
			g.fillRect(base.getCentreX() - TAP_LINE_W / 2.f, base.getY() - h, TAP_LINE_W, h);
		};

	auto drawWickLeft = [&](Rectangle<float> base, Tap& tap)
		{
			float h = (viewb.getHeight() / 2.f - TAP_BASE_H) * tap.ampL;
			g.fillRect(base.getCentreX() - TAP_LINE_W / 2.f, base.getY() - h, TAP_LINE_W, h);
		};

	auto drawWickRight = [&](Rectangle<float> base, Tap& tap)
		{
			float h = (viewb.getHeight() / 2.f - TAP_BASE_H) * tap.ampR;
			g.fillRect(base.getCentreX() - TAP_LINE_W / 2.f, base.getBottom(), TAP_LINE_W, h);
		};

	// draw dry input tap
	g.setColour(Colours::white.darker(0.5f));
	g.fillRect(viewb.getX() - TAP_LINE_W, viewb.getY(), TAP_LINE_W, viewb.getHeight());

	if (mode == Delay::Mono)
	{
		for (int t = 0; t < ntaps; ++t)
		{
			g.setColour(Colour(COLOR_TAP));
			if (mouseOverBase == t) g.setColour(Colour(COLOR_TAP_HOVER));
			drawBase(bases_mono[t]);
			drawWickMono(bases_mono[t], taps[t]);
		}
	}
	else
	{
		for (int t = 0; t < ntaps; ++t)
		{
			g.setColour(Colour(mode == Delay::PingPong && t % 2 == 1 ? COLOR_TAP2 : COLOR_TAP));
			if (mouseOverBase == t || link && mouseOverBase == t + 1337) 
				g.setColour(Colour(COLOR_TAP_HOVER));
			drawBase(bases_left[t]);
			drawWickLeft(bases_left[t], taps[t]);
			g.setColour(Colour(mode == Delay::PingPong && t % 2 == 1 ? COLOR_TAP : COLOR_TAP2));
			if (mouseOverBase == t + 1337 || link && mouseOverBase == t) 
				g.setColour(Colour(COLOR_TAP_HOVER));
			drawBaseInverted(bases_right[t]);
			drawWickRight(bases_right[t], taps[t]);
		}
	}
}