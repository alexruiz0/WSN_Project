void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 OK");
}

void loop() {
  Serial.println(millis());
  delay(1000);
}