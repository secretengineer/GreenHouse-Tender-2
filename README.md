# GreenHouse Tender 2
 A new revision of GreenHouse Tender 1

The objective of this project is to create a new version of GreenHouse Tender 1 with more detailed requirements, a mobile app component and a video streaming function to enhance the user experience.


### 1. Define Requirements
You’ve already outlined the core features:
- **Sensors:**
  - Ambient temperature and humidity
  - Soil humidity
  - Soil pH
  - Heat/thermostat settings
  - Fan and vent status (on/off or open/closed)
- **Video Feed:**
  - Real-time camera streaming
- **Mobile App:**
  - Display sensor data and video
  - Allow user interaction (e.g., adjust thermostat, toggle fan/vent)

Additional considerations:
- Do you want real-time updates or periodic polling?
- Should the app send notifications (e.g., if temperature exceeds a threshold)?
- Will this be local (Wi-Fi) or cloud-based?

### 2. Hardware Selection
Here’s a suggested hardware setup:
- **Microcontroller:** Raspberry Pi (good for video and processing) or Arduino (simpler for sensors).
- **Sensors:**
  - **Ambient Temp/Humidity:** DHT22 or BME280 (I2C-based, more accurate).
  - **Soil Humidity:** Capacitive soil moisture sensor (less corrosion than resistive ones).
  - **Soil pH:** Analog pH sensor with a module (e.g., SEN0161).
  - **Thermostat Control:** Relay module to control heaters + DS18B20 for temp feedback.
  - **Fan/Vent Status:** Relays or limit switches to detect/control state.
- **Camera:** USB webcam or Raspberry Pi Camera Module.
- **Power Supply:** Ensure stable power for all components (e.g., 5V/12V depending on setup).

### 3. System Architecture
- **Local Setup:**
  - Microcontroller collects sensor data and controls devices.
  - Camera streams video.
  - Data is sent to a local server (e.g., on Raspberry Pi) or directly to the app via Wi-Fi.
- **Cloud Setup (Optional):**
  - Use a service like AWS IoT, Google Cloud IoT, or Blynk to store data and relay it to the app.
  - Video streaming via RTSP or a service like AWS Kinesis Video Streams.

### 4. Software Development
#### Backend (Microcontroller)
- **Language:** Python (Raspberry Pi) or C++ (Arduino).
- **Libraries:**
  - For DHT22/BME280: Adafruit libraries.
  - For soil sensors: Analog-to-digital conversion (ADC) if needed.
  - For relays: GPIO control.
- **Video Streaming:** Use GStreamer or OpenCV on Raspberry Pi for RTSP streaming.
- **Data Protocol:** MQTT or HTTP to send sensor data to the app/server.

#### Mobile App
- **Framework:** Flutter (cross-platform) or React Native; native options like Swift (iOS) or Kotlin (Android) work too.
- **Features:**
  - **UI:** Dashboard with gauges/charts for sensor data (e.g., using Syncfusion or Fl_chart in Flutter).
  - **Video:** Embed RTSP stream using a library like VLC or a WebView.
  - **Controls:** Buttons/sliders for thermostat, fan, and vent.
- **Connectivity:** WebSocket or MQTT for real-time updates; REST API if polling.

### 5. Step-by-Step Plan
1. **Prototype Hardware:**
   - Connect sensors to the microcontroller and test readings.
   - Wire relays for fan/vent/thermostat control.
   - Set up the camera and test streaming.
2. **Backend Code:**
   - Write a script to read sensors and publish data (e.g., via MQTT).
   - Add control logic for relays based on app commands.
3. **Mobile App:**
   - Design a simple UI with placeholders for data and video.
   - Connect to the backend (MQTT/HTTP) and display sensor values.
   - Integrate video feed and test controls.
4. **Testing:**
   - Simulate greenhouse conditions (e.g., heat lamp, water soil) and verify data accuracy.
   - Ensure app updates in real-time and controls work.
5. **Deployment:**
   - Secure the system (e.g., password-protect streams, encrypt data if cloud-based).
   - Mount hardware in the greenhouse and finalize the app.

### 6. Example Tools & Costs
- **Hardware:** ~$50–$100 (Raspberry Pi ~$35, sensors ~$5–$15 each, camera ~$20).
- **Software:** Free/open-source (Python, Flutter, MQTT brokers like Mosquitto).
- **Cloud (Optional):** ~$5–$20/month depending on usage.



