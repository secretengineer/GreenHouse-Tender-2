/**
 * Greenhouse Environment Control System
 * 
 * This firmware manages a greenhouse environment using:
 * - DHT22 for ambient temperature and humidity
 * - Soil moisture sensor (analog)
 * - pH sensor (analog)
 * - DS18B20 temperature probe
 * - OLED display for real-time readings
 * - Relay-controlled fan, vent, and heater
 */

// Required libraries
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>    // MQTT client
#include <Adafruit_Sensor.h> // Required by DHT library
#include <DHT.h>            // DHT sensor
#include <OneWire.h>        // Required for DS18B20
#include <DallasTemperature.h>  // DS18B20 sensor
#include <Wire.h>           // I2C communication
#include <Adafruit_GFX.h>   // Graphics library
#include <Adafruit_SSD1306.h> // OLED display
#include "../include/config.h"

// Add after includes
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();

// Pin Definitions
#define DHTPIN 25           // DHT22 data pin
#define DHTTYPE DHT22       // DHT22 sensor type
#define SOIL_PIN 34         // Soil moisture sensor (ADC1_CH6)
#define PH_PIN 35          // pH sensor (ADC1_CH7)
#define TEMP_PIN 26        // DS18B20 data pin
#define FAN_PIN 13         // Relay control pins (active LOW)
#define VENT_PIN 12
#define HEATER_PIN 27

// OLED Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C   // Default I2C address for SSD1306
#define SDA_OLED 4         // Heltec OLED SDA pin
#define SCL_OLED 15        // Heltec OLED SCL pin

// Initialize communication clients and sensors
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(TEMP_PIN);
DallasTemperature tempSensor(&oneWire);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, 16);  // Use GPIO 16 as reset pin

/**
 * Initial setup function
 * Configures pins, initializes sensors, and establishes network connections
 */
void setup() {
    Serial.begin(115200);
    
    // Heltec WiFi Kit 32 I2C configuration
    Wire.begin(SDA_OLED, SCL_OLED);  // SDA = GPIO 4, SCL = GPIO 15 for Heltec board
    
    // Initialize relay control pins (HIGH = OFF, LOW = ON)
    pinMode(FAN_PIN, OUTPUT); digitalWrite(FAN_PIN, HIGH);
    pinMode(VENT_PIN, OUTPUT); digitalWrite(VENT_PIN, HIGH);
    pinMode(HEATER_PIN, OUTPUT); digitalWrite(HEATER_PIN, HIGH);
    
    // Initialize sensors
    dht.begin();
    tempSensor.begin();
    
    // Initialize OLED display
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        Serial.println(F("OLED initialization failed"));
        for(;;);  // Halt if OLED fails
    }
    
    // Setup initial display
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("   GREENHOUSE TENDER");
    display.println("");
    display.println("   Version 1.0.0");
    display.println("");
    display.println("   (C) 2025");
    display.println("");
    display.println("   Pat Ryan Design");
   
    
    display.display();
    
    // Connect to WiFi network
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi connected");

    // Configure MQTT client
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    reconnect();
}

/**
 * MQTT Reconnection Handler
 * Attempts to reconnect to MQTT broker if connection is lost
 */
void reconnect() {
    while (!client.connected()) {
        if (client.connect("HeltecGreenhouse", mqtt_user, mqtt_pass)) {
            client.subscribe("greenhouse/control/#");
        } else {
            Serial.printf("MQTT failed, rc=%d\n", client.state());
            delay(5000);
        }
    }
}

/**
 * MQTT Message Handler
 * Processes incoming control messages for devices
 * 
 * @param topic - MQTT topic (greenhouse/control/{device})
 * @param payload - Message content (ON/OFF)
 * @param length - Message length
 */
void callback(char* topic, byte* payload, unsigned int length) {
    // Create a null-terminated string from the payload
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';

    // Handle incoming messages
    if (strcmp(topic, "greenhouse/control/fan") == 0) {
        digitalWrite(FAN_PIN, message[0] == '1' ? LOW : HIGH);
    } 
    else if (strcmp(topic, "greenhouse/control/vent") == 0) {
        digitalWrite(VENT_PIN, message[0] == '1' ? LOW : HIGH);
    }
    else if (strcmp(topic, "greenhouse/control/heater") == 0) {
        digitalWrite(HEATER_PIN, message[0] == '1' ? LOW : HIGH);
    }
}

/**
 * Main Program Loop
 * Reads sensors, publishes data, and updates display every 5 seconds
 */
void loop() {
    // Ensure MQTT connection
    if (!client.connected()) reconnect();
    client.loop();

    // Read all sensor values
    float temp = dht.readTemperature();
    float humidity = dht.readHumidity();
    int soilRaw = analogRead(SOIL_PIN);
    int phRaw = analogRead(PH_PIN);
    tempSensor.requestTemperatures();
    float thermoTemp = tempSensor.getTempCByIndex(0);
    
    // Convert raw readings to meaningful values
    float soilMoisture = map(soilRaw, 4095, 0, 0, 100);  // 0-100%
    float ph = map(phRaw, 0, 4095, 0, 14);              // pH 0-14

    // Publish sensor readings to MQTT
    if (!isnan(temp)) client.publish("greenhouse/ambient/temp", String(temp).c_str());
    if (!isnan(humidity)) client.publish("greenhouse/ambient/humidity", String(humidity).c_str());
    client.publish("greenhouse/soil/moisture", String(soilMoisture).c_str());
    client.publish("greenhouse/soil/ph", String(ph).c_str());
    if (thermoTemp != -127) client.publish("greenhouse/thermo/temp", String(thermoTemp).c_str());
    
    // Publish device states
    client.publish("greenhouse/status/fan", digitalRead(FAN_PIN) == LOW ? "ON" : "OFF");
    client.publish("greenhouse/status/vent", digitalRead(VENT_PIN) == LOW ? "ON" : "OFF");
    client.publish("greenhouse/status/heater", digitalRead(HEATER_PIN) == LOW ? "ON" : "OFF");

    // Update OLED display with current readings
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Temp: "); display.print(temp); display.println(" C");
    display.print("Hum: "); display.print(humidity); display.println(" %");
    display.print("Soil: "); display.print(soilMoisture); display.println(" %");
    display.print("pH: "); display.println(ph);
    display.print("Thermo: "); display.print(thermoTemp); display.println(" C");
    display.print("Fan: "); display.println(digitalRead(FAN_PIN) == LOW ? "ON" : "OFF");
    display.print("Vent: "); display.println(digitalRead(VENT_PIN) == LOW ? "ON" : "OFF");
    display.print("Heat: "); display.println(digitalRead(HEATER_PIN) == LOW ? "ON" : "OFF");
    display.display();

    // Wait before next update
    delay(5000);  // 5 second interval
}