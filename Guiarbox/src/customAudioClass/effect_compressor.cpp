#include "effect_compressor.h"
#include <Arduino.h>
#include <Audio.h>

void AudioEffectCompressor::update() {
    audio_block_t *block = receiveWritable(0);
    if (!block) return;

    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
        float input = (float)block->data[i] / 32768.0f;
        float amplitude = fabsf(input);

        if (amplitude > envelope)
            envelope = amplitude;  // instant attack
        else
            envelope += (amplitude - envelope) * releaseCoeff;

        float gain = 1.0f;
        if (envelope > threshold) {
            float over = envelope - threshold;
            gain = threshold + over / ratio;
            gain /= envelope;
            Serial.println("Compression active!");
        }
    

        float out = input * gain * level;
        if (out > 1.0f) out = 1.0f;
        else if (out < -1.0f) out = -1.0f;

        block->data[i] = (int16_t)(out * 32767.0f);
    }

    transmit(block);
    release(block);
}
