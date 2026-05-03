#pragma once
#include "../EffectManager.h"

extern AudioEffectDistortion distortion;

class DistortionManager : public EffectManager {
private:
    float level = 5.0f;
    float drive = 5.0f;
    float tone = 5.0f;

    EffectParameter params[3] = {
        {"Level", ParamType::Float, &level, 0.0f, 10.0f, 0.5f, nullptr, 0, 1},
        {"Drive", ParamType::Float, &drive, 0.0f, 10.0f, 0.5f, nullptr, 0, 1},
        {"Tone", ParamType::Float, &tone, 0.0f, 10.0f, 0.5f, nullptr, 0, 1}
    };

public:
    const char* getName() const override {
        return "Distortion";
    }

    const EffectParameter* getParameters() const override {
        return params;
    }

    int getParameterCount() const override {
        return 3;
    }

    void syncToChain() override {
        if(enabled) {
            distortion.setDrive(drive/10.0f);
            distortion.setLevel(level/10.0f);
            distortion.setTone(tone/10.0f);
            
            distortion.enable();
        }
        else {
            distortion.disable();
        }
    }
};