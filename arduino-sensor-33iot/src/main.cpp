#include <Arduino.h>
#include <MIDIUSB.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <Adafruit_NeoPixel.h>

// Update animation state based on sensor reading
void updateSensorAnimation(int sensorIndex, int distance);

// Breathe animation for idle state (slowly pulsing)
void breatheAnimation(Adafruit_NeoPixel &strip, uint16_t hue, uint8_t sat);

// Wave animation for active state (chasing pattern based on animation phase)
void waveAnimation(Adafruit_NeoPixel &strip, uint16_t hue, uint8_t sat, uint16_t phase, int speed);

// Time of Flight Sensors
void setupSensor(Adafruit_VL53L0X &sensor, int xshutPin, uint8_t address);

// Init Time of Flight
Adafruit_VL53L0X sensor1, sensor2, sensor3, sensor4;
#define XSHUT_1 17
#define XSHUT_2 16
#define XSHUT_3 20
#define XSHUT_4 21

// Init LEDs
#define LED_PIN_POT1 7
#define LED_PIN_SENSOR1A 4
#define LED_PIN_SENSOR1B 5
#define LED_PIN_POT2 6
#define LED_PIN_SENSOR2A 2
#define LED_PIN_SENSOR2B 3

#define LED_COUNT_RING 24
#define LED_COUNT_DOT 7

Adafruit_NeoPixel potRing1(LED_COUNT_RING, LED_PIN_POT1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel potRing2(LED_COUNT_RING, LED_PIN_POT2, NEO_GRB + NEO_KHZ800);

Adafruit_NeoPixel sensorRing1A(LED_COUNT_RING, LED_PIN_SENSOR1A, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel sensorRing1B(LED_COUNT_RING, LED_PIN_SENSOR1B, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel sensorDot2A(LED_COUNT_DOT, LED_PIN_SENSOR2A, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel sensorDot2B(LED_COUNT_DOT, LED_PIN_SENSOR2B, NEO_GRB + NEO_KHZ800);

int potInput1 = A0;
int potInput2 = A1;

// distance ranges (mm)
const int SENSOR_MIN_DISTANCE = 50;
const int SENSOR_MAX_DISTANCE = 1200;

// Animation parameters
const int ACTIVITY_THRESHOLD = 10; // mm change to trigger activity
const unsigned long IDLE_TIMEOUT = 2000; // ms until idle animation
const int ANIMATION_MIN_SPEED = 20; // Slowest animation speed (closer objects)
const int ANIMATION_MID_SPEED = 100; // Medium animation speed (at threshold)
const int ANIMATION_MAX_SPEED = 250; // Fastest animation speed (further objects)
const uint8_t IDLE_BRIGHTNESS = 50; // Idle brightness level
const uint8_t ACTIVE_BRIGHTNESS = 255; // Active brightness level

// Animation variables
unsigned long lastActivityTime[4] = {0, 0, 0, 0}; // Track when last activity was detected
uint16_t animationPhase[4] = {0, 0, 0, 0}; // Animation phase for each sensor
int animationSpeed[4] = {0, 0, 0, 0}; // Animation speed for each sensor
bool isActive[4] = {false, false, false, false}; // Activity state for each sensor
int prevDistance[4] = {0, 0, 0, 0}; // Previous distance readings

// control values
int controlVal1 = 0;
int controlVal2 = 0;
int controlVal3 = 0;
int controlVal4 = 0;

// Last values
int lastPotVal1 = 0;
int lastPotVal2 = 0;
int lastControlVal1 = 0;
int lastControlVal2 = 0;
int lastControlVal3 = 0;
int lastControlVal4 = 0;

// MIDI change threshold to reduce unnecessary transmissions
const int MIDI_CHANGE_THRESHOLD = 3;

// Time of Flight Sensors
void setupSensor(Adafruit_VL53L0X &sensor, int xshutPin, uint8_t address) {
    digitalWrite(xshutPin, HIGH);
    delay(100);

    if (!sensor.begin()) {  
        Serial.print("Failed to initialize sensor at pin ");
        Serial.println(xshutPin);
        while (1);
    }

    sensor.setAddress(address);
    Serial.print("Sensor initialized at 0x");
    Serial.println(address, HEX);
}

// Map sensors to control values
int mapSensorToMIDI (int distance) {
  distance = constrain(distance, SENSOR_MIN_DISTANCE, SENSOR_MAX_DISTANCE);

  return map(distance, SENSOR_MIN_DISTANCE, SENSOR_MAX_DISTANCE, 0, 127);
}

// MIDI functions
void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, uint8_t(0x90 | channel), pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, uint8_t(0x80 | channel), pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, uint8_t(0xB0 | channel), control, value};
  MidiUSB.sendMIDI(event);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing...");

  // LED setup
  potRing1.begin();
  potRing2.begin();
  sensorRing1A.begin();
  sensorRing1B.begin();
  sensorDot2A.begin();
  sensorDot2B.begin();

  // Time of Flight Pins / Setup
  pinMode(XSHUT_1, OUTPUT);
  pinMode(XSHUT_2, OUTPUT);
  pinMode(XSHUT_3, OUTPUT);
  pinMode(XSHUT_4, OUTPUT);

  digitalWrite(XSHUT_1, LOW);
  digitalWrite(XSHUT_2, LOW);
  digitalWrite(XSHUT_3, LOW);
  digitalWrite(XSHUT_4, LOW);
  delay(10);

  setupSensor(sensor1, XSHUT_1, 0x30);
  setupSensor(sensor2, XSHUT_2, 0x31);
  setupSensor(sensor3, XSHUT_3, 0x32);
  setupSensor(sensor4, XSHUT_4, 0x33);


  // Small startup delay - optional
  delay(100);
}

void loop() {  

  // Time of Flight Measurement
  VL53L0X_RangingMeasurementData_t measure;
    
    sensor1.rangingTest(&measure, false);
    int distance1 = measure.RangeMilliMeter;
    controlVal1 = mapSensorToMIDI(distance1);
    updateSensorAnimation(0, distance1);

    sensor2.rangingTest(&measure, false);
    int distance2 = measure.RangeMilliMeter;
    controlVal2 = mapSensorToMIDI(distance2);
    updateSensorAnimation(1, distance2);

    sensor3.rangingTest(&measure, false);
    int distance3 = measure.RangeMilliMeter;
    controlVal3 = mapSensorToMIDI(distance3);
    updateSensorAnimation(2, distance3);

    sensor4.rangingTest(&measure, false);
    int distance4 = measure.RangeMilliMeter;
    controlVal4 = mapSensorToMIDI(distance4);
    updateSensorAnimation(3, distance4);
  
  // LED stuff
  int potValMapped1 = map(analogRead(potInput1), 0, 1023, 0, LED_COUNT_RING-1);
  int potValMapped2 = map(analogRead(potInput2), 0, 1023, 0, LED_COUNT_RING-1);

  int potVal1 = map(analogRead(A0), 0, 1023, 0, 127);
  int potVal2 = map(analogRead(A1), 0, 1023, 0, 127);

  // Potentiometer 1 change detection
  if (abs(potVal1 - lastPotVal1) > MIDI_CHANGE_THRESHOLD) {
    lastPotVal1 = potVal1;
    controlChange(0, 14, potVal1);
  }

  // Potentiometer 2 change detection
  if (abs(potVal2 - lastPotVal2) > MIDI_CHANGE_THRESHOLD) {
    lastPotVal2 = potVal2;
    controlChange(0, 15, potVal2);
  }

  // Sensor 1 change detection
  if (abs(controlVal1 - lastControlVal1) > MIDI_CHANGE_THRESHOLD) {
    lastControlVal1 = controlVal1;
    controlChange(0, 10, controlVal1);
  }

  // Sensor 2 change detection
  if (abs(controlVal2 - lastControlVal2) > MIDI_CHANGE_THRESHOLD) {
    lastControlVal2 = controlVal2;
    controlChange(0, 11, controlVal2);
  }

  // Sensor 3 change detection
  if (abs(controlVal3 - lastControlVal3) > MIDI_CHANGE_THRESHOLD) {
    lastControlVal3 = controlVal3;
    controlChange(0, 12, controlVal3);
  }

  // Sensor 4 change detection
  if (abs(controlVal4 - lastControlVal4) > MIDI_CHANGE_THRESHOLD) {
    lastControlVal4 = controlVal4;
    controlChange(0, 13, controlVal4);
  }

  // Periodic MIDI flush to ensure messages are sent
  static unsigned long lastFlushTime = 0;
  if (millis() - lastFlushTime > 10) {  // Flush every 10ms
    MidiUSB.flush();
    lastFlushTime = millis();
  }

  // Clear all LED strips
  potRing1.clear();
  potRing2.clear();
  sensorRing1A.clear();
  sensorRing1B.clear();
  sensorDot2A.clear();
  sensorDot2B.clear();

  // Potentiometer LED updates (keep existing behavior)
  potRing1.fill(potRing1.ColorHSV(1000, 255, 100));
  potRing2.fill(potRing2.ColorHSV(1000, 255, 100));
  
  potRing1.setPixelColor(potValMapped1-1, potRing1.ColorHSV(60, 130, 80));
  potRing1.setPixelColor(potValMapped1, potRing1.ColorHSV(60, 130, 255));
  potRing1.setPixelColor(potValMapped1+1, potRing1.ColorHSV(60, 130, 80));

  potRing2.setPixelColor(potValMapped2-1, potRing2.ColorHSV(60, 130, 80));
  potRing2.setPixelColor(potValMapped2, potRing2.ColorHSV(60, 130, 255));
  potRing2.setPixelColor(potValMapped2+1, potRing2.ColorHSV(60, 130, 80));

  // Sensor LED animations - map each ToF sensor to its dedicated LED

  // ToF sensor 1 (index 0) = LED ring 1A
  if (isActive[0]) {
    waveAnimation(sensorRing1A, 0, 255, animationPhase[0], animationSpeed[0]); // Red color
  } else {
    breatheAnimation(sensorRing1A, 0, 255); // Red color
  }

  // ToF sensor 2 (index 1) = LED ring 1B
  if (isActive[1]) {
    waveAnimation(sensorRing1B, 0, 255, animationPhase[1], animationSpeed[1]); // Red color
  } else {
    breatheAnimation(sensorRing1B, 0, 255); // Red color
  }

  // ToF sensor 3 (index 2) = LED dot 2A
  if (isActive[2]) {
    waveAnimation(sensorDot2A, 43690, 255, animationPhase[2], animationSpeed[2]); // Purple color
  } else {
    breatheAnimation(sensorDot2A, 43690, 255); // Purple color
  }

  // ToF sensor 4 (index 3) = LED dot 2B
  if (isActive[3]) {
    waveAnimation(sensorDot2B, 43690, 255, animationPhase[3], animationSpeed[3]); // Purple color
  } else {
    breatheAnimation(sensorDot2B, 43690, 255); // Purple color
  }

  // Note: Sensors 3 and 4 don't have dedicated LEDs in the current setup
  // You can add more LED animations if you connect additional LED strips

  // Write all LED changes
  potRing1.show();
  potRing2.show();
  sensorRing1A.show();
  sensorRing1B.show();
  sensorDot2A.show();
  sensorDot2B.show();

  // Comprehensive formatted debug output
  static unsigned long lastDebugTime = 0;
  if (millis() - lastDebugTime > 100) {  // Update debug output every 500ms to avoid flooding serial
    Serial.println("\n===== THEREMIN CONTROLLER DEBUG =====");
    
    // Potentiometer readings
    Serial.println("--- Potentiometers ---");
    Serial.print("POT1 (A0): Raw: ");
    Serial.print(analogRead(A0));
    Serial.print(" | Mapped LED: ");
    Serial.print(potValMapped1);
    Serial.print(" | MIDI CC14: ");
    Serial.println(potVal1);
    
    Serial.print("POT2 (A1): Raw: ");
    Serial.print(analogRead(A1));
    Serial.print(" | Mapped LED: ");
    Serial.print(potValMapped2);
    Serial.print(" | MIDI CC15: ");
    Serial.println(potVal2);
    
    // Time of Flight sensors
    Serial.println("--- Time of Flight Sensors ---");
    Serial.print("SENSOR1: Raw: ");
    Serial.print(distance1);
    Serial.print(" mm | MIDI CC10: ");
    Serial.println(controlVal1);
    
    Serial.print("SENSOR2: Raw: ");
    Serial.print(distance2);
    Serial.print(" mm | MIDI CC11: ");
    Serial.println(controlVal2);
    
    Serial.print("SENSOR3: Raw: ");
    Serial.print(distance3);
    Serial.print(" mm | MIDI CC12: ");
    Serial.println(controlVal3);
    
    Serial.print("SENSOR4: Raw: ");
    Serial.print(distance4);
    Serial.print(" mm | MIDI CC13: ");
    Serial.println(controlVal4);
    
    Serial.println("==================================");
    
    lastDebugTime = millis();
  }

  // Small delay to prevent overwhelming the MIDI bus
  delay(1);
}

// Update animation state based on sensor reading
void updateSensorAnimation(int sensorIndex, int distance) {
  // Calculate absolute change in distance
  int distanceChange = abs(distance - prevDistance[sensorIndex]);
  prevDistance[sensorIndex] = distance;
  
  // Check if there's significant movement (using much more sensitive threshold)
  if (distanceChange > 3) { // Reduced from 10 to 3 for higher sensitivity
    // Activity detected, update state
    isActive[sensorIndex] = true;
    lastActivityTime[sensorIndex] = millis();
    
    // Map distance to animation speed with detection threshold at 300mm
    // If distance is less than 300mm, it's close/calm, otherwise it's far/intense
    int thresholdDistance = 300; // New detection threshold at 300mm
    
    // Constrain distance first
    int constrainedDistance = constrain(distance, SENSOR_MIN_DISTANCE, SENSOR_MAX_DISTANCE);
    
    // If distance is below threshold, map to slower animations (close = calm)
    // If distance is above threshold, map to faster animations (far = intense)
    if (constrainedDistance < thresholdDistance) {
      // Below threshold - map from MIN_SPEED to MID_SPEED
      animationSpeed[sensorIndex] = map(constrainedDistance, 
                                      SENSOR_MIN_DISTANCE, thresholdDistance,
                                      ANIMATION_MIN_SPEED, ANIMATION_MID_SPEED);
    } else {
      // Above threshold - map from MID_SPEED to MAX_SPEED
      animationSpeed[sensorIndex] = map(constrainedDistance, 
                                      thresholdDistance, SENSOR_MAX_DISTANCE,
                                      ANIMATION_MID_SPEED, ANIMATION_MAX_SPEED);
    }
  } 
  // Check if we should switch to idle mode - reduce timeout for faster response
  else if (isActive[sensorIndex] && 
          (millis() - lastActivityTime[sensorIndex] > IDLE_TIMEOUT)) {
    isActive[sensorIndex] = false;
  }
  
  // Always increment animation phase - use larger increments for more obvious movement
  animationPhase[sensorIndex] += isActive[sensorIndex] ? 
                              animationSpeed[sensorIndex] : ANIMATION_MIN_SPEED/2;
}

// Breathe animation for idle state (slowly pulsing)
void breatheAnimation(Adafruit_NeoPixel &strip, uint16_t hue, uint8_t sat) {
  // Create breathing effect with sine wave
  float breath = (sin(millis() / 2000.0 * PI) + 1.0) / 2.0;
  uint8_t brightness = IDLE_BRIGHTNESS * breath + IDLE_BRIGHTNESS/2;
  
  // Apply to all pixels
  for(int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.ColorHSV(hue, sat, brightness));
  }
}

// Wave animation for active state (chasing pattern based on animation phase)
void waveAnimation(Adafruit_NeoPixel &strip, uint16_t hue, uint8_t sat, uint16_t phase, int speed) {
  int numPixels = strip.numPixels();
  
  // Calculate brightness based on speed (higher speed = more intense color)
  uint8_t maxBrightness = map(speed, ANIMATION_MIN_SPEED, ANIMATION_MAX_SPEED, 
                           ACTIVE_BRIGHTNESS/2, ACTIVE_BRIGHTNESS);
                           
  // Calculate intensity falloff based on speed (faster = sharper waves)
  int falloff = map(speed, ANIMATION_MIN_SPEED, ANIMATION_MAX_SPEED, 2, 4);
  
  // Create wave pattern
  for(int i=0; i < numPixels; i++) {
    // Calculate wave position with wraparound
    float wavePos = (i + (phase / 100.0)) / (float)numPixels;
    wavePos = wavePos - floor(wavePos);  // Keep between 0-1
    
    // Create sine wave brightness
    float wave = (sin(wavePos * 2 * PI * falloff) + 1.0) / 2.0;
    uint8_t brightness = wave * maxBrightness;
    
    strip.setPixelColor(i, strip.ColorHSV(hue, sat, brightness));
  }
}