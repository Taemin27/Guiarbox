#pragma once

#include <Arduino.h>
#include <SD.h>
#include <ArduinoJson.h>

#include "EffectsRegistry.h"

/*

Static class to manage the generation, saving, and loading of presets.

Files on the SD card:
  /presets/preset_<index>.json   — user presets
  /presets/preset_default.json   — defaults, created from init values on first boot
  /presets/preset_lastUsed.json  — { "index": N }

*/
class PresetManager {
    PresetManager() = delete;

    static constexpr const char* PRESETS_DIR = "/presets";
    static constexpr const char* DEFAULT_PRESET_FILE = "/presets/preset_default.json";
    static constexpr const char* LAST_USED_PRESET_FILE = "/presets/preset_lastUsed.json";
    static constexpr int CATEGORY_COUNT = sizeof(effectsRegistry) / sizeof(effectsRegistry[0]);
    static constexpr int MIN_PRESET_INDEX = 0;
    static constexpr int MAX_PRESET_INDEX = 99;
    static constexpr const char* ENABLED_KEY = "enabled";

    static void ensurePresetsDir() {
        if (!SD.exists(PRESETS_DIR)) {
            SD.mkdir(PRESETS_DIR);
        }
    }

    static bool readDocument(const String& path, JsonDocument& doc) {
        doc.clear();
        ensurePresetsDir();

        File file = SD.open(path.c_str(), FILE_READ);
        if (!file) {
            return false;
        }

        DeserializationError err = deserializeJson(doc, file);
        file.close();

        if (err) {
            Serial.print("Preset parse failed (");
            Serial.print(path);
            Serial.print("): ");
            Serial.println(err.c_str());
            doc.clear();
            return false;
        }

        if (doc.overflowed()) {
            Serial.print("Preset JSON too large (");
            Serial.println(path);
            doc.clear();
            return false;
        }

        return true;
    }

    static bool writeDocument(const String& path, const JsonDocument& doc) {
        ensurePresetsDir();

        if (doc.overflowed()) {
            Serial.print("Preset document overflow (");
            Serial.println(path);
            return false;
        }

        // Teensy SD FILE_WRITE appends (O_AT_END); FILE_WRITE_BEGIN overwrites from the start.
        File file = SD.open(path.c_str(), FILE_WRITE_BEGIN);
        if (!file) {
            Serial.print("Error opening preset file for write: ");
            Serial.println(path);
            return false;
        }

        if (serializeJson(doc, file) == 0) {
            Serial.print("Error writing preset file: ");
            Serial.println(path);
            file.close();
            return false;
        }

        file.truncate(file.position());
        file.close();
        return true;
    }

    static int optionMaxIndex(const EffectParameter& param) {
        if (param.optionCount > 0) {
            return param.optionCount - 1;
        }
        return (int)param.maxValue;
    }

    // Clamp the in-memory value for a parameter to its allowed range.
    static void clampParameter(const EffectParameter& param) {
        switch (param.type) {
            case ParamType::Float:
                *(float*)param.valuePtr = constrain(
                    *(float*)param.valuePtr, param.minValue, param.maxValue);
                break;
            case ParamType::Int:
                *(int*)param.valuePtr = constrain(
                    *(int*)param.valuePtr, (int)param.minValue, (int)param.maxValue);
                break;
            case ParamType::Option:
                *(int*)param.valuePtr = constrain(
                    *(int*)param.valuePtr, (int)param.minValue, optionMaxIndex(param));
                break;
            case ParamType::Bool:
                break;
        }
    }

    // Write a clamped parameter value into a JSON object.
    static void writeParameter(JsonObject obj, const EffectParameter& param) {
        clampParameter(param);
        switch (param.type) {
            case ParamType::Float:
                obj[param.name] = *(float*)param.valuePtr;
                break;
            case ParamType::Int:
            case ParamType::Option:
                obj[param.name] = *(int*)param.valuePtr;
                break;
            case ParamType::Bool:
                obj[param.name] = *(bool*)param.valuePtr;
                break;
        }
    }

    // Read a JSON value into a parameter if the type is compatible; value is clamped on success.
    static bool applyParameter(const EffectParameter& param, JsonVariantConst value) {
        if (value.isNull()) {
            return false;
        }

        switch (param.type) {
            case ParamType::Float:
                if (!value.is<float>() && !value.is<int>()) {
                    return false;
                }
                *(float*)param.valuePtr = constrain(
                    value.as<float>(), param.minValue, param.maxValue);
                return true;
            case ParamType::Int:
                if (!value.is<int>() && !value.is<float>()) {
                    return false;
                }
                *(int*)param.valuePtr = constrain(
                    value.as<int>(), (int)param.minValue, (int)param.maxValue);
                return true;
            case ParamType::Option:
                if (!value.is<int>() && !value.is<float>()) {
                    return false;
                }
                *(int*)param.valuePtr = constrain(
                    value.as<int>(), (int)param.minValue, optionMaxIndex(param));
                return true;
            case ParamType::Bool:
                if (!value.is<bool>() && !value.is<int>()) {
                    return false;
                }
                if (value.is<int>()) {
                    *(bool*)param.valuePtr = value.as<int>() != 0;
                } else {
                    *(bool*)param.valuePtr = value.as<bool>();
                }
                return true;
        }

        return false;
    }

    // Read enabled state from JSON, using fallback if missing from the primary document.
    static void applyEffectEnabled(
        EffectManager* mgr, JsonObjectConst effect, JsonObjectConst fallbackEffect) {
        JsonVariantConst value = effect[ENABLED_KEY];
        if (value.isNull() && !fallbackEffect.isNull()) {
            value = fallbackEffect[ENABLED_KEY];
        }
        if (value.isNull()) {
            return;
        }
        if (!value.is<bool>() && !value.is<int>()) {
            return;
        }
        if (value.is<int>()) {
            mgr->enabled = value.as<int>() != 0;
        } else {
            mgr->enabled = value.as<bool>();
        }
    }

    // Build a JSON document from the current in-memory effect enabled states and parameter values.
    static JsonDocument buildDocument() {
        JsonDocument doc;
        for (int i = 0; i < CATEGORY_COUNT; i++) {
            for (EffectManager* mgr : effectsRegistry[i].effectManagers) {
                JsonObject effect = doc[mgr->getName()].to<JsonObject>();
                effect[ENABLED_KEY] = mgr->enabled;
                for (int k = 0; k < mgr->getParameterCount(); k++) {
                    writeParameter(effect, mgr->getParameters()[k]);
                }
            }
        }

        if (doc.overflowed()) {
            Serial.println("Preset document overflow while building");
        }

        return doc;
    }

    // Apply JSON values to effect managers, using fallback for any missing effect, enabled state, or parameter.
    // Fields missing from both documents are left unchanged. Syncs each effect to the audio chain.
    static void applyDocument(const JsonDocument& doc, const JsonDocument& fallback) {
        for (int i = 0; i < CATEGORY_COUNT; i++) {
            for (EffectManager* mgr : effectsRegistry[i].effectManagers) {
                JsonObjectConst effect = doc[mgr->getName()];
                JsonObjectConst fallbackEffect = fallback[mgr->getName()];

                applyEffectEnabled(mgr, effect, fallbackEffect);

                for (int k = 0; k < mgr->getParameterCount(); k++) {
                    const EffectParameter& param = mgr->getParameters()[k];
                    JsonVariantConst value = effect[param.name];
                    if (value.isNull() && !fallbackEffect.isNull()) {
                        value = fallbackEffect[param.name];
                    }
                    applyParameter(param, value);
                }
                mgr->syncToChain();
            }
        }
    }

    static int clampPresetIndex(int presetIndex) {
        return constrain(presetIndex, MIN_PRESET_INDEX, MAX_PRESET_INDEX);
    }

    static void syncAllToChain() {
        for (int i = 0; i < CATEGORY_COUNT; i++) {
            for (EffectManager* mgr : effectsRegistry[i].effectManagers) {
                mgr->syncToChain();
            }
        }
    }

public:
    // Ensure preset_default.json exists on the SD card, creating it from init values if missing or invalid.
    static void createDefaultPreset() {
        JsonDocument doc;
        if (readDocument(DEFAULT_PRESET_FILE, doc)) {
            return;
        }
        writeDocument(DEFAULT_PRESET_FILE, buildDocument());
    }

    // Read the last-used preset index from preset_lastUsed.json; returns 0 if missing or invalid.
    static int getLastUsedPresetIndex() {
        JsonDocument doc;
        if (!readDocument(LAST_USED_PRESET_FILE, doc)) {
            return MIN_PRESET_INDEX;
        }
        if (doc["index"].isNull()) {
            return MIN_PRESET_INDEX;
        }
        return clampPresetIndex(doc["index"].as<int>());
    }

    // Write the last-used preset index to preset_lastUsed.json.
    static void saveCurrentPresetIndex(int presetIndex) {
        JsonDocument doc;
        doc["index"] = clampPresetIndex(presetIndex);
        writeDocument(LAST_USED_PRESET_FILE, doc);
    }

    // Load preset_<index>.json from the SD card.
    // If that file is missing or invalid, loads preset_default.json instead.
    // If both are missing or invalid, writes preset_default.json from current init values and leaves them as-is.
    // When loading a user preset, missing fields fall back to preset_default.json.
    static void loadPreset(int presetIndex) {
        presetIndex = clampPresetIndex(presetIndex);

        const String presetPath = String(PRESETS_DIR) + "/preset_" + String(presetIndex) + ".json";

        JsonDocument primary;
        JsonDocument fallback;
        JsonDocument empty;

        if (readDocument(presetPath, primary)) {
            readDocument(DEFAULT_PRESET_FILE, fallback);
            applyDocument(primary, fallback);
            saveCurrentPresetIndex(presetIndex);
            return;
        }

        if (readDocument(DEFAULT_PRESET_FILE, primary)) {
            applyDocument(primary, empty);
            saveCurrentPresetIndex(presetIndex);
            return;
        }

        writeDocument(DEFAULT_PRESET_FILE, buildDocument());
        syncAllToChain();
        saveCurrentPresetIndex(presetIndex);
    }

    // Write the current in-memory effect parameter and enabled values to preset_<index>.json.
    static void savePreset(int presetIndex) {
        presetIndex = clampPresetIndex(presetIndex);
        const String presetPath = String(PRESETS_DIR) + "/preset_" + String(presetIndex) + ".json";
        writeDocument(presetPath, buildDocument());
        saveCurrentPresetIndex(presetIndex);
    }
};

