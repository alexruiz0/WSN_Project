#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define TFT_CS 15
#define TFT_RST 4
#define TFT_DC 2

Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);

void setup() {

  Serial.begin(115200);

  tft.begin();

  tft.fillScreen(ILI9341_BLACK);

  tft.setCursor(20,20);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);

  tft.println("Hello world");
}

void loop() {
}