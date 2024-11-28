#include <Ewma.h>
#include <EwmaT.h>

#include <Arduino.h>
#include <BLEMidi.h> // https://github.com/max22-/ESP32-BLE-MIDI

const int ledPin = LED_BUILTIN;  // set ledPin to on-board LED
const int buttonPin = 10;
const int trigPin1 = 2;
const int echoPin1 = 3;
const int trigPin2 = 4;
const int echoPin2 = 5;

int buttonState = 0;
int toggleState = 0;
Ewma smooth1(0.08);
Ewma smooth2(0.08);

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing bluetooth");
  BLEMidiServer.begin("Arduino Nano ESP32");
  Serial.println("Waiting for connections...");
  BLEMidiServer.enableDebugging();  // Uncomment if you want to see some debugging output from the library

  pinMode(ledPin, OUTPUT);    // use the LED as an output
  pinMode(buttonPin, INPUT);  // use button pin as an input
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

}

// Function to measure the distance using the ultrasonic sensor
float measureDistance(int trigPin, int echoPin) {
  // Take the trigger pin low to start a pulse:
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Take the trigger pin high:
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  // Take the trigger pin low again to complete the pulse:
  digitalWrite(trigPin, LOW);

  // Listen for a pulse on the echo pin:
  long duration = pulseIn(echoPin, HIGH);

  // Calculate the distance in cm.
  float distance = (duration * 0.0343) / 2;

  // Limit the distance to the range between 0 and 47 cm
  if (distance > 30) {
    distance = 30;
  } else if (distance < 0) {
    distance = 0;
  }

  return distance;
}

void loop() {
  bool buttonVal = digitalRead(buttonPin);
  float controlVal1, controlVal2;

  // Measure distance for all sensors
  float distance1 = measureDistance(trigPin1, echoPin1);
  float distance2 = measureDistance(trigPin2, echoPin2);

  if(BLEMidiServer.isConnected()) {  // Only act if MIDI is connected

    // Button latch
    if (buttonVal != buttonState) {
      buttonState = buttonVal;
      if (!buttonState) {
        toggleState = !toggleState;
      }
      toggleState ? BLEMidiServer.noteOn(0, 42, 127) : BLEMidiServer.noteOff(0, 42, 127);
    }

    // MIDI CC
    controlVal1 = map(smooth1.filter(distance1), 0, 30, 0, 127);
    controlVal2 = map(smooth2.filter(distance2), 0, 30, 0, 127);    
    BLEMidiServer.controlChange(1, 12, controlVal1);
    BLEMidiServer.controlChange(2, 13, controlVal2);

    Serial.print(buttonState);
    Serial.print(", ");
    Serial.print(controlVal1);
    Serial.print(", ");
    Serial.println(controlVal2);
  }
}