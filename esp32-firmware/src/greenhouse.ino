#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"  // Credentials

#define DHTPIN 15
#define DHTTYPE DHT22
#define SOIL_PIN 34
#define PH_PIN 35
#define TEMP_PIN 4
#define FAN_PIN 13
#define VENT_PIN 12
#define HEATER_PIN 14

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(TEMP_PIN);
DallasTemperature tempSensor(&oneWire);

void setup() {
  Serial.begin(115200);
  
  pinMode(FAN_PIN, OUTPUT); digitalWrite(FAN_PIN, HIGH);
  pinMode(VENT_PIN, OUTPUT); digitalWrite(VENT_PIN, HIGH);
  pinMode(HEATER_PIN, OUTPUT); digitalWrite(HEATER_PIN, HIGH);
  
  dht.begin();
  tempSensor.begin();
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Greenhouse", mqtt_user, mqtt_pass)) {
      client.subscribe("greenhouse/control/#");
    } else {
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message = String((char*)payload).substring(0, length);
  String device = String(topic).substring(17); // After "greenhouse/control/"
  
  int pin = -1;
  if (device == "fan") pin = FAN_PIN;
  else if (device == "vent") pin = VENT_PIN;
  else if (device == "heater") pin = HEATER_PIN;
  
  if (pin != -1) {
    digitalWrite(pin, message == "ON" ? LOW : HIGH);
    client.publish(("greenhouse/status/" + device).c_str(), message.c_str());
  }
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  int soilRaw = analogRead(SOIL_PIN);
  int phRaw = analogRead(PH_PIN);
  tempSensor.requestTemperatures();
  float thermoTemp = tempSensor.getTempCByIndex(0);

  if (!isnan(temp)) client.publish("greenhouse/ambient/temp", String(temp).c_str());
  if (!isnan(humidity)) client.publish("greenhouse/ambient/humidity", String(humidity).c_str());
  float soilMoisture = map(soilRaw, 4095, 0, 0, 100); // Adjust range after testing
  client.publish("greenhouse/soil/moisture", String(soilMoisture).c_str());
  float ph = map(phRaw, 0, 4095, 0, 14); // Rough, calibrate later
  client.publish("greenhouse/soil/ph", String(ph).c_str());
  if (thermoTemp != -127) client.publish("greenhouse/thermo/temp", String(thermoTemp).c_str());

  client.publish("greenhouse/status/fan", digitalRead(FAN_PIN) == LOW ? "ON" : "OFF");
  client.publish("greenhouse/status/vent", digitalRead(VENT_PIN) == LOW ? "ON" : "OFF");
  client.publish("greenhouse/status/heater", digitalRead(HEATER_PIN) == LOW ? "ON" : "OFF");

  delay(5000);
}