#ifndef effect_overdrive_h_
#define effect_overdrive_h_

#include <Arduino.h>
#include "AudioStream.h"
#include "arm_math.h"

class AudioEffectOverdrive : public AudioStream {
public:
  AudioEffectOverdrive() : AudioStream(1, inputQueueArray) {
    tempGain = 1.0f;
    drive = 1.0f;
    level = 1.0f;
    overtoneMix = 0.5f;
    positiveClip = 1.0f;
    negativeClip = 1.0f;
  }

  virtual void update(void);

  void setTempGain(float tempGain) {
    AudioEffectOverdrive::tempGain = tempGain;
  }

  void setDrive(float drive) {
    AudioEffectOverdrive::drive = constrain(drive, 0.0f, 10.0f);
  }

  void setLevel(float level) {
    AudioEffectOverdrive::level = constrain(level, 0.0f, 2.0f);
  }

  void setClip(float positiveClip, float negativeClip) {
    AudioEffectOverdrive::positiveClip = constrain(positiveClip, 0.0f, 1.0f);
    AudioEffectOverdrive::negativeClip = constrain(negativeClip, 0.0f, 1.0f);
  }

  void setOvertoneMix(float overtoneMix) {
    AudioEffectOverdrive::overtoneMix = constrain(overtoneMix, 0.0f, 1.0f);
  }


private:
  audio_block_t *inputQueueArray[1];
  float tempGain;
  float drive;
  float level;
  float overtoneMix;
  float positiveClip;
  float negativeClip;
};

#endif