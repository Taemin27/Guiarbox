#include "effect_noiseGate.h"
#include <math.h>

namespace {
inline float clamp01(float x) { return constrain(x, 0.0f, 1.0f); }
} // namespace

float AudioEffectNoiseGate::coeffFromMs(float ms, float sampleRate) {
    if (ms <= 0.0f) {
        return 1.0f;
    }
    const float tauSec = ms * 0.001f;
    return 1.0f - expf(-1.0f / (tauSec * max(sampleRate, 1.0f)));
}

AudioEffectNoiseGate::AudioEffectNoiseGate() : AudioStream(1, inputQueueArray) {
    const float sr = AUDIO_SAMPLE_RATE_EXACT;

    setThreshold(0.10f);
    setAttackMs(5.0f);
    setReleaseMs(120.0f);
    setHoldMs(20.0f);
    setRange(1.0f);

    // Fixed detector release to reduce chatter; gate timing is handled by attack/release + hold.
    detectorReleaseAlpha = coeffFromMs(50.0f, sr);
}

void AudioEffectNoiseGate::setThreshold(float t) {
    threshold = clamp01(t);
}

void AudioEffectNoiseGate::setAttackMs(float ms) {
    attackAlpha = coeffFromMs(max(ms, 0.0f), AUDIO_SAMPLE_RATE_EXACT);
}

void AudioEffectNoiseGate::setReleaseMs(float ms) {
    releaseAlpha = coeffFromMs(max(ms, 0.0f), AUDIO_SAMPLE_RATE_EXACT);
}

void AudioEffectNoiseGate::setHoldMs(float ms) {
    const float sr = AUDIO_SAMPLE_RATE_EXACT;
    if (ms <= 0.0f) {
        holdSamples = 0;
        return;
    }
    const float samplesF = (ms * 0.001f) * sr;
    // Clamp to something sane to avoid huge counters if misconfigured.
    const float maxHoldSamples = sr * 2.0f; // 2 seconds
    holdSamples = (uint32_t)constrain(samplesF, 0.0f, maxHoldSamples);
}

void AudioEffectNoiseGate::setRange(float range01) {
    range = clamp01(range01);
}

void AudioEffectNoiseGate::enable() {
    enabled = true;
}

void AudioEffectNoiseGate::disable() {
    enabled = false;
}

void AudioEffectNoiseGate::update(void) {
    audio_block_t *block = receiveWritable(0);
    if (!block) {
        return;
    }

    if (!enabled) {
        transmit(block);
        release(block);
        return;
    }

    const float closedGain = 1.0f - range; // range=1 => mute

    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i) {
        const float x = (float)block->data[i] * (1.0f / 32768.0f);
        const float a = fabsf(x);

        // Envelope detector: instantaneous attack, smoothed release.
        if (a > env) {
            env = a;
        } else {
            env += (a - env) * detectorReleaseAlpha;
        }

        const bool above = (env >= threshold);
        if (above) {
            holdCounter = holdSamples;
        } else if (holdCounter > 0) {
            holdCounter--;
        }

        const bool gateOpen = above || (holdCounter > 0);
        const float targetGain = gateOpen ? 1.0f : closedGain;
        const float alpha = (targetGain > gain) ? attackAlpha : releaseAlpha;
        gain += (targetGain - gain) * alpha;

        float y = x * gain;
        y = constrain(y, -1.0f, 1.0f);
        block->data[i] = (int16_t)(y * 32767.0f);
    }

    transmit(block);
    release(block);
}

