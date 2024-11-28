#include <Ewma.h>
#include <EwmaT.h>
#include <Arduino.h>
#include <BLEMidi.h> // https://github.com/max22-/ESP32-BLE-MIDI
#include <Adafruit_NeoPixel.h>

#define LED_PIN 12
#define LED_COUNT 144

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN);

const int buttonPin = 10;
const int trigPin1 = 2;
const int echoPin1 = 3;
const int trigPin2 = 4;
const int echoPin2 = 5;

int buttonState = 0;
int toggleState = 0;
Ewma smooth1(0.1);
Ewma smooth2(0.1);
Ewma smooth3(1);

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing bluetooth");
  BLEMidiServer.begin("Arduino Nano ESP32");
  Serial.println("Waiting for connections...");
  // BLEMidiServer.enableDebugging();  // Uncomment if you want to see some debugging output from the library

  pinMode(buttonPin, INPUT);  // use button pin as an input
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  strip.begin();
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
  int potVal = map(smooth3.filter(analogRead(A0)), 0, 4096, 0, 127);
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

    // Potentiometer 
    // BLEMidiServer.controlChange(2, 13, controlVal2);

    // MIDI CC
    controlVal1 = map(smooth1.filter(distance1), 0, 30, 0, 127);
    controlVal2 = map(smooth2.filter(distance2), 0, 30, 0, 127);    
    BLEMidiServer.controlChange(0, 12, controlVal1);
    BLEMidiServer.controlChange(0, 13, controlVal2);
    BLEMidiServer.controlChange(0, 14, potVal);

    Serial.print(buttonState);
    Serial.print(", ");
    Serial.print(potVal);
    Serial.print(", ");
    Serial.print(controlVal1);
    Serial.print(", ");
    Serial.println(controlVal2);
  }
}

//
//
// Neopixel test
//
//

// #include <Adafruit_NeoPixel.h>

// #ifdef __AVR__
// #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
// #endif
// class Strip
// {
// public:
//   uint8_t   effect;
//   uint8_t   effects;
//   uint16_t  effStep;
//   unsigned long effStart;
//   Adafruit_NeoPixel strip;
//   Strip(uint16_t leds, uint8_t pin, uint8_t toteffects, uint16_t striptype) : strip(leds, pin, striptype) {
//     effect = -1;
//     effects = toteffects;
//     Reset();
//   }
//   void Reset(){
//     effStep = 0;
//     effect = (effect + 1) % effects;
//     effStart = millis();
//   }
// };

// struct Loop
// {
//   uint8_t currentChild;
//   uint8_t childs;
//   bool timeBased;
//   uint16_t cycles;
//   uint16_t currentTime;
//   Loop(uint8_t totchilds, bool timebased, uint16_t tottime) {currentTime=0;currentChild=0;childs=totchilds;timeBased=timebased;cycles=tottime;}
// };

// Strip strip_0(144, 17, 144, NEO_GRB + NEO_KHZ800);
// struct Loop strip0loop0(1, false, 1);

// //[GLOBAL_VARIABLES]

// void setup() {

//   #if defined(__AVR_ATtiny85__) && (F_CPU == 8000000)
//   clock_prescale_set(clock_div_1);
//   #endif
//   //Your setup here:

//   strip_0.strip.begin();
// }

// uint8_t strip0_loop0_eff0() {
//     // Strip ID: 0 - Effect: Rainbow - LEDS: 144
//     // Steps: 144 - Delay: 20
//     // Colors: 3 (255.0.0, 0.255.0, 0.0.255)
//     // Options: rainbowlen=144, toLeft=true, 
//   if(millis() - strip_0.effStart < 20 * (strip_0.effStep)) return 0x00;
//   float factor1, factor2;
//   uint16_t ind;
//   for(uint16_t j=0;j<144;j++) {
//     ind = strip_0.effStep + j * 1;
//     switch((int)((ind % 144) / 48)) {
//       case 0: factor1 = 1.0 - ((float)(ind % 144 - 0 * 48) / 48);
//               factor2 = (float)((int)(ind - 0) % 144) / 48;
//               strip_0.strip.setPixelColor(j, 255 * factor1 + 0 * factor2, 0 * factor1 + 255 * factor2, 0 * factor1 + 0 * factor2);
//               break;
//       case 1: factor1 = 1.0 - ((float)(ind % 144 - 1 * 48) / 48);
//               factor2 = (float)((int)(ind - 48) % 144) / 48;
//               strip_0.strip.setPixelColor(j, 0 * factor1 + 0 * factor2, 255 * factor1 + 0 * factor2, 0 * factor1 + 255 * factor2);
//               break;
//       case 2: factor1 = 1.0 - ((float)(ind % 144 - 2 * 48) / 48);
//               factor2 = (float)((int)(ind - 96) % 144) / 48;
//               strip_0.strip.setPixelColor(j, 0 * factor1 + 255 * factor2, 0 * factor1 + 0 * factor2, 255 * factor1 + 0 * factor2);
//               break;
//     }
//   }
//   if(strip_0.effStep >= 144) {strip_0.Reset(); return 0x03; }
//   else strip_0.effStep++;
//   return 0x01;
// }

// uint8_t strip0_loop0() {
//   uint8_t ret = 0x00;
//   switch(strip0loop0.currentChild) {
//     case 0: 
//            ret = strip0_loop0_eff0();break;
//   }
//   if(ret & 0x02) {
//     ret &= 0xfd;
//     if(strip0loop0.currentChild + 1 >= strip0loop0.childs) {
//       strip0loop0.currentChild = 0;
//       if(++strip0loop0.currentTime >= strip0loop0.cycles) {strip0loop0.currentTime = 0; ret |= 0x02;}
//     }
//     else {
//       strip0loop0.currentChild++;
//     }
//   };
//   return ret;
// }

// void strips_loop() {
//   if(strip0_loop0() & 0x01)
//     strip_0.strip.show();
// }

// void loop() {

//   //Your code here:

//   strips_loop();
// }