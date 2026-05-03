#pragma once
#include "../EffectManager.h"

class ReverbManager : public EffectManager {
private:
    float roomSize = 5.0f;
    float damping = 5.0f;
    float mix = 5.0f;

    EffectParameter params[3] = {
        {"Room Size", ParamType::Float, &roomSize, 0.0f, 10.0f, 0.5f, nullptr, 0, 1},
        {"Damping", ParamType::Float, &damping, 0.0f, 10.0f, 0.5f, nullptr, 0, 1},
        {"Mix", ParamType::Float, &mix, 0.0f, 10.0f, 0.5f, nullptr, 0, 1}
    };

public:
    const char* getName() const override {
        return "Reverb";
    }

    const EffectParameter* getParameters() const override {
        return params;
    }

    int getParameterCount() const override {
        return 3;
    }

    void syncToChain() override {
        if (enabled) {
            freeverb1.roomsize(roomSize/10.0f);
            freeverb1.damping(damping/10.0f);
            reverbAmp.gain(1.0f);
            reverbMixer.gain(1, mix / 10.0f);
            reverbMixer.gain(0, 1.0f - (mix / 10.0f));
        }
        else {
            reverbAmp.gain(0.0f);
            reverbMixer.gain(0, 1.0f);
            reverbMixer.gain(1, 0.0f);
        }
    }

};

