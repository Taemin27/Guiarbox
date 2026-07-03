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

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=459,1073
AudioAmplifier           delayAmp;       //xy=563,1647
AudioEffectPhaser        phaser;         //xy=653,1371
AudioRecordQueue         recordGuitarRawQueue; //xy=659,860
AudioMixer4              delayMixer;     //xy=662,1499
AudioAmplifier           notefreqAmp;    //xy=676,967
AudioEffectAutoWah       autoWah;        //xy=691,1093
AudioEffectDelay         delay1;         //xy=703,1648
AudioEffectOverdrive     overdrive;      //xy=710,1291
AudioEffectTranspose     transpose;    //xy=739,1181
AudioEffectMultiChorus   chorus;         //xy=809,1371
AudioAnalyzeNoteFrequency notefreq1;      //xy=832,969
AudioEffectNoiseGate     noiseGate;      //xy=874,1093
AudioEffectDistortion    distortion;     //xy=887,1291
AudioEffectAutoWham      autoWham;       //xy=918,1180
AudioEffectFlanger       flanger;        //xy=956,1371
AudioEffectFreeverbFP    freeverbFP;    //xy=981,1503
AudioEffectCompressor    compressor;     //xy=1071,1093
AudioEffectFuzzFF        fuzzFF;         //xy=1071,1292
AudioEffectTremolo       tremolo;        //xy=1111,1371
AudioRecordQueue         recordGuitarQueue; //xy=1209,1583
AudioPlaySdWav           recorderPlayWav; //xy=1851,1264
AudioPlaySdWav           backingTrackPlayWav; //xy=1865,1220
AudioSynthSimpleDrum     metronomeDrum;  //xy=2025,1324
AudioMixer4              playSDMixer;    //xy=2080,1230
AudioInputUSB            usb1;           //xy=2157,1099
AudioMixer4              usbMixer;       //xy=2325,1117
AudioRecordQueue         recordEverythingQueue; //xy=2357,1513
AudioMixer4              masterMixer;    //xy=2363,1371
AudioOutputI2S           i2s2;           //xy=2526,1365

AudioConnection          patchCord1(i2s1, 0, notefreqAmp, 0);
AudioConnection          patchCord2(i2s1, 0, autoWah, 0);
AudioConnection          patchCord3(i2s1, 0, recordGuitarRawQueue, 0);
AudioConnection          patchCord4(delayAmp, delay1);
AudioConnection          patchCord5(phaser, chorus);
AudioConnection          patchCord6(delayMixer, delayAmp);
AudioConnection          patchCord7(delayMixer, freeverbFP);
AudioConnection          patchCord8(notefreqAmp, notefreq1);
AudioConnection          patchCord9(autoWah, noiseGate);
AudioConnection          patchCord10(delay1, 0, delayMixer, 1);
AudioConnection          patchCord11(overdrive, distortion);
AudioConnection          patchCord12(transpose, autoWham);
AudioConnection          patchCord13(chorus, flanger);
AudioConnection          patchCord14(noiseGate, compressor);
AudioConnection          patchCord15(distortion, fuzzFF);
AudioConnection          patchCord16(autoWham, overdrive);
AudioConnection          patchCord17(flanger, tremolo);
AudioConnection          patchCord18(freeverbFP, 0, masterMixer, 3);
AudioConnection          patchCord19(freeverbFP, recordGuitarQueue);
AudioConnection          patchCord20(compressor, transpose);
AudioConnection          patchCord21(fuzzFF, phaser);
AudioConnection          patchCord22(tremolo, 0, delayMixer, 0);
AudioConnection          patchCord23(recorderPlayWav, 0, playSDMixer, 2);
AudioConnection          patchCord24(recorderPlayWav, 1, playSDMixer, 3);
AudioConnection          patchCord25(backingTrackPlayWav, 0, playSDMixer, 0);
AudioConnection          patchCord26(backingTrackPlayWav, 1, playSDMixer, 1);
AudioConnection          patchCord27(metronomeDrum, 0, masterMixer, 2);
AudioConnection          patchCord28(playSDMixer, 0, masterMixer, 1);
AudioConnection          patchCord29(usb1, 0, usbMixer, 0);
AudioConnection          patchCord30(usb1, 1, usbMixer, 1);
AudioConnection          patchCord31(usbMixer, 0, masterMixer, 0);
AudioConnection          patchCord32(masterMixer, 0, i2s2, 0);
AudioConnection          patchCord33(masterMixer, 0, i2s2, 1);
AudioConnection          patchCord34(masterMixer, recordEverythingQueue);

AudioControlSGTL5000     sgtl5000_1;     //xy=527,744
// GUItool: end automatically generated code
