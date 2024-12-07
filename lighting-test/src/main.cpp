#include <Adafruit_NeoPixel.h>
#define LED_PIN1 16
#define LED_PIN2 17
#define LED_COUNT 24

Adafruit_NeoPixel ring1(LED_COUNT, LED_PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ring2(LED_COUNT, LED_PIN2, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 500

void setup() {
  ring1.setBrightness(50);
  ring1.begin();

  ring2.setBrightness(50);
  ring2.begin();
}

void loop() {
  ring1.clear();
  ring2.clear();

  for(int i=0; i<LED_COUNT; i++) {
    ring1.setPixelColor(i, ring1.Color(0, 150, 0));
    ring1.show();
    ring2.setPixelColor(i, ring2.Color(123, 0, 55));
    ring2.show();
    delay(DELAYVAL);
  }
}