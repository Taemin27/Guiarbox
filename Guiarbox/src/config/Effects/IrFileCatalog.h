#pragma once

#include <Arduino.h>
#include <SD.h>
#include <vector>

// Scans /irs on the SD card for WAV impulse files.
class IrFileCatalog {
public:
    static constexpr const char* DIR_NAME = "irs";
    static constexpr int MAX_FILES = 64;

    static void init() {
        if (ready) {
            return;
        }

        paths.clear();
        displayNames.clear();

        if (!SD.exists(DIR_NAME)) {
            SD.mkdir(DIR_NAME);
        }

        File dir = SD.open(DIR_NAME);
        if (dir) {
            while (true) {
                File entry = dir.openNextFile();
                if (!entry) {
                    break;
                }
                if (!entry.isDirectory()) {
                    String name = entry.name();
                    entry.close();
                    if (!isWavFile(name)) {
                        continue;
                    }
                    if ((int)paths.size() >= MAX_FILES) {
                        entry.close();
                        break;
                    }

                    String path = name;
                    if (path.charAt(0) != '/') {
                        path = "/" + String(DIR_NAME) + "/" + path;
                    }

                    String fileName = path.substring(path.lastIndexOf('/') + 1);
                    paths.push_back(path);
                    displayNames.push_back(stripWavExtension(fileName));
                } else {
                    entry.close();
                }
            }
            dir.close();
        }

        rebuildOptionPointers();
        if (paths.empty()) {
            paths.push_back("");
            displayNames.push_back("(no IRs)");
            rebuildOptionPointers();
        }
        ready = true;
    }

    static bool isReady() {
        return ready;
    }

    static bool hasUsableFiles() {
        return ready && !paths.empty() && paths[0].length() > 0;
    }

    static int count() {
        return (int)paths.size();
    }

    static const char* const* displayOptions() {
        return optionPtrs.data();
    }

    static const char* pathForIndex(int index) {
        if (index < 0 || index >= count()) {
            return nullptr;
        }
        return paths[(size_t)index].c_str();
    }

    static const char* displayNameForIndex(int index) {
        if (!ready || index < 0 || index >= (int)displayNames.size()) {
            return "";
        }
        return displayNames[(size_t)index].c_str();
    }

    static int indexForDisplayName(const char* name) {
        if (!ready || !name || name[0] == '\0') {
            return -1;
        }
        for (int i = 0; i < (int)displayNames.size(); ++i) {
            if (displayNames[(size_t)i].equals(name)) {
                return i;
            }
        }
        return -1;
    }

    // Load mono PCM WAV (16- or 24-bit, 44100 or 48000 Hz) into float samples at 44100 Hz.
    static bool loadWavMono(const char* path, float* dest, int maxSamples, int& outLength) {
        outLength = 0;
        if (!path || !dest || maxSamples <= 0) {
            return false;
        }

        File f = SD.open(path, FILE_READ);
        if (!f) {
            return false;
        }

        char riff[4];
        if (f.read(riff, 4) != 4 || memcmp(riff, "RIFF", 4) != 0) {
            f.close();
            return false;
        }

        uint32_t riffSize = 0;
        if (f.read(&riffSize, 4) != 4) {
            f.close();
            return false;
        }

        char wave[4];
        if (f.read(wave, 4) != 4 || memcmp(wave, "WAVE", 4) != 0) {
            f.close();
            return false;
        }

        uint16_t audioFormat = 0;
        uint16_t numChannels = 0;
        uint32_t sampleRate = 0;
        uint16_t bitsPerSample = 0;
        (void)riffSize;
        uint32_t dataOffset = 0;
        bool foundFmt = false;
        bool foundData = false;

        while (f.available()) {
            char chunkId[4];
            if (f.read(chunkId, 4) != 4) {
                break;
            }
            uint32_t chunkSize = 0;
            if (f.read(&chunkSize, 4) != 4) {
                break;
            }

            if (memcmp(chunkId, "fmt ", 4) == 0) {
                if (chunkSize < 16) {
                    f.close();
                    return false;
                }
                f.read(&audioFormat, 2);
                f.read(&numChannels, 2);
                f.read(&sampleRate, 4);
                f.seek(f.position() + 6);
                f.read(&bitsPerSample, 2);
                if (chunkSize > 16) {
                    f.seek(f.position() + (chunkSize - 16));
                }
                foundFmt = true;
            } else if (memcmp(chunkId, "data", 4) == 0) {
                dataOffset = f.position();
                foundData = true;
                break;
            } else {
                f.seek(f.position() + chunkSize);
            }
        }

        if (!foundFmt || !foundData || audioFormat != 1) {
            f.close();
            return false;
        }
        if (bitsPerSample != 16 && bitsPerSample != 24) {
            f.close();
            return false;
        }
        if (sampleRate != 44100 && sampleRate != 48000) {
            f.close();
            return false;
        }
        if (numChannels < 1 || numChannels > 2) {
            f.close();
            return false;
        }

        f.seek(dataOffset);
        const int maxFrames = maxSamples;
        int framesRead = 0;

        while (framesRead < maxFrames && f.available()) {
            float left = 0.0f;
            float right = 0.0f;

            if (bitsPerSample == 16) {
                int16_t left16 = 0;
                int16_t right16 = 0;
                if (f.read(&left16, 2) != 2) {
                    break;
                }
                if (numChannels == 2) {
                    if (f.read(&right16, 2) != 2) {
                        break;
                    }
                }
                left = (float)left16 / 32768.0f;
                right = (float)right16 / 32768.0f;
            } else {
                uint8_t sampleBytes[6];
                const int bytesPerChannel = 3;
                const int frameBytes = bytesPerChannel * numChannels;
                if (f.read(sampleBytes, frameBytes) != frameBytes) {
                    break;
                }
                left = pcm24ToFloat(sampleBytes);
                if (numChannels == 2) {
                    right = pcm24ToFloat(sampleBytes + bytesPerChannel);
                }
            }

            const float mono = (numChannels == 2) ? (0.5f * (left + right)) : left;
            dest[framesRead++] = mono;
        }

        f.close();

        if (sampleRate == 48000 && framesRead > 0) {
            const double ratio = 44100.0 / 48000.0;
            const int dstLen = min(maxSamples, (int)((double)framesRead * ratio));
            for (int i = dstLen - 1; i >= 0; --i) {
                const double srcPos = (double)i / ratio;
                const int idx = (int)srcPos;
                const float frac = (float)(srcPos - (double)idx);
                const float s0 = dest[idx];
                const float s1 = (idx + 1 < framesRead) ? dest[idx + 1] : s0;
                dest[i] = s0 + frac * (s1 - s0);
            }
            framesRead = dstLen;
        }

        outLength = framesRead;
        return framesRead > 0;
    }

private:
    static inline bool ready = false;
    static inline std::vector<String> paths;
    static inline std::vector<String> displayNames;
    static inline std::vector<const char*> optionPtrs;

    static String stripWavExtension(const String& fileName) {
        if (fileName.length() >= 4) {
            String lower = fileName;
            lower.toLowerCase();
            if (lower.endsWith(".wav")) {
                return fileName.substring(0, fileName.length() - 4);
            }
        }
        return fileName;
    }

    static float pcm24ToFloat(const uint8_t* bytes) {
        int32_t sample = (int32_t)(bytes[0] | (bytes[1] << 8) | (bytes[2] << 16));
        if (sample & 0x800000) {
            sample |= (int32_t)0xFF000000;
        }
        return (float)sample / 8388608.0f;
    }

    static bool isWavFile(const String& name) {
        if (name.length() < 4) {
            return false;
        }
        String lower = name;
        lower.toLowerCase();
        return lower.endsWith(".wav");
    }

    static void rebuildOptionPointers() {
        optionPtrs.clear();
        optionPtrs.reserve(displayNames.size());
        for (const String& s : displayNames) {
            optionPtrs.push_back(s.c_str());
        }
    }
};
