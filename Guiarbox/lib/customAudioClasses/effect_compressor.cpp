#include "effect_compressor.h"
#include <Arduino.h>
#include <math.h>

static constexpr float DEFAULT_SAMPLE_RATE = 44100.0f;
static constexpr float LN_SETTLE_RATIO = 2.99573227355f;

void AudioEffectCompressor::setAttackMs(float ms) {
    if (ms <= 0.0f) {
        attackAlpha = 1.0f;
    } else {
        attackAlpha = 1.0f - expf(-LN_SETTLE_RATIO / (ms * 0.001f * DEFAULT_SAMPLE_RATE));
    }
}

void AudioEffectCompressor::setReleaseMs(float ms) {
    if (ms <= 0.0f) {
        releaseAlpha = 1.0f;
    } else {
        releaseAlpha = 1.0f - expf(-LN_SETTLE_RATIO / (ms * 0.001f * DEFAULT_SAMPLE_RATE));
    }
}

void AudioEffectCompressor::update() {
    audio_block_t *block = receiveWritable(0);
    if (!block) return;

    if (!isEnabled()) {
        transmit(block);
        release(block);
        return;
    }

    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
        float input = (float)block->data[i] / 32768.0f;
        float amplitude = fabsf(input);

        if (amplitude > envelope) {
            envelope += (amplitude - envelope) * attackAlpha;
        } else {
            envelope += (amplitude - envelope) * releaseAlpha;
        }

        float gain = 1.0f;
        if (envelope > threshold) {
            float over = envelope - threshold;
            gain = threshold + over / ratio;
            gain /= envelope;
        }

        float out = input * gain * level;
        if (out > 1.0f) out = 1.0f;
        else if (out < -1.0f) out = -1.0f;

        block->data[i] = (int16_t)(out * 32767.0f);
    }

    transmit(block);
    release(block);
}
