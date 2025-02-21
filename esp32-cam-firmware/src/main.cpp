#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include "../include/config.h"

// Define camera model - AI Thinker ESP32-CAM
#define CAMERA_MODEL_AI_THINKER

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    
    // Camera configuration structure
    camera_config_t config;
    
    // LED and timing configuration
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    
    // Camera pin configuration for AI Thinker ESP32-CAM
    // Data pins (parallel interface)
    config.pin_d0 = 5;      // Y2_GPIO_NUM
    config.pin_d1 = 18;     // Y3_GPIO_NUM
    config.pin_d2 = 19;     // Y4_GPIO_NUM
    config.pin_d3 = 21;     // Y5_GPIO_NUM
    config.pin_d4 = 36;     // Y6_GPIO_NUM
    config.pin_d5 = 39;     // Y7_GPIO_NUM
    config.pin_d6 = 34;     // Y8_GPIO_NUM
    config.pin_d7 = 35;     // Y9_GPIO_NUM
    
    // Control pins
    config.pin_xclk = 0;    // XCLK GPIO
    config.pin_pclk = 22;   // PCLK GPIO
    config.pin_vsync = 25;  // VSYNC GPIO
    config.pin_href = 23;   // HREF GPIO
    
    // I2C pins for SCCB (Serial Camera Control Bus)
    config.pin_sscb_sda = 26;
    config.pin_sscb_scl = 27;
    
    // Reset and power down pins
    config.pin_pwdn = 32;   // Power down pin
    config.pin_reset = -1;  // -1 = not used
    
    // Camera settings
    config.xclk_freq_hz = 20000000;           // 20MHz clock frequency
    config.pixel_format = PIXFORMAT_JPEG;      // Output format
    config.frame_size = FRAMESIZE_VGA;         // Resolution 640x480
    config.jpeg_quality = 10;                  // Lower number = higher quality (0-63)
    config.fb_count = 1;                       // Number of frame buffers

    // Initialize the camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera initialization failed with error 0x%x\n", err);
        return;
    }

    // Connect to WiFi network
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    
    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    // Display connection info
    Serial.println("\nWiFi connected");
    Serial.print("Camera stream available at: http://");
    Serial.println(WiFi.localIP());
}

void loop() {
    // Main program loop
    delay(1000);  // 1 second delay
    // Add your main program logic here
}