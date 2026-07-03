#include "effect_autoWah.h"
#include <math.h>

namespace {

constexpr float DEFAULT_SAMPLE_RATE = 44100.0f;
constexpr float LN_SETTLE_RATIO = 2.99573227355f;

constexpr float MIN_FREQ_HZ = 30.0f;
constexpr float MIN_Q = 0.3f;
constexpr float MAX_Q = 20.0f;

constexpr float ATTACK_MS_MIN = 0.0f;
constexpr float ATTACK_MS_MAX = 500.0f;
constexpr float RELEASE_MS_MIN = 0.0f;
constexpr float RELEASE_MS_MAX = 500.0f;

inline float clamp01(float x) { return constrain(x, 0.0f, 1.0f); }

} // namespace

AudioEffectAutoWah::AudioEffectAutoWah() : AudioStream(1, inputQueueArray) {
    setSensitivity(sensitivity);
    setQ(q);
    setRange(lowHz, highHz);
    setAttackMs(attackMs);
    setReleaseMs(releaseMs);
}

void AudioEffectAutoWah::enable() {
    if (!enabled) {
        env = 0.0f;
        ctrl = 0.0f;
        x1 = x2 = y1 = y2 = 0.0f;
        lastFcHz = -1.0f;
        enabled = true;
    }
}

void AudioEffectAutoWah::disable() {
    enabled = false;
}

void AudioEffectAutoWah::updateEnvelopeCoeffs() {
    if (attackMs <= 0.0f) {
        attackAlpha = 1.0f;
    } else {
        attackAlpha = 1.0f - expf(-LN_SETTLE_RATIO / (attackMs * 0.001f * DEFAULT_SAMPLE_RATE));
    }
    if (releaseMs <= 0.0f) {
        releaseAlpha = 1.0f;
    } else {
        releaseAlpha = 1.0f - expf(-LN_SETTLE_RATIO / (releaseMs * 0.001f * DEFAULT_SAMPLE_RATE));
    }
}

void AudioEffectAutoWah::setLowFreq(float hz) {
    lowHz = max(MIN_FREQ_HZ, hz);
    if (highHz < lowHz) {
        highHz = lowHz;
    }
}

void AudioEffectAutoWah::setHighFreq(float hz) {
    highHz = max(MIN_FREQ_HZ, hz);
    if (lowHz > highHz) {
        lowHz = highHz;
    }
}

void AudioEffectAutoWah::setRange(float lowHz, float highHz) {
    setLowFreq(lowHz);
    setHighFreq(highHz);
}

void AudioEffectAutoWah::setSensitivity(float amount01) {
    sensitivity = clamp01(amount01);
}

void AudioEffectAutoWah::setAttackMs(float ms) {
    attackMs = constrain(ms, ATTACK_MS_MIN, ATTACK_MS_MAX);
    updateEnvelopeCoeffs();
}

void AudioEffectAutoWah::setReleaseMs(float ms) {
    releaseMs = constrain(ms, RELEASE_MS_MIN, RELEASE_MS_MAX);
    updateEnvelopeCoeffs();
}

void AudioEffectAutoWah::setQ(float qIn) {
    q = constrain(qIn, MIN_Q, MAX_Q);
}

void AudioEffectAutoWah::setDirection(Direction dir) {
    direction = dir;
}

float AudioEffectAutoWah::mapDirection(float u01) const {
    const float u = clamp01(u01);
    switch (direction) {
        case Direction::Up:
            return u;
        case Direction::Down:
            return 1.0f - u;
        case Direction::UpDown: {
            // Low @ u=0, high @ u=0.5, low @ u=1 (triangle).
            const float tri = 1.0f - fabsf(2.0f * u - 1.0f);
            return clamp01(tri);
        }
        default:
            return u;
    }
}

float AudioEffectAutoWah::computeFcHz(float u01) const {
    const float lo = max(lowHz, MIN_FREQ_HZ);
    const float hi = max(highHz, lo + 1.0f);

    // Exponential mapping feels more "musical" than linear Hz.
    const float ratio = hi / lo;
    const float u = mapDirection(u01);
    return lo * powf(ratio, u);
}

void AudioEffectAutoWah::updateBiquad(float fcHz, float sampleRate) {
    const float sr = max(sampleRate, 1.0f);
    float fc = fcHz;
    if (fc < MIN_FREQ_HZ) {
        fc = MIN_FREQ_HZ;
    } else if (fc > 0.45f * sr) {
        fc = 0.45f * sr;
    }

    const float qClamped = constrain(q, MIN_Q, MAX_Q);

    // RBJ cookbook band-pass (constant skirt gain, peak gain = Q).
    const float w0 = TWO_PI * fc / sr;
    const float cw = cosf(w0);
    const float sw = sinf(w0);
    const float alpha = sw / (2.0f * qClamped);

    const float bb0 = alpha;
    const float bb1 = 0.0f;
    const float bb2 = -alpha;
    const float aa0 = 1.0f + alpha;
    const float aa1 = -2.0f * cw;
    const float aa2 = 1.0f - alpha;

    const float invA0 = 1.0f / aa0;
    b0 = bb0 * invA0;
    b1 = bb1 * invA0;
    b2 = bb2 * invA0;
    a1 = aa1 * invA0;
    a2 = aa2 * invA0;
}

float AudioEffectAutoWah::processBiquad(float x) {
    const float y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
    x2 = x1;
    x1 = x;
    y2 = y1;
    y1 = y;
    return y;
}

void AudioEffectAutoWah::update(void) {
    audio_block_t *block = receiveWritable(0);
    if (!block) {
        return;
    }

    if (!enabled) {
        transmit(block);
        release(block);
        return;
    }

    const float sr = AUDIO_SAMPLE_RATE_EXACT;

    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i) {
        const float x = (float)block->data[i] * (1.0f / 32768.0f);
        const float a = fabsf(x);

        if (a > env) {
            env += (a - env) * attackAlpha;
        } else {
            env += (a - env) * releaseAlpha;
        }

        const float sens = clamp01(sensitivity);
        const float envGain = 0.5f + 63.5f * (sens * sens);
        const float target = clamp01(env * envGain);

        ctrl += (target - ctrl) * (0.35f * releaseAlpha);

        const float fc = computeFcHz(ctrl);
        if (lastFcHz < 0.0f || fabsf(fc - lastFcHz) > 0.01f * lastFcHz) {
            updateBiquad(fc, sr);
            lastFcHz = fc;
        }

        float y = sinhf(processBiquad(x) * 3.0f);

        block->data[i] = (int16_t)(y * 32767.0f);
    }

    transmit(block);
    release(block);
}
