#include <Arduino.h>
#include <MIDIUSB.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <Adafruit_NeoPixel.h>

// Init Time of Flight
Adafruit_VL53L0X sensor1, sensor2, sensor3, sensor4;
#define XSHUT_1 16
#define XSHUT_2 17
#define XSHUT_3 20
#define XSHUT_4 21

// Init LEDs
#define LED_PIN_POT1 7
#define LED_PIN_SENSOR1A 5
#define LED_PIN_SENSOR1B 4
#define LED_PIN_POT2 6
#define LED_PIN_SENSOR2A 3
#define LED_PIN_SENSOR2B 2

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
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
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

    sensor2.rangingTest(&measure, false);
    int distance2 = measure.RangeMilliMeter;
    controlVal2 = mapSensorToMIDI(distance2);

    sensor3.rangingTest(&measure, false);
    int distance3 = measure.RangeMilliMeter;
    controlVal3 = mapSensorToMIDI(distance3);

    sensor4.rangingTest(&measure, false);
    int distance4 = measure.RangeMilliMeter;
    controlVal4 = mapSensorToMIDI(distance3);
  
  // LED stuff
  int potValMapped1 = map(analogRead(potInput1), 0, 1023, 0, LED_COUNT_RING-1);
  int potValMapped2 = map(analogRead(potInput2), 0, 1023, 0, LED_COUNT_RING-1);

  Serial.print(">potValMapped1:");
  Serial.println(potValMapped1);

  Serial.print(">potValMapped2:");
  Serial.println(potValMapped2);

  int potVal1 = map(analogRead(A0), 0, 1023, 0, 127);
  int potVal2 = map(analogRead(A1), 0, 1023, 0, 127);

  Serial.print(">potVal1:");
  Serial.println(potVal1);

  Serial.print(">potVal2:");
  Serial.println(potVal2);

  Serial.print("Sensor 1: "); Serial.println(distance1);
  Serial.print("Sensor 2: "); Serial.println(distance2);
  Serial.print("Sensor 3: "); Serial.println(distance3);
  Serial.print("Sensor 4: "); Serial.println(distance4);

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

  potRing1.clear();
  potRing2.clear();
  sensorRing1A.clear();
  sensorRing1B.clear();
  sensorDot2A.clear();
  sensorDot2B.clear();

  potRing1.fill(potRing1.ColorHSV(1000, 255, 100));
  potRing2.fill(potRing2.ColorHSV(1000, 255, 100));
  sensorRing1A.fill(sensorRing1A.ColorHSV(255,80,100));
  sensorRing1B.fill(sensorRing1B.ColorHSV(0, 255, 100));
  sensorDot2A.fill(sensorDot2A.ColorHSV(0, 255, 100));
  sensorDot2B.fill(sensorDot2B.ColorHSV(0, 255, 100));
  
  potRing1.setPixelColor(potValMapped1-1, potRing1.ColorHSV(60, 130, 80));
  potRing1.setPixelColor(potValMapped1, potRing1.ColorHSV(60, 130, 255));
  potRing1.setPixelColor(potValMapped1+1, potRing1.ColorHSV(60, 130, 80));

  potRing2.setPixelColor(potValMapped2-1, potRing2.ColorHSV(60, 130, 80));
  potRing2.setPixelColor(potValMapped2, potRing2.ColorHSV(60, 130, 255));
  potRing2.setPixelColor(potValMapped2+1, potRing2.ColorHSV(60, 130, 80));

  // Write to LEDs
  potRing1.show();
  potRing2.show();
  sensorRing1A.show();
  sensorRing1B.show();
  sensorDot2A.show();
  sensorDot2B.show();

  // Debug output
  Serial.print(", Pot1: ");
  Serial.print(potVal1);
  Serial.print(", Pot2: ");
  Serial.print(potVal2);
  Serial.print(", Sensor 1: ");
  Serial.print(controlVal1);
  Serial.print(", Sensor 2: ");
  Serial.print(controlVal2);
  Serial.print(", Sensor 3: ");
  Serial.print(controlVal3);
  Serial.print(", Sensor 4: ");
  Serial.println(controlVal4);

  // Small delay to prevent overwhelming the MIDI bus
  delay(1);
}