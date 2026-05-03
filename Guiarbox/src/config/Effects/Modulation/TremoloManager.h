#pragma once
#include "../EffectManager.h"
#include "../../../../lib/customAudioClasses/effect_tremolo.h"

extern AudioEffectTremolo tremolo;

class TremoloManager : public EffectManager {
private:
    float rateHz = 4.0f;
    float depth = 5.0f;
    int shape = 0;
    float bias = 0.0f;

    static constexpr const char* const SHAPE_OPTIONS[] = {"Sine", "Tri", "Square"};

    EffectParameter params[4] = {
        {"Rate(Hz)", ParamType::Float, &rateHz, 0.0f, 20.0f, 0.2f, nullptr, 0, 1},
        {"Depth", ParamType::Float, &depth, 0.0f, 10.0f, 0.5f, nullptr, 0, 1},
        {"Shape", ParamType::Option, &shape, 0, 2, 1, SHAPE_OPTIONS, 3},
        {"Bias", ParamType::Float, &bias, -10.0f, 10.0f, 0.5f, nullptr, 0, 1}
    };

public:
    const char* getName() const override {
        return "Tremolo";
    }

    const EffectParameter* getParameters() const override {
        return params;
    }

    int getParameterCount() const override {
        return 4;
    }

    void syncToChain() override {
        if (enabled) {
            tremolo.setRate(rateHz);
            tremolo.setDepth(depth / 10.0f);
            tremolo.setShape(static_cast<AudioEffectTremolo::Shape>(shape));
            tremolo.setBias(bias / 10.0f);

            tremolo.enable();
        }
        else {
            tremolo.disable();
        }
    }
};