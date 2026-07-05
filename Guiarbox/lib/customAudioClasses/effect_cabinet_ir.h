#ifndef effect_cabinet_ir_h_
#define effect_cabinet_ir_h_

#include <Arduino.h>
#include <AudioStream.h>

// Partitioned FFT convolution for cabinet IRs (20 ms @ 44.1 kHz).
class AudioEffectCabinetIR : public AudioStream {
public:
    static constexpr int PARTITION_SIZE = 128;
    static constexpr int FFT_SIZE = 256;
    static constexpr int MAX_IR_SAMPLES = 882; // 20 ms @ 44.1 kHz
    static constexpr int MAX_PARTITIONS =
        (MAX_IR_SAMPLES + PARTITION_SIZE - 1) / PARTITION_SIZE;

    AudioEffectCabinetIR();

    void update(void) override;

    bool loadImpulse(const float* samples, int length);
    void clearImpulse();

    void enable();
    void disable();
    bool isEnabled() const { return enabled; }

    void setLevel(float level);
    void setBass(float amount);    // 0..1, 0.5 = flat
    void setMid(float amount);
    void setTreble(float amount);

    static float* loadScratchBuffer();
    static int loadScratchBufferSamples();

private:
    audio_block_t *inputQueueArray[1];
    bool enabled = false;
    float levelGain = 0.5f;
    int irLength = 0;
    int numPartitions = 0;
    int ringIndex = 0;

    float (*const irFreq)[FFT_SIZE];
    float (*const inputFreqRing)[FFT_SIZE];
    float *const overlap;
    float *const workA;

    void rebuildIrPartitions(const float* samples, int length);
    void resetProcessingState();
    void updateEqCoeffs();
    float processEq(float x) const;

    float bassAmount = 0.5f;
    float midAmount = 0.5f;
    float trebleAmount = 0.5f;
    bool eqBypass = true;

    float eqB[3][3] = {};
    float eqA[3][2] = {};
    mutable float eqZ[3][4] = {};
};

#endif
