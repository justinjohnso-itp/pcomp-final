#include "DaisyDuino.h" 
#include <Wire.h>
#include <SparkFun_VL53L5CX_Library.h>
#include <Adafruit_NeoPixel.h>

// Daisy
DaisyHardware hardware;

// VL53L5CX - need to update pins for Daisy
SparkFun_VL53L5CX myImager;
VL53L5CX_ResultsData measurementData; // result data class structure, 1356 bytes of RAM

int imageResolution = 0;
int imageWidth = 0;

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
#define LED_PIN_SENSOR_B3 4  // Fix: was incorrectly defined as B2

// Animation Constants
#define LED_COUNT_RING 24

// Distance Constants
const int SENSOR_MIN_DISTANCE = 50;    // 1m min range
const int SENSOR_MAX_DISTANCE = 200;    // 4m max range
const int ACTIVITY_THRESHOLD = 5;       // mm change to trigger activity

// Timing Constants
const unsigned long IDLE_TIMEOUT = 100;   // ms until idle

// Animation Speed Constants
const int ANIMATION_MIN_SPEED = 20;
const int ANIMATION_MID_SPEED = 100;
const int ANIMATION_MAX_SPEED = 250;

// Brightness Constants
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

// Foward declarations for animation
void updateSensorAnimation(int sensorIndex, int distance);
void breatheAnimation(Adafruit_NeoPixel &strip, uint16_t hue, uint8_t sat);
void waveAnimation(Adafruit_NeoPixel &strip, uint16_t hue, uint8_t sat, uint16_t phase, int speed);

// Animation state (per VL53L5CX)
unsigned long lastActivityTime[4] = {0, 0, 0, 0};
uint16_t animationPhase[4] = {0, 0, 0, 0};
int animationSpeed[4] = {0, 0, 0, 0};
bool isActive[4] = {false, false, false, false};
int prevDistance[4] = {0, 0, 0, 0};

void setup() {
    // Initialize Serial for debugging
    Serial.begin(115200);
    delay(1000);

    // Initialize I2C
    Wire.begin();
    Wire.setClock(400000); // Set I2C clock to 400kHz

    // Initialize LED rings
    sensorRingA1.begin();
    sensorRingA2.begin();
    sensorRingA3.begin();
    sensorRingB1.begin();
    sensorRingB2.begin();
    sensorRingB3.begin();

    // Initialize VL53L5CX sensor
    Serial.println("Initializing VL53L5CX...");
    if (myImager.begin() == false) {
        Serial.println(F("Sensor not found - check wiring."));
        while(1); // Halt if sensor not found
    }

    // Configure VL53L5CX
    myImager.setResolution(64);        // 8x8 resolution
    myImager.setRangingFrequency(15);  // 15Hz update rate
    
    // Optional but recommended settings
    myImager.setRangingMode(SF_VL53L5CX_RANGING_MODE_CONTINUOUS);
    myImager.setIntegrationTime(40);   // Integration time in ms
    
    // Get resolution for processing
    imageResolution = myImager.getResolution();
    imageWidth = sqrt(imageResolution);

    // Start ranging
    myImager.startRanging();

    Serial.println("Setup complete!");
}

void loop() {
    if (myImager.isDataReady() == true) {
        if (myImager.getRangingData(&measurementData)) {
            // Clear all LEDs before updates
            clearAllLEDs();
            
            // The VL53L5CX returns an 8x8 array of distance measurements
            // We can access specific zones for different areas
            
            // Example mapping of zones to our sensors:
            int distanceA1 = measurementData.distance_mm[0];  // Top left zone
            int distanceA2 = measurementData.distance_mm[7];  // Top right zone
            int distanceB1 = measurementData.distance_mm[56]; // Bottom left zone
            int distanceB2 = measurementData.distance_mm[63]; // Bottom right zone
            
            // Update animations based on distances
            updateSensorAnimation(0, distanceA1);
            updateSensorAnimation(1, distanceA2);
            updateSensorAnimation(2, distanceB1);
            updateSensorAnimation(3, distanceB2);

            // LED Animations (ToF sensors)
            // Half A: tofA1 controls sensorRingA1; tofA2 controls sensorRingA2.
            if (isActive[0]) { 
                waveAnimation(sensorRingA1, 0, 255, animationPhase[0], animationSpeed[0]);
            } else {
                breatheAnimation(sensorRingA1, 0, 255);
            }
            if (isActive[1]) {
                waveAnimation(sensorRingA2, 0, 255, animationPhase[1], animationSpeed[1]);
            } else {
                breatheAnimation(sensorRingA2, 0, 255);
            }
            // Half B: tofB1 controls sensorDotB1; tofB2 controls sensorDotB2.
            if (isActive[2]) {
                waveAnimation(sensorDotB1, 43690, 255, animationPhase[2], animationSpeed[2]);
            } else {
                breatheAnimation(sensorDotB1, 43690, 255);
            }
            if (isActive[3]) {
                waveAnimation(sensorDotB2, 43690, 255, animationPhase[3], animationSpeed[3]);
            } else {
                breatheAnimation(sensorDotB2, 43690, 255);
            }

            // Update all LEDs at once
            updateAllLEDs();
        }
    }
    delay(1);
}

// LED Animation Functions

void updateSensorAnimation(int sensorIndex, int distance) {
    // If the sensor reading indicates something is present
    if (distance < SENSOR_MAX_DISTANCE) {
      // Always mark as active if below max—even if there's no change.
      isActive[sensorIndex] = true;
      lastActivityTime[sensorIndex] = millis();
    } else {
      // If the sensor reads its default max value, then check for idle timeout.
      if (millis() - lastActivityTime[sensorIndex] > IDLE_TIMEOUT) {
        isActive[sensorIndex] = false;
      }
    }
    
    // Additionally, if there is a significant change in the sensor reading,
    // treat the sensor as active and update the last activity time.
    if (abs(distance - prevDistance[sensorIndex]) > ACTIVITY_THRESHOLD) {
      isActive[sensorIndex] = true;
      lastActivityTime[sensorIndex] = millis();
    }
    
    // Map the animation speed directly from the sensor reading.
    // A lower (closer) reading gives a slower animation, and a further reading gives a faster animation.
    animationSpeed[sensorIndex] = map(distance, SENSOR_MIN_DISTANCE, SENSOR_MAX_DISTANCE,
                                      ANIMATION_MIN_SPEED, ANIMATION_MAX_SPEED);
    
    // Advance the animation phase. If inactive, use a slow idle rate.
    animationPhase[sensorIndex] += isActive[sensorIndex] ? animationSpeed[sensorIndex] : (ANIMATION_MIN_SPEED / 2);
    
    prevDistance[sensorIndex] = distance;
  }
  
  void breatheAnimation(Adafruit_NeoPixel &strip, uint16_t hue, uint8_t sat) {
    float breath = (sin(millis() / 2000.0 * PI) + 1.0) / 2.0;
    uint8_t brightness = IDLE_BRIGHTNESS * breath + IDLE_BRIGHTNESS / 2;
    for (int i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.ColorHSV(hue, sat, brightness));
    }
  }
  
  void waveAnimation(Adafruit_NeoPixel &strip, uint16_t hue, uint8_t sat, uint16_t phase, int speed) {
    int numPixels = strip.numPixels();
    uint8_t maxBrightness = map(speed, ANIMATION_MIN_SPEED, ANIMATION_MAX_SPEED,
                                ACTIVE_BRIGHTNESS / 2, ACTIVE_BRIGHTNESS);
    int falloff = map(speed, ANIMATION_MIN_SPEED, ANIMATION_MAX_SPEED, 2, 4);
    for (int i = 0; i < numPixels; i++) {
      float wavePos = (i + (phase / 100.0)) / float(numPixels);
      wavePos -= floor(wavePos);
      float wave = (sin(wavePos * 2 * PI * falloff) + 1.0) / 2.0;
      uint8_t brightness = wave * maxBrightness;
      strip.setPixelColor(i, strip.ColorHSV(hue, sat, brightness));
    }
  }

// LED Helper Functions

  void updateAllLEDs() {
    // Update all LED rings
    sensorRingA1.show();
    sensorRingA2.show();
    sensorRingA3.show();
    sensorRingB1.show();
    sensorRingB2.show();
    sensorRingB3.show();
}

void clearAllLEDs() {
    // Clear all LED rings
    sensorRingA1.clear();
    sensorRingA2.clear();
    sensorRingA3.clear();
    sensorRingB1.clear();
    sensorRingB2.clear();
    sensorRingB3.clear();
}
