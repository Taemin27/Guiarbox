#include "effect_multiChorus.h"
#include <math.h>

namespace {

// Slight per-voice variation (hard-coded, deterministic).
constexpr float RATE_MULT[AudioEffectMultiChorus::MAX_VOICES] = {
    1.00f, 1.035f, 0.965f, 1.052f, 0.942f};

constexpr float VOICE_DEPTH_MULT[AudioEffectMultiChorus::MAX_VOICES] = {
    1.00f, 0.91f, 1.09f, 0.94f, 1.07f};

// Extra static delay (samples) on top of the shared center; keeps taps decorrelated.
constexpr float VOICE_DELAY_OFFSET_SAMPLES[AudioEffectMultiChorus::MAX_VOICES] = {
    0.0f, 4.25f, -2.75f, 5.5f, -4.1f};

constexpr float MIN_RATE_HZ = 0.03f;
constexpr float MAX_RATE_HZ = 8.0f;
// Chorus-style short delays: keep a fixed minimum, sweep up to a maximum.
// This avoids negative/near-zero delay times, which would otherwise hard-clamp and "snap".
constexpr float MIN_DELAY_MS = 5.0f;
constexpr float MAX_DELAY_MS = 50.0f;

} // namespace

AudioEffectMultiChorus::AudioEffectMultiChorus() : AudioStream(1, inputQueueArray) {
    for (uint32_t i = 0; i < BUFFER_SIZE; ++i) {
        delayLine[i] = 0.0f;
    }
    redistributePhases();
}

void AudioEffectMultiChorus::redistributePhases() {
    const int n = constrain(voiceCount, 1, MAX_VOICES);
    for (int v = 0; v < MAX_VOICES; ++v) {
        if (v < n) {
            lfoPhase[v] = TWO_PI * (float)v / (float)n;
        } else {
            lfoPhase[v] = 0.0f;
        }
    }
}

void AudioEffectMultiChorus::setRate(float hz) {
    rateHz = constrain(hz, MIN_RATE_HZ, MAX_RATE_HZ);
}

void AudioEffectMultiChorus::setDepthMs(float ms) {
    // Max delay time (ms). We keep a fixed minimum delay to stay chorus-like.
    maxDelayMs = constrain(ms, MIN_DELAY_MS, MAX_DELAY_MS);
}

void AudioEffectMultiChorus::setDepth(float depth01) {
    // Map normalized 0..1 to [MIN_DELAY_MS .. MAX_DELAY_MS] for the sweep maximum.
    const float d01 = constrain(depth01, 0.0f, 1.0f);
    setDepthMs(MIN_DELAY_MS + d01 * (MAX_DELAY_MS - MIN_DELAY_MS));
}

void AudioEffectMultiChorus::setWetLevel(float level01) {
    wetLevel = constrain(level01, 0.0f, 1.0f);
}

void AudioEffectMultiChorus::setVoices(int count) {
    const int n = constrain(count, 1, MAX_VOICES);
    if (n != voiceCount) {
        voiceCount = n;
        redistributePhases();
    }
}

void AudioEffectMultiChorus::enable() {
    enabled = true;
}

void AudioEffectMultiChorus::disable() {
    enabled = false;
}

float AudioEffectMultiChorus::readInterpolated(float delaySamples, uint32_t writeRef) const {
    const float maxDelay = (float)(BUFFER_SIZE - 2u);
    if (delaySamples < 1.0f) {
        delaySamples = 1.0f;
    } else if (delaySamples > maxDelay) {
        delaySamples = maxDelay;
    }

    const int32_t id = (int32_t)floorf(delaySamples);
    const float frac = delaySamples - (float)id;

    const uint32_t i0 = (writeRef - (uint32_t)id) & BUFFER_MASK;
    const uint32_t i1 = (writeRef - (uint32_t)id - 1u) & BUFFER_MASK;

    return delayLine[i0] * (1.0f - frac) + delayLine[i1] * frac;
}

void AudioEffectMultiChorus::update(void) {
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
    const float samplesPerMs = sr * 0.001f;
    const float minDelaySamples = MIN_DELAY_MS * samplesPerMs;
    const float maxDelaySamples = maxDelayMs * samplesPerMs;
    const float rangeSamples = max(0.0f, maxDelaySamples - minDelaySamples);
    const int n = constrain(voiceCount, 1, MAX_VOICES);

    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i) {
        const float dry = (float)block->data[i] * (1.0f / 32768.0f);

        const uint32_t w = writeIndex;
        delayLine[w] = dry;

        float wet = 0.0f;
        for (int v = 0; v < n; ++v) {
            const float mod01 = 0.5f * (1.0f + sinf(lfoPhase[v])); // 0..1
            lfoPhase[v] += TWO_PI * rateHz * RATE_MULT[v] * invSr;
            if (lfoPhase[v] >= TWO_PI) {
                lfoPhase[v] -= TWO_PI;
            } else if (lfoPhase[v] < 0.0f) {
                lfoPhase[v] += TWO_PI;
            }

            const float tapDelay =
                (minDelaySamples + rangeSamples * VOICE_DEPTH_MULT[v] * mod01) +
                VOICE_DELAY_OFFSET_SAMPLES[v];
            wet += readInterpolated(tapDelay, w);
        }
        wet *= (1.0f / (float)n);
        const float out = dry + wet * wetLevel;

        float y = out;
        if (y > 1.0f) {
            y = 1.0f;
        } else if (y < -1.0f) {
            y = -1.0f;
        }
        block->data[i] = (int16_t)(y * 32767.0f);

        writeIndex = (writeIndex + 1u) & BUFFER_MASK;
    }

    transmit(block);
    release(block);
}
