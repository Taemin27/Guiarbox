#pragma once
#include "../EffectManager.h"
#include "../AudioLevels.h"
#include "../../../../lib/customAudioClasses/effect_autoWham.h"

extern AudioEffectAutoWham autoWham;

class AutoWhamManager : public EffectManager {
private:
    float targetPitch = 12.0f;
    /** dBFS; converted to linear peak for the envelope gate. */
    int gateDb = -24;
    int attackMs = 80;
    int releaseMs = 80;

    EffectParameter params[4] = {
        {"Target", ParamType::Float, &targetPitch, -24.0f, 24.0f, 0.5f, nullptr, 0, 1},
        {"Thr", ParamType::Int, &gateDb, -60, 0, 1},
        {"Atk(ms)", ParamType::Int, &attackMs, 0, 500, 5},
        {"Rel(ms)", ParamType::Int, &releaseMs, 0, 500, 5},
    };

public:
    const char* getName() const override {
        return "Auto Wham";
    }

    const EffectParameter* getParameters() const override {
        return params;
    }

    int getParameterCount() const override {
        return 4;
    }

    void syncToChain() override {
        if (enabled) {
            autoWham.setTargetSemitones(targetPitch);
            autoWham.setThreshold(clampedLinearPeakFromDbfs((float)gateDb, 0.001f, 1.0f));
            autoWham.setAttackMs((float)attackMs);
            autoWham.setReleaseMs((float)releaseMs);
            autoWham.enable();
        } else {
            autoWham.disable();
        }
    }
};
