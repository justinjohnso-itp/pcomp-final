#include <Arduino.h>
#include <MIDIUSB.h>
#include <Adafruit_NeoPixel.h>
#include <CapacitiveSensor.h>

CapacitiveSensor cs_12_10 = CapacitiveSensor(12,10); // 10 megohm resistor between pins 4 & 2, pin 2 is sensor pin, add wire, foil
CapacitiveSensor cs_13_11 = CapacitiveSensor(13,11);

const long capacitiveThreshold = 350;

// Init LEDs
#define LED_PIN_POT1 18
#define LED_PIN_SENSOR1A 16
#define LED_PIN_SENSOR1B 17 
#define LED_PIN_POT2 21
#define LED_PIN_SENSOR2A 19
#define LED_PIN_SENSOR2B 20

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
// const int numReadings = 25;

// int readings[numReadings];  // the readings from the analog input
// int readIndex = 0;          // the index of the current reading
// int total = 0;              // the running total
// int potValAverage1 = 0;     // the average

// Pin definitions
const int buttonPin1 = 10;
const int buttonPin2 = 11;
const int trigPin1 = 3;
const int echoPin1 = 2;
const int trigPin2 = 5;
const int echoPin2 = 4;
const int trigPin3 = 7;
const int echoPin3 = 6;
const int trigPin4 = 9;
const int echoPin4 = 8;

// State variables
// int buttonState1 = 0;
// int toggleState1 = 0;
// int buttonState2 = 0;
// int toggleState2 = 0;

int buttonState = 0;
int toggleState = 0;

// Last state tracking for change detection
bool lastButtonState1 = LOW;
bool lastButtonState2 = LOW;
int lastPotVal1 = 0;
int lastPotVal2 = 0;
int lastControlVal1 = 0;
int lastControlVal2 = 0;
int lastControlVal3 = 0;
int lastControlVal4 = 0;

// MIDI change threshold to reduce unnecessary transmissions
const int MIDI_CHANGE_THRESHOLD = 3;

// Function to measure the distance using the ultrasonic sensor
float measureDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  float distance = (duration * 0.0343) / 2;

  // Clamp distance to range 0-30 cm
  if (distance > 30) {
    distance = 30;
  } else if (distance < 0) {
    distance = 0;
  }

  return distance;
}

// MIDI functions
void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
  MidiUSB.flush();
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
  MidiUSB.flush();
}

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
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

  // // Set up smoothing
  // for (int thisReading = 0; thisReading < numReadings; thisReading++) {
  //   readings[thisReading] = 0;
  // }

  // Capacative sensors
  cs_12_10.set_CS_AutocaL_Millis(0xFFFFFFFF); // turn off autocalibrate on channel 1 - just as an example Serial.begin(9600);
  cs_13_11.set_CS_AutocaL_Millis(0xFFFFFFFF); 

  // Pin setup
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(trigPin3, OUTPUT);
  pinMode(echoPin3, INPUT);
  pinMode(trigPin4, OUTPUT);
  pinMode(echoPin4, INPUT);

  // Optional: Small startup delay
  delay(100);
}

void loop() {  
  // /*
  // // Start smoothing
  // */
  
  // total = total - readings[readIndex]; // subtract the last reading
  // readings[readIndex] = analogRead(potInput1); // read from the sensor
  // total = total + readings[readIndex]; // add the reading to the total
  // readIndex = readIndex + 1; // advance to the next position in the array

  // if (readIndex >= numReadings) { // if we're at the end of the array...
  //   readIndex = 0; // ...wrap around to the beginning
  // }

  // potValAverage1 = total / numReadings; // calculate the average:
  // delay(1);  // delay in between reads for stability

  // /*
  // // Endmoothing
  // */

  // LED stuff
  int potValMapped1 = map(analogRead(potInput1), 0, 1023, 0, LED_COUNT_RING-1);
  int potValMapped2 = map(analogRead(potInput2), 0, 1023, 0, LED_COUNT_RING-1);

  // Serial.print(">potValMapped1:");
  // Serial.println(potValMapped1);

  // Serial.print(">potValMapped2:");
  // Serial.println(potValMapped2);

  // Read values
  // long total1 = cs_12_10.capacitiveSensor(30);
  long capacitiveVal = cs_13_11.capacitiveSensor(30);
  bool buttonVal = capacitiveVal > capacitiveThreshold ? 1 : 0;

  // Read button and analog values
  // bool buttonVal1 = digitalRead(buttonPin1);
  // bool buttonVal2 = digitalRead(buttonPin2);
  int potVal1 = map(analogRead(A0), 0, 1024, 0, 127);
  int potVal2 = map(analogRead(A1), 0, 1024, 0, 127);

  // Serial.print(">potVal1:");
  // Serial.println(potVal1);

  // Serial.print(">potVal2:");
  // Serial.println(potVal2);

  // // Measure distances
  int controlVal1 = map(measureDistance(trigPin1, echoPin1), 0, 30, 0, 127);
  int controlVal2 = map(measureDistance(trigPin2, echoPin2), 0, 30, 0, 127);
  int controlVal3 = map(measureDistance(trigPin3, echoPin3), 0, 30, 0, 127);
  int controlVal4 = map(measureDistance(trigPin4, echoPin4), 0, 30, 0, 127);

  //  // Button latch #1
  //  if (buttonVal1 != buttonState1) {
  //    buttonState1 = buttonVal1;
  //    if (!buttonState1) {
  //      toggleState1 = !toggleState1;
  //    }
  //    toggleState1 ? controlChange(0, 6, 127) : controlChange(0, 7, 127);
  //  }

  //  // Button latch #2
  //  if (buttonVal2 != buttonState2) {
  //    buttonState2 = buttonVal2;
  //    if (!buttonState2) {
  //      toggleState2 = !toggleState2;
  //    }
  //    toggleState2 ? controlChange(0, 8, 127) : controlChange(0, 9, 127);
  //  }

  // Capacitive latch
  if (buttonVal != buttonState) {
    buttonState = buttonVal;
    if (!buttonState) {
      toggleState = !toggleState;
    }
    toggleState ? controlChange(0, 6, 127) : controlChange(0, 7, 127);
  }

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

  // Distance sensor 1 change detection
  if (abs(controlVal1 - lastControlVal1) > MIDI_CHANGE_THRESHOLD) {
    lastControlVal1 = controlVal1;
    controlChange(0, 10, controlVal1);
  }

  // Distance sensor 2 change detection
  if (abs(controlVal2 - lastControlVal2) > MIDI_CHANGE_THRESHOLD) {
    lastControlVal2 = controlVal2;
    controlChange(0, 11, controlVal2);
  }

  // Distance sensor 3 change detection
  if (abs(controlVal3 - lastControlVal3) > MIDI_CHANGE_THRESHOLD) {
    lastControlVal3 = controlVal3;
    controlChange(0, 12, controlVal3);
  }

  // Distance sensor 4 change detection
  if (abs(controlVal4 - lastControlVal4) > MIDI_CHANGE_THRESHOLD) {
    lastControlVal4 = controlVal4;
    controlChange(0, 13, controlVal4);
  }

  // Optional periodic MIDI flush to ensure messages are sent
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
  Serial.print(" Button: ");
  Serial.print(toggleState);
  Serial.print(", Pot1: ");
  Serial.print(potVal1);
  Serial.print(", Pot2: ");
  Serial.print(potVal2);
  Serial.print(", Distance1: ");
  Serial.print(controlVal1);
  Serial.print(", Distance2: ");
  Serial.print(controlVal2);
  Serial.print(", Distance3: ");
  Serial.print(controlVal3);
  Serial.print(", Distance4: ");
  Serial.println(controlVal4);

  // Small delay to prevent overwhelming the MIDI bus
  delay(1);
}