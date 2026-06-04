#include <DHT.h>

#define PIN_DHT 27
#define DHT_TYPE DHT11

DHT dht(PIN_DHT, DHT_TYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
}

void loop() {

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  Serial.print("Temp: ");
  Serial.print(t);

  Serial.print("  Hum: ");
  Serial.println(h);

  delay(2000);
}
