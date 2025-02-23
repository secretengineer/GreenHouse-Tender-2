# Greenhouse Monitoring

A cloud-based (or local, non-cloud, non-internet option for grid-down scenarios) system to monitor and control a greenhouse environment using an ESP32 microcontroller, ESP32-CAM for video, Firebase for data storage and notifications, and a Flutter mobile app for a modern user interface.

## Overview

This project creates a comprehensive greenhouse monitoring solution that tracks key environmental parameters and allows remote control via a mobile app. It features real-time sensor data (temperature, humidity, soil moisture, pH), relay-controlled devices (fan, vent, heater), and live video streaming, all presented in an aesthetic UI with graphs and customizable notifications.

### Features
- **Sensors:** Ambient temperature/humidity (DHT22), soil moisture (capacitive sensor), soil pH (SEN0161), thermostat temperature (DS18B20).
- **Controls:** Fan, vent, and heater via relays.
- **Video:** Live feed from ESP32-CAM.
- **Mobile App:** Flutter-based UI with real-time data, time-series graphs, modern icons/fonts (Poppins, FontAwesome), and user-defined high/low threshold notifications.
- **Cloud:** HiveMQ MQTT for data transmission, Firebase Firestore for storage, and Firebase Cloud Messaging for alerts.

### Architecture
- **ESP32:** Collects sensor data, controls relays, and publishes to MQTT.
- **ESP32-CAM:** Streams video over HTTP.
- **Cloud:** HiveMQ MQTT broker relays data; Firebase Cloud Functions sync MQTT to Firestore and trigger notifications.
- **Flutter App:** Pulls data from Firestore, displays graphs, and sends control commands.
- **Offline Mode:** User option to manage system locally, in the event of network failure scenario.

## Project Structure

```
greenhouse-monitoring/
├── esp32-firmware/         # ESP32 sensor and control firmware (C++, PlatformIO)
├── esp32-cam-firmware/     # ESP32-CAM video streaming firmware (C++, PlatformIO)
├── cloud-functions/        # Firebase Cloud Functions (Node.js)
├── flutter-app/            # Flutter mobile app (Dart)
├── docs/                   # Documentation (wiring, architecture)
├── .gitignore              # Git ignore rules
├── README.md               # This file
└── LICENSE                 # MIT License
```

## Prerequisites

- **Hardware:** ESP32 DevKit, ESP32-CAM (e.g., AI-Thinker), DHT22, capacitive soil moisture sensor, pH sensor (SEN0161), DS18B20, relays, Wi-Fi.
- **Software:** VS Code with PlatformIO extension (for ESP32 firmware), Node.js (for Firebase), Flutter SDK, Git.
- **Services:** HiveMQ Cloud account, Firebase project.

## Setup Instructions

1. **Clone the Repo:**
   ```bash
   git clone https://github.com/secretengineer/Greenhouse-Tender-2.git
   cd greenhouse-monitoring
   ```
2. **ESP32 Firmware:**
   - Open `esp32-firmware/` in VS Code with PlatformIO.
   - Configure `include/config.h` with WiFi and MQTT credentials.
   - See `esp32-firmware/README.md` for wiring and upload instructions.
3. **ESP32-CAM Firmware:**
   - Open `esp32-cam-firmware/` in VS Code with PlatformIO.
   - Configure `include/config.h` with WiFi credentials.
   - See `esp32-cam-firmware/README.md` for wiring and upload instructions.
4. **Cloud Functions:** See `cloud-functions/README.md` for Firebase deployment.
5. **Flutter App:** See `flutter-app/README.md` for app setup and run commands.
6. **Documentation:** Check `docs/` for wiring diagrams and architecture details.

## Status

- **Current Stage:** Planning and initial code snippets completed (as of February 20, 2025).
- **Next Steps:** Implement and test ESP32 firmware, deploy Firebase functions, build Flutter UI.

## Contributing

Feel free to fork, submit issues, or open pull requests. See individual component READMEs for specific contribution guidelines.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contact

Created by Pat Ryan - reach out via GitHub Issues for questions or feedback.
```


