**ESP32 Radar Scanner**

- **Summary:** ESP32-based 180° radar scanner using an HC-SR04 ultrasonic sensor, a sweep servo, a DHT11 environmental sensor and an ILI9341 TFT for local display. The ESP32 also serves a small JSON API consumed by a browser dashboard (`radar_dashboard.html`).

**Prerequisites**
- **Arduino IDE:** Version 2.x (recommended).
- **Board package:** Espressif ESP32 board package (v2.x or newer) installed in Arduino IDE Boards Manager.
- **Libraries:** install the following via Library Manager or GitHub:
	- **ESP32Servo**: servo support for ESP32
	- **DHT sensor library** (Adafruit)
	- **Adafruit GFX**
	- **Adafruit ILI9341**
	- **ESPAsyncWebServer**
	- **AsyncTCP**
	- **ArduinoJson** (v6)

**Files**
- **Firmware:** [radar_esp32/radar_esp32.ino](radar_esp32/radar_esp32.ino)
- **Browser dashboard:** [radar_dashboard.html](radar_dashboard.html)
- **Other sketches & tests:** see the `Tests/` folder for example sketches.

**Hardware / Wiring (as used in firmware)**
- **HC-SR04:** TRIG = 13, ECHO = 12
- **Servo:** SERVO = 14 (SM-S4303R — firmware sets pulse range 500–2400 µs)
- **DHT11:** PIN_DHT = 27
- **Buzzer:** PIN_BUZZER = 26
- **ILI9341 TFT:** TFT_CS = 15, TFT_RST = 4, TFT_DC = 2

Refer to [radar_esp32/radar_esp32.ino](radar_esp32/radar_esp32.ino) for exact pin constants and additional comments.

**Configuration**
- Open the firmware file [radar_esp32/radar_esp32.ino](radar_esp32/radar_esp32.ino).
- Set your WiFi credentials by editing the `SSID` and `PASSWORD` constants near the top of the sketch:

	- `const char* SSID = "your-ssid";`
	- `const char* PASSWORD = "your-password";`

**Upload**
- Select **ESP32 Dev Module** (or appropriate ESP32 board) in Arduino IDE.
- Set the upload baud to **921600** (recommended for faster uploads) and choose the correct COM port.
- Click Upload in Arduino IDE.

**Running & Accessing the Dashboard**
- Open the Serial Monitor at `115200` baud after the ESP32 boots. The sketch prints the assigned IP address on successful WiFi connection.
- Open `radar_dashboard.html` in any browser on the same network.
	- Enter the ESP32 IP (including `http://`) into the **HOST** field and click **Refresh**.
	- The dashboard polls two JSON endpoints on the ESP32:
		- `/scan` — returns an array of `{angle, dist}` objects (degrees, cm)
		- `/env`  — returns `{temp, hum}`

- Alternatively, opening `radar_dashboard.html` locally via `file://` is supported; on first use it will prompt you for the ESP32 IP.

**Notes & Behavior**
- The firmware performs a full 180° sweep and serves the latest scan as JSON on `/scan` (port 80).
- The TFT shows a local polar radar and basic environment readout (`temperature`, `humidity`).
- If WiFi fails, the firmware will continue scanning and display locally; the web API will be unavailable until a connection is established.

**Troubleshooting**
- If the browser cannot fetch data due to CORS or mixed-content: ensure the `HOST` uses `http://` and the browser and ESP32 are on the same LAN.
- If the servo angles look reversed, swap servo wiring or invert the mapping in the sketch.
- Increase `Serial` baud or add debug `Serial.println()` calls in the sketch if the device won't connect.

**Development & Tests**
- Example/test sketches live in the `Tests/` folder — useful for verifying individual components (buzzer, DHT11, servo, display, WiFi).

**Acknowledgements**
- Uses Adafruit display libraries, `ESPAsyncWebServer` and `ArduinoJson` for networking and payload serialization.
