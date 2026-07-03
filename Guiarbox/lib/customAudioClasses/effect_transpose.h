#ifndef effect_transpose_h_
#define effect_transpose_h_

#include <Arduino.h>
#include "AudioStream.h"

class AudioEffectTranspose : public AudioStream {
public:
    AudioEffectTranspose();
    virtual void update(void);

    void setSemitones(float semitones);
    void setMix(float mix);

    void enable();
    void disable();
    bool isEnabled() const;

private:
    static constexpr uint32_t BUFFER_SIZE = 8192;
    static constexpr uint32_t BUFFER_MASK = BUFFER_SIZE - 1;
    static constexpr int DELAY_BASE = 4;
    static constexpr float UNITY_RATIO_EPS = 1e-4f;

    audio_block_t *inputQueueArray[1];
    bool enabled = false;

    float shiftRatio = 1.0f;
    float mix = 1.0f;

    float buffer[BUFFER_SIZE];
    int64_t writePos = 0;
    double phase = 0.0;
    int grainSamples = 0;

    float processSample(float drySample);
    float readHermite(double readPos) const;
};

#endif
