#include <Servo.h>
#include <DHT.h>

// ================= PIN CONFIG =================
#define PIN_TRIG    7
#define PIN_ECHO    6
#define PIN_SERVO   9
#define PIN_DHT     8
#define PIN_BUZZER  4

// ================= SENSOR CONFIG =================
#define DHTTYPE DHT11
#define MAX_DIST_CM 300
#define ALERT_DIST_CM 20

// ================= OBJECTS =================
Servo radarServo;
DHT dht(PIN_DHT, DHTTYPE);

// ================= FUNCTIONS =================

// HC-SR04 distance measurement
int measureDistance() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  long duration = pulseIn(PIN_ECHO, HIGH, 30000);

  if (duration == 0) return MAX_DIST_CM;

  int cm = duration * 0.034 / 2;
  return constrain(cm, 1, MAX_DIST_CM);
}

// Servo sweep + ultrasonic test
void testRadar() {
  Serial.println("\n--- RADAR SCAN START ---");

  for (int angle = 0; angle <= 180; angle += 10) {
    radarServo.write(angle);
    delay(200); // servo settle time

    int dist = measureDistance();

    Serial.print("Angle: ");
    Serial.print(angle);
    Serial.print("°  Distance: ");
    Serial.print(dist);
    Serial.println(" cm");

    if (dist < ALERT_DIST_CM) {
      tone(PIN_BUZZER, 1000, 100);
    }
  }

  Serial.println("--- RADAR SCAN END ---\n");
}

// DHT test
void testDHT() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  Serial.println("\n--- DHT11 ---");

  if (isnan(t) || isnan(h)) {
    Serial.println("Failed to read DHT11!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println(" %");
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  radarServo.attach(PIN_SERVO);
  radarServo.write(90);

  dht.begin();

  Serial.println("SYSTEM READY (UNO SENSOR TEST)");
}

// ================= LOOP =================
void loop() {
  testDHT();
  testRadar();

  delay(3000); // pause between full cycles
}