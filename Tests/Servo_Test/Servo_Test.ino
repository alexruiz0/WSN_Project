#define PIN_SERVO 14

#include <ESP32Servo.h>

Servo servo;

void setup() {
  servo.setPeriodHertz(50);
  servo.attach(PIN_SERVO, 500, 2400);
}

void loop() {
  servo.write(0);
  delay(1000);

  servo.write(90);
  delay(1000);

  servo.write(180);
  delay(1000);
}