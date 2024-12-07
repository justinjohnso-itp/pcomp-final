#include <Adafruit_NeoPixel.h>
#define LED_PIN1 16
#define LED_PIN2 17
#define LED_COUNT 24

Adafruit_NeoPixel ring1(LED_COUNT, LED_PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ring2(LED_COUNT, LED_PIN2, NEO_GRB + NEO_KHZ800);

const int numReadings = 25;

int readings[numReadings];  // the readings from the analog input
int readIndex = 0;          // the index of the current reading
int total = 0;              // the running total
int potValAverage1 = 0;     // the average

int inputPin = A0;

void setup() {
  ring1.setBrightness(50);
  ring1.begin();

  ring2.setBrightness(50);
  ring2.begin();

  Serial.begin(115200);
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
}

void loop() {
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = analogRead(inputPin);
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  potValAverage1 = total / numReadings;
  delay(1);  // delay in between reads for stability

  Serial.print(">potValAverage1:");
  Serial.println(potValAverage1);

  int potValClean1 = map(potValAverage1, 0, 1023, 0, LED_COUNT-1);

  ring1.clear();
  ring2.clear();

  ring1.fill(ring1.ColorHSV(0, 255, 20), 0);
  ring1.fill(ring1.ColorHSV(0, 255, 20), 13);
  ring1.show();

  ring1.setPixelColor(potValClean1-1, ring1.ColorHSV(60, 130, 80));
  ring1.setPixelColor(potValClean1, ring1.ColorHSV(60, 130, 255));
  ring1.setPixelColor(potValClean1+1, ring1.ColorHSV(60, 130, 80));
  ring1.show();

  // for(int i=0; i<LED_COUNT; i++) {
  //   ring1.setPixelColor(i, ring1.Color(0, 150, 0));
  //   ring1.show();
  //   ring2.setPixelColor(i, ring2.Color(123, 0, 55));
  //   ring2.show();
  //   delay(500);
  // }
}