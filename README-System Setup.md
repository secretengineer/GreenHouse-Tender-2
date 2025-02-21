### Finalized Setup
- **Hardware:** Raspberry Pi 4 (or 3), DHT22 (temp/humidity), capacitive soil moisture sensor, pH sensor module, DS18B20 (thermostat temp), relays (for fan/vent/heat), Pi Camera.
- **Software:** Python on Pi for sensor reading/control, MQTT for communication, Flutter app for mobile interface, RTSP for video streaming.
- **Architecture:** Local setup with real-time updates via MQTT, no cloud needed unless you later decide to add it.

---

### 1. Hardware Wiring
Connect everything to the Raspberry Pi:

- **DHT22 (Ambient Temp/Humidity):**
  - VCC → 3.3V (Pin 1)
  - GND → Ground (Pin 6)
  - Data → GPIO 4 (Pin 7)
  - Add a 10kΩ pull-up resistor between VCC and Data.

- **Capacitive Soil Moisture Sensor:**
  - VCC → 3.3V (Pin 1)
  - GND → Ground (Pin 9)
  - Analog Out → ADS1115 (I2C ADC) → Pi’s I2C pins (SDA: Pin 3, SCL: Pin 5).

- **pH Sensor (e.g., SEN0161):**
  - VCC → 5V (Pin 2)
  - GND → Ground (Pin 14)
  - Analog Out → ADS1115 (second channel).
  - Calibrate with pH 4.0/7.0 solutions later.

- **DS18B20 (Thermostat Temp):**
  - VCC → 3.3V (Pin 1)
  - GND → Ground (Pin 20)
  - Data → GPIO 17 (Pin 11)
  - 4.7kΩ pull-up resistor between VCC and Data.

- **Relays (Fan, Vent, Heater):**
  - VCC → 5V (Pin 4)
  - GND → Ground (Pin 25)
  - Inputs: GPIO 27 (Fan, Pin 13), GPIO 22 (Vent, Pin 15), GPIO 23 (Heater, Pin 16).

- **Pi Camera:**
  - Connect to the CSI camera port on the Pi.

Power everything via the Pi’s GPIO or a separate 5V rail if current draw is high (e.g., relays).

---

### 2. Raspberry Pi Software
#### Setup
- Install Raspberry Pi OS, enable I2C and 1-Wire in `raspi-config`.
- Install dependencies:
  ```bash
  sudo pip3 install paho-mqtt adafruit-circuitpython-dht smbus2 RPi.GPIO
  sudo apt-get install libatlas-base-dev  # For DHT22
  ```

#### Python Code (`greenhouse.py`)
This script reads sensors, controls relays, and publishes data via MQTT.

```python
import time
import board
import adafruit_dht
import RPi.GPIO as GPIO
from smbus2 import SMBus
import paho.mqtt.client as mqtt
import w1thermsensor  # For DS18B20

# Pin setup
GPIO.setmode(GPIO.BCM)
RELAY_PINS = {"fan": 27, "vent": 22, "heater": 23}
for pin in RELAY_PINS.values():
    GPIO.setup(pin, GPIO.OUT, initial=GPIO.HIGH)  # Relays off (active LOW)

# Sensor setup
dht_sensor = adafruit_dht.DHT22(board.D4)
soil_adc = SMBus(1)  # I2C for ADS1115
ph_channel, soil_channel = 0, 1  # ADS1115 channels
temp_sensor = w1thermsensor.W1ThermSensor()

# MQTT setup
client = mqtt.Client("greenhouse_pi")
client.connect("localhost", 1883)  # Assumes local Mosquitto broker
client.loop_start()

# Control callbacks
def on_message(client, userdata, msg):
    topic, payload = msg.topic, msg.payload.decode()
    pin = RELAY_PINS.get(topic.split("/")[-1])
    if pin:
        GPIO.output(pin, GPIO.LOW if payload == "ON" else GPIO.HIGH)

client.on_message = on_message
client.subscribe("greenhouse/control/#")

# Main loop
while True:
    try:
        # Ambient
        temp = dht_sensor.temperature
        humidity = dht_sensor.humidity
        client.publish("greenhouse/ambient/temp", temp)
        client.publish("greenhouse/ambient/humidity", humidity)

        # Soil
        soil_raw = soil_adc.read_word_data(0x48, soil_channel)  # ADS1115 address 0x48
        ph_raw = soil_adc.read_word_data(0x48, ph_channel)
        soil_moisture = (soil_raw / 65535) * 100  # Normalize 0-100%
        ph = -0.0178 * ph_raw + 14.2  # Rough calibration, adjust after testing
        client.publish("greenhouse/soil/moisture", soil_moisture)
        client.publish("greenhouse/soil/ph", ph)

        # Thermostat temp
        thermo_temp = temp_sensor.get_temperature()
        client.publish("greenhouse/thermo/temp", thermo_temp)

        # Relay status
        for device, pin in RELAY_PINS.items():
            state = "ON" if GPIO.input(pin) == GPIO.LOW else "OFF"
            client.publish(f"greenhouse/status/{device}", state)

        time.sleep(5)  # Update every 5s
    except Exception as e:
        print(f"Error: {e}")
        time.sleep(1)

GPIO.cleanup()
client.loop_stop()
client.disconnect()
```

#### Video Streaming
Install GStreamer:
```bash
sudo apt-get install gstreamer1.0-tools gstreamer1.0-plugins-good
```
Start RTSP stream:
```bash
raspivid -o - -t 0 -w 640 -h 480 -fps 25 | gst-launch-1.0 -v fdsrc ! h264parse ! rtph264pay config-interval=1 pt=96 ! udpsink host=127.0.0.1 port=5000
```
Run an RTSP server (e.g., `python -m SimpleHTTPServer` with an RTSP wrapper) or use a dedicated tool like `v4l2loopback`.

---

### 3. Mobile App (Flutter)
#### Setup
Install Flutter, then create a new project:
```bash
flutter create greenhouse_app
cd greenhouse_app
flutter pub add mqtt_client flutter_vlc_player charts_flutter
```

#### Main Code (`lib/main.dart`)
```dart
import 'package:flutter/material.dart';
import 'package:mqtt_client/mqtt_client.dart';
import 'package:flutter_vlc_player/flutter_vlc_player.dart';
import 'package:charts_flutter/flutter.dart' as charts;

void main() => runApp(GreenhouseApp());

class GreenhouseApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(home: Dashboard());
  }
}

class Dashboard extends StatefulWidget {
  @override
  _DashboardState createState() => _DashboardState();
}

class _DashboardState extends State<Dashboard> {
  final client = MqttClient('localhost', 'flutter_client');
  Map<String, String> data = {};
  late VlcPlayerController _videoController;

  @override
  void initState() {
    super.initState();
    _videoController = VlcPlayerController.network(
      'rtsp://localhost:8554/stream',  // Adjust URL
      autoPlay: true,
    );
    connectMqtt();
  }

  void connectMqtt() async {
    client.connect();
    client.subscribe("greenhouse/#", MqttQos.atLeastOnce);
    client.updates.listen((msg) {
      final topic = msg[0].topic;
      final payload = msg[0].payload as MqttPublishMessage;
      setState(() {
        data[topic] = MqttPublishPayload.bytesToStringAsString(payload.payload.message);
      });
    });
  }

  void sendControl(String device, String state) {
    client.publishMessage("greenhouse/control/$device", MqttQos.atLeastOnce, state.codeUnits);
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text("Greenhouse Monitor")),
      body: Column(
        children: [
          VlcPlayer(controller: _videoController, aspectRatio: 4 / 3, placeholder: Text("Loading...")),
          Text("Temp: ${data['greenhouse/ambient/temp'] ?? '--'}°C"),
          Text("Humidity: ${data['greenhouse/ambient/humidity'] ?? '--'}%"),
          Text("Soil Moisture: ${data['greenhouse/soil/moisture'] ?? '--'}%"),
          Text("Soil pH: ${data['greenhouse/soil/ph'] ?? '--'}"),
          Text("Thermo Temp: ${data['greenhouse/thermo/temp'] ?? '--'}°C"),
          Row(
            children: [
              ElevatedButton(onPressed: () => sendControl("fan", "ON"), child: Text("Fan ON")),
              ElevatedButton(onPressed: () => sendControl("fan", "OFF"), child: Text("Fan OFF")),
            ],
          ),
          // Add similar buttons for vent/heater
        ],
      ),
    );
  }
}
```

---

### 4. Deployment
1. **Pi:** Run `sudo apt-get install mosquitto` for the MQTT broker. Start `greenhouse.py` and the video stream.
2. **App:** Test on an emulator first, then deploy to your phone (same Wi-Fi network as Pi).
3. **Calibration:** Adjust pH and soil moisture readings with known values.

