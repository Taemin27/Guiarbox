#pragma once
#include "../EffectManager.h"
#include "../../../../lib/customAudioClasses/effect_wah.h"

extern AudioEffectWah wah;

class WahManager : public EffectManager {
private:
    float position = 5.0f;

    float lowFreq = 0.3f;
    float highFreq = 2.0f;

    float q = 3.0f;

    EffectParameter params[4] = {
        {"Pos", ParamType::Float, &position, 0.0f, 10.0f, 0.1f, nullptr, 0, 1},
        {"Low(kHz)", ParamType::Float, &lowFreq, 0.0f, 0.5f, 0.05f, nullptr, 0, 2},
        {"High(kHz)", ParamType::Float, &highFreq, 0.8f, 2.5f, 0.05f, nullptr, 0, 2},
        {"Q", ParamType::Float, &q, 0.3f, 20.0f, 0.1f, nullptr, 0, 1}
    };

public:
    const char* getName() const override {
        return "Wah";
    }

    const EffectParameter* getParameters() const override {
        return params;
    }

    int getParameterCount() const override {
        return 4;
    }
    
    void syncToChain() override {
        if (enabled) {
            wah.setPosition(position / 10.0f);
            wah.setLowFreq(lowFreq * 1000.0f);
            wah.setHighFreq(highFreq * 1000.0f);
            wah.setQ(q);
            wah.enable();
        }
        else {
            wah.disable();
        }
    }
};
