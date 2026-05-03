#ifndef effect_autoWah_h_
#define effect_autoWah_h_

#include <Arduino.h>
#include <AudioStream.h>

/**
 * Envelope-controlled band-pass "auto wah".
 *
 * Controls:
 * - low/high: sweep range for band-pass center frequency (Hz)
 * - sensitivity: how strongly input level drives the sweep (0..1)
 * - attack/release: envelope response (ms)
 * - Q: band-pass resonance (roughly 0.3..20)
 * - direction: mapping shape (up, down, up-down)
 */
class AudioEffectAutoWah : public AudioStream {
public:
    enum class Direction : uint8_t {
        Up = 0,
        Down = 1,
        UpDown = 2,
    };

    AudioEffectAutoWah();

    void update(void) override;

    void setLowFreq(float hz);
    void setHighFreq(float hz);
    void setRange(float lowHz, float highHz);

    void setSensitivity(float amount01);
    void setAttackMs(float ms);
    void setReleaseMs(float ms);
    void setQ(float q);
    void setDirection(Direction dir);

    void enable();
    void disable();
    bool isEnabled() const { return enabled; }

private:
    audio_block_t *inputQueueArray[1];

    bool enabled = false;

    float lowHz = 300.0f;
    float highHz = 2200.0f;
    float sensitivity = 0.65f; // 0..1
    float q = 3.0f;
    Direction direction = Direction::Up;

    // Envelope follower state.
    float env = 0.0f;
    float attackAlpha = 1.0f;
    float releaseAlpha = 1.0f;
    float attackMs = 8.0f;
    float releaseMs = 140.0f;

    // Smoothed control (post-sensitivity), 0..1.
    float ctrl = 0.0f;

    // Biquad band-pass state (Direct Form I).
    float b0 = 0.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f;
    float x1 = 0.0f, x2 = 0.0f, y1 = 0.0f, y2 = 0.0f;

    float lastFcHz = -1.0f;

    static float coeffFromMs(float ms, float sampleRate);
    void updateEnvelopeCoeffs();

    float mapDirection(float u01) const;
    float computeFcHz(float u01) const;
    void updateBiquad(float fcHz, float sampleRate);
    float processBiquad(float x);
};

#endif

