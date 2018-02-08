/**!
 * Clockxer
 *
 * Ken Frederick
 * ken.frederick@gmx.de
 * http://kenfrederick.de/
 *
 * Required 3rd Party libraries
 * https://github.com/NicoHood/MSGEQ7
 * https://github.com/adafruit/Adafruit_LED_Backpack
 * https://code.google.com/archive/p/arduinode/downloads
 *
 * License:
 * https://opensource.org/licenses/MIT
 *
 */



//-----------------------------------------------------------------------------
//
// Includes
//
//-----------------------------------------------------------------------------
#include "MSGEQ7.h"
#include "pt.h"
#include "RotaryEncoder.h"

// These two have been installed globally in your root Arduino/libraries folder
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"



//-----------------------------------------------------------------------------
//
// Constants
//
//-----------------------------------------------------------------------------
// MSGEQ7
#define STROBE 4
#define RESET 5
#define CHANNEL_LEFT A0
#define CHANNEL_RIGHT A1

#define MSGEQ7_INTERVAL ReadsPerSecond(100)
#define MSGEQ7_SMOOTH 0 // Range: 0-255
#define BAND_LENGTH 7

// Rotary Encoder
#define BUTTON 12
#define ACTIVE_PIN 8
#define MIN_BPM 60 * 4
#define MAX_BPM 240 * 4

// Sync output
#define SYNC_PIN 13

// Protothread
static struct pt protothreadMain, protothreadSync;



//-----------------------------------------------------------------------------
//
// Properties
//
//-----------------------------------------------------------------------------
// MSGEQ7 IC with 7 different frequencies from range 0-255
// Bass ------------------Mid-----------------Treble
// 63Hz, 160Hz, 400Hz, 1kHz, 2.5kHz, 6.25KHz, 16kHz
CMSGEQ7<MSGEQ7_SMOOTH, RESET, STROBE, CHANNEL_LEFT, CHANNEL_RIGHT> MSGEQ7;

int LED[] = {9, 10, 11}; // Low, Mid, High

uint8_t freqMax[BAND_LENGTH];
byte band = 0;

// Rotary Encoder
RotaryEncoder rotary(BUTTON);
long ms = 1000;

// 7 Segment Display
Adafruit_7segment disp = Adafruit_7segment();



//-----------------------------------------------------------------------------
//
// Methods
//
//-----------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);

  // Setup MSGEQ7/Spectrum Shield for readings
  MSGEQ7.begin();

  for (band = 0; band < BAND_LENGTH; band++) {
    // Setup array for max values
    freqMax[band] = 1;
  }

  // Setup rotary encoder for reading.
  rotary.init();
  rotary.setMinVal(60);
  rotary.setMaxVal(240);
  pinMode(ACTIVE_PIN, OUTPUT);

  // Setup sync pin signal
  pinMode(SYNC_PIN, OUTPUT);

  // Setup display
  disp.begin(0x70);
  disp.setBrightness(3);

  // Setup Protothreads
  PT_INIT(&protothreadSync);
}

void loop() {
  // Call rotary handlers
  rotary.encoderHandler();
  rotary.clickHandler();

  disp.print(rotary.getVal(), DEC);
//  disp.

  long ms = bpmToMs(rotary.getVal());
//  Serial.print("MS: ");
//  Serial.print(ms);
//  Serial.print(" ROTARY_ACTIVE: ");
//  Serial.println(rotary.isActive);

  // reset SYNC_PIN and ACTIVE_PIN before calling protothread method
  digitalWrite(SYNC_PIN, LOW);
  digitalWrite(ACTIVE_PIN, LOW);

  syncHandler(&protothreadSync, ms);

  // Write to display
  disp.writeDisplay();
}

/**
 * Handler for syncing; audio pulse and LED flash
 * @param  pt       pt address operator
 * @param  interval rate in MS to repeat function
 */
static int syncHandler(struct pt *pt, int interval) {
  static unsigned long timestamp = 0;
  PT_BEGIN(pt);
  while(1) {
    PT_WAIT_UNTIL(pt, millis() - timestamp > interval);
    timestamp = millis();

    if (rotary.isActive == 1) {
      if (digitalRead(ACTIVE_PIN) == HIGH) {
  //      disp.drawColon(fals);
        digitalWrite(ACTIVE_PIN, LOW);
        digitalWrite(SYNC_PIN, LOW);
      } else {
  //      disp.drawColon(true);
        digitalWrite(ACTIVE_PIN, HIGH);
        digitalWrite(SYNC_PIN, HIGH);
      }
    }

  }
  PT_END(pt);
}

/**
 * Frequency analyzer
 */
void analyze() {
  // Analyze without delay every interval.
  bool isNewReading = MSGEQ7.read(MSGEQ7_INTERVAL);

  if (isNewReading) {
    for (band = 0; band < BAND_LENGTH; band++) {
      // Read frequency value
      uint8_t freq = MSGEQ7.get(band);

      // reduce noise
      freq = mapNoise(freq);

      // Keep track of peak value.
      if (freq > freqMax[band]) {
        freqMax[band] = freq;
      }

//      uint8_t val = normalizePWM(freq, freqMax[band]);

      // Output PWM signal (LED) to the music beat
//      analogWrite(LED[band], val);

//      Serial.print(" --- ");
//      Serial.print(band);
//      Serial.print(" --- ");
//      Serial.println("Freq ");
//      Serial.println(freq);
//      Serial.println(" --- ");
//      Serial.print(freqMax[band]);
//      Serial.print(" --- ");
//      Serial.println(val);
    }
  }
}

/**
 * @param  val        the input value
 * @param  max        the maximum value
 * @return normalized value e.g. betwen 0-255
 */
uint8_t normalizePWM(uint8_t val, uint8_t max) {
  return (uint8_t) map(val, 0, max, 0, 255);
}

/**
 * Convert BPM value into milliseconds.
 * @param  bpm
 * @return milliseconds
 */
unsigned int bpmToMs(unsigned int bpm) {
  return 60000 / bpm;
}

//
//bpmToHz(unsigned int bpm) {
//  return bpm * 0.01666667
//}
