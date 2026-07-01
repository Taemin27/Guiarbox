#include <Arduino.h>
#include "lib/Adafruit-GFX-Library-1.12.1/Adafruit_GFX.h"        //One character at size 1 = 6*8 pixels
#include "lib/Adafruit_GFX_Buffer/src/Adafruit_GFX_Buffer.h"  // https://github.com/vjmuzik/Adafruit_GFX_Buffer.git
#include "lib/Adafruit-ST7735-Library-1.11.0/Adafruit_ST7735.h"

#include <Encoder.h>
#include <Bounce2.h>
#include "src/config/SystemBitmaps.h"
#include "src/config/Display.h"
#include "src/config/AudioChain.h"


// Display
#define CS 4
#define DC 5
#define RST 9

typedef Adafruit_ST7735 display_t;
typedef Adafruit_GFX_Buffer<display_t> GFXBuffer_t;
GFXBuffer_t display = GFXBuffer_t(80, 160, display_t(&SPI1, CS, DC, RST));

#include "src/utils/SystemMessage.h"

void flushDisplay() {
    SystemMessage::compositeIfVisible();
    while (!display.displayComplete()) {
    }
    while (!display.display()) {
    }
}

#include "src/config/Effects/PresetManager.h"

// Encoder
Encoder enc(3, 2);
long oldPosition = 0;

const int buttonPin = 0;
Bounce bounce = Bounce();

// Pages
#include "src/pages/Page.h"
#include "src/pages/TunerPage.h"
#include "src/pages/MetronomePage.h"
#include "src/pages/EffectsPage.h"
#include "src/pages/BackingTrackPage.h"
#include "src/pages/RecordPage.h"

// Forward declarations. Temporary.
void effects_setup();
void effects_loop();

TunerPage tunerPage;
MetronomePage metronomePage;
EffectsPage effectsPage;
BackingTrackPage backingTrackPage;
RecordPage recordPage;

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

// SD
#define SDCARD_CS_PIN 10
#define SDCARD_MOSI_PIN 7
#define SDCARD_SCK_PIN 14

void setup() {
  /* System */
  Serial.begin(9600);
  while (!Serial && millis() < 3000) {
  }
  if (CrashReport) {
    Serial.print(CrashReport);
  }

  bounce.attach(buttonPin, INPUT_PULLUP);
  bounce.interval(5);

  /* Audio */
  AudioMemory(800);

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
  flushDisplay();

  delay(2000);

  
  


  /* SD */
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    display.fillScreen(BLACK);
    display.drawBitmap(0, 0, noSDCard, 160, 80, WHITE);
    flushDisplay();
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(1000);
    }
  }

  /* Tuner */
  notefreqAmp.gain(0);
  notefreq1.begin(0.15);

  /* Effects */
  overdrive.setLineInLevel(1);
  distortion.setLineInLevel(1);

  PresetManager::createDefaultPreset();
  PresetManager::initCache();
  const int effectsPreset = PresetManager::getLastUsedPresetIndex();
  PresetManager::loadPreset(effectsPreset);
  effectsPage.setPresetIndex(effectsPreset);

  /* Metronome */
  metronomeDrum.pitchMod(0.5);
  metronomeDrum.length(50);

  backingTrackPage.initFiles();
  recordPage.init();

  Serial.print("Audio memory max: ");
  Serial.println(AudioMemoryUsageMax());

  sgtl5000_1.unmuteLineout();

  switchPage(0);
}

int previousMemoryUsage = 0;

void refreshCurrentPage() {
  pages[currentPage]->refresh();
}

void showPageEntryBlockedMessage() {
  if (pages[currentPage] == &recordPage) {
    SystemMessage::show("Can't record while backing track is active.", 2000, refreshCurrentPage);
  } else if (pages[currentPage] == &backingTrackPage) {
    SystemMessage::show("Can't switch to backing track while recording.", 2000, refreshCurrentPage);
  }
}

bool pageEntryAllowed(int page) {
  if (page < 0 || page >= pageCount) {
    return false;
  }
  if (pages[page] == &recordPage && backingTrackPage.isTrackPlaying()) {
    return false;
  }
  if (pages[page] == &backingTrackPage && recordPage.isRecorderBusy()) {
    return false;
  }
  return true;
}

void loop() {
  printAudioMemoryUsage();
  float requestedVolume = usb1.volume();
  usbMixer.gain(0, requestedVolume);
  usbMixer.gain(1, requestedVolume);

  bounce.update();

  // Drain the record queue before other page work or display flushing can block SD I/O.
  recordPage.update();
  for (int i = 0; i < pageCount; i++) {
    if (pages[i] != &recordPage) {
      pages[i]->update();
    }
  }

  Page* p = pages[currentPage];
  bool isCurrentPageActive = p -> isActive();

  // Run page loop if active
  if (isCurrentPageActive) {
    p -> loop();
  }
  else {
    
    if (bounce.fell()) {
      if (pageEntryAllowed(currentPage)) {
        p -> setActive(true);
        p -> setup();
      } else {
        showPageEntryBlockedMessage();
      }
    }
  }

  // Use current active state — not the snapshot from the start of loop(), or encoder
  // navigation can run in the same iteration as setup() after activating a page (stale false).
  if (!pages[currentPage] -> isActive()) {
    int encoder = readEncoder();
    if (encoder != 0) {
      const int nextPage = currentPage + encoder;
      if (nextPage >= 0 && nextPage < pageCount) {
        switchPage(nextPage);
      }
    }
  }

  // Page draw/flush can block for tens of ms; drain again after UI work.
  if (recordPage.isRecording()) {
    recordPage.update();
  }

  PresetManager::flushPendingWrites();
  SystemMessage::update();
}

void switchPage(int page) {
  if (page >= 0 && page < pageCount) {
    currentPage = page;
    pages[page] -> home();

    drawArrows();
    flushDisplay();
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
