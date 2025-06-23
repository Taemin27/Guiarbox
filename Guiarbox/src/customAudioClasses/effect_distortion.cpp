#include "effect_distortion.h"

void AudioEffectDistortion::update(void) {
    audio_block_t *block = receiveWritable(0);
    if (!block) return;

    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
        float sample = (float)block->data[i] / 32768.0f;

        envelope = smoothing * envelope + (1.0f - smoothing) * fabs(sample);

        // gain
        float dynamicGain = (1.0f + envelope * dynamicGainResponse) * gain;
        sample *= dynamicGain;

        // Stage 1 soft clipping
        sample = tanh(sample);

        // clipping
        float posClip;
        float negClip;

        posClip = positiveClip - envelope * positiveClipResponse;
        negClip = negativeClip + envelope * negativeClipResponse; 

        sample = constrain(sample, negClip, posClip);

        // level
        sample *= level;

        // clamp to [-1.0, 1.0]
        sample = constrain(sample, -1.0f, 1.0f);

        block->data[i] = (int16_t) (sample * 32767.0f);
    }

    transmit(block);
    release(block);
}
