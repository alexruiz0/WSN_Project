// ============================================================
//  ESP32 Radar Scanner — Main Firmware
//  Hardware: HC-SR04 + SM-S4303R + DHT11 + ILI9341
// ============================================================

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP32Servo.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <ArduinoJson.h>

// ---------- PIN CONFIG ----------
#define PIN_TRIG      13
#define PIN_ECHO      12
#define PIN_SERVO     14
#define PIN_DHT       27
#define PIN_BUZZER    26

#define TFT_CS        15
#define TFT_RST        4
#define TFT_DC         2

// ---------- CONSTANTS ----------
#define SCAN_STEP       2          // degrees per step
#define SCAN_POINTS    (180/SCAN_STEP + 1)  // 91 points
#define MAX_DIST_CM   300
#define DHT_TYPE      DHT11
#define ALERT_DIST_CM  20          // buzzer threshold (cm)

// ---------- WIFI CREDENTIALS ----------
const char* SSID     = "hololelo";
const char* PASSWORD = "wsns1234";

// ---------- GLOBALS ----------
int   distances[SCAN_POINTS];
float temperature = 0.0;
float humidity    = 0.0;
bool  scanReady   = false;

Servo          radarServo;
DHT            dht(PIN_DHT, DHT_TYPE);
Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);
AsyncWebServer server(80);

// ============================================================
//  ULTRASONIC — returns distance in cm (0 = timeout/error)
// ============================================================
int measureDistance() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  long duration = pulseIn(PIN_ECHO, HIGH, 30000); // 30ms timeout ~5m
  if (duration == 0) return MAX_DIST_CM;
  int cm = (int)(duration * 0.034 / 2);
  return constrain(cm, 1, MAX_DIST_CM);
}

// ============================================================
//  SCAN — full 180° sweep, populates distances[]
// ============================================================
void performScan() {
  for (int i = 0; i < SCAN_POINTS; i++) {
    int angle = i * SCAN_STEP;
    radarServo.write(angle);
    delay(50); // settle time

    distances[i] = measureDistance();

    // Buzzer alert for close objects
    if (distances[i] < ALERT_DIST_CM) {
      tone(PIN_BUZZER, 1000, 100);
    }
  }
  scanReady = true;
}

// ============================================================
//  DHT11 READ
// ============================================================
void readEnvSensor() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t)) temperature = t;
  if (!isnan(h)) humidity    = h;
}

// ============================================================
//  ILI9341 DISPLAY — polar radar mini-map
// ============================================================
#define TFT_BG    ILI9341_BLACK
#define TFT_GRID  0x1082  // dark grey
#define TFT_DOT   ILI9341_GREEN
#define TFT_TEXT  ILI9341_WHITE

// Display centre (bottom-centre of screen for 180° arc)
#define CX  120
#define CY  220
#define MAX_R 140  // max radius pixels (maps MAX_DIST_CM)

void drawRadarFrame() {
  tft.fillScreen(TFT_BG);
  // Draw range rings at 25%, 50%, 75%, 100%
  for (int r = 1; r <= 4; r++) {
    int pr = (MAX_R * r) / 4;
    // Draw arc (half circle) using points
    for (int a = 0; a <= 180; a += 2) {
      float rad = a * PI / 180.0;
      int x = CX - (int)(pr * cos(rad));
      int y = CY - (int)(pr * sin(rad));
      tft.drawPixel(x, y, TFT_GRID);
    }
  }
  // Spoke lines at 30° intervals
  for (int a = 0; a <= 180; a += 30) {
    float rad = a * PI / 180.0;
    int x2 = CX - (int)(MAX_R * cos(rad));
    int y2 = CY - (int)(MAX_R * sin(rad));
    tft.drawLine(CX, CY, x2, y2, TFT_GRID);
  }
  // Labels
  tft.setTextColor(TFT_TEXT);
  tft.setTextSize(1);
  tft.setCursor(2, 215);   tft.print("0");
  tft.setCursor(113, 70);  tft.print("90");
  tft.setCursor(222, 215); tft.print("180");
}

void updateDisplay() {
  drawRadarFrame();

  // Plot scan points
  for (int i = 0; i < SCAN_POINTS; i++) {
    int angle = i * SCAN_STEP;
    float rad  = angle * PI / 180.0;
    int   r    = map(distances[i], 0, MAX_DIST_CM, 0, MAX_R);
    r = constrain(r, 1, MAX_R);

    // ESP32 servo: 0° = right, 90° = up, 180° = left
    int x = CX - (int)(r * cos(rad));
    int y = CY - (int)(r * sin(rad));

    // Color by distance: green=far, yellow=mid, red=close
    uint16_t color;
    if (distances[i] < 50)       color = ILI9341_RED;
    else if (distances[i] < 150) color = ILI9341_YELLOW;
    else                          color = ILI9341_GREEN;

    tft.fillCircle(x, y, 2, color);
  }

  // Env data overlay
  tft.setTextColor(TFT_TEXT);
  tft.setTextSize(1);
  tft.setCursor(2, 2);
  tft.printf("%.1fC  %.0f%%", temperature, humidity);
}

// ============================================================
//  HTTP SERVER — JSON endpoints
// ============================================================
void setupServer() {
  // GET /scan → JSON array [{angle, dist}, ...]
  server.on("/scan", HTTP_GET, [](AsyncWebServerRequest* req) {
    StaticJsonDocument<2048> doc;
    JsonArray arr = doc.to<JsonArray>();
    for (int i = 0; i < SCAN_POINTS; i++) {
      JsonObject obj = arr.createNestedObject();
      obj["angle"] = i * SCAN_STEP;
      obj["dist"]  = distances[i];
    }
    String out;
    serializeJson(doc, out);
    req->send(200, "application/json", out);
  });

  // GET /env → {temp, hum}
  server.on("/env", HTTP_GET, [](AsyncWebServerRequest* req) {
    StaticJsonDocument<128> doc;
    doc["temp"] = temperature;
    doc["hum"]  = humidity;
    String out;
    serializeJson(doc, out);
    req->send(200, "application/json", out);
  });

  // CORS header for local dev
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

  server.begin();
}

// ============================================================
//  SETUP
// ============================================================
void setup() {
  Serial.begin(115200);

  // Pins
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  // Servo
  radarServo.setPeriodHertz(50);
  radarServo.attach(PIN_SERVO, 500, 2400); // SM-S4303R pulse range
  radarServo.write(0);

  // DHT
  dht.begin();

  // TFT
  tft.begin();
  tft.setRotation(0); // portrait
  tft.fillScreen(TFT_BG);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 10);
  tft.print("Connecting WiFi...");

  // WiFi
  WiFi.begin(SSID, PASSWORD);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 30) {
    delay(500);
    Serial.print(".");
    tries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected: " + WiFi.localIP().toString());
    tft.fillScreen(TFT_BG);
    tft.setCursor(10, 10);
    tft.print("IP: " + WiFi.localIP().toString());
    delay(2000);
  } else {
    Serial.println("\nWiFi failed — running in offline mode");
  }

  setupServer();

  // Initial scan
  readEnvSensor();
  performScan();
  updateDisplay();
}

// ============================================================
//  LOOP
// ============================================================
unsigned long lastScanMs = 0;
unsigned long lastEnvMs  = 0;
const unsigned long SCAN_INTERVAL = 5000;  // ms between scans
const unsigned long ENV_INTERVAL  = 10000; // ms between DHT reads

void loop() {
  unsigned long now = millis();

  if (now - lastEnvMs > ENV_INTERVAL) {
    readEnvSensor();
    lastEnvMs = now;
  }

  if (now - lastScanMs > SCAN_INTERVAL) {
    performScan();
    updateDisplay();
    lastScanMs = now;
  }
}
