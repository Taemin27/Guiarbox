#ifndef effect_noiseGate_h_
#define effect_noiseGate_h_

#include <Arduino.h>
#include <AudioStream.h>

/**
 * Noise gate effect (level-dependent attenuation).
 *
 * Controls:
 * - threshold: linear amplitude threshold (0..1), same convention as compressor/pitch shift
 * - attackMs: time to open (ms)
 * - releaseMs: time to close (ms)
 * - holdMs: time to stay open after falling below threshold (ms)
 * - range: amount of reduction when closed (0..1). 0=no reduction, 1=mute when closed
 */
class AudioEffectNoiseGate : public AudioStream {
public:
    AudioEffectNoiseGate();

    void update(void) override;

    void setThreshold(float t);
    void setAttackMs(float ms);
    void setReleaseMs(float ms);
    void setHoldMs(float ms);
    void setRange(float range01);

    void enable();
    void disable();
    bool isEnabled() const { return enabled; }

private:
    audio_block_t *inputQueueArray[1];

    bool enabled = false;

    // Parameters
    float threshold = 0.10f;    // linear amplitude 0..1
    float attackAlpha = 1.0f;   // smoothing coefficient for opening
    float releaseAlpha = 1.0f;  // smoothing coefficient for closing
    uint32_t holdSamples = 0;   // hold time in samples
    float range = 1.0f;         // 0..1 (1=mute when closed)

    // State
    float env = 0.0f;           // detector envelope (linear 0..1)
    float gain = 1.0f;          // current applied gain
    uint32_t holdCounter = 0;   // remaining samples in hold

    // Detector smoothing (fixed to avoid chatter)
    float detectorReleaseAlpha = 0.0f;

    static float coeffFromMs(float ms, float sampleRate);
};

#endif

