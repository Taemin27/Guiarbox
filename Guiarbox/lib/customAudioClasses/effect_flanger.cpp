#include "effect_flanger.h"
#include <math.h>

namespace {

inline float clamp01(float x) { return constrain(x, 0.0f, 1.0f); }
inline float clampSigned1(float x) { return constrain(x, -1.0f, 1.0f); }

inline float softClip(float x) {
    // Cheap saturation to keep feedback stable and "jetty".
    // Continuous and symmetric, roughly tanh-ish for small MCU cost.
    const float a = 0.70f;
    if (x > 1.0f)
        x = 1.0f;
    else if (x < -1.0f)
        x = -1.0f;
    return x - a * x * x * x;
}

} // namespace

AudioEffectFlanger::AudioEffectFlanger() : AudioStream(1, inputQueueArray) {
    for (int i = 0; i < DELAY_BUFFER_SAMPLES; ++i) {
        delayBuffer[i] = 0.0f;
    }
}

void AudioEffectFlanger::setRate(float hz) {
    // Typical flanger range; allow 0 for manual-only.
    rateHz = constrain(hz, 0.0f, 10.0f);
}

void AudioEffectFlanger::setDepth(float depth01) {
    depth = clamp01(depth01);
}

void AudioEffectFlanger::setManual(float manual01) {
    manual = clamp01(manual01);
}

void AudioEffectFlanger::setFeedback(float feedbackSigned1) {
    feedback = clampSigned1(feedbackSigned1);
}

void AudioEffectFlanger::setMix(float mix01) {
    mix = clamp01(mix01);
}

void AudioEffectFlanger::enable() {
    enabled = true;
}

void AudioEffectFlanger::disable() {
    enabled = false;
}

float AudioEffectFlanger::readDelaySamples(float delaySamples) const {
    // delaySamples >= 0
    if (delaySamples < 0.0f) {
        delaySamples = 0.0f;
    }

    const float readPos = (float)writeIndex - delaySamples;
    const float floorPos = floorf(readPos);
    const int32_t i0 = (int32_t)floorPos;
    const float frac = readPos - floorPos;

    // power-of-two wrap
    const uint32_t mask = (uint32_t)(DELAY_BUFFER_SAMPLES - 1);
    const uint32_t idx0 = (uint32_t)i0 & mask;
    const uint32_t idx1 = (uint32_t)(i0 + 1) & mask;

    const float y0 = delayBuffer[idx0];
    const float y1 = delayBuffer[idx1];
    return y0 + (y1 - y0) * frac;
}

void AudioEffectFlanger::writeDelaySample(float x) {
    delayBuffer[writeIndex & (DELAY_BUFFER_SAMPLES - 1)] = x;
    writeIndex++;
}

void AudioEffectFlanger::update(void) {
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

    // Flanger delay range (ms). Keep it short for comb-filter "jet" character.
    // Min is non-zero to avoid denorm-ish edge cases and to keep interpolation stable.
    constexpr float MIN_DELAY_MS = 0.10f;
    constexpr float MAX_DELAY_MS = 8.00f;

    const float manual01 = clamp01(manual);
    const float depth01 = clamp01(depth);

    const float baseDelayMs = MIN_DELAY_MS + manual01 * (MAX_DELAY_MS - MIN_DELAY_MS);
    const float sweepMs = 0.5f * depth01 * (MAX_DELAY_MS - MIN_DELAY_MS);

    const float fb = clampSigned1(feedback);
    const float wetMix = clamp01(mix);
    const float dryMix = 1.0f - wetMix;

    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i) {
        const float x = (float)block->data[i] * (1.0f / 32768.0f);

        const float lfo = sinf(lfoPhase); // -1..1
        float delayMs = baseDelayMs + sweepMs * lfo;
        if (delayMs < MIN_DELAY_MS)
            delayMs = MIN_DELAY_MS;
        else if (delayMs > MAX_DELAY_MS)
            delayMs = MAX_DELAY_MS;

        const float delaySamples = delayMs * 0.001f * sr;
        const float delayed = readDelaySamples(delaySamples);

        // Feedback into delay line (classic flanger structure).
        // Saturate a bit to keep high feedback musical.
        const float toDelay = softClip(x + fb * delayed);
        writeDelaySample(toDelay);

        float y = dryMix * x + wetMix * delayed;

        lfoPhase += phaseInc;
        if (lfoPhase >= TWO_PI) {
            lfoPhase -= TWO_PI;
        } else if (lfoPhase < 0.0f) {
            lfoPhase += TWO_PI;
        }

        if (y > 1.0f)
            y = 1.0f;
        else if (y < -1.0f)
            y = -1.0f;
        block->data[i] = (int16_t)(y * 32767.0f);
    }

    transmit(block);
    release(block);
}

