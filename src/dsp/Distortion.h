// Copyright (C) 2025 tilr
// Tape distortion based of JSFXClones/JClones_TapeHead
#pragma once
#include <JuceHeader.h>

class SirialAudioProcessor;

class Distortion
{
public:
	float color = -1.f;
	float drive = -1.f;
	float driveGain = 0.f;
	float trimGain = 1.f;
	float trim = -100.f;

	Distortion(SirialAudioProcessor& p);
	~Distortion();

	void prepare(float _srate);
	void onSlider();
	float saturate(float x) const;
	void processBlock(float* left, float* right, int nsamps);
	void process(float& left, float& right, float drygain, float wetgain);
	void clear();

private:
	float srate = 88200.0;
	SirialAudioProcessor& audioProcessor;

	// coeffs
	float k1 = 0.f;
	float k2 = 0.f;
	float k3 = 0.f;
	float g3 = 0.f;
	float y1l = 0.f;
	float y2l = 0.f;
	float y3l = 0.f;
	float y1r = 0.f;
	float y2r = 0.f;
	float y3r = 0.f;
};