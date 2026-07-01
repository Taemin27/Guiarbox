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
AudioInputI2S            i2s1;           //xy=230,559
AudioAmplifier           delayAmp;       //xy=334,1133
AudioEffectPhaser        phaser;         //xy=424,857
AudioRecordQueue         recordGuitarRawQueue;         //xy=430,346
AudioEffectOverdrive     overdrive;      //xy=434,675
AudioMixer4              delayMixer;     //xy=433,985
AudioEffectPitchShift    pitchShifter;   //xy=440,769
AudioAmplifier           notefreqAmp;    //xy=447,453
AudioEffectDelay         delay1;         //xy=474,1134
AudioEffectMultiChorus   chorus;         //xy=580,857
AudioAnalyzeNoteFrequency notefreq1;      //xy=603,455
AudioEffectAutoWah       autoWah;        //xy=607,568
AudioEffectDistortion    distortion;     //xy=611,675
AudioEffectFlanger       flanger;        //xy=727,857
AudioMixer4              reverbMixer;    //xy=767,984
AudioEffectFreeverb      freeverb1;      //xy=767,1035
AudioAmplifier           reverbAmp;      //xy=771,1073
AudioEffectNoiseGate     noiseGate;      //xy=790,568
AudioEffectFuzzFF          fuzzFF;          //xy=795,676
AudioEffectTremolo       tremolo;        //xy=882,857
AudioEffectCompressor    compressor;     //xy=987,568
AudioRecordQueue         recordGuitarQueue;         //xy=1011,1019
AudioPlaySdWav           recorderPlayWav; //xy=1622,750
AudioPlaySdWav           backingTrackPlayWav; //xy=1636,706
AudioSynthSimpleDrum     metronomeDrum;  //xy=1796,810
AudioMixer4              playSDMixer;    //xy=1851,716
AudioInputUSB            usb1;           //xy=1928,585
AudioMixer4              usbMixer;       //xy=2096,603
AudioRecordQueue         recordEverythingQueue;    //xy=2128,999
AudioMixer4              masterMixer;    //xy=2134,857
AudioOutputI2S           i2s2;           //xy=2297,851

AudioConnection          patchCord1(i2s1, 0, notefreqAmp, 0);
AudioConnection          patchCord2(i2s1, 0, autoWah, 0);
AudioConnection          patchCord3(i2s1, 0, recordGuitarRawQueue, 0);
AudioConnection          patchCord4(delayAmp, delay1);
AudioConnection          patchCord5(phaser, chorus);
AudioConnection          patchCord6(overdrive, distortion);
AudioConnection          patchCord7(delayMixer, 0, reverbMixer, 0);
AudioConnection          patchCord8(delayMixer, reverbAmp);
AudioConnection          patchCord9(delayMixer, delayAmp);
AudioConnection          patchCord10(pitchShifter, phaser);
AudioConnection          patchCord11(notefreqAmp, notefreq1);
AudioConnection          patchCord12(delay1, 0, delayMixer, 1);
AudioConnection          patchCord13(chorus, flanger);
AudioConnection          patchCord14(autoWah, noiseGate);
AudioConnection          patchCord15(distortion, fuzzFF);
AudioConnection          patchCord16(flanger, tremolo);
AudioConnection          patchCord17(reverbMixer, 0, masterMixer, 3);
AudioConnection          patchCord18(reverbMixer, recordGuitarQueue);
AudioConnection          patchCord19(freeverb1, 0, reverbMixer, 1);
AudioConnection          patchCord20(reverbAmp, freeverb1);
AudioConnection          patchCord21(noiseGate, compressor);
AudioConnection          patchCord22(fuzzFF, pitchShifter);
AudioConnection          patchCord23(tremolo, 0, delayMixer, 0);
AudioConnection          patchCord24(compressor, overdrive);
AudioConnection          patchCord25(recorderPlayWav, 0, playSDMixer, 2);
AudioConnection          patchCord26(recorderPlayWav, 1, playSDMixer, 3);
AudioConnection          patchCord27(backingTrackPlayWav, 0, playSDMixer, 0);
AudioConnection          patchCord28(backingTrackPlayWav, 1, playSDMixer, 1);
AudioConnection          patchCord29(metronomeDrum, 0, masterMixer, 2);
AudioConnection          patchCord30(playSDMixer, 0, masterMixer, 1);
AudioConnection          patchCord31(usb1, 0, usbMixer, 0);
AudioConnection          patchCord32(usb1, 1, usbMixer, 1);
AudioConnection          patchCord33(usbMixer, 0, masterMixer, 0);
AudioConnection          patchCord34(masterMixer, 0, i2s2, 0);
AudioConnection          patchCord35(masterMixer, 0, i2s2, 1);
AudioConnection          patchCord36(masterMixer, recordEverythingQueue);

AudioControlSGTL5000     sgtl5000_1;     //xy=298,230
// GUItool: end automatically generated code











