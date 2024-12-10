#include <CapacitiveSensor.h>

// CapacitiveSensor cs_12_10 = CapacitiveSensor(12,10); // 10 megohm resistor between pins 4 & 2, pin 2 is sensor pin, add wire, foil
CapacitiveSensor cs_13_11 = CapacitiveSensor(13,11);

void setup(){

// cs_12_10.set_CS_AutocaL_Millis(0xFFFFFFFF); // turn off autocalibrate on channel 1 - just as an example Serial.begin(9600);
cs_13_11.set_CS_AutocaL_Millis(0xFFFFFFFF); 

}

void loop(){

long start = millis();

// long total1 = cs_12_10.capacitiveSensor(30);
long total2 = cs_13_11.capacitiveSensor(30);

Serial.print(millis() - start); // check on performance in milliseconds

// Serial.print("\t"); // tab character for debug window spacing

// Serial.println(total1); // print sensor output 1

Serial.print("\t"); // tab character for debug window spacing

Serial.println(total2); // print sensor output 1

delay(10); // arbitrary delay to limit data to serial port

}