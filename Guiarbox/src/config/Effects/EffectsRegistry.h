#pragma once
#include "EffectManager.h"

//#include "Dynamic/WahManager.h"
#include "Dynamic/AutoWahManager.h"
#include "Dynamic/NoiseGateManager.h"
#include "Dynamic/CompressorManager.h"

#include "Pitch/TransposeManager.h"
#include "Pitch/AutoWhamManager.h"

#include "Drive/OverdriveManager.h"
#include "Drive/DistortionManager.h"
#include "Drive/FuzzManager.h"

#include "Modulation/PhaserManager.h"
#include "Modulation/ChorusManager.h"
#include "Modulation/FlangerManager.h"
#include "Modulation/TremoloManager.h"

#include "Spacial/DelayManager.h"
#include "Spacial/ReverbManager.h"

#include "Cabinet/CabinetIRManager.h"

EffectCategory effectsRegistry[] = {
    {"Dynamic", {   //new WahManager(),
                    new AutoWahManager(),
                    new NoiseGateManager(),
                    new CompressorManager()}},

    {"Pitch", {     new TransposeManager(),
                    new AutoWhamManager()}},

    {"Drive", {     new OverdriveManager(),
                    new DistortionManager()}},

    {"Modulation", { new PhaserManager(),
                    new ChorusManager(),
                    new FlangerManager(),
                    new TremoloManager()}},

    {"Amp/Cab", {    new CabinetIRManager()}},

    {"Spatial", {    new DelayManager(),
                    new ReverbManager()}}
};
