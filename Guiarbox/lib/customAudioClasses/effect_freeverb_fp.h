#ifndef effect_freeverb_fp_h_
#define effect_freeverb_fp_h_

#include <Arduino.h>
#include <AudioStream.h>

// Floating-point Freeverb with internal dry/wet mix. Large delay buffers live in RAM2 (DMAMEM).
class AudioEffectFreeverbFP : public AudioStream {
public:
    static constexpr int PREDELAY_BUFFER_SIZE = 4410; // 100 ms at 44.1 kHz
    static constexpr int COMB_BUFFER_SIZE = 1116 + 1188 + 1277 + 1356 + 1422 + 1491 + 1557 + 1617;
    static constexpr int ALLPASS_BUFFER_SIZE = 556 + 441 + 341 + 225;

    AudioEffectFreeverbFP();

    void update(void) override;

    void setDecay(float decay01);
    void setTone(float tone01);
    void setPredelayMs(float ms);
    void setDryLevel(float level01);
    void setWetLevel(float level01);
    void mute();

    void enable();
    void disable();
    bool isEnabled() const;

private:
    audio_block_t *inputQueueArray[1];
    bool enabled = false;

    struct Comb {
        const int size;
        float *const buffer;
        int index = 0;
        float filterStore = 0.0f;
        float feedback = 0.84f;
        float damp1 = 0.2f;
        float damp2 = 0.8f;

        Comb(int combSize, float *storage);

        void setFeedback(float value);
        void setDamp(float value);
        float process(float input);
        void clear();
    };

    struct Allpass {
        const int size;
        float *const buffer;
        int index = 0;

        Allpass(int apSize, float *storage);

        float process(float input);
        void clear();
    };

    float *const combStorage;
    float *const allpassStorage;
    float *const predelayBuffer;

    Comb comb1;
    Comb comb2;
    Comb comb3;
    Comb comb4;
    Comb comb5;
    Comb comb6;
    Comb comb7;
    Comb comb8;

    Allpass allpass1;
    Allpass allpass2;
    Allpass allpass3;
    Allpass allpass4;

    uint32_t predelayWriteIndex = 0;
    float predelaySamples = 0.0f;
    float dryGain = 1.0f;
    float wetGain = 0.0f;

    static float flushDenormal(float value);
    float readPredelay() const;
    void writePredelay(float sample);
};

#endif
