#ifndef effect_flanger_h_
#define effect_flanger_h_

#include <Arduino.h>
#include <AudioStream.h>

/**
 * Flanger effect (short modulated delay with feedback).
 *
 * Controls:
 * - rate: LFO rate in Hz
 * - depth: modulation depth (0..1)
 * - manual: base delay position (0..1)
 * - feedback: feedback amount (-1..1). Negative feedback gives a different "hollow" character.
 * - mix: dry/wet mix (0..1). 0=dry, 1=wet
 */
class AudioEffectFlanger : public AudioStream {
public:
    AudioEffectFlanger();

    void update(void) override;

    void setRate(float hz);
    void setDepth(float depth01);
    void setManual(float manual01);
    void setFeedback(float feedbackSigned1);
    void setMix(float mix01);

    void enable();
    void disable();
    bool isEnabled() const { return enabled; }

private:
    audio_block_t *inputQueueArray[1];

    bool enabled = false;

    float rateHz = 0.25f;
    float depth = 0.75f;   // 0..1
    float manual = 0.35f;  // 0..1
    float feedback = 0.35f; // -1..1
    float mix = 0.50f;     // 0..1

    float lfoPhase = 0.0f; // radians [0..2pi)

    static constexpr int DELAY_BUFFER_SAMPLES = 2048; // power-of-two for fast wrap
    float delayBuffer[DELAY_BUFFER_SAMPLES];
    uint32_t writeIndex = 0;

    float readDelaySamples(float delaySamples) const;
    void writeDelaySample(float x);
};

#endif

