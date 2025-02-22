# ESP32 Firmware

Firmware for the Heltec WiFi Kit 32 to monitor sensors and control relays in the Greenhouse-Tender2 project, built with PlatformIO.

## Setup
1. **Install PlatformIO:**
   - Install VS Code and the PlatformIO extension.
2. **Open Project:**
   - Open this folder (`Greenhouse-Tender2/esp32-firmware/`) in VS Code.
3. **Dependencies:**
   - Defined in `platformio.ini`; PlatformIO will install them automatically.
4. **Wiring:**
   - DHT22: GPIO 25
   - Soil Moisture: GPIO 34
   - pH Sensor: GPIO 35
   - DS18B20: GPIO 26
   - Relays: GPIO 13 (Fan), 12 (Vent), 27 (Heater)
5. **Config:**
   - Edit `include/config.h` with your WiFi and HiveMQ credentials.
6. **Build & Upload:**
   - Use PlatformIO toolbar: Build (✓), Upload (→).
   - Adjust `upload_port` in `platformio.ini` if needed (e.g., `/dev/ttyUSB0` or `COM3`).

## Notes
- GPIO 4 and 15 are reserved for the onboard OLED (I2C).
- Calibrate soil moisture and pH readings with known values.
- Relays are active-low (HIGH = off).