/**
 * ESP32 Greenhouse Controller
 * 
 * This firmware manages a greenhouse environment using various sensors and actuators:
 * - DHT22 for ambient temperature and humidity
 * - Soil moisture sensor
 * - pH sensor
 * - Dallas DS18B20 temperature sensor
 * - Relay-controlled fan, vent, and heater
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "../include/config.h"

// Pin Definitions
#define DHTPIN 15          // DHT22 data pin
#define DHTTYPE DHT22      // DHT sensor type
#define SOIL_PIN 34        // Soil moisture sensor analog pin
#define PH_PIN 35         // pH sensor analog pin
#define TEMP_PIN 4         // DS18B20 temperature sensor data pin
#define FAN_PIN 13        // Relay pin for fan control
#define VENT_PIN 12       // Relay pin for vent control
#define HEATER_PIN 14     // Relay pin for heater control

// Initialize network and sensor objects
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(TEMP_PIN);
DallasTemperature tempSensor(&oneWire);

/**
 * Initial setup function
 * Configures pins, initializes sensors, and establishes network connections
 */
void setup() {
    Serial.begin(115200);
    
    // Configure relay pins (relays are active LOW)
    pinMode(FAN_PIN, OUTPUT); digitalWrite(FAN_PIN, HIGH);
    pinMode(VENT_PIN, OUTPUT); digitalWrite(VENT_PIN, HIGH);
    pinMode(HEATER_PIN, OUTPUT); digitalWrite(HEATER_PIN, HIGH);
    
    // Initialize sensors
    dht.begin();
    tempSensor.begin();
    
    // Connect to WiFi
    Serial.print("Connecting to WiFi");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");

    // Configure MQTT
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    reconnect();
}

/**
 * MQTT reconnection function
 * Attempts to reconnect to MQTT broker if connection is lost
 */
void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("ESP32Greenhouse", mqtt_user, mqtt_pass)) {
            Serial.println("connected");
            client.subscribe("greenhouse/control/#");
        } else {
            Serial.print("failed, rc=");
            Serial.println(client.state());
            delay(5000);
        }
    }
}

/**
 * MQTT message callback
 * Handles incoming control messages for devices
 * 
 * @param topic The MQTT topic of the incoming message
 * @param payload The message content
 * @param length Length of the message
 */
void callback(char* topic, byte* payload, unsigned int length) {
    String message = String((char*)payload).substring(0, length);
    String device = String(topic).substring(17); // Extract device name after "greenhouse/control/"
    
    // Determine which device to control
    int pin = -1;
    if (device == "fan") pin = FAN_PIN;
    else if (device == "vent") pin = VENT_PIN;
    else if (device == "heater") pin = HEATER_PIN;
    
    // Update device state and publish status
    if (pin != -1) {
        digitalWrite(pin, message == "ON" ? LOW : HIGH);
        client.publish(("greenhouse/status/" + device).c_str(), message.c_str());
    }
}

/**
 * Main program loop
 * Reads sensors and publishes data every 5 seconds
 */
void loop() {
    // Ensure MQTT connection
    if (!client.connected()) reconnect();
    client.loop();

    // Read sensor values
    float temp = dht.readTemperature();
    float humidity = dht.readHumidity();
    int soilRaw = analogRead(SOIL_PIN);
    int phRaw = analogRead(PH_PIN);
    tempSensor.requestTemperatures();
    float thermoTemp = tempSensor.getTempCByIndex(0);

    // Publish sensor readings to MQTT
    if (!isnan(temp)) client.publish("greenhouse/ambient/temp", String(temp).c_str());
    if (!isnan(humidity)) client.publish("greenhouse/ambient/humidity", String(humidity).c_str());
    
    // Convert and publish soil moisture (mapped from 4095-0 to 0-100%)
    float soilMoisture = map(soilRaw, 4095, 0, 0, 100);
    client.publish("greenhouse/soil/moisture", String(soilMoisture).c_str());
    
    // Convert and publish pH (mapped from 0-4095 to 0-14 pH)
    float ph = map(phRaw, 0, 4095, 0, 14);
    client.publish("greenhouse/soil/ph", String(ph).c_str());
    
    // Publish thermometer temperature if valid
    if (thermoTemp != -127) client.publish("greenhouse/thermo/temp", String(thermoTemp).c_str());

    // Publish current device states
    client.publish("greenhouse/status/fan", digitalRead(FAN_PIN) == LOW ? "ON" : "OFF");
    client.publish("greenhouse/status/vent", digitalRead(VENT_PIN) == LOW ? "ON" : "OFF");
    client.publish("greenhouse/status/heater", digitalRead(HEATER_PIN) == LOW ? "ON" : "OFF");

    // Wait before next update
    delay(5000);
}