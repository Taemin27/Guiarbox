#include "effect_autoWham.h"
#include <math.h>

namespace {

constexpr float DEFAULT_SAMPLE_RATE = 44100.0f;
constexpr float DEFAULT_GRAIN_MS = 48.0f;
constexpr float ENV_SETTLE_MS = 50.0f;
constexpr float LN_SETTLE_RATIO = 2.99573227355f;

float onePoleCoefFromSettleMs(float settleMs) {
    if (settleMs <= 0.0f) {
        return 0.0f;
    }
    return expf(-LN_SETTLE_RATIO / (settleMs * 0.001f * DEFAULT_SAMPLE_RATE));
}

int grainSamplesFromMs(float ms) {
    const int g = (int)lroundf(ms * DEFAULT_SAMPLE_RATE / 1000.0f);
    return constrain(g, 64, 4096);
}

} // namespace

AudioEffectAutoWham::AudioEffectAutoWham() : AudioStream(1, inputQueueArray) {
    targetShiftRatio = 2.0f;
    shiftRatio = 1.0f;
    grainSamples = grainSamplesFromMs(DEFAULT_GRAIN_MS);

    for (uint32_t i = 0; i < BUFFER_SIZE; ++i) {
        buffer[i] = 0.0f;
    }

    envReleaseCoef = onePoleCoefFromSettleMs(ENV_SETTLE_MS);
    pitchAttackCoef = onePoleCoefFromSettleMs(30.0f);
    pitchReleaseCoef = onePoleCoefFromSettleMs(30.0f);
}

void AudioEffectAutoWham::setTargetSemitones(float semitones) {
    semitones = constrain(semitones, -24.0f, 24.0f);
    targetShiftRatio = powf(2.0f, semitones / 12.0f);
}

void AudioEffectAutoWham::setThreshold(float threshold) {
    dynamicThreshold = constrain(threshold, 0.001f, 1.0f);
}

void AudioEffectAutoWham::setAttackMs(float attackMs) {
    pitchAttackCoef = onePoleCoefFromSettleMs(max(attackMs, 0.0f));
}

void AudioEffectAutoWham::setReleaseMs(float releaseMs) {
    pitchReleaseCoef = onePoleCoefFromSettleMs(max(releaseMs, 0.0f));
}

void AudioEffectAutoWham::enable() {
    enabled = true;
}

void AudioEffectAutoWham::disable() {
    enabled = false;
}

bool AudioEffectAutoWham::isEnabled() const {
    return enabled;
}

float AudioEffectAutoWham::readHermite(double readPos) const {
    const int64_t i0 = (int64_t)floor(readPos);
    const float f = (float)(readPos - (double)i0);

    const float xm1 = buffer[(size_t)((i0 - 1) & BUFFER_MASK)];
    const float x0 = buffer[(size_t)(i0 & BUFFER_MASK)];
    const float x1 = buffer[(size_t)((i0 + 1) & BUFFER_MASK)];
    const float x2 = buffer[(size_t)((i0 + 2) & BUFFER_MASK)];

    const float c = (x1 - xm1) * 0.5f;
    const float v = x0 - x1;
    const float w = c + v;
    const float a = w + v + (x2 - x0) * 0.5f;
    const float b = w + a;
    return ((a * f - b) * f + c) * f + x0;
}

float AudioEffectAutoWham::processSample(float drySample) {
    buffer[(size_t)(writePos & BUFFER_MASK)] = drySample;

    float absSample = fabsf(drySample);

    if (absSample > envLevel) {
        envLevel = absSample;
    } else {
        envLevel = envLevel * envReleaseCoef + absSample * (1.0f - envReleaseCoef);
    }

    bool isTriggered = (envLevel > dynamicThreshold);
    float targetRatio = isTriggered ? targetShiftRatio : 1.0f;
    float smoothingCoef = isTriggered ? pitchAttackCoef : pitchReleaseCoef;
    shiftRatio = shiftRatio * smoothingCoef + targetRatio * (1.0f - smoothingCoef);

    const double ratio = (double)shiftRatio;
    const double grain = (double)grainSamples;
    const double inc = (1.0 - ratio) / grain;
    const double twoPi = 2.0 * PI;

    double ph1 = phase;
    double ph2 = phase + 0.5;
    if (ph2 >= 1.0) {
        ph2 -= 1.0;
    }

    const double d1 = (double)DELAY_BASE + ph1 * grain;
    const double d2 = (double)DELAY_BASE + ph2 * grain;

    const float s1 = readHermite((double)writePos - d1);
    const float s2 = readHermite((double)writePos - d2);

    const float w1 = 0.5f * (1.0f - (float)cos(twoPi * ph1));
    const float w2 = 0.5f * (1.0f - (float)cos(twoPi * ph2));

    ++writePos;
    phase += inc;
    if (phase >= 1.0) {
        phase -= 1.0;
    } else if (phase < 0.0) {
        phase += 1.0;
    }

    return w1 * s1 + w2 * s2;
}

void AudioEffectAutoWham::update(void) {
    audio_block_t *block = receiveWritable(0);
    if (!block) {
        return;
    }

    if (!enabled) {
        transmit(block);
        release(block);
        return;
    }

    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
        float drySample = (float)block->data[i] / 32768.0f;
        float outSample = processSample(drySample);

        outSample = constrain(outSample, -1.0f, 1.0f);
        block->data[i] = (int16_t)(outSample * 32767.0f);
    }

    transmit(block);
    release(block);
}
