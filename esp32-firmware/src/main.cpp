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
#include <WiFiClientSecure.h> // Required for SSL/TLS MQTT connection
#include <PubSubClient.h>    // MQTT client
#include <Adafruit_Sensor.h> // Required by DHT library
#include <DHT.h>             // DHT sensor
#include <OneWire.h>         // Required for DS18B20
#include <DallasTemperature.h>  // DS18B20 sensor
#include <Wire.h>           // I2C communication
#include <Adafruit_GFX.h>   // Graphics library
#include <Adafruit_SSD1306.h>  // OLED display
#include <esp_task_wdt.h>   // Watchdog timer
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

// Watchdog Timer Configuration
#define WDT_TIMEOUT 30      // Watchdog timeout in seconds

// Sensor value bounds for validation
#define TEMP_MIN -40.0      // Minimum valid temperature (°C)
#define TEMP_MAX 80.0       // Maximum valid temperature (°C)
#define HUMIDITY_MIN 0.0    // Minimum valid humidity (%)
#define HUMIDITY_MAX 100.0  // Maximum valid humidity (%)
#define SOIL_MIN 0.0        // Minimum valid soil moisture (%)
#define SOIL_MAX 100.0      // Maximum valid soil moisture (%)
#define PH_MIN 0.0          // Minimum valid pH
#define PH_MAX 14.0         // Maximum valid pH

// Initialize communication clients and sensors
WiFiClientSecure espClient; // Use Secure Client for HiveMQ Cloud
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
    
    // Startup delay to allow sensors to stabilize
    Serial.println("Starting up...");
    delay(2000);  // 2 second startup delay
    
    // Configure watchdog timer
    esp_task_wdt_init(WDT_TIMEOUT, true);  // Enable panic so ESP32 restarts
    esp_task_wdt_add(NULL);  // Add current thread to WDT watch
    Serial.println("Watchdog timer configured");
    
    // Heltec WiFi Kit 32 I2C configuration
    Wire.begin(SDA_OLED, SCL_OLED);  // SDA = GPIO 4, SCL = GPIO 15 for Heltec board
    
    // Initialize relay control pins (HIGH = OFF, LOW = ON)
    pinMode(FAN_PIN, OUTPUT); digitalWrite(FAN_PIN, HIGH);
    pinMode(VENT_PIN, OUTPUT); digitalWrite(VENT_PIN, HIGH);
    pinMode(HEATER_PIN, OUTPUT); digitalWrite(HEATER_PIN, HIGH);
    Serial.println("Relay pins configured");
    
    // Initialize sensors with additional startup delay
    dht.begin();
    tempSensor.begin();
    delay(1000);  // Allow sensors to stabilize
    Serial.println("Sensors initialized");
    
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
    display.println("     Version 1.0.0");
    display.println("");
    display.println("       (C) 2025");
    display.println("");
    display.println("    Pat Ryan Things");
   
    
    display.display();
    
    // Connect to WiFi network
    Serial.print("Connecting to WiFi");
    WiFi.begin(ssid, password);
    int wifiAttempts = 0;
    while (WiFi.status() != WL_CONNECTED && wifiAttempts < 30) {
        delay(500);
        Serial.print(".");
        wifiAttempts++;
        esp_task_wdt_reset();  // Reset watchdog during WiFi connection
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        
        // Configure Secure Client
        espClient.setInsecure(); // Skip certificate validation (Not recommended for production)
        // TODO: For production, use espClient.setCACert(root_ca) with the HiveMQ root certificate
    } else {
        Serial.println("\nWiFi connection failed! Restarting...");
        ESP.restart();
    }

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
    int attempts = 0;
    while (!client.connected() && attempts < 5) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("HeltecGreenhouse", mqtt_user, mqtt_pass)) {
            Serial.println("connected");
            if (client.subscribe("greenhouse/control/#")) {
                Serial.println("Subscribed to control topics");
            } else {
                Serial.println("Subscription failed");
            }
        } else {
            Serial.printf("failed, rc=%d\n", client.state());
            attempts++;
            delay(5000);
            esp_task_wdt_reset();  // Reset watchdog during reconnection
        }
    }
    
    // If still not connected after 5 attempts, restart
    if (!client.connected()) {
        Serial.println("MQTT connection failed after 5 attempts. Restarting...");
        ESP.restart();
    }
}

/**
 * MQTT Message Handler
 * Processes incoming control messages for devices
 * 
 * @param topic - MQTT topic (greenhouse/control/{device})
 * @param payload - Message content (ON/OFF or 1/0)
 * @param length - Message length
 */
void callback(char* topic, byte* payload, unsigned int length) {
    // Create a string from the payload safely
    String message = "";
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    
    // Determine if the command is to turn ON (supports "ON", "1", "true")
    bool turnOn = (message == "ON") || 
                  (message == "1") || 
                  (message == "true");

    // Handle incoming messages and control devices
    if (strcmp(topic, "greenhouse/control/fan") == 0) {
        digitalWrite(FAN_PIN, turnOn ? LOW : HIGH);
        Serial.printf("Fan turned %s\n", turnOn ? "ON" : "OFF");
    } 
    else if (strcmp(topic, "greenhouse/control/vent") == 0) {
        digitalWrite(VENT_PIN, turnOn ? LOW : HIGH);
        Serial.printf("Vent turned %s\n", turnOn ? "ON" : "OFF");
    }
    else if (strcmp(topic, "greenhouse/control/heater") == 0) {
        digitalWrite(HEATER_PIN, turnOn ? LOW : HIGH);
        Serial.printf("Heater turned %s\n", turnOn ? "ON" : "OFF");
    }
}

/**
 * Main Program Loop
 * Reads sensors, publishes data, and updates display every 5 seconds
 */
void loop() {
    // Reset watchdog timer to prevent restart
    esp_task_wdt_reset();
    
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

    // Validate and publish sensor readings with error checking
    // Temperature
    if (!isnan(temp) && temp >= TEMP_MIN && temp <= TEMP_MAX) {
        if (!client.publish("greenhouse/ambient/temp", String(temp, 2).c_str())) {
            Serial.println("Failed to publish temperature");
        }
    } else {
        Serial.printf("Invalid temperature reading: %.2f\n", temp);
    }
    
    // Humidity
    if (!isnan(humidity) && humidity >= HUMIDITY_MIN && humidity <= HUMIDITY_MAX) {
        if (!client.publish("greenhouse/ambient/humidity", String(humidity, 2).c_str())) {
            Serial.println("Failed to publish humidity");
        }
    } else {
        Serial.printf("Invalid humidity reading: %.2f\n", humidity);
    }
    
    // Soil Moisture
    if (soilMoisture >= SOIL_MIN && soilMoisture <= SOIL_MAX) {
        if (!client.publish("greenhouse/soil/moisture", String(soilMoisture, 2).c_str())) {
            Serial.println("Failed to publish soil moisture");
        }
    } else {
        Serial.printf("Invalid soil moisture reading: %.2f\n", soilMoisture);
    }
    
    // pH
    if (ph >= PH_MIN && ph <= PH_MAX) {
        if (!client.publish("greenhouse/soil/ph", String(ph, 2).c_str())) {
            Serial.println("Failed to publish pH");
        }
    } else {
        Serial.printf("Invalid pH reading: %.2f\n", ph);
    }
    
    // Thermometer temperature
    if (thermoTemp != -127 && thermoTemp >= TEMP_MIN && thermoTemp <= TEMP_MAX) {
        if (!client.publish("greenhouse/thermo/temp", String(thermoTemp, 2).c_str())) {
            Serial.println("Failed to publish thermometer temperature");
        }
    } else {
        Serial.printf("Invalid thermometer reading: %.2f\n", thermoTemp);
    }
    
    // Publish device states with error checking
    if (!client.publish("greenhouse/status/fan", digitalRead(FAN_PIN) == LOW ? "ON" : "OFF")) {
        Serial.println("Failed to publish fan status");
    }
    if (!client.publish("greenhouse/status/vent", digitalRead(VENT_PIN) == LOW ? "ON" : "OFF")) {
        Serial.println("Failed to publish vent status");
    }
    if (!client.publish("greenhouse/status/heater", digitalRead(HEATER_PIN) == LOW ? "ON" : "OFF")) {
        Serial.println("Failed to publish heater status");
    }

    // Update OLED display with current readings (display only valid values)
    display.clearDisplay();
    display.setCursor(0, 0);
    
    // Only display valid sensor values
    if (!isnan(temp) && temp >= TEMP_MIN && temp <= TEMP_MAX) {
        display.print("Temp: "); display.print(temp, 1); display.println(" C");
    } else {
        display.println("Temp: ERROR");
    }
    
    if (!isnan(humidity) && humidity >= HUMIDITY_MIN && humidity <= HUMIDITY_MAX) {
        display.print("Hum: "); display.print(humidity, 1); display.println(" %");
    } else {
        display.println("Hum: ERROR");
    }
    
    if (soilMoisture >= SOIL_MIN && soilMoisture <= SOIL_MAX) {
        display.print("Soil: "); display.print(soilMoisture, 1); display.println(" %");
    } else {
        display.println("Soil: ERROR");
    }
    
    if (ph >= PH_MIN && ph <= PH_MAX) {
        display.print("pH: "); display.println(ph, 1);
    } else {
        display.println("pH: ERROR");
    }
    
    if (thermoTemp != -127 && thermoTemp >= TEMP_MIN && thermoTemp <= TEMP_MAX) {
        display.print("Thermo: "); display.print(thermoTemp, 1); display.println(" C");
    } else {
        display.println("Thermo: ERROR");
    }
    
    display.print("Fan: "); display.println(digitalRead(FAN_PIN) == LOW ? "ON" : "OFF");
    display.print("Vent: "); display.println(digitalRead(VENT_PIN) == LOW ? "ON" : "OFF");
    display.print("Heat: "); display.println(digitalRead(HEATER_PIN) == LOW ? "ON" : "OFF");
    display.display();

    // Wait before next update
    delay(5000);  // 5 second interval
}