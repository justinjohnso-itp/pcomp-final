#include <CapacitiveSensor.h>

CapacitiveSensor cs_4_2 = CapacitiveSensor(4, 2); 
CapacitiveSensor cs_5_3 = CapacitiveSensor(5, 3);


const long threshold = 1000;

bool buttonState1 = false;
bool buttonState2 = false;
bool buttonVal1 = false;
bool buttonVal2 = false;

void setup() {

  cs_4_2.set_CS_AutocaL_Millis(0xFFFFFFFF);
  cs_5_3.set_CS_AutocaL_Millis(0xFFFFFFFF);


  Serial.begin(9600);
}

void loop() {
  // Read values
  long total1 = cs_4_2.capacitiveSensor(30);
  long total2 = cs_5_3.capacitiveSensor(30);

  buttonVal1 = (total1 > threshold);
  buttonVal2 = (total2 > threshold);

  if (buttonVal1 != buttonState1) {
    buttonState1 = buttonVal1;
    if (!buttonState1) {
      toggleState1 = !toggleState1;
    }
    toggleState1 ? controlChange(0, 6, 127) : controlChange(0, 7, 127);
  }

  if (buttonVal2 != buttonState2) {
    buttonState2 = buttonVal2;
    if (!buttonState2) {
      toggleState2 = !toggleState2;
    }
    toggleState2 ? controlChange(0, 8, 127) : controlChange(0, 9, 127);
  }

  }

}
