#ifndef effect_autoWham_h_
#define effect_autoWham_h_

#include <Arduino.h>
#include "AudioStream.h"

class AudioEffectAutoWham : public AudioStream {
public:
    AudioEffectAutoWham();
    virtual void update(void);

    void setTargetSemitones(float semitones);
    void setThreshold(float threshold);
    void setAttackMs(float attackMs);
    void setReleaseMs(float releaseMs);

    void enable();
    void disable();
    bool isEnabled() const;

private:
    static constexpr uint32_t BUFFER_SIZE = 8192;
    static constexpr uint32_t BUFFER_MASK = BUFFER_SIZE - 1;
    static constexpr int DELAY_BASE = 4;

    audio_block_t *inputQueueArray[1];
    bool enabled = false;

    float shiftRatio = 1.0f;
    float targetShiftRatio = 1.0f;
    float dynamicThreshold = 1.0f;

    float pitchAttackCoef = 0.0f;
    float pitchReleaseCoef = 0.0f;
    float envLevel = 0.0f;
    float envReleaseCoef = 0.0f;

    float buffer[BUFFER_SIZE];
    int64_t writePos = 0;
    double phase = 0.0;
    int grainSamples = 0;

    float processSample(float drySample);
    float readHermite(double readPos) const;
};

#endif
