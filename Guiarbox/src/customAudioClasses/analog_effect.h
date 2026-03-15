#ifndef analog_effect_h_
#define analog_effect_h_

#include <Arduino.h>
#include "AudioStream.h"
#include <math.h>

class AudioAnalogEffect : public AudioStream {
public:
    AudioAnalogEffect() : AudioStream(1, inputQueueArray) {
        lineInLevel = 5; // Default line in level
    }

    void update(void) override {
        audio_block_t *block = receiveWritable(0);
        if (!block) return;

        for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
            float sample = sampleToVolts(block->data[i]);
            sample = processSample(sample);
            block->data[i] = voltsToSample((sample));
        }

        transmit(block);
        release(block);
    }

    void setLineInLevel(uint8_t level) {
        lineInLevel = constrain(level, 0, 15);
    }


protected:
    audio_block_t *inputQueueArray[1];

    uint8_t lineInLevel;
    
    virtual float processSample(float sample) = 0; 

     /* Sample to volts scaling depending on lineInLevel setting */
    

    static constexpr float lineInVpp[16] = {
        3.12f, 2.63f, 2.22f, 1.87f,
        1.58f, 1.33f, 1.11f, 0.94f,
        0.79f, 0.67f, 0.56f, 0.48f,
        0.40f, 0.34f, 0.29f, 0.24f
    };

    inline float sampleToVolts(int16_t counts) const {
        const float vPeak = 0.5f * lineInVpp[lineInLevel];
        return (counts / 32768.0f) * vPeak;
    }

    inline int16_t voltsToSample(float volts) const {
        const float vPeak = 0.5f * lineInVpp[lineInLevel];

        const float x = constrain(volts / vPeak, -1.0f, 1.0f);

        return (int16_t)(x * 32767.0f);
    }

    /* Modeling tools */

    static constexpr float dt = 1.0f / AUDIO_SAMPLE_RATE_EXACT;  // Sample period
    
    static inline float sinh_clamped(float x) {
        x = constrain(x, -12.0f, 12.0f);
        return 0.5f * (expf(x) - expf(-x));
    }
    static inline float cosh_clamped(float x) {
        x = constrain(x, -12.0f, 12.0f);
        return 0.5f * (expf(x) + expf(-x));
    }

    // Antiparallel diode pair current. Useful for modeling clipping stages
    static inline float Id(float v, float saturationCurrent, float idealityFactor, float thermalVoltage) {
        const float x = v / (idealityFactor * thermalVoltage);
        return 2.0f * saturationCurrent * sinh_clamped(x);
    }
    static inline float dId(float v, float saturationCurrent, float idealityFactor, float thermalVoltage) {
        const float x = v / (idealityFactor * thermalVoltage);
        return (2.0f * saturationCurrent / (idealityFactor * thermalVoltage)) * cosh_clamped(x);
    }

    // Call every sample to update capacitor voltage
    static inline float updateShuntCapVoltage(float x, float &Vc, float R, float C) {
        const float alpha = dt / (R * C + dt);
        Vc += alpha * (x - Vc);
        return Vc;
    }

    static inline float softLimit(float x) {
        return x / (1.0f + fabsf(x));
    }
    
    inline float railLimit(float x) const {
        const float Vpos = 3.6f;
        const float Vneg = 3.2f;
        return (x >= 0.0f)
            ? Vpos * tanhf(x / Vpos)
            : Vneg * tanhf(x / Vneg);
    }
};
#endif