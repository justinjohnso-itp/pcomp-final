#include <Adafruit_NeoPixel.h>

#define LED_PIN_POT1 0
#define LED_PIN_POT2 1
#define LED_PIN_SENSOR1A 2
#define LED_PIN_SENSOR1B 3
#define LED_PIN_SENSOR2A 4
#define LED_PIN_SENSOR2B 5

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
const int numReadings = 25;

int readings[numReadings];  // the readings from the analog input
int readIndex = 0;          // the index of the current reading
int total = 0;              // the running total
int potValAverage1 = 0;     // the average

void setup() {
  // potRing1.setBrightness(50);
  // potRing2.setBrightness(50);

  potRing1.begin();
  potRing2.begin();
  sensorRing1A.begin();
  sensorRing1B.begin();
  sensorDot2A.begin();
  sensorDot2B.begin();

  Serial.begin(115200);

  // Set up smoothing
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }

}

void loop() {

  /*
  // Start smoothing
  */
  
  total = total - readings[readIndex]; // subtract the last reading
  readings[readIndex] = analogRead(potInput1); // read from the sensor
  total = total + readings[readIndex]; // add the reading to the total
  readIndex = readIndex + 1; // advance to the next position in the array

  if (readIndex >= numReadings) { // if we're at the end of the array...
    readIndex = 0; // ...wrap around to the beginning
  }

  potValAverage1 = total / numReadings; // calculate the average:
  delay(1);  // delay in between reads for stability

  /*
  // Endmoothing
  */

  Serial.print(">potValAverage1:");
  Serial.println(potValAverage1);

  int potValMapped1 = map(potValAverage1, 0, 1023, 0, LED_COUNT_RING-1);

  potRing1.clear();
  potRing2.clear();
  sensorRing1A.clear();
  sensorRing1B.clear();
  sensorDot2A.clear();
  sensorDot2B.clear();

  potRing1.fill(potRing1.ColorHSV(0, 255, 100));
  potRing2.fill(potRing2.ColorHSV(0, 255, 100));
  sensorRing1A.fill(sensorRing1A.ColorHSV(0, 255, 100));
  sensorRing1B.fill(sensorRing1B.ColorHSV(0, 255, 100));
  sensorDot2A.fill(sensorDot2A.ColorHSV(0, 255, 100));
  sensorDot2B.fill(sensorDot2B.ColorHSV(0, 255, 100));
  
  // potRing1.setPixelColor(potValMapped1-1, potRing1.ColorHSV(60, 130, 80));
  // potRing1.setPixelColor(potValMapped1, potRing1.ColorHSV(60, 130, 255));
  // potRing1.setPixelColor(potValMapped1+1, potRing1.ColorHSV(60, 130, 80));

  // Write to LEDs
  potRing1.show();
  potRing2.show();
  sensorRing1A.show();
  sensorRing1B.show();
  sensorDot2A.show();
  sensorDot2B.show();


  // potRing1.show();

  // for(int i=0; i<LED_COUNT_RING; i++) {
  //   potRing1.setPixelColor(i, potRing1.Color(0, 150, 0));
  //   potRing1.show();
  //   potRing2.setPixelColor(i, potRing2.Color(123, 0, 55));
  //   potRing2.show();
  //   delay(500);
  // }
}