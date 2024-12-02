#include <Arduino.h>
#include <MIDIUSB.h>

// Pin definitions
const int buttonPin1 = 10;
const int buttonPin2 = 11;
const int trigPin1 = 2;
const int echoPin1 = 3;
const int trigPin2 = 4;
const int echoPin2 = 5;
const int trigPin3 = 6;
const int echoPin3 = 7;
const int trigPin4 = 8;
const int echoPin4 = 9;

// State variables
int buttonState1 = 0;
int toggleState1 = 0;
int buttonState2 = 0;
int toggleState2 = 0;

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
  // Read button and analog values
  bool buttonVal1 = digitalRead(buttonPin1);
  bool buttonVal2 = digitalRead(buttonPin2);
  int potVal1 = map(analogRead(A0), 0, 1024, 0, 127);
  int potVal2 = map(analogRead(A1), 0, 1024, 0, 127);

  // Measure distances
  int controlVal1 = map(measureDistance(trigPin1, echoPin1), 0, 30, 0, 127);
  int controlVal2 = map(measureDistance(trigPin2, echoPin2), 0, 30, 0, 127);
  int controlVal3 = map(measureDistance(trigPin3, echoPin3), 0, 30, 0, 127);
  int controlVal4 = map(measureDistance(trigPin4, echoPin4), 0, 30, 0, 127);

  // Button 1 state change detection
  if (buttonVal1 != lastButtonState1) {
    lastButtonState1 = buttonVal1;
    toggleState1 = !toggleState1;
    toggleState1 ? controlChange(0, 6, 127) : controlChange(0, 7, 127);
  }

  // Button 2 state change detection
  if (buttonVal2 != lastButtonState2) {
    lastButtonState2 = buttonVal2;
    toggleState2 = !toggleState2;
    toggleState2 ? controlChange(0, 8, 127) : controlChange(0, 9, 127);
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

  // Debug output
  Serial.print("Button1: ");
  Serial.print(buttonVal1);
  Serial.print(" Button2: ");
  Serial.print(buttonVal2);
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