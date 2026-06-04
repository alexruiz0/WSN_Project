#define PIN_BUZZER 26

void setup() {
}

void loop() {

  tone(PIN_BUZZER, 1000);
  delay(500);

  noTone(PIN_BUZZER);
  delay(500);
}