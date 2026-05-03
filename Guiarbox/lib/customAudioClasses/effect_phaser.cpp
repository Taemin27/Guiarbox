#include "effect_phaser.h"
#include <math.h>

namespace {

constexpr float MIN_RATE_HZ = 0.0f;
constexpr float MAX_RATE_HZ = 10.0f;

// Feedback (regen) is signed. Keep < 1.0 for stability with cascaded allpasses.
constexpr float MAX_ABS_FEEDBACK = 0.85f;

// Sweep range. Lowered to emphasize low/mid "whoosh" on guitar.
constexpr float FC_MIN_HZ = 120.0f;
constexpr float FC_MAX_HZ = 3000.0f;

inline float clamp01(float x) { return constrain(x, 0.0f, 1.0f); }
inline float clampSigned1(float x) { return constrain(x, -1.0f, 1.0f); }

// Soft saturation for feedback loop stability & a bit more "chew" at high regen.
inline float softClip(float x) {
    return tanhf(x);
}

} // namespace

AudioEffectPhaser::AudioEffectPhaser() : AudioStream(1, inputQueueArray) {
    for (int i = 0; i < MAX_STAGES; ++i) {
        stageZ1[i] = 0.0f;
    }
}

void AudioEffectPhaser::setRate(float hz) {
    rateHz = constrain(hz, MIN_RATE_HZ, MAX_RATE_HZ);
}

void AudioEffectPhaser::setDepth(float depth01) {
    depth = clamp01(depth01);
}

void AudioEffectPhaser::setMix(float mix01) {
    mix = clamp01(mix01);
}

void AudioEffectPhaser::setFeedback(float feedbackAmount) {
    feedback = clampSigned1(feedbackAmount) * MAX_ABS_FEEDBACK;
}

void AudioEffectPhaser::setStages(int count) {
    stages = constrain(count, 1, MAX_STAGES);
}

void AudioEffectPhaser::enable() {
    enabled = true;
    feedbackState = 0.0f;
}

void AudioEffectPhaser::disable() {
    enabled = false;
}

float AudioEffectPhaser::computeAllpassA(float fcHz, float sampleRate) const {
    const float sr = max(sampleRate, 1.0f);
    float fc = fcHz;
    if (fc < 5.0f) {
        fc = 5.0f;
    } else if (fc > 0.49f * sr) {
        fc = 0.49f * sr;
    }

    const float w = TWO_PI * fc / sr;
    const float t = tanf(0.5f * w);
    // 1st-order allpass coefficient for bilinear transform.
    return (t - 1.0f) / (t + 1.0f);
}

float AudioEffectPhaser::processAllpassCascade(float x, float a) {
    const int n = constrain(stages, 1, MAX_STAGES);
    for (int s = 0; s < n; ++s) {
        const float y = (-a * x) + stageZ1[s];
        stageZ1[s] = x + (a * y);
        x = y;
    }
    return x;
}

void AudioEffectPhaser::update(void) {
    audio_block_t *block = receiveWritable(0);
    if (!block) {
        return;
    }

    if (!enabled) {
        transmit(block);
        release(block);
        return;
    }

    const float sr = AUDIO_SAMPLE_RATE_EXACT;
    const float invSr = 1.0f / max(sr, 1.0f);

    // Depth crossfades between "fixed at mid sweep" and "full LFO sweep".
    const float sweepMid = 0.5f;
    const float sweepRatio = (FC_MAX_HZ / FC_MIN_HZ);
    // Small stage-to-stage frequency spread makes notches denser and more "swirly".
    const float stageSpread = 0.12f;

    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i) {
        const float dry = (float)block->data[i] * (1.0f / 32768.0f);

        const float lfo = 0.5f * (1.0f + sinf(lfoPhase)); // 0..1
        lfoPhase += TWO_PI * rateHz * invSr;
        if (lfoPhase >= TWO_PI) {
            lfoPhase -= TWO_PI;
        } else if (lfoPhase < 0.0f) {
            lfoPhase += TWO_PI;
        }

        const float sweepPos = (1.0f - depth) * sweepMid + depth * lfo; // 0..1
        const float fcBase = FC_MIN_HZ * powf(sweepRatio, sweepPos);

        // Signed regeneration around the allpass cascade.
        // Saturate to keep high feedback musical/stable.
        const float x = dry + softClip(feedbackState) * feedback;

        // Cascade with per-stage coefficient to create richer moving notch patterns.
        const int n = constrain(stages, 1, MAX_STAGES);
        float wet = x;
        const float center = 0.5f * (float)(n - 1);
        for (int s = 0; s < n; ++s) {
            const float norm = (n <= 1) ? 0.0f : ((float)s - center) / max(center, 1.0f); // ~[-1..1]
            const float fc = fcBase * (1.0f + stageSpread * norm);
            const float a = computeAllpassA(fc, sr);

            const float y = (-a * wet) + stageZ1[s];
            stageZ1[s] = wet + (a * y);
            wet = y;
        }
        feedbackState = wet;

        const float out = dry * (1.0f - mix) + wet * mix;

        float y = out;
        if (y > 1.0f) {
            y = 1.0f;
        } else if (y < -1.0f) {
            y = -1.0f;
        }
        block->data[i] = (int16_t)(y * 32767.0f);
    }

    transmit(block);
    release(block);
}

