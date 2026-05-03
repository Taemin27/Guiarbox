#pragma once
#include "../EffectManager.h"

extern AudioEffectOverdrive overdrive;

class OverdriveManager : public EffectManager {
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
        return "Overdrive";
    }

    const EffectParameter* getParameters() const override {
        return params;
    }

    int getParameterCount() const override {
        return 3;
    }

    void syncToChain() override {
        if (enabled) {
            overdrive.setDrive(drive/10.0f);
            overdrive.setLevel(level/10.0f);
            overdrive.setTone(tone/10.0f);

            overdrive.enable();
        }
        else {
           overdrive.disable();
        }
    }
};
