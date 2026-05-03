#pragma once
#include "../EffectManager.h"
#include "../../../../lib/customAudioClasses/effect_phaser.h"

extern AudioEffectPhaser phaser;

class PhaserManager : public EffectManager {
private:
    float rateHz = 5.0f;
    float depth = 5.0f;
    float mix = 5.0f;
    float feedback = 5.0f;
    int stages = 4;


    EffectParameter params[5] = {
        {"Rate(Hz)", ParamType::Float, &rateHz, 0.0f, 20.0f, 0.1f, nullptr, 0, 1},
        {"Depth", ParamType::Float, &depth, 0.0f, 10.0f, 0.5f, nullptr, 0, 1},
        {"Mix", ParamType::Float, &mix, 0.0f, 10.0f, 0.5f, nullptr, 0, 1},
        {"Feedback", ParamType::Float, &feedback, -10.0f, 10.0f, 0.5f, nullptr, 0, 1},
        {"Stages", ParamType::Int, &stages, 2, 12, 2, nullptr, 0, 1}
    };

public:
    const char* getName() const override {
        return "Phaser";
    }

    const EffectParameter* getParameters() const override {
        return params;
    }

    int getParameterCount() const override {
        return 5;
    }

    void syncToChain() override {
        if (enabled) {
            phaser.setRate(rateHz);
            phaser.setDepth(depth/10.0f);
            phaser.setMix(mix/10.0f);
            phaser.setFeedback(feedback/10.0f);
            phaser.setStages(stages);

            phaser.enable();
        }
        else {
            phaser.disable();
        }
    }
};