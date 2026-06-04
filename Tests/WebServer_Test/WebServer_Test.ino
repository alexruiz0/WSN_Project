#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* SSID = "mywifi";
const char* PASSWORD = "password";

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);

  WiFi.begin(SSID, PASSWORD);

  Serial.print("Conectando WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Ruta principal
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Servidor Async OK");
  });

  // Ruta de prueba extra
  server.on("/test", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", "{\"status\":\"ok\"}");
  });

  server.begin();

  Serial.println("Servidor iniciado");
}

void loop() {
  // vacío (async no necesita loop)
}