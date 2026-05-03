#pragma once
#include "../EffectManager.h"
#include "../../../../lib/customAudioClasses/effect_fuzz_FF.h"

extern AudioEffectFuzzFF fuzz;

class FuzzManager : public EffectManager {
private:
    int mode = 0;
    static constexpr const char* const MODE_OPTIONS[] = {"FF", "Muffy", "Octa"};

    float gain = 0.5f;
    float tone = 0.5f;
    float level = 0.5f;

    EffectParameter params[4] = {
        {"Mode", ParamType::Option, &mode, 0, 2, 1, MODE_OPTIONS, 3},
        {"Gain", ParamType::Float, &gain, 0.0f, 10.0f, 0.5f, nullptr, 0, 1},
        {"Tone", ParamType::Float, &tone, 0.0f, 10.0f, 0.5f, nullptr, 0, 1},
        {"Level", ParamType::Float, &level, 0.0f, 10.0f, 0.5f, nullptr, 0, 1}
    };

public:
    const char* getName() const override {
        return "Fuzz";
    }

    const EffectParameter* getParameters() const override {
        return params;
    }

    int getParameterCount() const override {
        return 4;
    }

    void syncToChain() override {
        if (enabled) {
           

            fuzz.enable();
        }
        else {
            fuzz.disable();
        }
    }
};