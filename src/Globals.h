#pragma once

namespace globals {
	constexpr float DB2LOG = 0.11512925464970228420089957273422f;
	constexpr float SQRT2 = 1.4142135623730951f;
	constexpr float ISQRT2 = 0.7071067811865475f;
	constexpr float EPSILON = 1e-6f;

	// filter consts
	constexpr float F_MIN_FREQ = 20.0f;
	constexpr float F_MAX_FREQ = 20000.0f;

	// DELAY
	constexpr int MAX_TAPS = 16;
	constexpr float MAX_HAAS = 25.f;
	constexpr float MAX_ACCENT = 0.5f;

	// UI
	constexpr float TAP_BASE_W = 13.f;
	constexpr float TAP_BASE_H = 10.f;
	constexpr float TAP_LINE_W = 3.f;
	constexpr int NAV_HEIGHT = 35; // top header
	constexpr int HEADER_HEIGHT = 20; // old header, now the row below nav
	constexpr int HSEPARATOR = 10;
	constexpr int VSEPARATOR = 20;
	constexpr int METER_WIDTH = 25;
	constexpr unsigned int COLOR_BACKGROUND = 0xff181818;
	constexpr unsigned int COLOR_ACTIVE = 0xff6DAAFF;
	constexpr unsigned int COLOR_NEUTRAL = 0xff666666;
	constexpr unsigned int COLOR_KNOB = 0xff272727;
	constexpr unsigned int COLOR_BEVEL = 0xff101010;
	constexpr unsigned int COLOR_TAP = 0xff80ffff;
	constexpr unsigned int COLOR_TAP2 = 0xffFE9E37;
	constexpr unsigned int COLOR_TAP_HOVER = 0xFF00ff00;
	constexpr float BEVEL_CORNER = 5.f;
	constexpr int KNOB_WIDTH = 70;
	constexpr int KNOB_HEIGHT = 75;

	// Envelope follower
	constexpr float ENV_MIN_ATTACK = 0.01f;
	constexpr float ENV_MAX_ATTACK = 200.0f;
	constexpr float ENV_MIN_RELEASE = 10.0f;
	constexpr float ENV_MAX_RELEASE = 10000.0f;

	// view consts
	constexpr int PLUG_WIDTH = 15 * 3 + 10 * 2 + 70 * 10 + 25; // plug padding + hseparator + knob width + meter
	constexpr int PLUG_HEIGHT = 15 * 2 + 20 + 10 * 3 + 20 + 75 * 3 + 35; // plug padding + vsep + header + knob height + nav
	constexpr int MAX_PLUG_WIDTH = 640 * 3;
	constexpr int MAX_PLUG_HEIGHT = 650 * 2;
	constexpr int PLUG_PADDING = 15;
};