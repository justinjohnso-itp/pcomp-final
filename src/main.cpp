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
}

void loop() {
}