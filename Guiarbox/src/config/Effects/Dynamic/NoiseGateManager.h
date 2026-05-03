#pragma once
#include "../EffectManager.h"
#include "../AudioLevels.h"

extern AudioEffectNoiseGate noiseGate;

class NoiseGateManager : public EffectManager {
private:
    int threshold = -60;
    int attackMs = 10;
    int releaseMs = 100;
    int holdMs = 20;
    float range = 10.0f;

    EffectParameter params[5] = {
        {"Thr", ParamType::Int, &threshold, -80, 0, 1},
        {"Attack", ParamType::Int, &attackMs, 0, 300, 1},
        {"Release", ParamType::Int, &releaseMs, 0, 300, 1},
        {"Hold", ParamType::Int, &holdMs, 0, 600, 1},
        {"Range", ParamType::Float, &range, 0.0f, 10.0f, 0.5f, nullptr, 0, 1}
    };

    public:
        const char* getName() const override {
            return "N-Gate";
        }
        const EffectParameter* getParameters() const override {
            return params;
        }
        int getParameterCount() const override {
            return 5;
        }
        void syncToChain() override {
            if (enabled) {
                noiseGate.setThreshold(clampedLinearPeakFromDbfs((float)threshold));
                noiseGate.setAttackMs((float)attackMs);
                noiseGate.setReleaseMs((float)releaseMs);
                noiseGate.setHoldMs((float)holdMs);
                noiseGate.setRange(range/10.0f);

                noiseGate.enable();
            }
            else {
                noiseGate.disable();
            }
        }
    };