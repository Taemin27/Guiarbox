#ifndef effect_overdrive_h_
#define effect_overdrive_h_

#include "analog_effect.h"

class AudioEffectOverdrive :public AudioAnalogEffect {
public:
  AudioEffectOverdrive() : AudioAnalogEffect() {
    Rdrive  = RdriveMin;
    Rtone = RtoneMax;
    level = 1.0f;
  }

  void setDrive(float drive);

  void setTone(float tone);

  void setLevel(float level);

  float processSample(float sample) override;

private:
  /* Schematics:

  Signal ───── Cin ───────┬────── + ┌──────┐                   ┌─────┐
                          │         │OP Amp├─┬──── Rtone ─┬────│Level├─── Out
                         Rin      - └──────┘ │            │    └─────┘
                          │       │          │            │
                          │       ├─── Cf ───┤           Ct
                         GND      │          │            │
                          │       ├────►|────┤            │
                          │       ├────|◄────┤           GND
                          │       │          │
                         Cg ─ Rg ─┴─ Rdrive ─┘

  */

  // Controls
  float Rdrive;       // Variable resistor in feedback loop for drive control
  float Rtone;        // Variable resistor for tone control
  float level;       // Variable resistor for output level

  // State variables
  float VoPrev = 0.0f; // Vo as in Vout(Node between op amp and Rtone). Starting point for Newton–Raphson solve

  float VCin = 0.0f;  // Voltage across Cin
  float VCf = 0.0f;   // Voltage across Cf
  float VCg = 0.0f;   // Voltage across Cg
  float VCt = 0.0f;   // Voltage across Ct

  // Potentiometer limits
  static constexpr float RdriveMin = 20000.0f;
  static constexpr float RdriveMax = 500000.0f;

  static constexpr float RtoneMin = 5000.0f;
  static constexpr float RtoneMax = 10000.0f;

  // Constants
  static constexpr float Rin = 1.0e6f;  // 1M Ohm resistor at input for high-pass
  static constexpr float Cin = 470e-9f; // 470nF Input capacitor at input for high-pass

  static constexpr float thermalVoltage = 0.025f;
  static constexpr float saturationCurrent = 2.52e-9f;
  static constexpr float idealityFactor = 1.4f;

  static constexpr float Cf = 51e-12f;     // 51pF Capacitor in feedback loop, parallel to the diodes

  static constexpr float Rg = 4000.0f;      // 2k Ohm Resistor to Cg at inverting input node
  static constexpr float Cg = 47e-9f;      // 47nF Capacitor to ground at Rg node

  static constexpr float Ct = 22e-9f;     // 22nF Tone capacitor shunt to ground

  inline float kcl(float Vo, float Vin, float Vx) const;
  inline float kclDerivative(float Vo, float Vin) const;
};

#endif
