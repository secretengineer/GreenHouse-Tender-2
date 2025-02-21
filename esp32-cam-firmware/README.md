# ESP32-CAM Firmware

Firmware for the ESP32-CAM to stream video from the greenhouse, built with PlatformIO.

## Setup
1. **Install PlatformIO:**
   - Install VS Code and the PlatformIO extension.
2. **Open Project:**
   - Open this folder in VS Code.
3. **Wiring:**
   - Use an ESP32-CAM module (e.g., AI-Thinker).
   - Power via 5V or 3.3V (check jumper).
   - Connect FTDI for upload (GPIO 0 to GND during flashing).
4. **Config:**
   - Edit `include/config.h` with your WiFi credentials.
5. **Build & Upload:**
   - Use PlatformIO toolbar: Build (✓), Upload (→).
   - Adjust `upload_port` in `platformio.ini` if needed.
6. **Stream:**
   - Access `http://<ESP32-CAM-IP>/stream` in a browser.

## Notes
- Adjust `frame_size` or `jpeg_quality` for bandwidth/performance.