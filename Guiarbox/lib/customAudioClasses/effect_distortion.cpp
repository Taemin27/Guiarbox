#include "effect_distortion.h"


void AudioEffectDistortion::enable() {
  enabled = true;
}
void AudioEffectDistortion::disable() {
  enabled = false;
}
bool AudioEffectDistortion::isEnabled() const {
  return enabled;
}

void AudioEffectDistortion::setDrive(float drive) {
  drive = constrain(drive, 0.0f, 1.0f);

  const float t = drive * drive * drive;

  Rdrive = RdriveMin + t * (RdriveMax - RdriveMin);
}
void AudioEffectDistortion::setTone(float tone) {
  tone = constrain(tone, 0.0f, 1.0f);
  // Use a log-style taper for a more musical sweep:
  // tone=0 -> RtoneMax (dark), tone=1 -> RtoneMin (bright)
  const float t = sqrtf(tone);
  Rtone = RtoneMax * powf((RtoneMin / RtoneMax), t);
}
void AudioEffectDistortion::setLevel(float level) {
  level = constrain(level, 0.0f, 1.0f);
  this->level = level * level / 10.0f;
}

inline float AudioEffectDistortion::solveGainStage1(float Vinv) {
  /*
  KCL at the inverting input node:
  (Vo - Vinv)/Rdrive + Cf1(d/dt)(Vo - Vinv) - (Vinv - VCg)/Rg = 0
  (Vinv - VCg)/Rg = Cg(d/dt)VCg

  Solve for Vo every sample with given Vinv
  */

  updateShuntCapVoltage(Vinv, VCg, Rg, Cg);

  /*
  Let x = Vo - Vinv
  VCf1 is the voltage across Cf1 from last sample
  Approximate Cf1(d/dt)x as Cf1/dt * (x[k] - x[k - 1]), where x[k - 1] = VCf1

  Then KCL becomes:
  x/Rdrive + (Cf1/dt)(x) = (Cf1/dt)(VCf1) + (Vinv - VCg)/Rg

  Solve for x:
  x(1/Rdrive + Cf1/dt) = (Cf1/dt)(VCf) + (Vinv - VCg)/Rg
  x = [ (Cf1/dt)(VCf1) + (Vinv - VCg)/Rg ] / [1/Rdrive + Cf1/dt]
  */

  const float x = ( (Cf1 / dt) * VCf1 + (Vinv - VCg) / Rg ) 
                  / ( (1.0f / Rdrive) + (Cf1 / dt) );

  // Update VCf1 for next sample
  VCf1 = x;
  // Return Vo
  return railLimit(Vinv + x);
}

inline float AudioEffectDistortion::solveGainStage2(float Vin) {
  /*
  KCL at the inverting input node:
  Vin/Ri + Vo/Rf + Cf2(d/dt)Vo = 0

  Solve for Vo every sample with given Vin
  */

  /*
  VCf2 is the voltage across Cf2 from last sample
  Approximate Cf2(d/dt)Vo as:
  Cf2/dt * (Vo[k] - Vo[k - 1]), where Vo[k - 1] = VCf2

  Then KCL becomes:
  Vin/Ri + Vo/Rf + (Cf2/dt)(Vo - VCf2) = 0

  Solve for Vo:
  Vo = [ (Cf2/dt)(VCf2) - Vin/Ri ] / [ 1/Rf + Cf2/dt ]
  */

  const float Vo = ( (Cf2 / dt) * VCf2 - (Vin / Ri) )
                   / ( (1.0f / Rf) + (Cf2 / dt) );

  // Update VCf2 for next sample
  VCf2 = Vo;
  // Return Vo
  return railLimit(Vo);
}

inline float AudioEffectDistortion::solveClipStage(float Vo2) {

  // Vo2 --> Cc --> (Va) --> Rc --> (Vc) --> diodes --> GND

  const float alpha = Cc / dt;
  const float invRc = 1.0f / Rc;
  const float D = invRc + alpha;

  float Vc = VclipPrev;

  for (int i = 0; i < 6; ++i) {
    const float a = invRc / D;
    const float Va = a * Vc + (alpha * (Vo2 - VCc)) / D;

    const float f  = (Va - Vc) * invRc
                    - Id(Vc, saturationCurrent, idealityFactor, thermalVoltage);

    const float df = (a - 1.0f) * invRc
                    - dId(Vc, saturationCurrent, idealityFactor, thermalVoltage);

    Vc -= f / (df + 1e-12f);
  }

  VCc = Vo2 - ( (invRc / D) * Vc
                + (alpha * (Vo2 - VCc)) / D );

  VclipPrev = Vc;

  Serial.println(Vc);
  return Vc;
}

inline float AudioEffectDistortion::clipStageKcl(float Vc, float Vo2) const {
  return (Vo2 - Vc) / Rc
        - Id(Vc, saturationCurrent, idealityFactor, thermalVoltage);
}

inline float AudioEffectDistortion::clipStageKclDerivative(float Vc) const {
  return -(1.0f / Rc)
        - dId(Vc, saturationCurrent, idealityFactor, thermalVoltage);
}

float AudioEffectDistortion::processSample(float sample) {
  if (!enabled) {
    return sample;
  }

  // Input high-pass
  updateShuntCapVoltage(sample, VCin, Rin, Cin);
  float Vin1 = sample - VCin;

  // First gain stage
  const float Vo1 = solveGainStage1(Vin1);

  // Interstage high-pass
  updateShuntCapVoltage(Vo1, VCi, Ri, Ci);
  const float Vin2 = Vo1 - VCi;

  // Second gain stage
  float Vo2 = solveGainStage2(Vin2);

  // Clipping stage
  float Vc = solveClipStage(Vo2);

  // Tone control
  updateShuntCapVoltage(Vc, VCt, Rtone, Ct);

  // Output level
  return VCt * level;
}