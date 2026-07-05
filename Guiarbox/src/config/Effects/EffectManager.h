#pragma once
#include <vector>

enum class ParamType {
    Float,
    Int,
    Bool,
    Option
};

struct EffectParameter {
    const char* name;
    ParamType type;
    void* valuePtr;
    float minValue = 0.0f;
    float maxValue = 1.0f;
    float step = 0.01f;
    const char* const* options = nullptr;
    int optionCount = 0;

    // decimal places for ParamType::Float (FloatEditor; clamped 1–6 in FloatEditor)
    int decimals = 2;

    // Optional Option hooks: save/load by stable name instead of index (e.g. SD file lists).
    const char* (*optionIndexToName)(int index) = nullptr;
    int (*optionNameToIndex)(const char* name) = nullptr;
};

class EffectManager {
public:
    bool enabled = false;

    virtual ~EffectManager() = default;
    virtual const char* getName() const = 0;
    virtual const EffectParameter* getParameters() const = 0;
    virtual int getParameterCount() const = 0;
    virtual void syncToChain() = 0;

    // Called after SD mount; override when parameters depend on SD file listings.
    virtual void onSdReady() {}
};

struct EffectCategory {
    const char* name;
    std::vector<EffectManager*> effectManagers;
};
