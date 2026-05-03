#pragma once
#include "../EffectManager.h"

class DelayManager : public EffectManager {
private:
    int timeMs = 150;
    float feedback = 5.0f;
    float level = 5.0f;

    unsigned long lastTapMs = 0;

    EffectParameter params[3] = {
        {"Time", ParamType::Int, &timeMs, 0, 1000, 10},
        {"Feedback", ParamType::Float, &feedback, 0.0f, 10.0f, 0.5f, nullptr, 0, 1},
        {"Level", ParamType::Float, &level, 0.0f, 10.0f, 0.5f, nullptr, 0, 1}
    };

public:
    const char* getName() const override {
        return "Delay";
    }

    const EffectParameter* getParameters() const override {
        return params;
    }

    int getParameterCount() const override {
        return 3;
    }

    void syncToChain() override {
        if (enabled) {
            delay1.delay(0, (float)timeMs);
            delayAmp.gain(feedback/10.0f);
            delayMixer.gain(1, level/10.0f);
        }
        else {
            delayAmp.gain(0.0f);
            delayMixer.gain(1, 0.0f);
        }
    }
};
