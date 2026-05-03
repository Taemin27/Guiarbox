#ifndef effect_fuzzFF_h_
#define effect_fuzzFF_h_

#include <Arduino.h>
#include "analog_effect.h"


class AudioEffectFuzzFF : public AudioAnalogEffect {
public:
    void setLevel(float level);
    void setFuzz(float fuzz);

    float processSample(float sample) {
        return sample;
    }

    void enable();
    void disable();
    bool isEnabled() const;

private:
    /* Schematics:
                    9v ────┬─────┬── C10nf         
                     │     │     │           
                   R33k  R8.2k   │           
                     │     │     │           
                     ├─┐   │     │           
    In - C2.2uf ─┬─ Q1 └─ Q2   R500kA ◄── Out
                 │   │     │     │           
                 │   ▼     ▼     │           
                 │  GND    │    GND          
                 │         │                 
              R100k ───────┤                 
                           │                 
                         R1kB ◄──────┐       
                           │         │       
                     GND ──┴─ C22uf ─┘       
    */

    static constexpr float C2_2uf = 2.2e-6f;
    static constexpr float R33k = 33e3f;
    static constexpr float R8_2k = 8.2e3f;
    static constexpr float R500kA = 500e3f;
    static constexpr float R100k = 100e3f;
    static constexpr float R1kB = 1e3f;
    static constexpr float C22uf = 22e-6f;


    
};

#endif

