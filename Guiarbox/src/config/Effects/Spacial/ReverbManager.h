#pragma once
#include "../EffectManager.h"

class ReverbManager : public EffectManager {
private:
    float decay = 5.0f;
    float tone = 5.0f;
    int predelayMs = 20;
    float dry = 10.0f;
    float wet = 5.0f;

    EffectParameter params[5] = {
        {"Decay", ParamType::Float, &decay, 0.0f, 10.0f, 0.5f, nullptr, 0, 1},
        {"Pre(ms)", ParamType::Int, &predelayMs, 0, 100, 5},
        {"Dry", ParamType::Float, &dry, 0.0f, 10.0f, 0.5f, nullptr, 0, 1},
        {"Wet", ParamType::Float, &wet, 0.0f, 10.0f, 0.5f, nullptr, 0, 1},
        {"Tone", ParamType::Float, &tone, 0.0f, 10.0f, 0.5f, nullptr, 0, 1}
    };

public:
    const char* getName() const override {
        return "Reverb";
    }

    const EffectParameter* getParameters() const override {
        return params;
    }

    int getParameterCount() const override {
        return 5;
    }

    void syncToChain() override {
        if (enabled) {
            freeverb1.setDecay(decay / 10.0f);
            freeverb1.setTone(tone / 10.0f);
            freeverb1.setPredelayMs((float)predelayMs);
            reverbAmp.gain(1.0f);
            reverbMixer.gain(0, dry / 10.0f);
            reverbMixer.gain(1, wet / 10.0f);
        } else {
            reverbAmp.gain(0.0f);
            reverbMixer.gain(0, 1.0f);
            reverbMixer.gain(1, 0.0f);
            freeverb1.setDecay(0.0f);
            freeverb1.setTone(0.0f);
            freeverb1.setPredelayMs(0.0f);
            freeverb1.mute();
        }
    }

};
