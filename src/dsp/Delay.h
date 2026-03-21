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
		StereoMono
	};

	enum ModMode
	{
		LFO,
		SnH,
		Perlin
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

	struct OnePole
	{
		float srate = 44100.f;
		float freq = -1.f;
		float g_targ = 1.f;
		float g = 0.f;
		float s1 = 0.f;

		void setup(float _srate) 
		{
			srate = _srate;
			float f = freq;
			freq = -1;
			init(f);
			g = g_targ;
		}

		void init(float _freq)
		{
			if (freq == _freq) return;
			freq = _freq;
			g_targ = std::tan(MathConstants<float>::pi * std::fmin(freq / srate, 0.49f));
			g_targ = g_targ / (1.0f + g_targ);
		}

		float processLP(float x)
		{
			if (freq >= 20000.f) return x;
			float delta = g * (x - s1);
			s1 += delta;
			g += (g_targ - g) * 0.005f;
			return s1;
		}

		float processHP(float x)
		{
			if (freq <= 20.f) return x;
			float delta = g * (x - s1);
			s1 += delta;
			g += (g_targ - g) * 0.005f;
			return x - s1;
		}

		void clear()
		{
			s1 = 0.f;
		}
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
		OnePole leftLP;
		OnePole leftHP;
		OnePole rightLP;
		OnePole rightHP;

		void clear()
		{
			left.clear();
			right.clear();
			leftLP.clear();
			leftHP.clear();
			rightLP.clear();
			rightHP.clear();
		}

		void setup(float srate)
		{
			stimeL.setup(0.15f, srate);
			stimeR.setup(0.15f, srate);
			leftLP.setup(srate);
			leftHP.setup(srate);
			rightLP.setup(srate);
			rightHP.setup(srate);
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

		void setDamping(float lowcut, float highcut)
		{
			leftLP.init(highcut);
			rightLP.init(highcut);
			leftHP.init(lowcut);
			rightHP.init(lowcut);
		}
	};

	Delay(SirialAudioProcessor& p);
	~Delay();

	void prepare(float _srate);
	void updateBaseSamples();
	float getRateSyncQN();
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
	float israte = 0.0001f;

	int ntaps = 4;
	std::array<Tap, MAX_TAPS> taps;

private:
	SirialAudioProcessor& audioProcessor;
	bool isMono = true;
	ModMode modMode = LFO;
	TimeMode modRateMode = Millis; // Hz
	float modRateSyncQN = 1.f;
	float modPhase = 0.f;
	float lmodPhase = 0.f; // last phase
	RCFilter modDepthSmooth{};
	PerlinGen perlin{(uint32_t)rand()};
	float modSnH = 0.f;
	RCFilter modSnHSmooth;


	TimeMode timeMode = Straight;
	TimeSync timeSync = k1o4;
	int timeMillis = 10;
	int baseSamples = 1; // time in samples for global base time
	float globalRand = 0.f; // amplitude randomizer global


	// reverse delay
	bool reverse = false;
	int revposL = 0;
	int revposR = 0;
	std::vector<float> revL;
	std::vector<float> revR;

	// randomized amplitudes (humanize)
	int sampsCounter = 0; // used for random amplitude modulation
	int sampsPreFade = 500; // number of samples offset to start randomize
	std::array<RCFilter, MAX_TAPS> ampsL; // smoothed amplitudes
	std::array<RCFilter, MAX_TAPS> ampsR; // smoothed amplitudes
};