#ifndef effect_tremolo_h_
#define effect_tremolo_h_

#include <Arduino.h>
#include <AudioStream.h>

/**
 * Tremolo (amplitude modulation) effect.
 *
 * Controls:
 * - rate: LFO rate in Hz
 * - depth: modulation depth (0..1). 0 = unity gain, 1 = full modulation to silence at LFO minimum
 * - shape: sine / triangle / square
 * - bias: skews the LFO time distribution (-1..1).
 *         positive => more time "loud", negative => more time "quiet"
 */
class AudioEffectTremolo : public AudioStream {
public:
    enum class Shape : uint8_t {
        Sine = 0,
        Tri = 1,
        Square = 2,
    };

    AudioEffectTremolo();

    void update(void) override;

    void setRate(float hz);
    void setDepth(float depth01);
    void setShape(Shape s);
    void setBias(float biasSigned01);

    void enable();
    void disable();
    bool isEnabled() const { return enabled; }

private:
    audio_block_t *inputQueueArray[1];

    bool enabled = false;

    float rateHz = 4.0f;
    float depth = 0.65f;
    Shape shape = Shape::Sine;
    float bias = 0.0f; // [-1..1]

    float lfoPhase = 0.0f; // radians [0..2pi)
    float squareLfoState = 0.0f; // smoothed 0..1 state for square shape

    float lfoUnipolar01(float phaseRad) const; // 0..1
    float applyBias01(float x01) const;        // 0..1
};

#endif

