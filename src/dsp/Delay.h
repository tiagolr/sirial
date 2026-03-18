// Copyright (C) 2025 tilr
#pragma once
#include <JuceHeader.h>
#include "DelayLine.h"
#include "Utils.h"

class SirialAudioProcessor;

class Delay
{
public:
	enum DelayMode
	{
		Mono,
		Stereo,
		PingPong,
	};

	enum TimeMode
	{
		Millis,
		Straight,
		Triplet,
		Dotted,
	};

	enum TimeSync
	{
		k1o64,
		k1o32,
		k1o16,
		k1o8,
		k1o4,
		k1o2,
		k1o1
	};

	struct Tap
	{
		float timePercL;
		float timePercR;
		int timeL;
		int timeR;
		RCFilter stimeL;
		RCFilter stimeR;
		float feedbackL;
		float feedbackR;
		float ampL;
		float ampR;
		bool globalFeedback;
		DelayLine left;
		DelayLine right;

		void clear()
		{
			left.clear();
			right.clear();
		}

		void setup(float srate)
		{
			stimeL.setup(0.15f, srate);
			stimeR.setup(0.15f, srate);
		}

		// balances left and right feedback
		void setFeedback(float fb)
		{
			float e = timePercL / (timePercR + EPSILON);
			if (timePercL < timePercR)
			{
				feedbackR = fb;
				feedbackL = std::pow(fb, e);
			}
			else
			{
				e = 1.f / e;
				feedbackL = fb;
				feedbackR = std::pow(fb, e);
			}
		}
	};

	Delay(SirialAudioProcessor& p);
	~Delay();

	void prepare(float _srate);
	void updateBaseSamples();
	void processBlock(float* left, float* right, int nsamps);
	void processReverse(float& left, float& right, int revsizeL, int revsizeR,
		int midL, int midR, int fadetotalL, int fadetotalR);
	void clear();
	void onSlider();

	// used to calculate host plugin tail
	int maxPreSamples = 0;
	int maxSizeSamples = 0;
	float maxFeedback = 0.f;
	float srate = 44100.f;

	int ntaps = 4;
	std::array<Tap, MAX_TAPS> taps;

private:
	SirialAudioProcessor& audioProcessor;

	TimeMode timeMode = Straight;
	TimeSync timeSync = k1o4;
	int timeMillis = 10;
	int baseSamples = 1; // time in samples for global base time

	float israte = 1.f / 44100.f;

	// reverse delay
	bool reverse = false;
	int revposL = 0;
	int revposR = 0;
	std::vector<float> revL;
	std::vector<float> revR;
};