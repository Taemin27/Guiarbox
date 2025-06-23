#ifndef effect_distortion_h_
#define effect_distortion_h_

#include <Arduino.h>
#include "AudioStream.h"
#include "arm_math.h"

class AudioEffectDistortion : public AudioStream {
public:
    AudioEffectDistortion() : AudioStream(1, inputQueueArray) {
        positiveClip = 1.0f;
        negativeClip = -1.0f;
        gain = 1.0f;
        level = 1.0f;
        smoothing = 0.9f;
        dynamicGainResponse = 2.0f;
        positiveClipResponse = 0.5f;
        negativeClipResponse = 0.5f;
    }

    virtual void update(void);

    void setClip(float positiveClip, float negativeClip) { 
        AudioEffectDistortion::positiveClip = fminf(fmaxf(positiveClip, 0.0f), 1.0f);
        AudioEffectDistortion::negativeClip = fmaxf(fminf(negativeClip, 0.0f), -1.0f);
    }

    void setGain(float gain) {
        AudioEffectDistortion::gain = gain;
    }

    void setLevel(float level) {
        AudioEffectDistortion::level = level;
    }

    void setSmoothing(float percentage) {
        AudioEffectDistortion::smoothing = fminf(fmaxf(percentage, 0.0f), 1.0f);
    }
    void setDynamicGainResponse(float dynamicGainResponse) {
        AudioEffectDistortion::dynamicGainResponse = dynamicGainResponse;
    }

    void setClipResponse(float positiveClipResponse, float negativeClipResponse) {
        AudioEffectDistortion::positiveClipResponse = positiveClipResponse;
        AudioEffectDistortion::negativeClipResponse = negativeClipResponse;
    }


    

private:
    audio_block_t *inputQueueArray[1];
    float positiveClip;
    float negativeClip;
    float gain;
    float level;
    float smoothing;
    float dynamicGainResponse;
    float positiveClipResponse;
    float negativeClipResponse;

    float envelope = 0.0f;

};

#endif
