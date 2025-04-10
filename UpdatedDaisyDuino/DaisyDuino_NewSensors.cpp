#include "DaisyDuino.h" 
#include <Wire.h>
#include <SparkFun_VL53L5CX_Library.h>
#include <Adafruit_NeoPixel.h>

// Multiplexer configuration
#define TCAADDR 0x70

// Daisy Hardware Setup
DaisyHardware hardware;

// VL53L5CX configuration
SparkFun_VL53L5CX sensors[6];
VL53L5CX_ResultsData measurementData[6];

// Multiplexer channel mapping; Channels for A1, A2, A3, B1, B2, B3
const uint8_t SENSOR_CHANNELS[6] = {0, 1, 2, 3, 4, 5};

// VL53L5CX resolution - if needed
// int imageResolution = 64;
// int imageWidth = sqrt(imageResolution);

// VL53L5CX Pins
#define TOF_A1 17
#define TOF_A2 16
#define TOF_A3 20
#define TOF_B1 21
#define TOF_B2 22
#define TOF_B3 23

// LED Pins
#define LED_PIN_SENSOR_A1 5
#define LED_PIN_SENSOR_A2 6
#define LED_PIN_SENSOR_A3 7

#define LED_PIN_SENSOR_B1 2
#define LED_PIN_SENSOR_B2 3
#define LED_PIN_SENSOR_B3 4

#define LED_COUNT_RING 24

// Distance Parameters
const int SENSOR_MIN_DISTANCE = 50;    // 1m min range
const int SENSOR_MAX_DISTANCE = 200;   // 4m max range
const int ACTIVITY_THRESHOLD = 5;      // mm change to trigger activity

// Timing Parameter
const unsigned long IDLE_TIMEOUT = 100;   // ms until idle

// Animation Speed Ranges
const int ANIMATION_MIN_SPEED = 20;   // slow
const int ANIMATION_MID_SPEED = 100;  // med
const int ANIMATION_MAX_SPEED = 250; // fast

// LED Brightness Levels
const uint8_t IDLE_BRIGHTNESS = 50;
const uint8_t ACTIVE_BRIGHTNESS = 255;

// LED Ring Initializations - Side A
Adafruit_NeoPixel sensorRingA1(LED_COUNT_RING, LED_PIN_SENSOR_A1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel sensorRingA2(LED_COUNT_RING, LED_PIN_SENSOR_A2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel sensorRingA3(LED_COUNT_RING, LED_PIN_SENSOR_A3, NEO_GRB + NEO_KHZ800);

// LED Ring Initializations - Side B
Adafruit_NeoPixel sensorRingB1(LED_COUNT_RING, LED_PIN_SENSOR_B1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel sensorRingB2(LED_COUNT_RING, LED_PIN_SENSOR_B2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel sensorRingB3(LED_COUNT_RING, LED_PIN_SENSOR_B3, NEO_GRB + NEO_KHZ800);

// Array for easy LED ring access
Adafruit_NeoPixel* sensorRings[6] = {
    &sensorRingA1, &sensorRingA2, &sensorRingA3,
    &sensorRingB1, &sensorRingB2, &sensorRingB3
};

// LED Colors
const uint16_t SIDE_HUES[2] = {
    0,      // Side A - Red
    43690   // Side B - Blue
};

// Foward declarations for animation
void updateSensorAnimation(int sensorIndex, int distance);
void breatheAnimation(Adafruit_NeoPixel &strip, uint16_t hue, uint8_t sat);
void waveAnimation(Adafruit_NeoPixel &strip, uint16_t hue, uint8_t sat, uint16_t phase, int speed);

// Animation state tracking per sensor
unsigned long lastActivityTime[6] = {0};
uint16_t animationPhase[6] = {0};
int animationSpeed[6] = {0};
bool isActive[6] = {false};
int prevDistance[6] = {0};

// helper function for multiplexer control
void tcaselect(uint8_t channel) {
    if (channel > 7) return;

    Wire.beginTransmission(TCAADDR);
    Wire.write(1 << channel);
    if (Wire.endTransmission() != 0) {
        Serial.print("Error: Failed to select TCA channel ");
        Serial.println(channel);
    }
}

void setup() {
    // Initialize Serial for debugging
    Serial.begin(115200);
    delay(1000);

    // Initialize I2C
    Wire.begin();
    Wire.setClock(400000);

    // Initialize LED rings
    sensorRingA1.begin();
    sensorRingA2.begin();
    sensorRingA3.begin();
    sensorRingB1.begin();
    sensorRingB2.begin();
    sensorRingB3.begin();

    // Initialize each sensor through multiplexer
    for (int i = 0; i < 6; i++) {
        tcaselect(SENSOR_CHANNELS[i]);
        delay(50);  //Give multiplexer time to switch
        
        Serial.print("Initializing sensor ");
        Serial.print(i);
        Serial.print(" on channel ");
        Serial.println(SENSOR_CHANNELS[i]);
        
        if (!sensors[i].begin()) {
            Serial.print("Sensor ");
            Serial.print(i);
            Serial.println(" not found - check wiring!");
            while(1);
        }

        // Configure each sensor
        sensors[i].setResolution(64);
        sensors[i].setRangingFrequency(15);
        sensors[i].setRangingMode(SF_VL53L5CX_RANGING_MODE_CONTINUOUS);
        sensors[i].setIntegrationTime(40);
        sensors[i].startRanging();
    }

    Serial.println("Setup complete!");
}

void loop() {
    bool newData = false;

    // Check all sensors using multiplexer
    for (int i = 0; i < 6; i++) {
        tcaselect(SENSOR_CHANNELS[i]);

        if (sensors[i].isDataReady()) {
            if (sensors[i].getRangingData(&measurementData[i])) {
                newData = true;
                // use center zone of 8x8 array for distance measurement
                int centerZone = measurementData[i].distance_mm[35];
                updateSensorAnimation(i, centerZone);
            }
        }
    }

    // update LED animations if new data is available
    if (newData) {
        clearAllLEDs();

        for (int i = 0; i < 6; i++) {
          // select color based on side A or B
            uint16_t hue = SIDE_HUES[i >= 3 ? 1 : 0]; 

            if (isActive[i]) {
                waveAnimation(*sensorRings[i], hue, 255, animationPhase[i], animationSpeed[i]);
            } else {
                breatheAnimation(*sensorRings[i], hue, 255);
            }
        }

        updateAllLEDs();
    }

    delay(1);
}

// ------------ LED Animation Functions ------------

void updateSensorAnimation(int sensorIndex, int distance) {
    // Check if object is in range
    if (distance < SENSOR_MAX_DISTANCE) {
        isActive[sensorIndex] = true;
        lastActivityTime[sensorIndex] = millis();
    } else {
        // check for timeout transition
        if (millis() - lastActivityTime[sensorIndex] > IDLE_TIMEOUT) {
            isActive[sensorIndex] = false;
        }
    }

    // detect significant change in movement
    if (abs(distance - prevDistance[sensorIndex]) > ACTIVITY_THRESHOLD) {
        isActive[sensorIndex] = true;
        lastActivityTime[sensorIndex] = millis();
    }

    // update animation speed based on distance
    int constrainedDistance = constrain(distance, SENSOR_MIN_DISTANCE, SENSOR_MAX_DISTANCE);
    animationSpeed[sensorIndex] = map(constrainedDistance, SENSOR_MIN_DISTANCE, SENSOR_MAX_DISTANCE,
                                      ANIMATION_MIN_SPEED, ANIMATION_MAX_SPEED);

    // update animation
    animationPhase[sensorIndex] += isActive[sensorIndex] ? animationSpeed[sensorIndex] : (ANIMATION_MIN_SPEED / 2);

    // Store the previous distance
    prevDistance[sensorIndex] = distance;
}

// Idle
void breatheAnimation(Adafruit_NeoPixel &strip, uint16_t hue, uint8_t sat) {
  // calculate breathing effect using a sine wave
    float breath = (sin(millis() / 2000.0 * PI) + 1.0) / 2.0;
    uint8_t brightness = IDLE_BRIGHTNESS * breath + IDLE_BRIGHTNESS / 2;

    // apply to all LEDs in the strip
    for (int i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.ColorHSV(hue, sat, brightness));
    }
}

// Active
void waveAnimation(Adafruit_NeoPixel &strip, uint16_t hue, uint8_t sat, uint16_t phase, int speed) {
    int numPixels = strip.numPixels();

    // map speed to brightness and wave animation
    uint8_t maxBrightness = map(speed, ANIMATION_MIN_SPEED, ANIMATION_MAX_SPEED,
                                ACTIVE_BRIGHTNESS / 2, ACTIVE_BRIGHTNESS);
    int falloff = map(speed, ANIMATION_MIN_SPEED, ANIMATION_MAX_SPEED, 2, 4);

    // calculate wave for each LED ring
    for (int i = 0; i < numPixels; i++) {
      float wavePos = (i + (phase / 100.0)) / float(numPixels);
      wavePos -= floor(wavePos);
      float wave = (sin(wavePos * 2 * PI * falloff) + 1.0) / 2.0;
      uint8_t brightness = wave * maxBrightness;
      strip.setPixelColor(i, strip.ColorHSV(hue, sat, brightness));
    }
}

// ------------ LED HelperFunctions ------------

void updateAllLEDs() {
    for (int i = 0; i < 6; i++) {
        sensorRings[i]->show();
    }
}

void clearAllLEDs() {
    for (int i = 0; i < 6; i++) {
        sensorRings[i]->clear();
    }
}
