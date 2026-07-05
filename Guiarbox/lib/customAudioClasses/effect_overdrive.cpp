#include "effect_overdrive.h"


void AudioEffectOverdrive::enable() {
  enabled = true;
}
void AudioEffectOverdrive::disable() {
  enabled = false;
}
bool AudioEffectOverdrive::isEnabled() const {
  return enabled;
}

void AudioEffectOverdrive::setDrive(float drive) {
  drive = constrain(drive, 0.0f, 1.0f);

  const float t = drive * drive * drive;

  Rdrive = RdriveMin + t * (RdriveMax - RdriveMin);
}

void AudioEffectOverdrive::setTone(float tone) {
  tone = constrain(tone, 0.0f, 1.0f);
  // Log-style taper for a more musical sweep:
  // tone=0 -> RtoneMax (dark), tone=1 -> RtoneMin (bright)
  const float t = sqrtf(tone);
  Rtone = RtoneMax * powf((RtoneMin / RtoneMax), t);
}

void AudioEffectOverdrive::setLevel(float level) {
  this->level = constrain(level, 0.0f, 1.0f);
}

float AudioEffectOverdrive::processSample(float sample) {
  if (!enabled) {
    return sample;
  }

  // Input high-pass
  updateShuntCapVoltage(sample, VCin, Rin, Cin);
  const float Vin = sample - VCin;

  // Update VCg
  updateShuntCapVoltage(Vin, VCg, Rg, Cg);

  // Newton–Raphson solve for Vo
  float Vo = VoPrev;
  for (int i = 0; i < 3; i++) {
    const float x = Vo - Vin;
    float id = 0.0f;
    float did = 0.0f;
    diodePair(x, saturationCurrent, idealityFactor, thermalVoltage, id, did);
    const float f = (x / Rdrive)
                  + (Cf / dt * (x - VCf))
                  + id
                  - ((Vin - VCg) / Rg);
    const float df = (1.0f / Rdrive) + (Cf / dt) + did;
    Vo -= 0.5f * f / (df + 1e-12f);
  }

  Vo = railLimit(Vo);

  // Update states
  VoPrev = Vo;
  VCf  = Vo - Vin;
  

  // Tone control
  updateShuntCapVoltage(Vo, VCt, Rtone, Ct);

  return VCt * level;
}
