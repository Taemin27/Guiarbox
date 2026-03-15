#include "effect_overdrive.h"

void AudioEffectOverdrive::setDrive(float drive) {
  drive = constrain(drive, 0.0f, 1.0f);

  const float t = drive * drive * drive;

  Rdrive = RdriveMin + t * (RdriveMax - RdriveMin);
}

void AudioEffectOverdrive::setTone(float tone) {
  tone = constrain(tone, 0.0f, 1.0f);
  Rtone = RtoneMax - tone * (RtoneMax - RtoneMin);
}

void AudioEffectOverdrive::setLevel(float level) {
  this->level = constrain(level, 0.0f, 1.0f);
}

// KCL at the inverting input node of the op-amp
inline float AudioEffectOverdrive::kcl(float Vo, float Vin, float Vx) const {
  const float x = Vo - Vin;
  return  (x / Rdrive) 
          + (Cf / dt * (x - VCf)) 
          + Id(x, saturationCurrent, idealityFactor, thermalVoltage) 
          - ((Vin - Vx) / Rg);
}
// Derivative of KCL equation wrt Vo
inline float AudioEffectOverdrive::kclDerivative(float Vo, float Vin) const {
  const float x = Vo - Vin;
  return  (1.0f / Rdrive) 
          + (Cf / dt) 
          + dId(x, saturationCurrent, idealityFactor, thermalVoltage);
}

float AudioEffectOverdrive::processSample(float sample) {
  // Input high-pass
  updateShuntCapVoltage(sample, VCin, Rin, Cin);
  const float Vin = sample - VCin;

  // Update VCg
  updateShuntCapVoltage(Vin, VCg, Rg, Cg);

  // Newton–Raphson solve for Vo
  float Vo = VoPrev;
  for (int i = 0; i < 5; i++) {
    const float f  = kcl(Vo, Vin, VCg);
    const float df = kclDerivative(Vo, Vin);
    Vo -= 0.5f *f / (df + 1e-12f);
  }
  
  // Update states
  VoPrev = Vo;
  VCf  = Vo - Vin;
  

  // Tone control
  updateShuntCapVoltage(Vo, VCt, Rtone, Ct);

  return VCt * level;
}
