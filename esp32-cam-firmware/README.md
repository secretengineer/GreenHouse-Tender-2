# ESP32-CAM Firmware

Firmware for the ESP32-CAM to stream video from the greenhouse.

## Setup
1. **Install Arduino IDE:**
   - Add ESP32 support: `https://dl.espressif.com/dl/package_esp32_index.json`.
2. **Libraries:**
   - None beyond ESP32 core (camera library is built-in).
3. **Wiring:**
   - Use an ESP32-CAM module (e.g., AI-Thinker).
   - Power via 5V or 3.3V (check jumper).
4. **Config:**
   - Edit `src/config.h` with your WiFi credentials.
5. **Upload:**
   - Open `src/camera.ino`, select ESP32 Wrover Module, connect FTDI for upload, and flash.
6. **Stream:**
   - Access `http://<ESP32-CAM-IP>/stream` in a browser to test.

## Notes
- Adjust `frame_size` or `jpeg_quality` for bandwidth/performance.