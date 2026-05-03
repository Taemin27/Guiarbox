#pragma once

// Custom Audio Classes
//#include "../../lib/customAudioClasses/effect_wah.h"
#include "../../lib/customAudioClasses/effect_autoWah.h"
#include "../../lib/customAudioClasses/effect_noiseGate.h"
#include "../../lib/customAudioClasses/effect_compressor.h"

#include "../../lib/customAudioClasses/effect_overdrive.h"
#include "../../lib/customAudioClasses/effect_distortion.h"
#include "../../lib/customAudioClasses/effect_fuzz_FF.h"

#include "../../lib/customAudioClasses/effect_pitchShift.h"

#include "../../lib/customAudioClasses/effect_phaser.h"
#include "../../lib/customAudioClasses/effect_multiChorus.h"
#include "../../lib/customAudioClasses/effect_flanger.h"
#include "../../lib/customAudioClasses/effect_tremolo.h"

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=882,846
AudioAmplifier           delayAmp;       //xy=986,1420
AudioEffectPhaser        phaser;         //xy=1076,1144
AudioEffectOverdrive     overdrive;      //xy=1086,962
AudioMixer4              delayMixer;     //xy=1085,1272
AudioEffectPitchShift    pitchShifter;   //xy=1092,1056
AudioAmplifier           notefreqAmp;    //xy=1099,740
AudioEffectDelay         delay1;         //xy=1126,1421
AudioEffectMultiChorus   chorus;         //xy=1232,1144
AudioAnalyzeNoteFrequency notefreq1;      //xy=1255,742
AudioEffectAutoWah       autoWah;        //xy=1259,855
AudioEffectDistortion    distortion;     //xy=1263,962
AudioEffectFlanger       flanger;        //xy=1379,1144
AudioMixer4              reverbMixer;    //xy=1419,1271
AudioEffectFreeverb      freeverb1;      //xy=1419,1322
AudioAmplifier           reverbAmp;      //xy=1423,1360
AudioEffectNoiseGate     noiseGate;      //xy=1442,855
AudioEffectFuzzFF        fuzz;           //xy=1445,962
AudioEffectTremolo       tremolo;        //xy=1534,1144
AudioEffectCompressor    compressor;     //xy=1639,855
AudioPlaySdWav           recorderPlayWav; //xy=2274,1037
AudioPlaySdWav           backingTrackPlayWav; //xy=2288,993
AudioSynthSimpleDrum     metronomeDrum;  //xy=2448,1097
AudioMixer4              playSDMixer;    //xy=2503,1003
AudioInputUSB            usb1;           //xy=2580,872
AudioMixer4              usbMixer;       //xy=2748,890
AudioRecordQueue         recordQueue;    //xy=2780,1286
AudioMixer4              masterMixer;    //xy=2786,1144
AudioOutputI2S           i2s2;           //xy=2949,1138

AudioConnection          patchCord1(i2s1, 0, notefreqAmp, 0);
AudioConnection          patchCord2(i2s1, 0, autoWah, 0);
AudioConnection          patchCord3(delayAmp, delay1);
AudioConnection          patchCord4(phaser, chorus);
AudioConnection          patchCord5(overdrive, distortion);
AudioConnection          patchCord6(delayMixer, 0, reverbMixer, 0);
AudioConnection          patchCord7(delayMixer, reverbAmp);
AudioConnection          patchCord8(delayMixer, delayAmp);
AudioConnection          patchCord9(pitchShifter, phaser);
AudioConnection          patchCord10(notefreqAmp, notefreq1);
AudioConnection          patchCord11(delay1, 0, delayMixer, 1);
AudioConnection          patchCord12(chorus, flanger);
AudioConnection          patchCord13(autoWah, noiseGate);
AudioConnection          patchCord14(distortion, fuzz);
AudioConnection          patchCord15(flanger, tremolo);
AudioConnection          patchCord16(reverbMixer, 0, masterMixer, 3);
AudioConnection          patchCord17(freeverb1, 0, reverbMixer, 1);
AudioConnection          patchCord18(reverbAmp, freeverb1);
AudioConnection          patchCord19(noiseGate, compressor);
AudioConnection          patchCord20(fuzz, pitchShifter);
AudioConnection          patchCord21(tremolo, 0, delayMixer, 0);
AudioConnection          patchCord22(compressor, overdrive);
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
AudioConnection          patchCord34(masterMixer, recordQueue);

AudioControlSGTL5000     sgtl5000_1;     //xy=950,517
// GUItool: end automatically generated code









