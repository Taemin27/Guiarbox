#pragma once

#include "../config/AudioChain.h"

inline bool isSdWavPlaybackActive() {
    return backingTrackPlayWav.isPlaying() || recorderPlayWav.isPlaying();
}

// Set by RecordPage while audio is being written to SD.
inline bool sdRecordingActive = false;

inline bool isSdRecordingActive() {
    return sdRecordingActive;
}

// True when the SD card is in use for streaming audio (read or write).
inline bool isSdAudioContended() {
    return isSdWavPlaybackActive() || isSdRecordingActive();
}
