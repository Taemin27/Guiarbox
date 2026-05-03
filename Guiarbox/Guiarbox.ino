#include <Arduino.h>
#include "lib/Adafruit-GFX-Library-1.12.1/Adafruit_GFX.h"        //One character at size 1 = 6*8 pixels
#include "lib/Adafruit_GFX_Buffer/src/Adafruit_GFX_Buffer.h"  // https://github.com/vjmuzik/Adafruit_GFX_Buffer.git
#include "lib/Adafruit-ST7735-Library-1.11.0/Adafruit_ST7735.h"
#include <Encoder.h>
#include <Bounce2.h>
#include "src/config/SystemBitmaps.h"
#include <vector>

#include "src/config/AudioChain.h"


// Display
#define CS 4
#define DC 5
#define RST 9

typedef Adafruit_ST7735 display_t;
typedef Adafruit_GFX_Buffer<display_t> GFXBuffer_t;
GFXBuffer_t display = GFXBuffer_t(80, 160, display_t(&SPI, CS, DC, RST));


// Encoder
Encoder enc(3, 2);
long oldPosition = 0;

const int buttonPin = 1;
Bounce bounce = Bounce();

// Pages
#include "src/pages/Page.h"
#include "src/pages/TunerPage.h"
#include "src/pages/MetronomePage.h"
#include "src/pages/EffectsPage.h"

// Forward declarations. Temporary.
void effects_setup();
void effects_loop();
void backingTrack_setup();
void backingTrack_loop();
void record_setup();
void record_loop();

void continueRecording();
void disableDrive();

TunerPage tunerPage;
MetronomePage metronomePage;
EffectsPage effectsPage;

LegacyPage backingTrackPage(backingTrack_setup, backingTrack_loop);
LegacyPage recordPage(record_setup, record_loop);

Page* pages[] = {
  &tunerPage,
  &effectsPage,
  &metronomePage,
  &backingTrackPage,
  &recordPage
};

int currentPage = 0;
int pageCount = 5;
bool pageSelected = false;

// Compressor
const float ef_compressor_thresholdMax = 0.5;  //Adjust if needed (use compressorPeak.read() to find the right value)
const float ef_compressor_ratio = 0.125;

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

  // "boot" screen
  delay(500);
  display.drawBitmap(0, 0, Guiarbox, 160, 80, BOOT);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 73);
  display.print("Ver:Development");
  display.display();

  delay(2000);

  
  


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
  //wah.disable();
  autoWah.disable();
  noiseGate.disable();
  compressor.disable();

  //Drive
  overdrive.setLineInLevel(1);
  distortion.setLineInLevel(1);

  overdrive.disable();
  distortion.disable();

  // Modulation
  phaser.disable();
  chorus.disable();
  flanger.disable();
  tremolo.disable();

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

  
  sgtl5000_1.unmuteLineout();

  switchPage(0);
}

int previousMemoryUsage = 0;

void loop() {
  //printAudioMemoryUsage();
  float requestedVolume = usb1.volume();
  usbMixer.gain(0, requestedVolume);
  usbMixer.gain(1, requestedVolume);

  bounce.update();

  Page* p = pages[currentPage];
  bool isCurrentPageActive = p -> isActive();

  // Run page loop if active
  if (isCurrentPageActive) {
    p -> loop();
  }
  else {
    
    if (bounce.fell()) {
      p -> setActive(true);
      p -> setup();
    }
  }

  // Run all background services
  for (int i = 0; i < pageCount; i++) {
    pages[i] -> update();
  }

  // Use current active state — not the snapshot from the start of loop(), or encoder
  // navigation can run in the same iteration as setup() after activating a page (stale false).
  if (!pages[currentPage] -> isActive()) {
    int encoder = readEncoder();
    if (encoder != 0) {
      switchPage(currentPage + encoder);
    }
  }

  

  // Record
  if (record_state == 1) {
    continueRecording();
  }
}

void switchPage(int page) {
  if (page >= 0 && page < pageCount) {
    currentPage = page;
    pages[page] -> home();

    drawArrows();
    display.display();
  }

}

void drawArrows() {
  if (currentPage > 0) {
    display.drawBitmap(0, 29, leftArrow, 13, 22, WHITE);
  }
  if (currentPage < pageCount - 1) {
    display.drawBitmap(147, 29, rightArrow, 13, 22, WHITE);
  }
}

int readEncoder() {
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
