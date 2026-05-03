#pragma once
#include "../EffectManager.h"
#include "../../../../lib/customAudioClasses/effect_autoWah.h"

extern AudioEffectAutoWah autoWah;

class AutoWahManager : public EffectManager {
private:
    int mode = 0;
    float sensitivity = 5.0f;
    int attackMs = 10;
    int releaseMs = 10;
    float lowFreq = 0.3f;
    float highFreq = 2.0f;
    float q = 3.0f;

    static constexpr const char* const MODE_OPTIONS[] = {"Up", "Down", "UpDown"};

    EffectParameter params[7] = {
        {"Mode", ParamType::Option, &mode, 0, 2, 1, MODE_OPTIONS, 3},
        {"Sens", ParamType::Float, &sensitivity, 0.0f, 10.0f, 0.5f, nullptr, 0, 1},
        {"Attack", ParamType::Int, &attackMs, 0, 500, 5, nullptr, 0, 2},
        {"Release", ParamType::Int, &releaseMs, 0, 500, 5, nullptr, 0, 2},
        {"Low(kHz)", ParamType::Float, &lowFreq, 0.0f, 0.5f, 0.05f, nullptr, 0, 2},
        {"High(kHz)", ParamType::Float, &highFreq, 0.8f, 2.5f, 0.05f, nullptr, 0, 2},
        {"Q", ParamType::Float, &q, 0.3f, 20.0f, 0.1f, nullptr, 0, 1}
    };

public:
    const char* getName() const override {
        return "Auto Wah";
    }

    const EffectParameter* getParameters() const override {
        return params;
    }

    int getParameterCount() const override {
        return 7;
    }

    void syncToChain() override {
        if (enabled) {
            autoWah.setDirection(static_cast<AudioEffectAutoWah::Direction>(mode));
            autoWah.setSensitivity(sensitivity / 10.0f);
            autoWah.setAttackMs((float)attackMs);
            autoWah.setReleaseMs((float)releaseMs);
            autoWah.setLowFreq(lowFreq * 1000.0f);
            autoWah.setHighFreq(highFreq * 1000.0f);
            autoWah.setQ(q);
            autoWah.enable();
        }
        else {
            autoWah.disable();
        }
    }
};