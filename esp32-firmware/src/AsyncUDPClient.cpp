#include <WiFi.h>
#include "ESPAsyncUDP.h"

const char * ssid = "INDUST2.4";
const char * password = "";

AsyncUDP udp;

void setup()
{
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("WiFi Failed");
        while(1) {
            delay(1000);
        }
    }
    if(udp.connect(IPAddress(192,168,1,100), 1234)) {
        Serial.println("UDP connected");
        udp.onPacket([](AsyncUDPPacket packet) {
            Serial.print("UDP Packet Type: ");
            Serial.print(packet.isBroadcast()?"Broadcast":packet.isMulticast()?"Multicast":"Unicast");
            Serial.print(", From: ");
            Serial.print(packet.remoteIP());
            Serial.print(":");
            Serial.print(packet.remotePort());
            Serial.print(", To: ");
            Serial.print(packet.localIP());
            Serial.print(":");
            Serial.print(packet.localPort());
            Serial.print(", Length: ");
            Serial.print(packet.length());
            Serial.print(", Data: ");
            Serial.write(packet.data(), packet.length());
            Serial.println();
            //reply to the client
            packet.printf("Got %u bytes of data", packet.length());
        });
        //Send unicast
        udp.print("Hello Server!");
    }
}

void loop()
{
    delay(1000);
    //Send broadcast on port 1234
    udp.broadcastTo("Anyone here?", 1234);
}
```
; filepath: /c:/Users/ptrck/Documents/GreenHouse-Tender-2/esp32-firmware/platformio.ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_port = COM3  ; Adjust for your system (COM port on Windows)
lib_deps =
  knolleary/PubSubClient@^2.8
  adafruit/DHT sensor library@^1.4.4
  paulstoffregen/OneWire@^2.3.7
  milesburton/DallasTemperature@^3.11.0
  me-no-dev/ESPAsyncUDP@^1.1.0