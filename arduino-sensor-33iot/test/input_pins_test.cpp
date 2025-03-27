#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>

// Define sensor objects and pins
Adafruit_VL53L0X sensor1, sensor2, sensor3, sensor4;
#define XSHUT_1 16
#define XSHUT_2 17
#define XSHUT_3 20
#define XSHUT_4 21

// Define analog potentiometer inputs
int potInput1 = A0;
int potInput2 = A1;

// Set up a sensor using the XSHUT pin and desired I2C address
void setupSensor(Adafruit_VL53L0X &sensor, int xshutPin, uint8_t address) {
  pinMode(xshutPin, OUTPUT);
  digitalWrite(xshutPin, LOW);
  delay(10);
  digitalWrite(xshutPin, HIGH);
  delay(100);

  if (!sensor.begin()) {
    Serial.print("Failed to initialize sensor at pin ");
    Serial.println(xshutPin);
    while (1);  // halt if sensor init fails
  }
  
  sensor.setAddress(address);
  Serial.print("Sensor initialized at 0x");
  Serial.println(address, HEX);
}

void setup() {
  Serial.begin(115200);
  // Allow time for Serial Monitor to start (or use while(!Serial); if supported)
  delay(4000);
  
  Serial.println("Input Pins Test: Starting...");
  
  // Initialize the sensors (using the defined XSHUT pins and addresses)
  setupSensor(sensor1, XSHUT_1, 0x30);
  setupSensor(sensor2, XSHUT_2, 0x31);
  setupSensor(sensor3, XSHUT_3, 0x32);
  setupSensor(sensor4, XSHUT_4, 0x33);
}

void loop() {
  // Read analog inputs for potentiometers
  int potVal1 = analogRead(potInput1);
  int potVal2 = analogRead(potInput2);
  
  Serial.print("Potentiometer 1 (A0): ");
  Serial.println(potVal1);
  
  Serial.print("Potentiometer 2 (A1): ");
  Serial.println(potVal2);
  
  // Read each VL53L0X sensor
  VL53L0X_RangingMeasurementData_t measure;
    
  sensor1.rangingTest(&measure, false);
  Serial.print("Sensor 1 (XSHUT ");
  Serial.print(XSHUT_1);
  Serial.print("): ");
  Serial.println(measure.RangeMilliMeter);
  
  sensor2.rangingTest(&measure, false);
  Serial.print("Sensor 2 (XSHUT ");
  Serial.print(XSHUT_2);
  Serial.print("): ");
  Serial.println(measure.RangeMilliMeter);
  
  sensor3.rangingTest(&measure, false);
  Serial.print("Sensor 3 (XSHUT ");
  Serial.print(XSHUT_3);
  Serial.print("): ");
  Serial.println(measure.RangeMilliMeter);
  
  sensor4.rangingTest(&measure, false);
  Serial.print("Sensor 4 (XSHUT ");
  Serial.print(XSHUT_4);
  Serial.print("): ");
  Serial.println(measure.RangeMilliMeter);
  
  Serial.println("---------------------------");
  delay(1000); // Wait 1 second between readings
}