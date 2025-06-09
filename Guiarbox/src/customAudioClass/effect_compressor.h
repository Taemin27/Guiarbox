#ifndef effect_compressor_h_
#define effect_compressor_h_

#include <Arduino.h>
#include <AudioStream.h>

class AudioEffectCompressor : public AudioStream {
public:
    AudioEffectCompressor() : AudioStream(1, inputQueueArray) {
        ratio = 4.0f;
        releaseCoeff = 0.01f;
        threshold = 0.5f;
        level = 1.0f;
        envelope = 0.0f;
    }

    virtual void update(void);

    void setThreshold(float t) {
        threshold = constrain(t, 0.0f, 1.0f);
    }

    void setRatio(float r) {
        ratio = max(1.0f, r);
    }

    void setRelease(float r) {
        releaseCoeff = constrain(r, 0.0f, 1.0f);
    }

    void setLevel(float l) {
        level = constrain(l, 0.0f, 2.0f);
    }

private:
    audio_block_t *inputQueueArray[1];
    float ratio;
    float releaseCoeff;
    float threshold;
    float level;
    float envelope;
};

#endif
