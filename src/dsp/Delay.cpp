#include "Delay.h"
#include "../PluginProcessor.h"

Delay::Delay(SirialAudioProcessor& p)
	: audioProcessor(p)
{
    for (int t = 0; t < MAX_TAPS; ++t)
    {
        auto& tap = taps[t];
        tap.stimeL.eps = 0.1f;
        tap.stimeR.eps = 0.1f;
    }
}

Delay::~Delay()
{
}

void Delay::clear()
{
    for (auto& tap : taps)
    {
        tap.clear();
    }
    revL.clear();
    revR.clear();
}

void Delay::prepare(float _srate)
{
    srate = _srate;
    israte = 1.f / srate;

    for (auto& tap : taps)
    {
        tap.setup(srate);
    }

    clear();
}

void Delay::onSlider()
{
    bool isMono = audioProcessor.params.getRawParameterValue("mode")->load() == 0.f;
    ntaps = (int)audioProcessor.params.getRawParameterValue("ntaps")->load();
    bool rev = (bool)audioProcessor.params.getRawParameterValue("reverse")->load();
    if (rev != reverse) clear();
    reverse = rev;
    float globalFeedback = audioProcessor.params.getRawParameterValue("feedback")->load();
    auto lowcut = audioProcessor.params.getRawParameterValue("lowcut")->load();
    auto highcut = audioProcessor.params.getRawParameterValue("highcut")->load();

    for (int t = 0; t < ntaps; ++t)
    {
        String prefix = "tap" + String(t) + "_";
        auto& tap = taps[t];
        tap.ampL = audioProcessor.params.getRawParameterValue(prefix + "amp_l")->load();
        tap.ampR = isMono 
            ? tap.ampL 
            : audioProcessor.params.getRawParameterValue(prefix + "amp_r")->load();
        tap.timePercL = audioProcessor.params.getRawParameterValue(prefix + "time_l")->load();
        tap.timePercR = isMono 
            ? tap.timePercL
            : audioProcessor.params.getRawParameterValue(prefix + "time_r")->load();
        bool useGlobalFeedback = audioProcessor.params.getRawParameterValue(prefix + "feedback_global")->load();
        tap.setFeedback(
            useGlobalFeedback ? globalFeedback :
            audioProcessor.params.getRawParameterValue(prefix + "feedback")->load()
        );
        tap.setDamping(lowcut, highcut);
    }

    timeMode = (TimeMode)audioProcessor.params.getRawParameterValue("time_mode")->load();
    timeSync = (TimeSync)audioProcessor.params.getRawParameterValue("time_sync")->load();
    timeMillis = (int)audioProcessor.params.getRawParameterValue("time_millis")->load();
}

void Delay::updateBaseSamples()
{
    if (timeMode == Millis)
    {
        baseSamples = (int)(timeMillis * srate * 0.001f);
        return;
    }

    auto secondsPerBeat = audioProcessor.secondsPerBeat;
    if (secondsPerBeat == 0.f) secondsPerBeat = 0.25f;

    float qn = 1.f;
    if (timeSync == k1o64) qn = 1.f / 16.f; // 1/64
    if (timeSync == k1o32) qn = 1.f / 8.f; // 1/32
    if (timeSync == k1o16) qn = 1.f / 4.f; // 1/16
    if (timeSync == k1o8) qn = 1.f / 2.f; // 1/8
    if (timeSync == k1o4) qn = 1.f / 1.f; // 1/4
    if (timeSync == k1o2) qn = 1.f * 2.f; // 1/2
    if (timeSync == k1o1) qn = 1.f * 4.f; // 1/1
    if (timeMode == Triplet) qn *= 2 / 3.f;
    if (timeMode == Dotted) qn *= 1.5f;

    baseSamples = (int)std::ceil(qn * secondsPerBeat * srate);
}

void Delay::processBlock(float* left, float* right, int nsamps)
{
    auto mode = (DelayMode)audioProcessor.params.getRawParameterValue("mode")->load();

    // resize taps delaylines
    updateBaseSamples();
    for (int t = 0; t < ntaps; ++t)
    {
        auto& tap = taps[t];
        tap.timeL = std::max(1, (int)(baseSamples * tap.timePercL));
        tap.timeR = std::max(1, (int)(baseSamples * tap.timePercR));
        if (tap.left.size < tap.timeL) tap.left.resize(tap.timeL);
        if (tap.right.size < tap.timeR) tap.right.resize(tap.timeR);
    }

    // reverse
    int revsizeL = 0;
    int revsizeR = 0;
    int fadetotalL = 0;
    int fadetotalR = 0;
    int midL = 0;
    int midR = 0;
    int inputOffsetL = 0;
    int inputOffsetR = 0;

    if (reverse)
    {
        // reverse uses double the size of delay buffers
        // the idea is that when the reverse read pointer crosses half
        // a full measure of reversed signal has been written
        auto& tap = taps[0];

        float tl = (tap.timeL) * 2.f;
        float tr = (tap.timeR) * 2.f;
        revL.resize((int)std::ceil(tl), 0.f);
        revR.resize((int)std::ceil(tr), 0.f);
        revsizeL = (int)revL.size();
        revsizeR = (int)revR.size();
        fadetotalL = (int)std::ceil(revsizeL * 0.05f);
        fadetotalR = (int)std::ceil(revsizeR * 0.05f);
        midL = revsizeL / 2;
        midR = revsizeR / 2;
        if (revsizeL > 1) inputOffsetL = -revsizeL / 2 + 1;
        if (revsizeR > 1) inputOffsetR = -revsizeR / 2 + 1;
    }


    // Process samples
    std::array<float, MAX_TAPS> lfeed{};
    std::array<float, MAX_TAPS> rfeed{};
    for (int i = 0; i < nsamps; ++i)
    {
        if (reverse)
        {
            processReverse(left[i], right[i], revsizeL, revsizeR, midL, midR, fadetotalL, fadetotalR);
        }

        // read from taps
        for (int t = 0; t < ntaps; ++t)
        {
            auto& tap = taps[t];
            float ltime = tap.stimeL.process((float)tap.timeL);
            float rtime = tap.stimeR.process((float)tap.timeR);
            float l = tap.left.read3(ltime);
            float r = tap.right.read3(rtime);
            l = tap.leftHP.processHP(tap.leftLP.processLP(l));
            r = tap.rightHP.processHP(tap.rightLP.processLP(r));
            lfeed[t] = l;
            rfeed[t] = r;
        }

        // write taps
        // first tap is special, it receives direct input and may be reversed
        {
            auto& tap = taps[0];
            if (mode == PingPong)
            {
                float monoIn = (left[i] + right[i]) * ISQRT2;
                tap.left.writeOffset(monoIn, inputOffsetL, inputOffsetL < 0);
                tap.right.writeOffset(monoIn, inputOffsetR, inputOffsetR < 0);
                tap.left.write(rfeed[ntaps-1] * tap.feedbackL, inputOffsetL >= 0);
                tap.right.write(lfeed[ntaps-1] * tap.feedbackR, inputOffsetR >= 0);
            }
            else
            {
                tap.left.writeOffset(left[i], inputOffsetL, inputOffsetL < 0);
                tap.right.writeOffset(right[i], inputOffsetR, inputOffsetR < 0);
                tap.left.write(lfeed[ntaps-1] * tap.feedbackL, inputOffsetL >= 0);
                tap.right.write(rfeed[ntaps-1] * tap.feedbackR, inputOffsetR >= 0);
            }
        }

        // write the rest of the taps
        if (mode == PingPong)
        {
            for (int t = 1; t < ntaps; ++t)
            {
                auto& tap = taps[t];
                tap.left.write(rfeed[t - 1] * tap.feedbackL);
                tap.right.write(lfeed[t - 1] * tap.feedbackR);
            }
        }
        else
        {
            for (int t = 1; t < ntaps; ++t)
            {
                auto& tap = taps[t];
                tap.left.write(lfeed[t - 1] * tap.feedbackL);
                tap.right.write(rfeed[t - 1] * tap.feedbackR);
            }
        }

        // write output
        left[i] = 0.f;
        right[i] = 0.f;

        for (int t = 0; t < ntaps; ++t)
        {
            auto& tap = taps[t];
            left[i] += lfeed[t] * tap.ampL;
            right[i] += rfeed[t] * tap.ampR;
        }
    }
}

// replaces left and right input samples with reverse buffer output
void Delay::processReverse(float& left, float& right, int revsizeL, int revsizeR, int midL, int midR,
    int fadetotalL, int fadetotalR)
{
    int playposL = revsizeL - revposL;
    int playposR = revsizeR - revposR;
    playposL = std::clamp(playposL, 0, revsizeL - 1);
    playposR = std::clamp(playposR, 0, revsizeR - 1);
    float fadeL = 1.f;

    // apply fades at reverse buffer begin, mid buffer, and end
    // because the reverse buffer is twice the size of the delay
    // fade at the middle as well to avoid clicks
    if (playposL < fadetotalL) // fade in
        fadeL = playposL / (float)fadetotalL;
    if (playposL > revsizeL - fadetotalL) // fade out
        fadeL = (revsizeL - playposL) / (float)fadetotalL;

    int midStart = midL - fadetotalL;
    int midEnd = midL + fadetotalL;
    if (playposL >= midStart && playposL <= midL)
        fadeL = 1.f - (playposL - midStart) / (float)fadetotalL;
    if (playposL > midL && playposL <= midEnd)
        fadeL = (playposL - midL) / (float)fadetotalL;

    float fadeR = 1.f;
    if (playposR < fadetotalR) // fade in
        fadeR = playposR / (float)fadetotalR;
    if (playposR > revsizeR - fadetotalR) // fade out
        fadeR = (revsizeR - playposR) / (float)fadetotalR;

    midStart = midR - fadetotalR;
    midEnd = midR + fadetotalR;
    if (playposR >= midStart && playposR <= midR)
        fadeR = 1.f - (playposR - midStart) / (float)fadetotalR;
    if (playposR > midR && playposR <= midEnd)
        fadeR = (playposR - midR) / (float)fadetotalR;

    // replace the input buffer in-place with the reversed buffer read
    float ll = left;
    float rr = right;
    if (revsizeL > 1) left = revL[playposL] * fadeL;
    if (revsizeR > 1) right = revR[playposR] * fadeR;

    revposL = (revposL + 1) % revsizeL;
    revposR = (revposR + 1) % revsizeR;
    revL[revposL] = ll;
    revR[revposR] = rr;
}