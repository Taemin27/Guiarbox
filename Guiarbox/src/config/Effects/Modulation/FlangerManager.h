#pragma once
#include "../EffectManager.h"
#include "../../../../lib/customAudioClasses/effect_flanger.h"

extern AudioEffectFlanger flanger;

class FlangerManager : public EffectManager {
private:
    float rateHz = 0.3f;
    float depth = 5.0f;
    float manual = 2.0f;
    float feedback = 5.0f;
    float mix = 5.0f;

    EffectParameter params[5] = {
        {"Rate(Hz)", ParamType::Float, &rateHz, 0.0f, 10.0f, 0.05f, nullptr, 0, 2},
        {"Depth", ParamType::Float, &depth, 0.0f, 10.0f, 0.5f, nullptr, 0, 1},
        {"Manual", ParamType::Float, &manual, 0.0f, 10.0f, 0.5f, nullptr, 0, 1},
        {"Feedback", ParamType::Float, &feedback, -10.0f, 10.0f, 0.5f, nullptr, 0, 1},
        {"Mix", ParamType::Float, &mix, 0.0f, 10.0f, 0.5f, nullptr, 0, 1}
    };

public:
    const char* getName() const override {
        return "Flanger";
    }

    const EffectParameter* getParameters() const override {
        return params;
    }

    int getParameterCount() const override {
        return 5;
    }
    
    void syncToChain() override {
        if (enabled) {
            flanger.setRate(rateHz);
            flanger.setDepth(depth / 10.0f);
            flanger.setManual(manual / 10.0f);
            flanger.setFeedback(feedback / 10.0f);
            flanger.setMix(mix / 10.0f);

            flanger.enable();
        }
        else {
            flanger.disable();
        }
    }
};