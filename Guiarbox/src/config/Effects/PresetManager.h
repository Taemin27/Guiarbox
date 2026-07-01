#pragma once

#include <Arduino.h>
#include <SD.h>
#include <ArduinoJson.h>

#include "../../utils/SdAudioActivity.h"
#include "../../utils/SystemMessage.h"
#include "EffectsRegistry.h"

extern void refreshCurrentPage();

/*

Static class to manage the generation, saving, and loading of presets.

RAM cache:
  All preset switching is served from in-memory JsonDocuments so SD wav playback
  is never interrupted. SD writes are deferred until no audio streaming uses the card.

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
    static constexpr int PRESET_SLOT_COUNT = MAX_PRESET_INDEX + 1;
    static constexpr const char* ENABLED_KEY = "enabled";

    static JsonDocument defaultDoc;
    static JsonDocument presetDocs[PRESET_SLOT_COUNT];
    static bool slotCached[PRESET_SLOT_COUNT];
    static bool slotDirty[PRESET_SLOT_COUNT];
    static bool defaultDirty;
    static int lastUsedIndex;
    static bool lastUsedDirty;
    static bool cacheReady;

    static String presetPath(int presetIndex) {
        return String(PRESETS_DIR) + "/preset_" + String(presetIndex) + ".json";
    }

    static void ensurePresetsDir() {
        if (!SD.exists(PRESETS_DIR)) {
            SD.mkdir(PRESETS_DIR);
        }
    }

    static bool readDocumentFromSd(const String& path, JsonDocument& doc) {
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

    static bool writeDocumentToSd(const String& path, const JsonDocument& doc) {
        if (doc.overflowed()) {
            Serial.print("Preset document overflow (");
            Serial.println(path);
            return false;
        }

        ensurePresetsDir();

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

    static bool copyDocument(JsonDocument& dst, const JsonDocument& src) {
        dst.clear();
        String buffer;
        serializeJson(src, buffer);
        if (buffer.length() == 0) {
            return false;
        }
        DeserializationError err = deserializeJson(dst, buffer);
        if (err || dst.overflowed()) {
            Serial.print("Preset document copy failed: ");
            Serial.println(err.c_str());
            dst.clear();
            return false;
        }
        return true;
    }

    static bool cacheDocument(int presetIndex, const JsonDocument& doc) {
        presetIndex = clampPresetIndex(presetIndex);
        if (!copyDocument(presetDocs[presetIndex], doc)) {
            return false;
        }
        slotCached[presetIndex] = true;
        return true;
    }

    static int optionMaxIndex(const EffectParameter& param) {
        if (param.optionCount > 0) {
            return param.optionCount - 1;
        }
        return (int)param.maxValue;
    }

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

    static void markLastUsedIndex(int presetIndex) {
        lastUsedIndex = clampPresetIndex(presetIndex);
        lastUsedDirty = true;
    }

    static bool flushLastUsedIndex() {
        if (!lastUsedDirty) {
            return true;
        }
        JsonDocument doc;
        doc["index"] = lastUsedIndex;
        if (!writeDocumentToSd(LAST_USED_PRESET_FILE, doc)) {
            return false;
        }
        lastUsedDirty = false;
        return true;
    }

public:
    static void createDefaultPreset() {
        JsonDocument doc;
        if (readDocumentFromSd(DEFAULT_PRESET_FILE, doc)) {
            return;
        }
        writeDocumentToSd(DEFAULT_PRESET_FILE, buildDocument());
    }

    static void initCache() {
        if (cacheReady) {
            return;
        }

        for (int i = 0; i < PRESET_SLOT_COUNT; i++) {
            slotCached[i] = false;
            slotDirty[i] = false;
            presetDocs[i].clear();
        }
        defaultDirty = false;
        lastUsedDirty = false;

        if (!readDocumentFromSd(DEFAULT_PRESET_FILE, defaultDoc)) {
            copyDocument(defaultDoc, buildDocument());
            defaultDirty = true;
        }

        for (int i = MIN_PRESET_INDEX; i <= MAX_PRESET_INDEX; i++) {
            if (readDocumentFromSd(presetPath(i), presetDocs[i])) {
                slotCached[i] = true;
            } else {
                presetDocs[i].clear();
            }
        }

        JsonDocument lastUsedDoc;
        if (readDocumentFromSd(LAST_USED_PRESET_FILE, lastUsedDoc) && !lastUsedDoc["index"].isNull()) {
            lastUsedIndex = clampPresetIndex(lastUsedDoc["index"].as<int>());
        } else {
            lastUsedIndex = MIN_PRESET_INDEX;
        }

        cacheReady = true;
        flushPendingWrites();
    }

    static int getLastUsedPresetIndex() {
        if (!cacheReady) {
            JsonDocument doc;
            if (!readDocumentFromSd(LAST_USED_PRESET_FILE, doc) || doc["index"].isNull()) {
                return MIN_PRESET_INDEX;
            }
            return clampPresetIndex(doc["index"].as<int>());
        }
        return lastUsedIndex;
    }

    static void loadPreset(int presetIndex) {
        if (!cacheReady) {
            initCache();
        }

        presetIndex = clampPresetIndex(presetIndex);
        JsonDocument empty;

        if (slotCached[presetIndex]) {
            applyDocument(presetDocs[presetIndex], defaultDoc);
        } else {
            applyDocument(defaultDoc, empty);
        }

        markLastUsedIndex(presetIndex);
        flushPendingWrites();
    }

    static void savePreset(int presetIndex) {
        if (!cacheReady) {
            initCache();
        }

        presetIndex = clampPresetIndex(presetIndex);
        JsonDocument doc = buildDocument();
        if (!cacheDocument(presetIndex, doc)) {
            return;
        }
        slotDirty[presetIndex] = true;
        markLastUsedIndex(presetIndex);
        flushPendingWrites();
        if (hasPendingWrites() && isSdAudioContended()) {
            SystemMessage::show("Saved to memory. SD write delayed until playback or recording is stopped.", 2500, refreshCurrentPage);
        }
    }

    static bool hasPendingWrites() {
        if (!cacheReady) {
            return false;
        }
        if (defaultDirty || lastUsedDirty) {
            return true;
        }
        for (int i = MIN_PRESET_INDEX; i <= MAX_PRESET_INDEX; i++) {
            if (slotDirty[i]) {
                return true;
            }
        }
        return false;
    }

    // Persist dirty cache entries when SD is not streaming audio.
    static void flushPendingWrites() {
        if (!cacheReady || isSdAudioContended()) {
            return;
        }

        if (defaultDirty) {
            if (writeDocumentToSd(DEFAULT_PRESET_FILE, defaultDoc)) {
                defaultDirty = false;
            }
        }

        for (int i = MIN_PRESET_INDEX; i <= MAX_PRESET_INDEX; i++) {
            if (!slotDirty[i]) {
                continue;
            }
            if (writeDocumentToSd(presetPath(i), presetDocs[i])) {
                slotDirty[i] = false;
            }
        }

        flushLastUsedIndex();
    }
};

inline JsonDocument PresetManager::defaultDoc;
inline JsonDocument PresetManager::presetDocs[PresetManager::PRESET_SLOT_COUNT];
inline bool PresetManager::slotCached[PresetManager::PRESET_SLOT_COUNT] = {};
inline bool PresetManager::slotDirty[PresetManager::PRESET_SLOT_COUNT] = {};
inline bool PresetManager::defaultDirty = false;
inline int PresetManager::lastUsedIndex = PresetManager::MIN_PRESET_INDEX;
inline bool PresetManager::lastUsedDirty = false;
inline bool PresetManager::cacheReady = false;
