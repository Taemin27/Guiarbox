#include "effect_freeverb_fp.h"
#include <string.h>

DMAMEM static float s_combStorage[AudioEffectFreeverbFP::kCombBufferSize];
DMAMEM static float s_allpassStorage[AudioEffectFreeverbFP::kAllpassBufferSize];
DMAMEM static float s_predelayBuffer[AudioEffectFreeverbFP::kPredelayBufferSize];

namespace {

constexpr float kInputScale = 1.0f / 32768.0f;
constexpr float kOutputGain = 0.55f;
constexpr float kDenormalFloor = 1.0e-12f;
constexpr float kSampleRate = AUDIO_SAMPLE_RATE_EXACT;
inline float clamp01(float value) { return constrain(value, 0.0f, 1.0f); }

} // namespace

AudioEffectFreeverbFP::Comb::Comb(int combSize, float *storage)
    : size(combSize), buffer(storage) {
    clear();
}

void AudioEffectFreeverbFP::Comb::setFeedback(float value) {
    feedback = constrain(value, 0.0f, 0.99f);
}

void AudioEffectFreeverbFP::Comb::setDamp(float value) {
    damp1 = constrain(value, 0.0f, 1.0f);
    damp2 = 1.0f - damp1;
}

float AudioEffectFreeverbFP::Comb::process(float input) {
    const float output = buffer[index];
    filterStore = (output * damp2) + (filterStore * damp1);
    buffer[index] = input + (filterStore * feedback);
    index++;
    if (index >= size) {
        index = 0;
    }
    return output;
}

void AudioEffectFreeverbFP::Comb::clear() {
    memset(buffer, 0, sizeof(float) * size);
    index = 0;
    filterStore = 0.0f;
}

AudioEffectFreeverbFP::Allpass::Allpass(int apSize, float *storage)
    : size(apSize), buffer(storage) {
    clear();
}

float AudioEffectFreeverbFP::Allpass::process(float input) {
    const float buffered = buffer[index];
    buffer[index] = input + (buffered * 0.5f);
    index++;
    if (index >= size) {
        index = 0;
    }
    return buffered - input;
}

void AudioEffectFreeverbFP::Allpass::clear() {
    memset(buffer, 0, sizeof(float) * size);
    index = 0;
}

float AudioEffectFreeverbFP::flushDenormal(float value) {
    if (value > -kDenormalFloor && value < kDenormalFloor) {
        return 0.0f;
    }
    return value;
}

float AudioEffectFreeverbFP::readPredelay() const {
    if (predelaySamples <= 0.0f) {
        return 0.0f;
    }

    const float readPos =
        (float)predelayWriteIndex - predelaySamples + (float)kPredelayBufferSize;
    const int readIndex =
        (((int)readPos % kPredelayBufferSize) + kPredelayBufferSize) % kPredelayBufferSize;
    return predelayBuffer[readIndex];
}

void AudioEffectFreeverbFP::writePredelay(float sample) {
    predelayBuffer[predelayWriteIndex % kPredelayBufferSize] = sample;
    predelayWriteIndex++;
}

AudioEffectFreeverbFP::AudioEffectFreeverbFP()
    : AudioStream(1, inputQueueArray),
      combStorage(s_combStorage),
      allpassStorage(s_allpassStorage),
      predelayBuffer(s_predelayBuffer),
      comb1(1116, combStorage + 0),
      comb2(1188, combStorage + 1116),
      comb3(1277, combStorage + 1116 + 1188),
      comb4(1356, combStorage + 1116 + 1188 + 1277),
      comb5(1422, combStorage + 1116 + 1188 + 1277 + 1356),
      comb6(1491, combStorage + 1116 + 1188 + 1277 + 1356 + 1422),
      comb7(1557, combStorage + 1116 + 1188 + 1277 + 1356 + 1422 + 1491),
      comb8(1617, combStorage + 1116 + 1188 + 1277 + 1356 + 1422 + 1491 + 1557),
      allpass1(556, allpassStorage + 0),
      allpass2(441, allpassStorage + 556),
      allpass3(341, allpassStorage + 556 + 441),
      allpass4(225, allpassStorage + 556 + 441 + 341) {
    memset(predelayBuffer, 0, sizeof(float) * kPredelayBufferSize);
    setDecay(0.5f);
    setTone(0.5f);
    setPredelayMs(0.0f);
}

void AudioEffectFreeverbFP::setDecay(float decay01) {
    decay01 = clamp01(decay01);
    // Use a gentle curve so the top end doesn't jump to "infinite cave".
    // sqrt() gives more resolution in the mid range and a softer knee near 1.0.
    const float t = sqrtf(decay01);
    const float feedback = 0.5f + t * 0.48f;
    comb1.setFeedback(feedback);
    comb2.setFeedback(feedback);
    comb3.setFeedback(feedback);
    comb4.setFeedback(feedback);
    comb5.setFeedback(feedback);
    comb6.setFeedback(feedback);
    comb7.setFeedback(feedback);
    comb8.setFeedback(feedback);
}

void AudioEffectFreeverbFP::setTone(float tone01) {
    tone01 = clamp01(tone01);
    const float damp = (1.0f - tone01) * 0.85f;
    comb1.setDamp(damp);
    comb2.setDamp(damp);
    comb3.setDamp(damp);
    comb4.setDamp(damp);
    comb5.setDamp(damp);
    comb6.setDamp(damp);
    comb7.setDamp(damp);
    comb8.setDamp(damp);
}

void AudioEffectFreeverbFP::setPredelayMs(float ms) {
    ms = constrain(ms, 0.0f, 100.0f);
    predelaySamples = (ms * 0.001f) * kSampleRate;
    if (predelaySamples >= (float)(kPredelayBufferSize - 1)) {
        predelaySamples = (float)(kPredelayBufferSize - 2);
    }
}

void AudioEffectFreeverbFP::mute() {
    comb1.clear();
    comb2.clear();
    comb3.clear();
    comb4.clear();
    comb5.clear();
    comb6.clear();
    comb7.clear();
    comb8.clear();
    allpass1.clear();
    allpass2.clear();
    allpass3.clear();
    allpass4.clear();
    memset(predelayBuffer, 0, sizeof(float) * kPredelayBufferSize);
    predelayWriteIndex = 0;
}

void AudioEffectFreeverbFP::update(void) {
    audio_block_t *block = receiveReadOnly(0);
    audio_block_t *outBlock = allocate();
    if (!outBlock) {
        if (block) {
            release(block);
        }
        return;
    }

    if (!block) {
        memset(outBlock->data, 0, sizeof(outBlock->data));
        transmit(outBlock);
        release(outBlock);
        return;
    }

    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i) {
        const float input = (float)block->data[i] * kInputScale;

        writePredelay(input);
        const float reverbInput =
            (predelaySamples > 0.0f) ? readPredelay() : input;

        float wet = 0.0f;
        wet += comb1.process(reverbInput);
        wet += comb2.process(reverbInput);
        wet += comb3.process(reverbInput);
        wet += comb4.process(reverbInput);
        wet += comb5.process(reverbInput);
        wet += comb6.process(reverbInput);
        wet += comb7.process(reverbInput);
        wet += comb8.process(reverbInput);
        wet *= 0.125f;

        wet = allpass1.process(wet);
        wet = allpass2.process(wet);
        wet = allpass3.process(wet);
        wet = allpass4.process(wet);

        wet = flushDenormal(wet * kOutputGain);
        if (wet > 1.0f) {
            wet = 1.0f;
        } else if (wet < -1.0f) {
            wet = -1.0f;
        }

        outBlock->data[i] = (int16_t)(wet * 32767.0f);
    }

    transmit(outBlock);
    release(outBlock);
    release(block);
}
