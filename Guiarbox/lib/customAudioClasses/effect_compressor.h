#ifndef effect_compressor_h_
#define effect_compressor_h_

#include <Arduino.h>
#include <AudioStream.h>

class AudioEffectCompressor : public AudioStream {
public:
    AudioEffectCompressor() : AudioStream(1, inputQueueArray) {
        ratio = 4.0f;
        threshold = 0.5f;
        level = 1.0f;
        envelope = 0.0f;
        setAttackMs(10.0f);
        setReleaseMs(150.0f);
    }

    virtual void update(void);

    void setThreshold(float t) {
        threshold = constrain(t, 0.0f, 1.0f);
    }

    void setRatio(float r) {
        ratio = max(1.0f, r);
    }

    void setAttackMs(float ms);

    void setReleaseMs(float ms);

    void setLevel(float l) {
        level = constrain(l, 0.0f, 2.0f);
    }

    void enable() {
        enabled = true;
    }
    void disable() {
        enabled = false;
    }
    bool isEnabled() const {
        return enabled;
    }

private:
    audio_block_t *inputQueueArray[1];

    bool enabled = false;

    float ratio;
    float threshold;
    float level;
    float envelope;

    float attackAlpha;
    float releaseAlpha;

    static float envelopeCoeffFromMs(float ms, float sampleRate);
};

#endif
