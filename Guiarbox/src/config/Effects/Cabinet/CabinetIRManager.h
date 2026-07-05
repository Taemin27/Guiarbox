#pragma once

#include "../EffectManager.h"
#include "../IrFileCatalog.h"
#include "../../../../lib/customAudioClasses/effect_cabinet_ir.h"

extern AudioEffectCabinetIR cabinetIR;

class CabinetIRManager : public EffectManager {
public:
    static constexpr const char* EFFECT_NAME = "Cab IR";

    CabinetIRManager() {
        refreshParameterOptions();
    }

    const char* getName() const override {
        return EFFECT_NAME;
    }

    const EffectParameter* getParameters() const override {
        return params;
    }

    int getParameterCount() const override {
        return 5;
    }

    void onSdReady() override {
        IrFileCatalog::init();
        refreshParameterOptions();
        lastLoadedIndex = -1;
    }

    void syncToChain() override {
        if (!enabled) {
            cabinetIR.disable();
            return;
        }

        if (!IrFileCatalog::hasUsableFiles()) {
            cabinetIR.clearImpulse();
            cabinetIR.disable();
            return;
        }

        irIndex = constrain(irIndex, 0, max(IrFileCatalog::count(), 1) - 1);

        if (irIndex != lastLoadedIndex) {
            if (!loadSelectedIr()) {
                cabinetIR.disable();
                return;
            }
            lastLoadedIndex = irIndex;
        }

        cabinetIR.setLevel(level / 10.0f);
        cabinetIR.setBass(bass / 10.0f);
        cabinetIR.setMid(mid / 10.0f);
        cabinetIR.setTreble(treble / 10.0f);
        cabinetIR.enable();
    }

private:
    int irIndex = 0;
    int lastLoadedIndex = -1;
    float level = 5.0f;
    float bass = 5.0f;
    float mid = 5.0f;
    float treble = 5.0f;

    EffectParameter params[5] = {
        {"IR  ", ParamType::Option, &irIndex, 0.0f, 0.0f, 1.0f, nullptr, 0, 0,
         IrFileCatalog::displayNameForIndex, IrFileCatalog::indexForDisplayName},
        {"Bass", ParamType::Float, &bass, 0.0f, 10.0f, 0.5f, nullptr, 0, 1},
        {"Mid ", ParamType::Float, &mid, 0.0f, 10.0f, 0.5f, nullptr, 0, 1},
        {"Treble", ParamType::Float, &treble, 0.0f, 10.0f, 0.5f, nullptr, 0, 1},
        {"Level", ParamType::Float, &level, 0.0f, 10.0f, 0.5f, nullptr, 0, 1}
    };

    void refreshParameterOptions() {
        irIndex = constrain(irIndex, 0, max(IrFileCatalog::count(), 1) - 1);
        const int n = max(IrFileCatalog::count(), 1);
        params[0].options = IrFileCatalog::displayOptions();
        params[0].optionCount = n;
        params[0].maxValue = (float)(n - 1);
    }

    bool loadSelectedIr() {
        const char* path = IrFileCatalog::pathForIndex(irIndex);
        if (!path) {
            cabinetIR.clearImpulse();
            return false;
        }

        float* buffer = AudioEffectCabinetIR::loadScratchBuffer();
        const int maxSamples = AudioEffectCabinetIR::loadScratchBufferSamples();
        int length = 0;
        if (!IrFileCatalog::loadWavMono(path, buffer, maxSamples, length)) {
            Serial.print("Cab IR load failed: ");
            Serial.println(path);
            cabinetIR.clearImpulse();
            return false;
        }

        if (!cabinetIR.loadImpulse(buffer, length)) {
            cabinetIR.clearImpulse();
            return false;
        }

        return true;
    }
};
