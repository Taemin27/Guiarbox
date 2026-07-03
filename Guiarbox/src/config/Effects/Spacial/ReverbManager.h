#pragma once
#include "../EffectManager.h"
#include "../../../../lib/customAudioClasses/effect_freeverb_fp.h"

extern AudioEffectFreeverbFP freeverbFP;

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
            freeverbFP.setDecay(decay / 10.0f);
            freeverbFP.setTone(tone / 10.0f);
            freeverbFP.setPredelayMs((float)predelayMs);
            freeverbFP.setDryLevel(dry / 10.0f);
            freeverbFP.setWetLevel(wet / 10.0f);
            freeverbFP.enable();
        } else {
            freeverbFP.disable();
        }
    }
};
