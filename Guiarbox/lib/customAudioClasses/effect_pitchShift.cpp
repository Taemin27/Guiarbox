#include "effect_pitchShift.h"

namespace {

float pitchSmoothingCoefFromMs(float ms) {
    const float sampleRate = 44100.0f;
    ms = max(ms, 0.1f);
    return expf(-1.0f / ((ms / 1000.0f) * sampleRate));
}

}

AudioEffectPitchShift::AudioEffectPitchShift() : AudioStream(1, inputQueueArray) {
    shiftRatio = 1.0f;
    baseShiftRatio = 1.0f;
    mix = 1.0f;

    inputWriteIndex = 0;
    outputReadIndex = 0;
    hopCounter = 0;

    arm_rfft_fast_init_f32(&fftInstance, FFT_SIZE);
    
    for (int i = 0; i < FFT_SIZE; i++) {
        window[i] = 0.5f * (1.0f - cosf(2.0f * PI * i / (FFT_SIZE - 1)));
        inputBuffer[i] = 0.0f;
        outputBuffer[i] = 0.0f;
        fftData[i] = 0.0f;
    }

    for (int i = 0; i <= FFT_SIZE / 2; i++) {
        magnitudes[i] = 0.0f;
        phases[i] = 0.0f;
        synthMagnitudes[i] = 0.0f;
        synthPhases[i] = 0.0f;
        lastInputPhases[i] = 0.0f;
        summedOutputPhases[i] = 0.0f;
    }

    float sampleRate = 44100.0f; // Standard Teensy audio sample rate
    envReleaseCoef = expf(-1.0f / (0.05f * sampleRate));

    pitchAttackCoef = pitchSmoothingCoefFromMs(30.0f);
    pitchReleaseCoef = pitchSmoothingCoefFromMs(30.0f);
}

void AudioEffectPitchShift::setRatio(float ratio) {
    dynamicEnabled = false; 
    shiftRatio = constrain(ratio, 0.25f, 4.0f);
    baseShiftRatio = shiftRatio; 
}

void AudioEffectPitchShift::setDynamicMode(bool enabled) {
    if (dynamicEnabled && !enabled) {
        shiftRatio = baseShiftRatio;
    }
    dynamicEnabled = enabled;
}

void AudioEffectPitchShift::setTargetSemitones(float semitones) {
    semitones = constrain(semitones, -24.0f, 24.0f);
    targetShiftRatio = powf(2.0f, semitones / 12.0f);
}

void AudioEffectPitchShift::setDynamicThreshold(float threshold) {
    dynamicThreshold = constrain(threshold, 0.001f, 1.0f);
}

void AudioEffectPitchShift::setDynamicAttackMs(float attackMs) {
    pitchAttackCoef = pitchSmoothingCoefFromMs(attackMs);
}

void AudioEffectPitchShift::setDynamicReleaseMs(float releaseMs) {
    pitchReleaseCoef = pitchSmoothingCoefFromMs(releaseMs);
}

void AudioEffectPitchShift::setSemitones(float semitones) {
    semitones = constrain(semitones, -24.0f, 24.0f);
    float ratio = powf(2.0f, semitones / 12.0f);
    setRatio(ratio);
}

void AudioEffectPitchShift::setMix(float mixValue) {
    mix = constrain(mixValue, 0.0f, 1.0f);
}

void AudioEffectPitchShift::enable() {
    enabled = true;
}

void AudioEffectPitchShift::disable() {
    enabled = false;
}

bool AudioEffectPitchShift::isEnabled() const {
    return enabled;
}

void AudioEffectPitchShift::update(void) {
    audio_block_t *block = receiveWritable(0);
    if (!block) return;

    if (!enabled) {
        transmit(block);
        release(block);
        return;
    }

    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
        float drySample = (float)block->data[i] / 32768.0f;
        
        if (dynamicEnabled) {
            float absSample = fabsf(drySample);
            
            // Envelope Follower
            if (absSample > envLevel) {
                envLevel = absSample; 
            } else {
                envLevel = envLevel * envReleaseCoef + absSample * (1.0f - envReleaseCoef);
            }

            // Logic Gate
            bool isTriggered = (envLevel > dynamicThreshold);
            float targetRatio = isTriggered ? targetShiftRatio : baseShiftRatio;
            float smoothingCoef = isTriggered ? pitchAttackCoef : pitchReleaseCoef;

            // Slew Limiter
            shiftRatio = shiftRatio * smoothingCoef + targetRatio * (1.0f - smoothingCoef);
        }

        inputBuffer[inputWriteIndex] = drySample;
        inputWriteIndex = (inputWriteIndex + 1) & FFT_MASK;

        float wetSample = outputBuffer[outputReadIndex];
        outputBuffer[outputReadIndex] = 0.0f; 
        outputReadIndex = (outputReadIndex + 1) & FFT_MASK;

        float mixedSample = (drySample * (1.0f - mix)) + (wetSample * mix);

        mixedSample = constrain(mixedSample, -1.0f, 1.0f);
        block->data[i] = (int16_t)(mixedSample * 32767.0f);

        hopCounter++;
        if (hopCounter >= HOP_SIZE) {
            hopCounter = 0;
            processFrame();
        }
    }

    transmit(block);
    release(block);
}

void AudioEffectPitchShift::processFrame() {
    // inputWriteIndex points to the oldest sample in the logical FFT frame
    for (int i = 0; i < FFT_SIZE; i++) {
        int idx = (inputWriteIndex + i) & FFT_MASK;
        fftData[i] = inputBuffer[idx] * window[i];
    }

    arm_rfft_fast_f32(&fftInstance, fftData, fftData, 0);

    const float expectedPhaseAdvance = 2.0f * PI * (float)HOP_SIZE / (float)FFT_SIZE;

    for (int i = 0; i <= FFT_SIZE / 2; i++) {
        synthMagnitudes[i] = 0.0f;
        synthPhases[i] = 0.0f;
    }

    for (int k = 0; k <= FFT_SIZE / 2; k++) {
        float real, imag;

        if (k == 0) {
            real = fftData[0];
            imag = 0.0f;
        } else if (k == FFT_SIZE / 2) {
            real = fftData[1];
            imag = 0.0f;
        } else {
            real = fftData[2 * k];
            imag = fftData[2 * k + 1];
        }

        float mag = sqrtf(real * real + imag * imag);
        float phase = atan2f(imag, real);

        magnitudes[k] = mag;
        phases[k] = phase;

        float phaseDiff = phase - lastInputPhases[k] - ((float)k * expectedPhaseAdvance);
        lastInputPhases[k] = phase;

        phaseDiff -= 2.0f * PI * roundf(phaseDiff / (2.0f * PI));

        float trueAdvance = ((float)k * expectedPhaseAdvance) + phaseDiff;

        float exactTargetBin = (float)k * shiftRatio;
        int targetBin = (int)exactTargetBin;
        float fractional = exactTargetBin - (float)targetBin;

        if (targetBin <= FFT_SIZE / 2) {
            // Calculate the phase only once for the target
            summedOutputPhases[targetBin] += trueAdvance * shiftRatio;
            
            // Fast Wrapping
            summedOutputPhases[targetBin] -= 2.0f * PI * roundf(summedOutputPhases[targetBin] / (2.0f * PI));
            
            // Apply magnitude and phase to the primary bin
            synthMagnitudes[targetBin] += mag * (1.0f - fractional);
            synthPhases[targetBin] = summedOutputPhases[targetBin];

            // Handle the spillover bin
            int nextBin = targetBin + 1;
            if (nextBin <= FFT_SIZE / 2) {
                synthMagnitudes[nextBin] += mag * fractional;
                
                // CRITICAL: Force the next bin to have the EXACT same phase
                // plus a small offset to account for the frequency distance.
                // For a 512/1024 FFT, this "locks" the peak together.
                synthPhases[nextBin] = synthPhases[targetBin];
            }
        }
        
        int nextBin = targetBin + 1;
        if (nextBin <= FFT_SIZE / 2) {
            // 4. Spill magnitude into the neighbor
            synthMagnitudes[nextBin] += mag * fractional;
            
            // 5. Instead of locking phase, we "inherit" it IF the neighbor 
            // doesn't have its own primary phase yet. This reduces "phasiness."
            if (synthPhases[nextBin] == 0.0f) {
                synthPhases[nextBin] = synthPhases[targetBin];
            }
        }
    }

    for (int i = 0; i < FFT_SIZE; i++) {
        fftData[i] = 0.0f;
    }

    const float radToDeg = 180.0f / PI;

    for (int k = 0; k <= FFT_SIZE / 2; k++) {
        float32_t sinVal, cosVal;
        
        // 2. Convert radians to degrees (CMSIS-DSP requirement)
        float32_t phaseDegrees = synthPhases[k] * radToDeg;

        // 3. Compute both Sine and Cosine in a single optimized call
        arm_sin_cos_f32(phaseDegrees, &sinVal, &cosVal);

        // 4. Calculate Real and Imaginary components
        float real = synthMagnitudes[k] * cosVal;
        float imag = synthMagnitudes[k] * sinVal;

        // 5. Pack into the specific CMSIS Real FFT format
        if (k == 0) {
            fftData[0] = real; // DC component
        } else if (k == FFT_SIZE / 2) {
            fftData[1] = real; // Nyquist component (Real-only)
        } else {
            fftData[2 * k]     = real; // Interleaved Real
            fftData[2 * k + 1] = imag; // Interleaved Imaginary
        }
    }

    for (int i = 0; i <= FFT_SIZE / 2; i++) {
        if (synthMagnitudes[i] < 1e-10f) synthMagnitudes[i] = 0.0f;
    }

    arm_rfft_fast_f32(&fftInstance, fftData, fftData, 1);

    const float olaGain = 0.6666667f;

    for (int i = 0; i < FFT_SIZE; i++) {
        int idx = (outputReadIndex + i) & FFT_MASK;
        outputBuffer[idx] += fftData[i] * window[i] * olaGain;
    }
}