#pragma once
#include "../EffectManager.h"
#include "../../../../lib/customAudioClasses/effect_pitchShift.h"

extern AudioEffectPitchShift pitchShifter;

class PitchShifterManager : public EffectManager {
private:
    float pitch = 0.0f;
    float mix = 1.0f;

    EffectParameter params[2] = {
        {"Pitch", ParamType::Float, &pitch, -24.0f, 24.0f, 0.5f, nullptr, 0, 1},
        {"Mix",   ParamType::Float, &mix, 0.0f, 1.0f, 0.1f, nullptr, 0, 1},
    };

public:
    const char* getName() const override {
        return "Pitch Shift";
    }

    const EffectParameter* getParameters() const override {
        return params;
    }

    int getParameterCount() const override {
        return 2;
    }

    void syncToChain() override {
        if (enabled) {
            pitchShifter.setSemitones(pitch);
            pitchShifter.setMix(mix);
            pitchShifter.enable();
        } else {
            pitchShifter.disable();
        }
    }
};
