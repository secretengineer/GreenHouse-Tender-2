# ESP32 Firmware

Firmware for the ESP32 to monitor sensors and control relays in the greenhouse.

## Setup
1. **Install Arduino IDE:**
   - Add ESP32 support: `https://dl.espressif.com/dl/package_esp32_index.json`.
2. **Libraries:**
   - `PubSubClient` (MQTT)
   - `DHT sensor library by Adafruit`
   - `OneWire`
   - `DallasTemperature`
3. **Wiring:**
   - DHT22: GPIO 15
   - Soil Moisture: GPIO 34
   - pH Sensor: GPIO 35
   - DS18B20: GPIO 4
   - Relays: GPIO 13 (Fan), 12 (Vent), 14 (Heater)
4. **Config:**
   - Edit `src/config.h` with your WiFi and HiveMQ credentials.
5. **Upload:**
   - Open `src/greenhouse.ino` in Arduino IDE, select ESP32 Dev Module, and upload.

## Notes
- Calibrate soil moisture and pH readings with known values.
- Ensure relays are active-low (HIGH = off).