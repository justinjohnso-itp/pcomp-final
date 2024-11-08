#include <Arduino.h>

const int trigPin1 = 0;
const int echoPin1 = 1;
const int trigPin2 = 2;
const int echoPin2 = 3;

void setup() {
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  Serial.begin(9600);
}

// Function to measure the distance using the ultrasonic sensor
int measureDistance(int trigPin, int echoPin) {
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
  int distance = (duration * 0.0343) / 2;

  // Limit the distance to the range between 0 and 47 cm
  if (distance > 47) {
    distance = 47;
  } else if (distance < 0) {
    distance = 0;
  }

  return distance;
}

void loop() {
  // Measure distance for all sensors
  int distance1 = measureDistance(trigPin1, echoPin1);
  int distance2 = measureDistance(trigPin2, echoPin2);

  Serial.print(distance1);
  Serial.print(", ");
  Serial.println(distance2);
}