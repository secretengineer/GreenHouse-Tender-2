#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_server.h"
#include "../include/config.h"

// Define camera model - AI Thinker ESP32-CAM
#define CAMERA_MODEL_AI_THINKER

// Forward declaration
void startCameraServer();

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
    
    // Start the web server
    startCameraServer();

    Serial.print("Camera stream available at: http://");
    Serial.println(WiFi.localIP());
}

void loop() {
    // Main program loop
    delay(10000);  // 10 second delay
}

// ========================================
// Web Server Functions
// ========================================

httpd_handle_t stream_httpd = NULL;

static esp_err_t stream_handler(httpd_req_t *req) {
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t *_jpg_buf = NULL;
    char *part_buf[64];

    res = httpd_resp_set_type(req, "_multipart/x-mixed-replace;boundary=frame");
    if (res != ESP_OK) {
        return res;
    }

    while (true) {
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Camera capture failed");
            res = ESP_FAIL;
        } else {
            if (fb->format != PIXFORMAT_JPEG) {
                bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                esp_camera_fb_return(fb);
                fb = NULL;
                if (!jpeg_converted) {
                    Serial.println("JPEG compression failed");
                    res = ESP_FAIL;
                }
            } else {
                _jpg_buf_len = fb->len;
                _jpg_buf = fb->buf;
            }
        }
        if (res == ESP_OK) {
            size_t hlen = snprintf((char *)part_buf, 64, "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", _jpg_buf_len);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, "\r\n--frame\r\n", 12);
        }
        if (fb) {
            esp_camera_fb_return(fb);
            fb = NULL;
            _jpg_buf = NULL;
        } else if (_jpg_buf) {
            free(_jpg_buf);
            _jpg_buf = NULL;
        }
        if (res != ESP_OK) {
            break;
        }
    }
    return res;
}

void startCameraServer() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;

    httpd_uri_t index_uri = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = stream_handler,
        .user_ctx  = NULL
    };

    Serial.printf("Starting web server on port: '%d'\n", config.server_port);
    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(stream_httpd, &index_uri);
    }
}