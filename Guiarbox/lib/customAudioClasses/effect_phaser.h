#ifndef effect_phaser_h_
#define effect_phaser_h_

#include <Arduino.h>
#include <AudioStream.h>

/**
 * Multi-stage phaser built from a cascade of 1st-order all-pass filters.
 *
 * Controls:
 * - rate: LFO speed in Hz
 * - depth: modulation amount (0..1)
 * - mix: dry/wet (0..1)
 * - feedback: wet feedback amount (-1..1), internally clamped for stability
 * - stages: number of all-pass stages (1..MAX_STAGES), typically even
 */
class AudioEffectPhaser : public AudioStream {
public:
    static constexpr int MAX_STAGES = 12;

    AudioEffectPhaser();

    void update(void) override;

    void setRate(float hz);
    void setDepth(float depth01);
    void setMix(float mix01);
    void setFeedback(float feedbackAmount);
    void setStages(int count);

    void enable();
    void disable();
    bool isEnabled() const { return enabled; }

private:
    audio_block_t *inputQueueArray[1];

    bool enabled = false;

    float rateHz = 0.6f;
    float depth = 0.75f;
    float mix = 0.5f;
    float feedback = 0.2f;
    int stages = 6;

    float lfoPhase = 0.0f;
    float stageZ1[MAX_STAGES];
    float feedbackState = 0.0f;

    float computeAllpassA(float fcHz, float sampleRate) const;
    float processAllpassCascade(float x, float a);
};

#endif

