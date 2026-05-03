#pragma once
#include "../EffectManager.h"
#include "../AudioLevels.h"

extern AudioEffectCompressor compressor;

class CompressorManager : public EffectManager {
private:
    /** dBFS relative to digital full scale; converted to linear peak for the compressor. */
    int gateDb = -24;
    int ratio = 4;
    int attackMs = 20;
    int releaseMs = 100;
    float gain = 12.0f;

    EffectParameter params[5] = {
        {"Thr", ParamType::Int, &gateDb, -60, 0, 1},
        {"Ratio", ParamType::Int, &ratio, 1, 20, 1},
        {"Attack", ParamType::Int, &attackMs, 0, 500, 1},
        {"Release", ParamType::Int, &releaseMs, 1, 500, 1},
        {"Gain", ParamType::Float, &gain, 0.0f, 20.0f, 0.5f, nullptr, 0, 1}
    };

public:
    const char* getName() const override {
        return "Compressor";
    }

    const EffectParameter* getParameters() const override {
        return params;
    }

    int getParameterCount() const override {
        return 5;
    }

    void syncToChain() override {
        if (enabled) {
            compressor.setThreshold(clampedLinearPeakFromDbfs((float)gateDb));
            compressor.setRatio((float)ratio);
            compressor.setAttackMs((float)attackMs);
            compressor.setReleaseMs((float)releaseMs);
            compressor.setLevel(gain/10.0f);

            compressor.enable();
        }
        else {
            compressor.disable();
        }
    }
};
