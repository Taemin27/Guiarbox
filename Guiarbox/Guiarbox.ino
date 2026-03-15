// Define page orders
#define page0_setup tuner_setup
#define page0_loop tuner_loop

#define page1_setup effects_setup
#define page1_loop effects_loop

#define page2_setup metronome_setup
#define page2_loop metronome_loop

#define page3_setup backingTrack_setup
#define page3_loop backingTrack_loop

#define page4_setup record_setup
#define page4_loop record_loop


// Include Statements
#include <Arduino.h>
#include "src/Adafruit-GFX-Library-1.12.1/Adafruit_GFX.h"        //One character at size 1 = 6*8 pixels
#include "src/Adafruit_GFX_Buffer/src/Adafruit_GFX_Buffer.h"  // https://github.com/vjmuzik/Adafruit_GFX_Buffer.git
#include "src/Adafruit-ST7735-Library-1.11.0/Adafruit_ST7735.h"
#include <Encoder.h>
#include <Bounce2.h>
#include "SystemBitmaps.h"
#include <vector>

// Custom Audio Classes
#include "src/customAudioClasses/effect_overdrive.h"
#include "src/customAudioClasses/effect_distortion.h"
#include "src/customAudioClasses/effect_compressor.h"

// Audio
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=935,1465
AudioAmplifier           notefreqAmp;    //xy=1117,1175
AudioEffectCompressor    compressor;     //xy=1216,1575
AudioAnalyzeNoteFrequency notefreq1;      //xy=1273,1177
AudioMixer4              compressorOnOffMixer; //xy=1426,1456
AudioEffectDistortion    distortion;     //xy=1947,1723
AudioAmplifier           distortionFirstAmp; //xy=1949,1673
AudioEffectWaveshaper    fuzzWaveShape;  //xy=1959,1837
AudioAmplifier           overdriveFirstAmp; //xy=1962,1506
AudioEffectOverdrive     overdrive;      //xy=1971,1553
AudioMixer4              driveMixer;     //xy=2188,1608
AudioMixer4              driveOnOffMixer; //xy=2477,1481
AudioEffectFlange        chorusFlange;   //xy=2908,1535
AudioMixer4              chorusMixer;    //xy=3114,1508
AudioAmplifier           delayAmp;       //xy=3262,1648
AudioMixer4              delayMixer;     //xy=3319,1529
AudioEffectDelay         delay1;         //xy=3396,1648
AudioMixer4              reverbMixer;    //xy=3644,1528
AudioEffectFreeverb      freeverb1;      //xy=3644,1579
AudioAmplifier           reverbAmp;      //xy=3648,1617
AudioPlaySdWav           recorderPlayWav; //xy=3654,1394
AudioPlaySdWav           backingTrackPlayWav; //xy=3668,1350
AudioSynthSimpleDrum     metronomeDrum;  //xy=3828,1454
AudioMixer4              playSDMixer;    //xy=3883,1360
AudioInputUSB            usb1;           //xy=3960,1229
AudioMixer4              usbMixer;       //xy=4128,1247
AudioRecordQueue         recordQueue;    //xy=4160,1643
AudioMixer4              masterMixer;    //xy=4166,1501
AudioOutputI2S           i2s2;           //xy=4329,1495

AudioConnection          patchCord1(i2s1, 0, notefreqAmp, 0);
AudioConnection          patchCord2(i2s1, 0, compressorOnOffMixer, 0);
AudioConnection          patchCord3(i2s1, 0, compressor, 0);
AudioConnection          patchCord4(notefreqAmp, notefreq1);
AudioConnection          patchCord5(compressor, 0, compressorOnOffMixer, 1);
AudioConnection          patchCord6(compressorOnOffMixer, 0, driveOnOffMixer, 0);
AudioConnection          patchCord7(compressorOnOffMixer, overdriveFirstAmp);
AudioConnection          patchCord8(compressorOnOffMixer, distortionFirstAmp);
AudioConnection          patchCord9(compressorOnOffMixer, fuzzWaveShape);
AudioConnection          patchCord10(distortion, 0, driveMixer, 1);
AudioConnection          patchCord11(distortionFirstAmp, distortion);
AudioConnection          patchCord12(fuzzWaveShape, 0, driveMixer, 2);
AudioConnection          patchCord13(overdriveFirstAmp, overdrive);
AudioConnection          patchCord14(overdrive, 0, driveMixer, 0);
AudioConnection          patchCord15(driveMixer, 0, driveOnOffMixer, 1);
AudioConnection          patchCord16(driveOnOffMixer, 0, chorusMixer, 0);
AudioConnection          patchCord17(driveOnOffMixer, chorusFlange);
AudioConnection          patchCord18(chorusFlange, 0, chorusMixer, 1);
AudioConnection          patchCord19(chorusMixer, 0, delayMixer, 0);
AudioConnection          patchCord20(delayAmp, delay1);
AudioConnection          patchCord21(delayMixer, 0, reverbMixer, 0);
AudioConnection          patchCord22(delayMixer, reverbAmp);
AudioConnection          patchCord23(delayMixer, delayAmp);
AudioConnection          patchCord24(delay1, 0, delayMixer, 1);
AudioConnection          patchCord25(reverbMixer, 0, masterMixer, 3);
AudioConnection          patchCord26(freeverb1, 0, reverbMixer, 1);
AudioConnection          patchCord27(reverbAmp, freeverb1);
AudioConnection          patchCord28(recorderPlayWav, 0, playSDMixer, 2);
AudioConnection          patchCord29(recorderPlayWav, 1, playSDMixer, 3);
AudioConnection          patchCord30(backingTrackPlayWav, 0, playSDMixer, 0);
AudioConnection          patchCord31(backingTrackPlayWav, 1, playSDMixer, 1);
AudioConnection          patchCord32(metronomeDrum, 0, masterMixer, 2);
AudioConnection          patchCord33(playSDMixer, 0, masterMixer, 1);
AudioConnection          patchCord34(usb1, 0, usbMixer, 0);
AudioConnection          patchCord35(usb1, 1, usbMixer, 1);
AudioConnection          patchCord36(usbMixer, 0, masterMixer, 0);
AudioConnection          patchCord37(masterMixer, 0, i2s2, 0);
AudioConnection          patchCord38(masterMixer, 0, i2s2, 1);
AudioConnection          patchCord39(masterMixer, recordQueue);

AudioControlSGTL5000     sgtl5000_1;     //xy=762,839
// GUItool: end automatically generated code





// Display
#define CS 4
#define DC 5
#define RST 9

//Adafruit_ST7735 display = Adafruit_ST7735(CS, DC, RST);
typedef Adafruit_ST7735 display_t;
typedef Adafruit_GFX_Buffer<display_t> GFXBuffer_t;
GFXBuffer_t display = GFXBuffer_t(80, 160, display_t(&SPI1, CS, DC, RST));

const uint16_t BLACK = 0x0000;
const uint16_t WHITE = 0xffff;
const uint16_t GRAY = 0x8430;
const uint16_t BLUE = 0x001f;
const uint16_t RED = 0xF800;
const uint16_t YELLOW = 0xffe0;
const uint16_t GREEN = 0x07e0;
const uint16_t BOOT = 0x0617;

// Encoder
Encoder enc(3, 2);
long oldPosition = 0;

const int buttonPin = 0;
Bounce bounce = Bounce();


// Count pages from 0
int currentPage = 0;
int pageCount = 4;
bool pageSelected = false;

// Compressor
const float ef_compressor_thresholdMax = 0.5;  //Adjust if needed (use compressorPeak.read() to find the right value)
const float ef_compressor_ratio = 0.125;

// Chorus
#define CHORUS_DELAY_LENGTH (16 * AUDIO_BLOCK_SAMPLES)
short delayline[CHORUS_DELAY_LENGTH];

// Metronome global variables
bool metronome_on = false;
float metronome_previousMillis;
int metronome_bpm = 100;
int metronome_beatsPerBar = 4;
int metronome_currentBeat = 0;

// BackingTrack global variables
std::vector<String> backingTrack_files;

// Recorder global variables
#define RECORD_BANK_SIZE 99
bool record_isBankUsed[RECORD_BANK_SIZE];
File recordFile;
int record_state = 0; // 0:Stopped 1:Recording 2:Playing
int record_currentBank = 1;

// SD
#define SDCARD_CS_PIN 10
#define SDCARD_MOSI_PIN 7
#define SDCARD_SCK_PIN 14

void setup() {
  /* System */
  Serial.begin(9600);
  bounce.attach(buttonPin, INPUT_PULLUP);
  bounce.interval(5);




  /* Audio */
  AudioMemory(1000);

  sgtl5000_1.enable();
  sgtl5000_1.dacVolumeRampDisable();
  sgtl5000_1.muteLineout();
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000_1.lineInLevel(1);
  sgtl5000_1.volume(0.5f);


  masterMixer.gain(0, 1);
  masterMixer.gain(1, 0.5);
  masterMixer.gain(2, 0);
  masterMixer.gain(3, 1);

  /* Display */
  display.initR(INITR_GREENTAB);
  display.setSPISpeed(24000000);
  display.setRotation(1);
  display.invertDisplay(false);

  // DMA
  Serial.println(display.initDMA(display.DMA0) ? "DMA: On" : "DMA: Off");

  delay(500);

  display.drawBitmap(0, 0, Guiarbox, 160, 80, BOOT);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 73);
  display.print("Ver:Development");
  display.display();
  delay(2000);

  sgtl5000_1.unmuteLineout();


  /* SD */
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    display.fillScreen(BLACK);
    display.drawBitmap(0, 0, noSDCard, 160, 80, WHITE);
    display.display();
    //while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    //}
  }


  /* Tuner */
  notefreqAmp.gain(0);
  notefreq1.begin(0.15);

  /* Effects */
  //Dynamic
  compressorOnOffMixer.gain(0, 1);
  compressorOnOffMixer.gain(1, 0);
  compressor.setRatio(5.0f);

  //Drive
  overdrive.setLineInLevel(1);
  distortion.setLineInLevel(1);

  disableDrive();

  //Chorus
  chorusFlange.begin(delayline, CHORUS_DELAY_LENGTH, 0, 0, 0);
  chorusMixer.gain(0, 1);
  chorusMixer.gain(1, 0);

  //Delay
  delayAmp.gain(0);
  delayMixer.gain(0, 1);
  delayMixer.gain(1, 0);
  for (int i = 1; i < 7; i++) {
    delay1.disable(i);
  }

  //Freeverb
  reverbAmp.gain(0);
  reverbMixer.gain(0, 1);
  reverbMixer.gain(1, 0);
  freeverb1.roomsize(0);
  freeverb1.damping(0);

  /* Metronome */
  metronomeDrum.pitchMod(0.5);
  metronomeDrum.length(50);

  /* BackingTrack*/
  File backingTrackDir = SD.open("backingtracks");
  while (true) {
    File entry = backingTrackDir.openNextFile();
    if (!entry) {
      break;
    }
    if (!entry.isDirectory()) {
      backingTrack_files.push_back((String)entry.name());
      Serial.println(entry.name());
    }
    entry.close();
  }
  backingTrackDir.close();

  playSDMixer.gain(0, 1);
  playSDMixer.gain(1, 1);
  

  /* Record */
  // Check used banks
  for (int i = 0; i < RECORD_BANK_SIZE; i++) {
    String filename = "recordings/RECORD" + String(i) + ".WAV";
    if (SD.exists(filename.c_str())) {
      record_isBankUsed[i] = true;
    } else {
      record_isBankUsed[i] = false;
    }
  }

  playSDMixer.gain(2, 1);
  playSDMixer.gain(3, 1);




  page0_setup();
}

int previousMemoryUsage = 0;

void loop() {
  //printAudioMemoryUsage();
  float requestedVolume = usb1.volume();
  usbMixer.gain(0, requestedVolume);
  usbMixer.gain(1, requestedVolume);

  switch (currentPage) {
    case 0:
      page0_loop();
      break;
    case 1:
      page1_loop();
      break;
    case 2:
      page2_loop();
      break;
    case 3:
      page3_loop();
      break;
    case 4:
      page4_loop();
      break;
  }

  if (!pageSelected) {
    int encoder = readEncoder();
    if (encoder != 0) {
      switchPage(currentPage + encoder);
    }
  }



  // Metronome
  if (metronome_on) {
    float currentMillis = millis();
    if (currentMillis - metronome_previousMillis >= 60000 / metronome_bpm) {
      if (++metronome_currentBeat > metronome_beatsPerBar || metronome_currentBeat == 1) {
        metronome_currentBeat = 1;
        metronomeDrum.frequency(440);
        metronomeDrum.noteOn();
      } else {
        metronomeDrum.frequency(220);
        metronomeDrum.noteOn();
      }
      if (currentPage == 2 && pageSelected) {
        metronome_drawIndicator();
        display.display();
      }
      metronome_previousMillis = currentMillis;
    }
  }

  // Record
  if (record_state == 1) {
    continueRecording();
  }
}

void switchPage(int page) {
  if (0 <= page && page <= pageCount) {
    currentPage = page;
    switch (page) {
      case 0:
        page0_setup();
        break;
      case 1:
        page1_setup();
        break;
      case 2:
        page2_setup();
        break;
      case 3:
        page3_setup();
        break;
      case 4:
        page4_setup();
        break;
    }
  }
}

void drawArrows() {
  if (currentPage > 0) {
    display.drawBitmap(0, 29, leftArrow, 13, 22, WHITE);
  }
  if (currentPage < pageCount) {
    display.drawBitmap(147, 29, rightArrow, 13, 22, WHITE);
  }
}

long readEncoder() {
  long newPosition = round(enc.read() / 4);
  if (newPosition - oldPosition > 0) {
    oldPosition = newPosition;
    return (1);
  } else if (newPosition - oldPosition < 0) {
    oldPosition = newPosition;
    return (-1);
  }
  return (0);
}

void printAudioMemoryUsage() {
  int memoryUsage = AudioMemoryUsage();
  if (memoryUsage != previousMemoryUsage) {
    previousMemoryUsage = memoryUsage;
    Serial.println(memoryUsage);
  }
}
