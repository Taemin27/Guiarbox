#include "effect_cabinet_ir.h"
#include <arm_math.h>
#include <string.h>

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
}

void AudioEffectCabinetIR::resetProcessingState() {
    ringIndex = 0;
    memset(inputFreqRing, 0, sizeof(float) * MAX_PARTITIONS * FFT_SIZE);
    memset(overlap, 0, sizeof(float) * PARTITION_SIZE);
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
        const float out = constrain((workA[i] + overlap[i]) * levelGain, -1.0f, 1.0f);
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
