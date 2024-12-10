#include <CapacitiveSensor.h>

CapacitiveSensor cs_4_2 = CapacitiveSensor(4, 2);
CapacitiveSensor cs_5_3 = CapacitiveSensor(5, 3);


const long threshold = 1000;

void setup() {

  cs_4_2.set_CS_AutocaL_Millis(0xFFFFFFFF);
  cs_5_3.set_CS_AutocaL_Millis(0xFFFFFFFF);


  Serial.begin(9600);
}

void loop() {
  // Read values
  long total1 = cs_4_2.capacitiveSensor(30);
  long total2 = cs_5_3.capacitiveSensor(30);

  if (total1 > threshold) {
    Serial.println("Button 1 Pressed");
  }
  if (total2 > threshold) {
    Serial.println("Button 2 Pressed");
  }

}
