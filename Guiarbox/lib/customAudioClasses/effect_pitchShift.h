#ifndef effect_pitchShift_h_
#define effect_pitchShift_h_

#include <Arduino.h>
#include "AudioStream.h"
#include "arm_math.h"
#include "arm_const_structs.h"

#define FFT_SIZE 1024
#define HOP_SIZE 256
#define OVERSAMP (FFT_SIZE / HOP_SIZE)
#define FFT_MASK (FFT_SIZE - 1)

class AudioEffectPitchShift : public AudioStream {
public:
    AudioEffectPitchShift();
    virtual void update(void);

    void setRatio(float ratio);
    void setSemitones(float semitones);
    void setMix(float mix);

    void setDynamicMode(bool enabled);

    void setTargetSemitones(float semitones);
    void setDynamicThreshold(float threshold);
    void setDynamicAttackMs(float attackMs);
    void setDynamicReleaseMs(float releaseMs);

    void enable();
    void disable();
    bool isEnabled() const;

private:
    audio_block_t *inputQueueArray[1];
    bool enabled = false;

    float shiftRatio;
    float mix;

    float inputBuffer[FFT_SIZE];
    int inputWriteIndex;

    float outputBuffer[FFT_SIZE];
    int outputReadIndex;

    float window[FFT_SIZE];

    float fftData[FFT_SIZE];
    float magnitudes[FFT_SIZE / 2 + 1];
    float phases[FFT_SIZE / 2 + 1];
    float synthMagnitudes[FFT_SIZE / 2 + 1];
    float synthPhases[FFT_SIZE / 2 + 1];

    float lastInputPhases[FFT_SIZE / 2 + 1];
    float summedOutputPhases[FFT_SIZE / 2 + 1];

    
    int hopCounter;

    arm_rfft_fast_instance_f32 fftInstance;

    void processFrame();

    // Dynamic Pitch variables
    bool dynamicEnabled = false;
    
    float baseShiftRatio = 1.0f;     // The resting pitch ratio
    float targetShiftRatio = 1.0f;   // The pitch ratio when triggered
    float dynamicThreshold = 1.0f;   // Linear peak amplitude (0…1), vs envelope follower
    
    float pitchAttackCoef = 0.0f;    // Smoothing coefficient for attack
    float pitchReleaseCoef = 0.0f;   // Smoothing coefficient for release
    
    float envLevel = 0.0f;           // Current state of the envelope follower
    float envReleaseCoef;            // Smoothing for the envelope tracker itself
};

#endif