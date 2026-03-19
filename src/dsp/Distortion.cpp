#include "Distortion.h"
#include "../PluginProcessor.h"

Distortion::Distortion(SirialAudioProcessor& p) : audioProcessor(p) {}
Distortion::~Distortion() {}

void Distortion::prepare(float _srate)
{
	srate = _srate;
	clear();
}

void Distortion::clear()
{
	y1l = 0.f;
	y2l = 0.f;
	y3l = 0.f;

	y1r = 0.f;
	y2r = 0.f;
	y3r = 0.f;
}

void Distortion::onSlider()
{
	float d = audioProcessor.params.getRawParameterValue("dist_drive")->load();
	if (d != drive) {
		drive = d;
		driveGain = std::pow(10.f, drive);
		driveGain *= 0.8f;
		// auto gain compensation
		trimGain = driveGain < 1.f ? 1.f : std::pow(driveGain, -0.3f);
	}

	float c = audioProcessor.params.getRawParameterValue("dist_color")->load();
	if (c != color) {
		color = c;
		float freq_hz = 2100.f + color * (5000.f - 2100.f);

		k1 = 5.0f / 7.0f;
		k2 = 2.0f * std::sin(freq_hz * MathConstants<float>::pi / srate);
		k3 = 1.4f * (k2 * -1.0f);

		float gain = (srate < 88200.0f) ? 1.4f : 2.0f;
		g3 = std::pow(10.0f, gain * 3.0f / 20.0f) * -1.0f * k1;
	}
}

float Distortion::saturate(float x) const
{
	float y;

	x *= driveGain;
	if (x < -1.f) y = -1.f;
	else if (x > 1.f) y = 1.f;
	else y = x * 1.5f - x * x * x * 0.5f;

	return y;
}

void Distortion::processBlock(float* left, float* right, int nsamps)
{
	for (int i = 0; i < nsamps; ++i)
	{
		// filter left
		y1l += y2l * k2;
		y3l = y1l * k1 + y2l - left[i];
		y2l += y3l * k3;

		// saturate left
		float y1_satl = saturate(y1l);
		float y2_satl = saturate(y2l);
		float y = (y3l * g3 + y1_satl + y2_satl) * trimGain;
		left[i] = y;

		// filter right
		y1r += y2r * k2;
		y3r = y1r * k1 + y2r - right[i];
		y2r += y3r * k3;

		// saturate right
		float y1_satr = saturate(y1r);
		float y2_satr = saturate(y2r);
		y = (y3r * g3 + y1_satr + y2_satr) * trimGain;
		right[i] = y;
	}
}

void Distortion::process(float& left, float& right, float drygain, float wetgain)
{
	// filter left
	y1l += y2l * k2;
	y3l = y1l * k1 + y2l - left;
	y2l += y3l * k3;

	// saturate left
	float y1_satl = saturate(y1l);
	float y2_satl = saturate(y2l);
	float y = (y3l * g3 + y1_satl + y2_satl) * trimGain;
	left = left * drygain + y * wetgain;

	// filter right
	y1r += y2r * k2;
	y3r = y1r * k1 + y2r - right;
	y2r += y3r * k3;

	// saturate right
	float y1_satr = saturate(y1r);
	float y2_satr = saturate(y2r);
	y = (y3r * g3 + y1_satr + y2_satr) * trimGain;
	right = right * drygain + y * wetgain;
}