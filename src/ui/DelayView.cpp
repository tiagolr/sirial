#include "DelayView.h"
#include "../PluginEditor.h"

DelayView::DelayView(SirialAudioProcessorEditor& p)
	: editor(p)
{
	editor.audioProcessor.params.addParameterListener("mode", this);
	editor.audioProcessor.params.addParameterListener("mix", this);
	editor.audioProcessor.params.addParameterListener("pan_wet", this);
	editor.audioProcessor.params.addParameterListener("pan_dry", this);
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
		editor.audioProcessor.params.addParameterListener(prefix + "feedback_global", this);
	}

	ampPicker = std::make_unique<ValuePicker>(p, "tap0_amp_l");
	ampPicker->prefix = "Amp:";
	ampPicker->suffix = "%";
	ampPicker->isPercentage = true;
	ampPicker->precision = 0;
	ampPicker->fontSize = 16.f;
	ampPicker->color = Colour(COLOR_NEUTRAL);
	addAndMakeVisible(ampPicker.get());

	feedbkPicker = std::make_unique<ValuePicker>(p, "tap0_feedback");
	feedbkPicker->prefix = "Feed:";
	feedbkPicker->suffix = "%";
	feedbkPicker->isPercentage = true;
	feedbkPicker->precision = 0;
	feedbkPicker->fontSize = 16.f;
	feedbkPicker->color = Colour(COLOR_NEUTRAL);
	addAndMakeVisible(feedbkPicker.get());

	addAndMakeVisible(globalFeedbk);
	globalFeedbk.setTooltip("Use global feedback or an amount for this tap");
	globalFeedbk.setAlpha(0.f);
	globalFeedbk.onClick = [this]
		{
			int idx = selectedTap >= 1337 ? selectedTap - 1337 : selectedTap;
			auto param = editor.audioProcessor.params.getParameter("tap" + String(idx) + "_feedback_global");
			param->setValueNotifyingHost(param->getValue() > 0.f ? 0.f : 1.f);
		};

	addAndMakeVisible(menuBtn);
	menuBtn.setAlpha(0.f);
	menuBtn.onClick = [this] { showMenu(); };

	updateInfo();
}

DelayView::~DelayView()
{
	editor.audioProcessor.params.removeParameterListener("mode", this);
	editor.audioProcessor.params.removeParameterListener("mix", this);
	editor.audioProcessor.params.removeParameterListener("pan_wet", this);
	editor.audioProcessor.params.removeParameterListener("pan_dry", this);
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
		editor.audioProcessor.params.removeParameterListener(prefix + "feedback_global", this);
	}
}

void DelayView::parameterChanged(const juce::String& parameterID, float)
{
	if (parameterID == "ntaps")
	{
		selectedTap = 0;
		mouseOverBase = -1;
	}

	juce::MessageManager::callAsync([this] 
		{ 
			updateInfo();
			repaint(); 
		});
}

void DelayView::mouseExit(const MouseEvent&)
{
	mouseOverBase = -1;
	repaint();
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
	updateInfo();
	int idx = selectedTap >= 1337 ? selectedTap - 1337 : selectedTap;
	taps[idx].snapacc = 0.f;

	last_mouse_position = e.getPosition();
	setMouseCursor(MouseCursor::NoCursor);
	e.source.enableUnboundedMouseMovement(true);
	repaint();
}

void DelayView::updateInfo()
{
	auto idx = selectedTap >= 1337 ? selectedTap - 1337 : selectedTap;
	auto isLeft = selectedTap < 1337;

	bool useGlobalFeedbk = (bool)editor.audioProcessor.params.getRawParameterValue("tap" + String(idx) + "_feedback_global")->load();

	ampPicker->setParam("tap" + String(idx) + "_amp_" + String(isLeft ? "l" : "r"));
	feedbkPicker->setParam(useGlobalFeedbk ? "feedback" : "tap" + String(idx) + "_feedback");
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
		updateAmplitudes(e);
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
	auto& nextTap = nextTapidx < ntaps ? taps[nextTapidx] : dummyTap;
	dummyTap.timeL = dummyTap.timeR = 100.f;

	float cumsumL = 0.f;
	float cumsumR = 0.f;
	for (int t = 0; t < tapidx; ++t)
	{
		cumsumL += taps[t].timeL;
		cumsumR += taps[t].timeR;
	}

	
	if (mode == Delay::Mono || link || !isRightTap)
	{
		float pos = std::clamp(tap.timeL + slider_change, 0.f, 10.f);
		float posnxt = std::clamp(nextTap.timeL - slider_change, 0.f, 10.f);

		// snap left channel to right channel, removes haas
		if (mode != Delay::Mono && !link && std::fabs(pos - tap.timeR) < 1e-2)
		{
			if (std::fabs(tap.snapacc) < 0.05)
			{
				tap.snapacc += slider_change;
				posnxt = nextTap.timeL - (tap.timeR - tap.timeL);
				pos = tap.timeR;
			}
		}
		// reset snapping when direction changes
		if (slider_change > 0.f && tap.snapacc < 0.f) tap.snapacc = 0.f;
		if (slider_change < 0.f && tap.snapacc > 0.f) tap.snapacc = 0.f;

		// clamp
		if (pos == 0 && tap.timeL == 0)
			posnxt = nextTap.timeL; // keep next pos at same place
		if (posnxt == 0 && tap.timeL >= 1.f)
			pos = tap.timeL; // keep pos at same place
		if (cumsumL + pos >= ntaps + 1.f)
			pos = tap.timeL; // prevent last tap from going off the viewport

		tap.timeL = pos;
		nextTap.timeL = posnxt;
	}

	if (mode == Delay::Mono || link || isRightTap)
	{
		float pos = std::clamp(tap.timeR + slider_change, 0.f, 10.f);
		float posnxt = std::clamp(nextTap.timeR - slider_change, 0.f, 10.f);

		// snap right channel to left channel, removes haas
		if (mode != Delay::Mono && !link && std::fabs(pos - tap.timeL) < 1e-2)
		{
			if (std::fabs(tap.snapacc) < 0.05)
			{
				tap.snapacc += slider_change;
				posnxt = nextTap.timeR - (tap.timeL - tap.timeR);
				pos = tap.timeL;
			}
		}
		// reset snapping when direction changes
		if (slider_change > 0.f && tap.snapacc < 0.f) tap.snapacc = 0.f;
		if (slider_change < 0.f && tap.snapacc > 0.f) tap.snapacc = 0.f;

		// clamp
		if (pos == 0 && tap.timeR == 0)
			posnxt = nextTap.timeR; // keep next pos at same place
		if (posnxt == 0 && tap.timeR >= 1.f)
			pos = tap.timeR; // keep pos at same place
		if (cumsumL + pos >= ntaps + 1.f)
			pos = tap.timeR; // prevent last tap from going off the viewport

		tap.timeR = pos;
		nextTap.timeR = posnxt;
	}

	String prefix = "tap" + String(tapidx) + "_";
	auto param = editor.audioProcessor.params.getParameter(prefix + "time_l");
	param->setValueNotifyingHost(param->convertTo0to1(tap.timeL));
	param = editor.audioProcessor.params.getParameter(prefix + "time_r");
	param->setValueNotifyingHost(param->convertTo0to1(tap.timeR));

	if (nextTapidx < ntaps)
	{
		prefix = "tap" + String(nextTapidx) + "_";
		param = editor.audioProcessor.params.getParameter(prefix + "time_l");
		param->setValueNotifyingHost(param->convertTo0to1(nextTap.timeL));
		param = editor.audioProcessor.params.getParameter(prefix + "time_r");
		param->setValueNotifyingHost(param->convertTo0to1(nextTap.timeR));
	}
}

void DelayView::updateAmplitudes(const MouseEvent& e)
{
	float mousex = (float)e.position.x;
	float mousey = (float)e.position.y;

	if (mode == Delay::Mono)
	{
		for (int t = 0; t < ntaps; ++t)
		{
			Rectangle<float> b = bases_mono[t];
			if (mousex > b.getCentreX() - 2 && mousex < b.getCentreX() + 2)
			{
				float normalY = std::clamp((viewb.getBottom() - mousey - TAP_BASE_H) / (viewb.getHeight() - TAP_BASE_H), 0.f, 1.f);

				String prefix = "tap" + String(t) + "_";
				auto param = editor.audioProcessor.params.getParameter(prefix + "amp_l");
				param->setValueNotifyingHost(normalY);
				param = editor.audioProcessor.params.getParameter(prefix + "amp_r");
				param->setValueNotifyingHost(normalY);
			}
		}
	}
	else
	{
		if (mousey < viewb.getCentreY())
		{
			for (int t = 0; t < ntaps; ++t)
			{
				Rectangle<float> b = bases_left[t];
				if (mousex > b.getCentreX() - 2 && mousex < b.getCentreX() + 2)
				{
					float normalY = std::clamp((viewb.getCentreY() - mousey - TAP_BASE_H) / (viewb.getHeight() / 2.f - TAP_BASE_H), 0.f, 1.f);

					String prefix = "tap" + String(t) + "_";
					auto param = editor.audioProcessor.params.getParameter(prefix + "amp_l");
					param->setValueNotifyingHost(normalY);
				}
			}
		}
		else
		{
			for (int t = 0; t < ntaps; ++t)
			{
				Rectangle<float> b = bases_right[t];
				if (mousex > b.getCentreX() - 2 && mousex < b.getCentreX() + 2)
				{
					float normalY = std::clamp((mousey - viewb.getCentreY() - TAP_BASE_H) / (viewb.getHeight() / 2.f - TAP_BASE_H), 0.f, 1.f);

					String prefix = "tap" + String(t) + "_";
					auto param = editor.audioProcessor.params.getParameter(prefix + "amp_r");
					param->setValueNotifyingHost(normalY);
				}
			}
		}
	}
}

void DelayView::resized()
{
	auto b = getLocalBounds().toFloat();
	viewb = b.reduced(20.f);
	infob = viewb.withHeight(20.f).withY(viewb.getBottom());

	ampPicker->setBounds(infob.withTrimmedLeft(75.f).withWidth(110.f).toNearestInt());
	feedbkPicker->setBounds(infob.withTrimmedLeft(75.f + 110.f).withWidth(110.f).toNearestInt());
	globalFeedbk.setBounds(infob.withTrimmedLeft(75.f + 220.f - 30.f).withWidth(50.f).toNearestInt());
	menuBtn.setBounds((int)b.getRight() - 20, (int)b.getY(), 20, 20);
}

void DelayView::paint(Graphics& g)
{
	auto b = getLocalBounds().toFloat();
	UIUtils::drawBevel(g, b.translated(0.5f, 0.5f).reduced(1.f), 6.f, Colour(COLOR_BEVEL));

	mode = (Delay::DelayMode)editor.audioProcessor.params.getRawParameterValue("mode")->load();
	link = (bool)editor.audioProcessor.params.getRawParameterValue("link")->load();
	ntaps = (int)editor.audioProcessor.params.getRawParameterValue("ntaps")->load();
	timeMode = (Delay::TimeMode)editor.audioProcessor.params.getRawParameterValue("time_mode")->load();
	timeSync = (Delay::TimeSync)editor.audioProcessor.params.getRawParameterValue("time_sync")->load();
	timeMillis = (int)editor.audioProcessor.params.getRawParameterValue("time_millis")->load();

	if (mode == Delay::Mono && selectedTap >= 1337)
	{
		selectedTap -= 1337;
		updateInfo();
	}

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

	float lastxl = 0;
	float lastxr = 0;
	for (int t = 0; t < ntaps; ++t)
	{
		auto& tap = taps[t];
		float gridsz = viewb.getWidth() / (ntaps + 1);
		float x = gridsz * tap.timeL + lastxl;
		if (t == 0) x += viewb.getX();
		lastxl = x;
		bases_mono[t] = Rectangle<float>(x - TAP_BASE_W / 2.f, viewb.getBottom() - TAP_BASE_H, TAP_BASE_W, TAP_BASE_H);
		bases_left[t] = bases_mono[t].withY(viewb.getCentreY() - TAP_BASE_H);

		x = gridsz * tap.timeR + lastxr;
		if (t == 0) x += viewb.getX();
		lastxr = x;
		bases_right[t] = Rectangle<float>(x - TAP_BASE_W / 2.f, viewb.getCentreY(), TAP_BASE_W, TAP_BASE_H);
	}

	UIUtils::drawTriangle(g, menuBtn.getBounds().reduced(6).translated(-1, 3).toFloat(), 2, Colour(COLOR_NEUTRAL));

	g.setColour(Colour(COLOR_NEUTRAL));
	g.drawHorizontalLine((int)viewb.getY(), viewb.getX(), viewb.getRight());
	g.drawHorizontalLine((int)viewb.getBottom(), viewb.getX(), viewb.getRight());

	g.saveState();
	g.reduceClipRegion(viewb.expanded(5.f, 20.f).toNearestInt());
	drawGrid(g);
	drawTaps(g);
	drawInfo(g);
	g.restoreState();
}

void DelayView::drawGrid(Graphics& g)
{
	bool isTriplet = (bool)editor.audioProcessor.params.getRawParameterValue("triplet_grid")->load();
	int grid = isTriplet ? 3 * (ntaps + 1) : 4 * (ntaps + 1);

	float gridx = viewb.getWidth() / grid;
	float gridy = viewb.getHeight() / 8;

	g.setColour(Colours::white.withAlpha(0.1f)); // map score into min + score * (max - min)
	for (int i = 0; i < grid + 1; ++i)
	{
		bool isBold = isTriplet ? i % 3 == 0 : i % 4 == 0;
		float x = (float)(viewb.getX() + std::round(gridx * i) + 0.5f);

		g.setColour(Colours::white.withAlpha(isBold ? 0.1f : 0.05f));
		if (grid > 36 && !isTriplet && i % 2 == 1)
			g.setColour(Colours::transparentBlack);
		g.drawLine(x, viewb.getY(), x, viewb.getY() + viewb.getHeight());
	}

	g.setColour(Colours::white.withAlpha(0.05f));
	for (int i = 0; i < 8 + 1; ++i) 
	{
		float y = (float)(viewb.getY() + std::round(gridy * i) + 0.5f);
		g.drawLine(viewb.getX(), y, viewb.getX() + viewb.getWidth(), y);
	}

	g.setFont(FontOptions(14.f));
	for (int i = 1; i <= ntaps; ++i)
	{
		int num = i;
		int den = 0;
		if (timeSync == Delay::k1o64) den = 64;
		if (timeSync == Delay::k1o32) den = 32;
		if (timeSync == Delay::k1o16) den = 16;
		if (timeSync == Delay::k1o8) den = 8;
		if (timeSync == Delay::k1o4) den = 4;
		if (timeSync == Delay::k1o2) den = 2;
		if (timeSync == Delay::k1o1) den = 1;

		g.setColour(Colours::white.withAlpha(0.25f));
		auto x = isTriplet 
			? (int)(viewb.getX() + i * gridx * 3)
			: (int)(viewb.getX() + i * gridx * 4);
		g.drawVerticalLine(x, viewb.getY() - 5, viewb.getY());

		if (grid > 32 && i % 2 == 1)
			g.setColour(Colours::transparentBlack);

		String text;
		if (timeMode == Delay::Millis)
		{
			auto ms = timeMillis * i;
			text = ms >= 1000 ? String(ms / 1000.f, 2) + "s" : String(ms) + "ms";
		}
		else 
		{
			text = String(num);
		}
		g.drawText(text, Rectangle<int>({ x - 43, (int)viewb.getY() - 15, 40, 15 }), Justification::centredRight);
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

	auto drawWickMono = [&](Rectangle<float> base, Tap& tap, float mix, Colour c)
		{
			g.setColour(c.withAlpha(0.5f));
			float h = (viewb.getHeight() - TAP_BASE_H) * tap.ampL;
			g.fillRect(base.getCentreX() - TAP_LINE_W / 2.f, base.getY() - h, TAP_LINE_W, h + 2);
			g.setColour(c.withAlpha(1.f));
			g.fillRect(base.getCentreX() - TAP_LINE_W / 2.f, base.getY() - h * mix, TAP_LINE_W, h * mix + 2);
		};

	auto drawWickLeft = [&](Rectangle<float> base, Tap& tap, float mix, Colour c)
		{
			g.setColour(c.withAlpha(0.5f));
			float h = (viewb.getHeight() / 2.f - TAP_BASE_H) * tap.ampL;
			g.fillRect(base.getCentreX() - TAP_LINE_W / 2.f, base.getY() - h, TAP_LINE_W, h + 2);
			g.setColour(c);
			g.fillRect(base.getCentreX() - TAP_LINE_W / 2.f, base.getY() - h * mix, TAP_LINE_W, h * mix + 2);
		};

	auto drawWickRight = [&](Rectangle<float> base, Tap& tap, float mix, Colour c)
		{
			g.setColour(c.withAlpha(0.5f));
			float h = (viewb.getHeight() / 2.f - TAP_BASE_H) * tap.ampR;
			g.fillRect(base.getCentreX() - TAP_LINE_W / 2.f, base.getBottom() - 2, TAP_LINE_W, h + 2);
			g.setColour(c);
			g.fillRect(base.getCentreX() - TAP_LINE_W / 2.f, base.getBottom() - 2, TAP_LINE_W, h * mix + 2);
		};

	auto mix = editor.audioProcessor.params.getRawParameterValue("mix")->load();
	auto drymix = mix <= 0.5f ? 1.f : 1.f - (mix - 0.5f) * 2.f;
	auto wetmix = mix <= 0.5f ? mix * 2.f : 1.f;
	auto panWet = editor.audioProcessor.params.getRawParameterValue("pan_wet")->load();
	auto panDry = editor.audioProcessor.params.getRawParameterValue("pan_dry")->load();

	// draw dry input tap
	if (mode == Delay::Mono)
	{
		g.setColour(Colours::white.darker(0.5f).withAlpha(0.5f));
		g.fillRect(viewb.getX() - TAP_LINE_W, viewb.getY(), TAP_LINE_W, viewb.getHeight());
		float h = drymix * viewb.getHeight();
		g.setColour(Colours::white.darker(0.5f));
		g.fillRect(viewb.getX() - TAP_LINE_W, viewb.getBottom() - h, TAP_LINE_W, h);
	}
	else
	{
		auto panLeft = panDry <= 0.5f ? 1.f : 1.f - (panDry - 0.5f) * 2.f;
		auto panRight = panDry <= 0.5 ? panDry * 2.f : 1.f;
		g.setColour(Colours::white.darker(0.5f).withAlpha(0.5f));
		g.fillRect(viewb.getX() - TAP_LINE_W, viewb.getY(), TAP_LINE_W, viewb.getHeight());
		float h = viewb.getHeight() / 2 * drymix * panLeft;
		g.setColour(Colours::white.darker(0.5f));
		g.fillRect(viewb.getX() - TAP_LINE_W, viewb.getCentreY() - h, TAP_LINE_W, h);
		h = viewb.getHeight() / 2 * drymix * panRight;
		g.fillRect(viewb.getX() - TAP_LINE_W, viewb.getCentreY(), TAP_LINE_W, h);
	}

	if (mode == Delay::Mono)
	{
		for (int t = ntaps-1; t >= 0; --t)
		{
			bool mouseover = mouseOverBase == t;
			Colour c = mouseover ? Colour(COLOR_TAP_HOVER) : Colour(COLOR_TAP);
			g.setColour(c);
			drawBase(bases_mono[t]);
			drawWickMono(bases_mono[t], taps[t], wetmix, c);
		}
	}
	else
	{
		auto panLeft = panWet <= 0.5f ? 1.f : 1.f - (panWet - 0.5f) * 2.f;
		auto panRight = panWet <= 0.5 ? panWet * 2.f : 1.f;
		for (int t = ntaps - 1; t >= 0; --t)
		{
			Colour c = mouseOverBase == t || link && mouseOverBase == t + 1337 
				? Colour(COLOR_TAP_HOVER) 
				: (Colour(mode == Delay::PingPong && t % 2 == 1 ? COLOR_TAP2 : COLOR_TAP));
			g.setColour(c);
			drawBase(bases_left[t].expanded(0, 0.5f));
			drawWickLeft(bases_left[t], taps[t], wetmix * panLeft, c);
			c = mouseOverBase == t + 1337 || link && mouseOverBase == t 
				? Colour(COLOR_TAP_HOVER)
				: Colour(mode == Delay::PingPong && t % 2 == 1 ? COLOR_TAP : COLOR_TAP2);
			g.setColour(c);
			drawBaseInverted(bases_right[t]);
			drawWickRight(bases_right[t], taps[t], wetmix * panRight, c);
		}
	}
}

void DelayView::drawInfo(Graphics& g)
{
	auto idx = selectedTap >= 1337 ? selectedTap - 1337 : selectedTap;
	auto isLeft = selectedTap < 1337;

	g.setColour(Colour(COLOR_NEUTRAL));
	g.setFont(FontOptions(16.f));
	auto txt = "Tap " + String(idx + 1);
	if (mode != Delay::Mono)
		txt += isLeft ? "L" : "R";
	g.drawText(txt, infob.withWidth(80), Justification::centredLeft);

	bool useGlobalFeedbk = (bool)editor.audioProcessor.params.getRawParameterValue("tap" + String(idx) + "_feedback_global")->load();
	
	if (useGlobalFeedbk)
	{
		g.setColour(Colour(COLOR_NEUTRAL));
		g.fillRect(globalFeedbk.getBounds().reduced(0, 4));
		g.setColour(Colour(COLOR_BEVEL));
		g.drawText("Global", globalFeedbk.getBounds(), Justification::centred);
	}
	else
	{
		g.setColour(Colour(COLOR_NEUTRAL));
		g.drawText("Local", globalFeedbk.getBounds(), Justification::centred);
	}
}

void DelayView::showMenu()
{
	auto tripletGrid = (bool)editor.audioProcessor.params.getRawParameterValue("triplet_grid")->load();

	PopupMenu menu;
	menu.addItem(1, "Triplet Grid", true, tripletGrid);
	menu.addSeparator();
	PopupMenu feedbackMenu;
	feedbackMenu.addItem(20, "Set all taps global feedback");
	feedbackMenu.addItem(21, "Set first tap global feedback");

	menu.addSubMenu("Feedback", feedbackMenu);

	auto menuPos = localPointToGlobal(menuBtn.getBounds().getBottomLeft());
	menu.showMenuAsync(PopupMenu::Options()
		.withTargetComponent(*this)
		.withTargetScreenArea({ menuPos.getX(), menuPos.getY(), 1, 1 }),
		[this](int result)
		{
			if (result == 0) return;
			else if (result == 1)
			{
				auto param = editor.audioProcessor.params.getParameter("triplet_grid");
				param->setValueNotifyingHost(param->getValue() > 0.f ? 0.f : 1.f);
				repaint();
			}
			else if (result == 20)
			{
				for (int t = 0; t < MAX_TAPS; ++t)
				{
					auto param = editor.audioProcessor.params.getParameter("tap" + String(t) + "_feedback_global");
					param->setValueNotifyingHost(1.f);
				}
			}
			else if (result == 21)
			{
				for (int t = 0; t < MAX_TAPS; ++t)
				{
					auto param = editor.audioProcessor.params.getParameter("tap" + String(t) + "_feedback_global");
					param->setValueNotifyingHost((float)(t == 0));
				}
			}
		}
	);
}