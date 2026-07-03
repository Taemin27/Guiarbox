#pragma once
#include "../EffectManager.h"
#include "../../../../lib/customAudioClasses/effect_transpose.h"

extern AudioEffectTranspose transpose;

class TransposeManager : public EffectManager {
private:
    float pitch = 0.0f;
    float mix = 1.0f;

    EffectParameter params[2] = {
        {"Pitch", ParamType::Float, &pitch, -24.0f, 24.0f, 0.5f, nullptr, 0, 1},
        {"Mix",   ParamType::Float, &mix, 0.0f, 1.0f, 0.1f, nullptr, 0, 1},
    };

public:
    const char* getName() const override {
        return "Transpose";
    }

    const EffectParameter* getParameters() const override {
        return params;
    }

    int getParameterCount() const override {
        return 2;
    }

    void syncToChain() override {
        if (enabled) {
            transpose.setSemitones(pitch);
            transpose.setMix(mix);
            transpose.enable();
        } else {
            transpose.disable();
        }
    }
};
