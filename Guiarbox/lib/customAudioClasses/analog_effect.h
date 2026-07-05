#ifndef analog_effect_h_
#define analog_effect_h_

#include <Arduino.h>
#include "AudioStream.h"
#include "wave_lut.h"
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

    // Antiparallel diode pair: Id = 2·Is·sinh(v/nVt); uses WAVE_LUT_SINH/COSH.
    static inline void diodePair(float v, float saturationCurrent, float idealityFactor,
                                 float thermalVoltage, float &id, float &did) {
        const float invNv = 1.0f / (idealityFactor * thermalVoltage);
        const float x = v * invNv;
        const float s = waveLutSample(WAVE_LUT_SINH, x);
        const float c = waveLutSample(WAVE_LUT_COSH, x);
        const float scale = 2.0f * saturationCurrent;
        id = scale * s;
        did = scale * invNv * c;
    }
    static inline float Id(float v, float saturationCurrent, float idealityFactor, float thermalVoltage) {
        float id = 0.0f;
        float did = 0.0f;
        diodePair(v, saturationCurrent, idealityFactor, thermalVoltage, id, did);
        return id;
    }

    static inline float dId(float v, float saturationCurrent, float idealityFactor, float thermalVoltage) {
        float id = 0.0f;
        float did = 0.0f;
        diodePair(v, saturationCurrent, idealityFactor, thermalVoltage, id, did);
        return did;
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
            ? Vpos * waveLutSample(WAVE_LUT_TANH, x / Vpos)
            : Vneg * waveLutSample(WAVE_LUT_TANH, x / Vneg);
    }
};
#endif