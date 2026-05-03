#pragma once
#include "../EffectManager.h"
#include "../../../../lib/customAudioClasses/effect_multiChorus.h"

extern AudioEffectMultiChorus chorus;

class ChorusManager : public EffectManager {
private:
    float rateHz = 0.3f;
    float depth = 5.0f;
    float wetLevel = 5.0f;
    int voices = 2;

    EffectParameter params[4] = {
        {"Rate(Hz)", ParamType::Float, &rateHz, 0.0f, 20.0f, 0.1f, nullptr, 0, 1},
        {"Depth", ParamType::Float, &depth, 0.0f, 10.0f, 0.5f, nullptr, 0, 1},
        {"Wet", ParamType::Float, &wetLevel, 0.0f, 10.0f, 0.5f, nullptr, 0, 1},
        {"Voices", ParamType::Int, &voices, 1, 5, 1, nullptr, 0, 1}
    };

public:
    const char* getName() const override {
        return "Chorus";
    }

    const EffectParameter* getParameters() const override {
        return params;
    }

    int getParameterCount() const override {
        return 4;
    }

    void syncToChain() override {
        if (enabled) {
            chorus.enable();
            chorus.setRate(rateHz);
            chorus.setDepth(depth / 10.0f);
            chorus.setWetLevel(wetLevel / 10.0f);
            chorus.setVoices(voices);
        } else {
            chorus.disable();
        }
    }
};
