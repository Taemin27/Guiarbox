#pragma once

// Custom Audio Classes
//#include "../../lib/customAudioClasses/effect_wah.h"
#include "../../lib/customAudioClasses/effect_autoWah.h"
#include "../../lib/customAudioClasses/effect_noiseGate.h"
#include "../../lib/customAudioClasses/effect_compressor.h"

#include "../../lib/customAudioClasses/effect_overdrive.h"
#include "../../lib/customAudioClasses/effect_distortion.h"
#include "../../lib/customAudioClasses/effect_fuzz_FF.h"

#include "../../lib/customAudioClasses/effect_transpose.h"
#include "../../lib/customAudioClasses/effect_autoWham.h"

#include "../../lib/customAudioClasses/effect_phaser.h"
#include "../../lib/customAudioClasses/effect_multiChorus.h"
#include "../../lib/customAudioClasses/effect_flanger.h"
#include "../../lib/customAudioClasses/effect_tremolo.h"
#include "../../lib/customAudioClasses/effect_freeverb_fp.h"
#include "../../lib/customAudioClasses/effect_cabinet_ir.h"

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=410,893
AudioEffectPhaser        phaser;         //xy=604,1191
AudioRecordQueue         recordGuitarRawQueue; //xy=610,680
AudioAmplifier           notefreqAmp;    //xy=627,787
AudioEffectCabinetIR     cabinetIR;      //xy=627,1283
AudioEffectAutoWah       autoWah;        //xy=642,913
AudioEffectOverdrive     overdrive;      //xy=661,1111
AudioEffectTranspose     transpose;      //xy=690,1001
AudioAmplifier           delayAmp;       //xy=726,1442
AudioEffectMultiChorus   chorus;         //xy=760,1191
AudioAnalyzeNoteFrequency notefreq1;      //xy=783,789
AudioEffectNoiseGate     noiseGate;      //xy=825,913
AudioMixer4              delayMixer;     //xy=830,1299
AudioEffectDistortion    distortion;     //xy=838,1111
AudioEffectDelay         delay1;         //xy=866,1443
AudioEffectAutoWham      autoWham;       //xy=869,1000
AudioEffectFlanger       flanger;        //xy=907,1191
AudioEffectCompressor    compressor;     //xy=1022,913
AudioEffectFuzzFF        fuzzFF;         //xy=1022,1112
AudioEffectFreeverbFP    freeverbFP;     //xy=1036,1292
AudioEffectTremolo       tremolo;        //xy=1062,1191
AudioRecordQueue         recordGuitarQueue; //xy=1488,1385
AudioPlaySdWav           recorderPlayWav; //xy=1802,1084
AudioPlaySdWav           backingTrackPlayWav; //xy=1816,1040
AudioSynthSimpleDrum     metronomeDrum;  //xy=1976,1144
AudioMixer4              playSDMixer;    //xy=2031,1050
AudioInputUSB            usb1;           //xy=2108,919
AudioMixer4              usbMixer;       //xy=2276,937
AudioRecordQueue         recordEverythingQueue; //xy=2308,1333
AudioMixer4              masterMixer;    //xy=2314,1191
AudioOutputI2S           i2s2;           //xy=2477,1185

AudioConnection          patchCord1(i2s1, 0, notefreqAmp, 0);
AudioConnection          patchCord2(i2s1, 0, autoWah, 0);
AudioConnection          patchCord3(i2s1, 0, recordGuitarRawQueue, 0);
AudioConnection          patchCord4(phaser, chorus);
AudioConnection          patchCord5(notefreqAmp, notefreq1);
AudioConnection          patchCord6(cabinetIR, 0, delayMixer, 0);
AudioConnection          patchCord7(autoWah, noiseGate);
AudioConnection          patchCord8(overdrive, distortion);
AudioConnection          patchCord9(transpose, autoWham);
AudioConnection          patchCord10(delayAmp, delay1);
AudioConnection          patchCord11(chorus, flanger);
AudioConnection          patchCord12(noiseGate, compressor);
AudioConnection          patchCord13(delayMixer, delayAmp);
AudioConnection          patchCord14(delayMixer, freeverbFP);
AudioConnection          patchCord15(distortion, fuzzFF);
AudioConnection          patchCord16(delay1, 0, delayMixer, 1);
AudioConnection          patchCord17(autoWham, overdrive);
AudioConnection          patchCord18(flanger, tremolo);
AudioConnection          patchCord19(compressor, transpose);
AudioConnection          patchCord20(fuzzFF, phaser);
AudioConnection          patchCord21(freeverbFP, recordGuitarQueue);
AudioConnection          patchCord22(freeverbFP, 0, masterMixer, 3);
AudioConnection          patchCord23(tremolo, cabinetIR);
AudioConnection          patchCord24(recorderPlayWav, 0, playSDMixer, 2);
AudioConnection          patchCord25(recorderPlayWav, 1, playSDMixer, 3);
AudioConnection          patchCord26(backingTrackPlayWav, 0, playSDMixer, 0);
AudioConnection          patchCord27(backingTrackPlayWav, 1, playSDMixer, 1);
AudioConnection          patchCord28(metronomeDrum, 0, masterMixer, 2);
AudioConnection          patchCord29(playSDMixer, 0, masterMixer, 1);
AudioConnection          patchCord30(usb1, 0, usbMixer, 0);
AudioConnection          patchCord31(usb1, 1, usbMixer, 1);
AudioConnection          patchCord32(usbMixer, 0, masterMixer, 0);
AudioConnection          patchCord33(masterMixer, 0, i2s2, 0);
AudioConnection          patchCord34(masterMixer, 0, i2s2, 1);
AudioConnection          patchCord35(masterMixer, recordEverythingQueue);

AudioControlSGTL5000     sgtl5000_1;     //xy=319,1625
// GUItool: end automatically generated code



