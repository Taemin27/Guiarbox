#pragma once
#include "../EffectManager.h"
#include "../AudioLevels.h"

extern AudioEffectPitchShift pitchShifter;

class PitchShifterManager : public EffectManager {
private:
    int mode = 0; // 0: Static, 1: Auto
    float pitch = 0.0f;
    float mix = 1.0f;

    // Auto mode params — hidden when mode == Static
    float targetPitch = 12.0f;
    /** dBFS; converted to linear peak for the envelope gate in auto mode. */
    int gateDb = -24;
    int attackMs = 30;
    int releaseMs = 30;

    static constexpr const char* const MODE_OPTIONS[] = {"Static", "Auto"};

    EffectParameter params[7] = {
        {"Mode",  ParamType::Option, &mode, 0, 1, 1, MODE_OPTIONS, 2},
        {"Pitch", ParamType::Float, &pitch, -24.0f, 24.0f, 0.5f, nullptr, 0, 1},
        {"Mix",   ParamType::Float, &mix, 0.0f, 1.0f, 0.1f, nullptr, 0, 1},
        {"Target", ParamType::Float, &targetPitch, -24.0f, 24.0f, 0.1f, nullptr, 0, 1},
        {"Thr", ParamType::Int, &gateDb, -60, 0, 1},
        {"Attack",  ParamType::Int, &attackMs, 0, 300, 5},
        {"Release", ParamType::Int, &releaseMs, 0, 300, 5}
    };

public:
    const char* getName() const override {
        return "Pitch Shift";
    }

    const EffectParameter* getParameters() const override {
        return params;
    }

    int getParameterCount() const override {
        return 7;
    }

    void syncToChain() override {
        if (enabled) {
            pitchShifter.setMix(mix);
            if (mode == 0) {
                pitchShifter.setDynamicMode(false);
                pitchShifter.setSemitones(pitch);
            } else {
                pitchShifter.setSemitones(pitch);
                pitchShifter.setTargetSemitones(targetPitch);
                pitchShifter.setDynamicThreshold(clampedLinearPeakFromDbfs((float)gateDb, 0.001f, 1.0f));
                pitchShifter.setDynamicAttackMs((float)attackMs);
                pitchShifter.setDynamicReleaseMs((float)releaseMs);
                pitchShifter.setDynamicMode(true);
            }
            pitchShifter.enable();
        } else {
            pitchShifter.disable();
        }
    }
};
