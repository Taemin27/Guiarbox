#pragma once

#include <Audio.h>
#include <SD.h>

namespace WavHeader {
constexpr uint16_t kHeaderSize = 44;
constexpr uint16_t kChannels = 1;
constexpr uint16_t kBitsPerSample = 16;

inline void writeUint32LE(File& file, uint32_t value) {
    file.write((uint8_t)(value & 0xFF));
    file.write((uint8_t)((value >> 8) & 0xFF));
    file.write((uint8_t)((value >> 16) & 0xFF));
    file.write((uint8_t)((value >> 24) & 0xFF));
}

inline void writeUint16LE(File& file, uint16_t value) {
    file.write((uint8_t)(value & 0xFF));
    file.write((uint8_t)((value >> 8) & 0xFF));
}

inline void write(File& file, uint32_t dataSize) {
    const uint32_t sampleRate = AUDIO_SAMPLE_RATE_EXACT;
    const uint32_t byteRate = sampleRate * kChannels * kBitsPerSample / 8;
    const uint16_t blockAlign = kChannels * kBitsPerSample / 8;

    file.write("RIFF", 4);
    writeUint32LE(file, 36 + dataSize);
    file.write("WAVE", 4);
    file.write("fmt ", 4);
    writeUint32LE(file, 16);
    writeUint16LE(file, 1);
    writeUint16LE(file, kChannels);
    writeUint32LE(file, sampleRate);
    writeUint32LE(file, byteRate);
    writeUint16LE(file, blockAlign);
    writeUint16LE(file, kBitsPerSample);
    file.write("data", 4);
    writeUint32LE(file, dataSize);
}

inline void patch(File& file, uint32_t dataSize) {
    if (!file) {
        return;
    }
    file.seek(0);
    write(file, dataSize);
}
} // namespace WavHeader
