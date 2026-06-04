#include <WiFi.h>

const char* SSID = "mywifi";
const char* PASSWORD = "password";

void setup() {
  Serial.begin(115200);

  WiFi.begin(SSID, PASSWORD);

  Serial.print("Conectando");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
}