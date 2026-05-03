#pragma once

#include <Arduino.h>

/** Linear peak amplitude (0…1, same as |normalized int16 sample|) from dB relative to digital full scale. */
inline float linearPeakFromDbfs(float dbFs) {
    return powf(10.0f, dbFs / 20.0f);
}

/** dBFS from linear peak; tiny values map to a floor so log is defined. */
inline float dbfsFromLinearPeak(float linearPeak) {
    if (linearPeak <= 1e-10f) {
        return -120.0f;
    }
    return 20.0f * log10f(linearPeak);
}

/** `linearPeakFromDbfs` then clamp to a usable linear range (e.g. detector / UI limits). */
inline float clampedLinearPeakFromDbfs(float dbFs, float linearMin = 1e-5f, float linearMax = 1.0f) {
    return constrain(linearPeakFromDbfs(dbFs), linearMin, linearMax);
}
