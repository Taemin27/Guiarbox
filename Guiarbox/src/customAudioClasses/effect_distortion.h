#ifndef effect_distortion_h_
#define effect_distortion_h_

#include "analog_effect.h"

class AudioEffectDistortion : public AudioAnalogEffect {
public:
    AudioEffectDistortion() : AudioAnalogEffect() {
        Rdrive = RdriveMin;
        Rtone = RtoneMax;
        level = 1.0f;
    }

    float processSample(float sample) override;

    void setDrive(float drive);
    void setTone(float tone);
    void setLevel(float level);

private:
    /* Schematics 

                                                         GND
                                                          │
    Signal ─── Cin───┬──────── + ┌──────┐                 └─ + ┌──────┐                                 ┌─────┐
                     │           │OP Amp├─┐                    │OP Amp├─┬── Cc ── Rc ─┬─┬─── Rtone ─┬───│Level├─── Out
                    Rin        - └──────┘ ├── Ci ── Ri ───── - └──────┘ │             ▼ _           │   └─────┘
                     │         │          │                  │          │             ─ ▲          Ct
                    GND        ├─── Cf1───┤                  ├─── Cf2───┤             └┬┘           │
                     │         │          │                  │          │              │            │
                    Cg ── Rg ──┴─ Rdrive ─┘                  └─── Rf ───┘             GND          GND
                                                                  
    */

    float Rdrive;
    float Rtone;
    float level;

    // State variables
    float Vo1Prev = 0.0f;
    float VclipPrev = 0.0f;

    float VCin = 0.0f;  // Voltage across Cin
    float VCi = 0.0f;    // Voltage across Ci
    float VCf1 = 0.0f;   // Voltage across Cf1
    float VCf2 = 0.0f;   // Voltage across Cf2

    float VCg = 0.0f;   // Voltage across Cg

    float VCc = 0.0f;   // Voltage across Cc

    float VCt = 0.0f;   // Voltage across Ct
    

    // ==================== Parameters ====================
    static constexpr float thermalVoltage = 0.025f;
    static constexpr float saturationCurrent = 1.7e-9f;
    static constexpr float idealityFactor = 1.2f;

    // Input high-pass
    static constexpr float Rin = 1.0e6f;            // 1M Ohm resistor at input for high-pass
    static constexpr float Cin = 4.7e-9f;           // 4.7nF capacitor at input for high-pass
    
    // First gain stage
    static constexpr float Cf1 = 100e-12f;          // 100pF feedback capacitor
    static constexpr float Rg = 1000.0f;              // 1k Ohm resistor at inverting input node
    static constexpr float Cg = 220e-9f;            // 220nF capacitor to groud
    static constexpr float RdriveMin = 1000.0f;
    static constexpr float RdriveMax = 150000.0f;


    // Interstage coupling
    static constexpr float Ci = 100e-9f;            // 100nF capacitor between gain stages
    static constexpr float Ri = 10000.0f;           // 10k Ohm resistor between gain stages

    // Second gain stage
    static constexpr float Cf2 = 100e-12f;          // 100pF feedback capacitor
    static constexpr float Rf = 1e6f;               // 1M Ohm feedback resistor

    // Before clipping
    static constexpr float Cc = 470e-9f;            // 470nF capacitor before clipping stage
    static constexpr float Rc = 47.0f;            // 47 Ohm resistor before clipping stage

    // Tone control
    static constexpr float Ct = 10e-9f;              // 10nF tone capacitor shunt to ground
    static constexpr float RtoneMin = 5000.0f;
    static constexpr float RtoneMax = 10000.0f;


    // ==================== Methods ====================
    inline float solveGainStage1(float Vinv);
    inline float solveGainStage2(float Vin);

    inline float solveClipStage(float Vo2);

    inline float clipStageKcl(float Vc, float Vo) const;
    inline float clipStageKclDerivative(float Vc) const;

};

#endif
