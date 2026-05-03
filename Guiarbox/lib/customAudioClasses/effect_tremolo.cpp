#include "effect_tremolo.h"
#include <math.h>

namespace {

constexpr float MIN_RATE_HZ = 0.0f;
constexpr float MAX_RATE_HZ = 20.0f;

inline float clamp01(float x) { return constrain(x, 0.0f, 1.0f); }
inline float clampSigned1(float x) { return constrain(x, -1.0f, 1.0f); }

// Map [0..1) triangle with peak at 0.5.
inline float tri01(float u01) {
    // 0..1..0 across 0..1
    return 1.0f - fabsf(2.0f * u01 - 1.0f);
}

inline float onePoleAlphaFromTauSec(float tauSec, float sampleRate) {
    const float sr = max(sampleRate, 1.0f);
    const float t = max(tauSec, 1e-6f);
    return 1.0f - expf(-1.0f / (t * sr));
}

} // namespace

AudioEffectTremolo::AudioEffectTremolo() : AudioStream(1, inputQueueArray) {}

void AudioEffectTremolo::setRate(float hz) {
    rateHz = constrain(hz, MIN_RATE_HZ, MAX_RATE_HZ);
}

void AudioEffectTremolo::setDepth(float depth01) {
    depth = clamp01(depth01);
}

void AudioEffectTremolo::setShape(Shape s) {
    shape = s;
}

void AudioEffectTremolo::setBias(float biasSigned01) {
    bias = clampSigned1(biasSigned01);
}

void AudioEffectTremolo::enable() {
    enabled = true;
}

void AudioEffectTremolo::disable() {
    enabled = false;
}

float AudioEffectTremolo::lfoUnipolar01(float phaseRad) const {
    // Normalize to [0..1)
    float u = phaseRad * (1.0f / TWO_PI);
    u = u - floorf(u);

    switch (shape) {
    case Shape::Tri:
        return tri01(u);
    case Shape::Square:
        return (u < 0.5f) ? 1.0f : 0.0f;
    case Shape::Sine:
    default:
        return 0.5f * (1.0f + sinf(phaseRad));
    }
}

float AudioEffectTremolo::applyBias01(float x01) const {
    // bias=0 -> unchanged.
    // bias>0 -> more time near 1.0 ("loud") via a concave curve.
    // bias<0 -> more time near 0.0 ("quiet") via a convex curve.
    const float b = clampSigned1(bias);
    if (b == 0.0f) {
        return clamp01(x01);
    }

    // Map bias to exponent:
    // - b in (0..1]  -> exp in (0,1)  (concave)
    // - b in [-1..0) -> exp in (1,inf) (convex)
    const float expv = (b > 0.0f) ? (1.0f / (1.0f + 8.0f * b)) : (1.0f + 8.0f * (-b));
    return powf(clamp01(x01), expv);
}

void AudioEffectTremolo::update(void) {
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
    const float phaseInc = TWO_PI * rateHz * invSr;

    const float d = clamp01(depth);
    const float dryMinGain = 1.0f - d;

    // Very small rounding for square edges to remove clicks.
    // Constant time (not rate-dependent) keeps "square" character at all rates.
    const float squareEdgeTauSec = 0.0020f; // 2 ms
    const float squareAlpha = onePoleAlphaFromTauSec(squareEdgeTauSec, sr);

    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i) {
        const float x = (float)block->data[i] * (1.0f / 32768.0f);

        float lfo = lfoUnipolar01(lfoPhase); // 0..1 (raw)
        if (shape == Shape::Square) {
            squareLfoState += squareAlpha * (lfo - squareLfoState);
            lfo = squareLfoState;
        }
        lfo = applyBias01(lfo);

        // Gain range: [1-depth .. 1]
        const float g = dryMinGain + d * lfo;
        float y = x * g;

        lfoPhase += phaseInc;
        if (lfoPhase >= TWO_PI) {
            lfoPhase -= TWO_PI;
        } else if (lfoPhase < 0.0f) {
            lfoPhase += TWO_PI;
        }

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

