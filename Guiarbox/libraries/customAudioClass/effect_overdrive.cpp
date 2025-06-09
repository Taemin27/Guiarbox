#include "effect_overdrive.h"

void AudioEffectOverdrive::update(void) {
    audio_block_t *block = receiveWritable(0);
    if (!block) return;

    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
        float sample = (float)block->data[i] / 32768.0f;

        // Apply tempGain to bring signal up to level
        sample *= tempGain;

        float cubicGain = drive * overtoneMix;
        float tanhGain = drive * (1.0f - overtoneMix);

        // Cubic distortion (creates overtones)
        sample *= cubicGain;
        sample = sample - (1.0f / 3.0f) * sample * sample * sample;

        // Tanh soft clip
        sample *= tanhGain;
        sample = tanhf(sample);

        // Revert tempGain and apply level
        sample /= tempGain;
        sample *= level;

        // Clamp to [-1.0, 1.0]
        sample = constrain(sample, -1.0f, 1.0f);
        
        block->data[i] = (int16_t)(sample * 32767.0f);
    }

    transmit(block);
    release(block);
}