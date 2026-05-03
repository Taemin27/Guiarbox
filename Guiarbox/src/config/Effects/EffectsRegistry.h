#pragma once
#include "EffectManager.h"

//#include "Dynamic/WahManager.h"
#include "Dynamic/AutoWahManager.h"
#include "Dynamic/NoiseGateManager.h"
#include "Dynamic/CompressorManager.h"

#include "Drive/OverdriveManager.h"
#include "Drive/DistortionManager.h"
#include "Drive/FuzzManager.h"

#include "Pitch/PitchShifterManager.h"

#include "Modulation/PhaserManager.h"
#include "Modulation/ChorusManager.h"
#include "Modulation/FlangerManager.h"
#include "Modulation/TremoloManager.h"

#include "Spacial/DelayManager.h"
#include "Spacial/ReverbManager.h"

EffectCategory effectsRegistry[] = {
    {"Dynamic", {   //new WahManager(),
                    new AutoWahManager(),
                    new NoiseGateManager(),
                    new CompressorManager()}},

    {"Drive", {     new OverdriveManager(), 
                    new DistortionManager()}},

    {"Pitch", {     new PitchShifterManager()}},
    
    {"Modulation", { new PhaserManager(),
                    new ChorusManager(),
                    new FlangerManager(),
                    new TremoloManager()}},

    {"Spacial", {   new DelayManager(),
                    new ReverbManager()}}
};
