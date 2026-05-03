#include "effect_compressor.h"
#include <Arduino.h>

// Match Teensy Audio default; envelope time constants are derived from this.
static constexpr float kAudioSampleRate = 44100.0f;

float AudioEffectCompressor::envelopeCoeffFromMs(float ms, float sampleRate) {
    if (ms <= 0.0f) {
        return 1.0f;
    }
    const float tauSec = ms * 0.001f;
    return 1.0f - expf(-1.0f / (tauSec * sampleRate));
}

void AudioEffectCompressor::setAttackMs(float ms) {
    attackAlpha = envelopeCoeffFromMs(ms, kAudioSampleRate);
}

void AudioEffectCompressor::setReleaseMs(float ms) {
    releaseAlpha = envelopeCoeffFromMs(ms, kAudioSampleRate);
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
