#include <Arduino.h>

const int trigPin1 = 2;
const int echoPin1 = 3;
const int trigPin2 = 4;
const int echoPin2 = 5;

void setup() {
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  Serial.begin(9600);
}

// Function to measure the distance using the ultrasonic sensor
int measureDistance(int trigPin, int echoPin) {
  // Pulse the trigger pin
  digitalWrite(trigPin, LOW); // Pin low (start pulse)
  digitalWrite(trigPin, HIGH); // Pin high (end pulse)

  long duration = pulseIn(echoPin, HIGH); // Listen for a pulse on the echo pin
  int distance = (duration * 0.0343) / 2; // Calculate the distance in cm.
  // Limit the distance to the range between 0 and 30 cm
  if (distance > 30) {
    distance = 30;
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