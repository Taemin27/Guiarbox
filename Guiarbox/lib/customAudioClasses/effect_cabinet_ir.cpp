#include "effect_cabinet_ir.h"
#include <arm_math.h>
#include <string.h>
#include <math.h>

DMAMEM static float s_irFreq[AudioEffectCabinetIR::MAX_PARTITIONS][AudioEffectCabinetIR::FFT_SIZE];
DMAMEM static float s_inputFreqRing[AudioEffectCabinetIR::MAX_PARTITIONS][AudioEffectCabinetIR::FFT_SIZE];
DMAMEM static float s_overlap[AudioEffectCabinetIR::PARTITION_SIZE];
DMAMEM static float s_workA[AudioEffectCabinetIR::FFT_SIZE];
DMAMEM static float s_fftAccum[AudioEffectCabinetIR::FFT_SIZE];
DMAMEM static float s_loadBuffer[AudioEffectCabinetIR::MAX_IR_SAMPLES];

static_assert(AudioEffectCabinetIR::PARTITION_SIZE == AUDIO_BLOCK_SAMPLES,
              "Cabinet IR partition size must match the audio block size");

static arm_rfft_fast_instance_f32 s_fft;
static bool s_fftReady = false;

namespace {

constexpr float INPUT_SCALE = 1.0f / 32768.0f;
constexpr float OUTPUT_SCALE = 32767.0f;

void ensureFftReady() {
    if (!s_fftReady) {
        arm_rfft_fast_init_f32(&s_fft, AudioEffectCabinetIR::FFT_SIZE);
        s_fftReady = true;
    }
}

void spectralMultiplyAccumulate(float* accum, const float* a, const float* b, int n) {
    accum[0] += a[0] * b[0];
    accum[1] += a[1] * b[1];
    for (int i = 2; i < n; i += 2) {
        const float ar = a[i];
        const float ai = a[i + 1];
        const float br = b[i];
        const float bi = b[i + 1];
        accum[i]     += ar * br - ai * bi;
        accum[i + 1] += ar * bi + ai * br;
    }
}

constexpr float EQ_MAX_DB = 12.0f;
constexpr float BASS_HZ = 120.0f;
constexpr float MID_HZ = 750.0f;
constexpr float TREBLE_HZ = 4000.0f;
constexpr float MID_Q = 1.0f;

float knobToDb(float amount) {
    return (amount - 0.5f) * 2.0f * EQ_MAX_DB;
}

void assignBiquad(float b[3], float a[2],
                  float bb0, float bb1, float bb2,
                  float aa0, float aa1, float aa2) {
    const float invA0 = 1.0f / aa0;
    b[0] = bb0 * invA0;
    b[1] = bb1 * invA0;
    b[2] = bb2 * invA0;
    a[0] = aa1 * invA0;
    a[1] = aa2 * invA0;
}

void setBiquadIdentity(float b[3], float a[2]) {
    b[0] = 1.0f;
    b[1] = 0.0f;
    b[2] = 0.0f;
    a[0] = 0.0f;
    a[1] = 0.0f;
}

void setLowShelf(float b[3], float a[2], float fs, float f0, float dBgain) {
    if (fabsf(dBgain) < 0.01f) {
        setBiquadIdentity(b, a);
        return;
    }

    const float A = powf(10.0f, dBgain / 40.0f);
    const float w0 = TWO_PI * f0 / fs;
    const float cosw = cosf(w0);
    const float sinw = sinf(w0);
    const float alpha = (sinw / 2.0f) * sqrtf(2.0f);
    const float twoSqrtAalpha = 2.0f * sqrtf(A) * alpha;

    assignBiquad(b, a,
        A * ((A + 1.0f) - (A - 1.0f) * cosw + twoSqrtAalpha),
        2.0f * A * ((A - 1.0f) - (A + 1.0f) * cosw),
        A * ((A + 1.0f) - (A - 1.0f) * cosw - twoSqrtAalpha),
        (A + 1.0f) + (A - 1.0f) * cosw + twoSqrtAalpha,
        -2.0f * ((A - 1.0f) + (A + 1.0f) * cosw),
        (A + 1.0f) + (A - 1.0f) * cosw - twoSqrtAalpha);
}

void setPeaking(float b[3], float a[2], float fs, float f0, float dBgain, float Q) {
    if (fabsf(dBgain) < 0.01f) {
        setBiquadIdentity(b, a);
        return;
    }

    const float A = powf(10.0f, dBgain / 40.0f);
    const float w0 = TWO_PI * f0 / fs;
    const float cosw = cosf(w0);
    const float sinw = sinf(w0);
    const float alpha = sinw / (2.0f * Q);

    assignBiquad(b, a,
        1.0f + alpha * A,
        -2.0f * cosw,
        1.0f - alpha * A,
        1.0f + alpha / A,
        -2.0f * cosw,
        1.0f - alpha / A);
}

void setHighShelf(float b[3], float a[2], float fs, float f0, float dBgain) {
    if (fabsf(dBgain) < 0.01f) {
        setBiquadIdentity(b, a);
        return;
    }

    const float A = powf(10.0f, dBgain / 40.0f);
    const float w0 = TWO_PI * f0 / fs;
    const float cosw = cosf(w0);
    const float sinw = sinf(w0);
    const float alpha = (sinw / 2.0f) * sqrtf(2.0f);
    const float twoSqrtAalpha = 2.0f * sqrtf(A) * alpha;

    assignBiquad(b, a,
        A * ((A + 1.0f) + (A - 1.0f) * cosw + twoSqrtAalpha),
        -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cosw),
        A * ((A + 1.0f) + (A - 1.0f) * cosw - twoSqrtAalpha),
        (A + 1.0f) - (A - 1.0f) * cosw + twoSqrtAalpha,
        2.0f * ((A - 1.0f) - (A + 1.0f) * cosw),
        (A + 1.0f) - (A - 1.0f) * cosw - twoSqrtAalpha);
}

} // namespace

float* AudioEffectCabinetIR::loadScratchBuffer() {
    return s_loadBuffer;
}

int AudioEffectCabinetIR::loadScratchBufferSamples() {
    return MAX_IR_SAMPLES;
}

AudioEffectCabinetIR::AudioEffectCabinetIR()
    : AudioStream(1, inputQueueArray),
      irFreq(s_irFreq),
      inputFreqRing(s_inputFreqRing),
      overlap(s_overlap),
      workA(s_workA) {
    ensureFftReady();
    clearImpulse();
    updateEqCoeffs();
}

void AudioEffectCabinetIR::resetProcessingState() {
    ringIndex = 0;
    memset(inputFreqRing, 0, sizeof(float) * MAX_PARTITIONS * FFT_SIZE);
    memset(overlap, 0, sizeof(float) * PARTITION_SIZE);
    memset(eqZ, 0, sizeof(eqZ));
}

void AudioEffectCabinetIR::clearImpulse() {
    irLength = 0;
    numPartitions = 0;
    memset(irFreq, 0, sizeof(float) * MAX_PARTITIONS * FFT_SIZE);
    resetProcessingState();
}

void AudioEffectCabinetIR::rebuildIrPartitions(const float* samples, int length) {
    ensureFftReady();
    memset(irFreq, 0, sizeof(float) * MAX_PARTITIONS * FFT_SIZE);

    numPartitions = (length + PARTITION_SIZE - 1) / PARTITION_SIZE;
    if (numPartitions > MAX_PARTITIONS) {
        numPartitions = MAX_PARTITIONS;
    }

    for (int p = 0; p < numPartitions; ++p) {
        memset(workA, 0, sizeof(float) * FFT_SIZE);
        const int start = p * PARTITION_SIZE;
        const int copyLen = min(PARTITION_SIZE, length - start);
        if (copyLen > 0) {
            memcpy(workA, samples + start, sizeof(float) * (size_t)copyLen);
        }
        arm_rfft_fast_f32(&s_fft, workA, irFreq[p], 0);
    }
}

bool AudioEffectCabinetIR::loadImpulse(const float* samples, int length) {
    disable();

    if (!samples || length <= 0) {
        clearImpulse();
        return false;
    }

    if (length > MAX_IR_SAMPLES) {
        length = MAX_IR_SAMPLES;
    }

    float peak = 0.0f;
    for (int i = 0; i < length; ++i) {
        const float a = fabsf(samples[i]);
        if (a > peak) {
            peak = a;
        }
    }

    const float norm = (peak > 1.0e-9f) ? (1.0f / peak) : 1.0f;
    for (int i = 0; i < length; ++i) {
        s_loadBuffer[i] = samples[i] * norm;
    }

    irLength = length;
    rebuildIrPartitions(s_loadBuffer, length);
    resetProcessingState();
    return true;
}

void AudioEffectCabinetIR::enable() {
    if (!enabled) {
        resetProcessingState();
    }
    enabled = true;
}

void AudioEffectCabinetIR::disable() {
    enabled = false;
}

void AudioEffectCabinetIR::setLevel(float level) {
    if (level < 0.0f) {
        level = 0.0f;
    } else if (level > 1.0f) {
        level = 1.0f;
    }
    levelGain = level;
}

void AudioEffectCabinetIR::setBass(float amount) {
    bassAmount = constrain(amount, 0.0f, 1.0f);
    updateEqCoeffs();
}

void AudioEffectCabinetIR::setMid(float amount) {
    midAmount = constrain(amount, 0.0f, 1.0f);
    updateEqCoeffs();
}

void AudioEffectCabinetIR::setTreble(float amount) {
    trebleAmount = constrain(amount, 0.0f, 1.0f);
    updateEqCoeffs();
}

void AudioEffectCabinetIR::updateEqCoeffs() {
    const float fs = AUDIO_SAMPLE_RATE_EXACT;
    const float bassDb = knobToDb(bassAmount);
    const float midDb = knobToDb(midAmount);
    const float trebleDb = knobToDb(trebleAmount);

    eqBypass = (fabsf(bassDb) < 0.01f)
            && (fabsf(midDb) < 0.01f)
            && (fabsf(trebleDb) < 0.01f);

    setLowShelf(eqB[0], eqA[0], fs, BASS_HZ, bassDb);
    setPeaking(eqB[1], eqA[1], fs, MID_HZ, midDb, MID_Q);
    setHighShelf(eqB[2], eqA[2], fs, TREBLE_HZ, trebleDb);
}

float AudioEffectCabinetIR::processEq(float x) const {
    for (int band = 0; band < 3; ++band) {
        const float x1 = eqZ[band][0];
        const float x2 = eqZ[band][1];
        const float y1 = eqZ[band][2];
        const float y2 = eqZ[band][3];
        const float y = eqB[band][0] * x
                      + eqB[band][1] * x1
                      + eqB[band][2] * x2
                      - eqA[band][0] * y1
                      - eqA[band][1] * y2;
        eqZ[band][0] = x;
        eqZ[band][1] = x1;
        eqZ[band][2] = y;
        eqZ[band][3] = y1;
        x = y;
    }
    return x;
}

void AudioEffectCabinetIR::update(void) {
    audio_block_t *block = receiveWritable(0);
    if (!block) {
        return;
    }

    if (!enabled || irLength <= 0 || numPartitions <= 0) {
        transmit(block);
        release(block);
        return;
    }

    ensureFftReady();

    memset(workA, 0, sizeof(float) * FFT_SIZE);
    for (int i = 0; i < PARTITION_SIZE; ++i) {
        workA[i] = (float)block->data[i] * INPUT_SCALE;
    }

    arm_rfft_fast_f32(&s_fft, workA, inputFreqRing[ringIndex], 0);

    memset(s_fftAccum, 0, sizeof(float) * FFT_SIZE);
    for (int p = 0; p < numPartitions; ++p) {
        int slot = ringIndex - p;
        if (slot < 0) {
            slot += numPartitions;
        }
        spectralMultiplyAccumulate(s_fftAccum, inputFreqRing[slot], irFreq[p], FFT_SIZE);
    }

    // arm_rfft_fast_f32 inverse already applies 1/fftLen scaling.
    arm_rfft_fast_f32(&s_fft, s_fftAccum, workA, 1);

    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i) {
        float out = workA[i] + overlap[i];
        if (!eqBypass) {
            out = processEq(out);
        }
        out = constrain(out * levelGain, -1.0f, 1.0f);
        block->data[i] = (int16_t)(out * OUTPUT_SCALE);
    }

    memcpy(overlap, &workA[PARTITION_SIZE], sizeof(float) * PARTITION_SIZE);

    ringIndex++;
    if (ringIndex >= numPartitions) {
        ringIndex = 0;
    }

    transmit(block);
    release(block);
}
