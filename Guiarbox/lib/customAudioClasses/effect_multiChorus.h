#ifndef effect_multiChorus_h_
#define effect_multiChorus_h_

#include <Arduino.h>
#include <AudioStream.h>

/**
 * Multi-voice modulated-delay chorus.
 *
 * Implementation notes:
 * - Each voice is a short delay tap swept by a sine LFO.
 * - Delay time is kept in an absolute range [minDelayMs..maxDelayMs] to avoid
 *   hitting the <1-sample clamp (which can cause audible "snaps").
 * - Per-voice tables provide slight deterministic spread (no randomness).
 * - Phases are evenly spaced for the active voice count.
 * - Dry stays at unity; wet is scaled by wet level.
 */
class AudioEffectMultiChorus : public AudioStream {
public:
    static constexpr int MAX_VOICES = 5;

    AudioEffectMultiChorus();

    void update(void) override;

    void setRate(float hz);
    // Sets the maximum delay time in milliseconds for the LFO sweep.
    // Minimum delay is fixed internally to keep it chorus-like (no flanging).
    void setDepthMs(float maxDelayMs);
    // Normalized depth (0..1) mapped to [minDelayMs .. maxDelayMs] for the sweep maximum.
    void setDepth(float depth01);
    void setWetLevel(float level01);
    void setVoices(int count);

    void enable();
    void disable();
    bool isEnabled() const { return enabled; }

private:
    audio_block_t *inputQueueArray[1];

    static constexpr uint32_t BUFFER_SIZE = 4096;
    static constexpr uint32_t BUFFER_MASK = BUFFER_SIZE - 1;

    bool enabled = false;

    float rateHz = 0.8f;
    float maxDelayMs = 20.0f;
    float wetLevel = 0.35f;
    int voiceCount = 3;

    float delayLine[BUFFER_SIZE];
    uint32_t writeIndex = 0;

    float lfoPhase[MAX_VOICES];

    void redistributePhases();
    float readInterpolated(float delaySamples, uint32_t writeRef) const;
};

#endif
