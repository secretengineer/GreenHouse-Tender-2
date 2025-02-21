/**
 * Firebase Cloud Functions for Greenhouse Monitoring System
 * 
 * This module handles:
 * - MQTT message processing from ESP32 sensors
 * - Data storage in Firestore
 * - Automated alerts based on sensor thresholds
 */

const functions = require('firebase-functions');
const admin = require('firebase-admin');
const mqtt = require('mqtt');

// Initialize Firebase Admin SDK
admin.initializeApp();
const db = admin.firestore();

// Example configuration using environment variables
const client = mqtt.connect(functions.config().mqtt.url, {
  username: functions.config().mqtt.username,
  password: functions.config().mqtt.password,
});

/**
 * MQTT Connection Handler
 * Subscribes to all greenhouse-related topics when connected
 */
client.on('connect', () => {
  client.subscribe('greenhouse/#');
  console.log('Connected to MQTT broker');
});

/**
 * MQTT Message Handler
 * Processes incoming messages and stores data in Firestore
 * 
 * Topic structure:
 * - greenhouse/ambient/* : Environmental readings (temp, humidity)
 * - greenhouse/soil/*    : Soil-related readings (moisture, pH)
 * - greenhouse/thermo/*  : Additional temperature readings
 * - greenhouse/status/*  : Device status updates
 * 
 * @param {string} topic - MQTT topic path
 * @param {Buffer} message - Message payload
 */
client.on('message', (topic, message) => {
  const value = message.toString();
  const timestamp = admin.firestore.FieldValue.serverTimestamp();
  // Convert MQTT topic path to Firestore key (e.g., 'ambient/temp' → 'ambient_temp')
  const key = topic.split('/').slice(1).join('_');
  
  // Handle sensor data
  if (topic.startsWith('greenhouse/ambient') || 
      topic.startsWith('greenhouse/soil') || 
      topic.startsWith('greenhouse/thermo')) {
    // Store sensor reading in Firestore
    db.collection('sensor_data').add({ 
      [key]: parseFloat(value), 
      timestamp 
    });
    // Check if reading exceeds configured thresholds
    checkThresholds(key, value);
  } 
  // Handle device status updates
  else if (topic.startsWith('greenhouse/status')) {
    db.collection('status')
      .doc(key.split('_')[1])
      .set({ 
        state: value, 
        timestamp 
      });
  }
});

/**
 * Threshold Checker
 * Monitors sensor readings against configured thresholds and sends alerts
 * 
 * @param {string} key - Sensor identifier (e.g., 'ambient_temp')
 * @param {string} value - Sensor reading
 */
async function checkThresholds(key, value) {
  // Retrieve threshold settings from Firestore
  const settings = (await db.collection('settings').doc('thresholds').get()).data();
  const numValue = parseFloat(value);
  let title, body;

  // Check for high threshold violation
  if (settings[`${key}_high`] && numValue > settings[`${key}_high`]) {
    title = `High ${key.split('_').join(' ')} Alert`;
    body = `${key.split('_').join(' ')} is ${value}, above ${settings[`${key}_high`]}`;
  } 
  // Check for low threshold violation
  else if (settings[`${key}_low`] && numValue < settings[`${key}_low`]) {
    title = `Low ${key.split('_').join(' ')} Alert`;
    body = `${key.split('_').join(' ')} is ${value}, below ${settings[`${key}_low`]}`;
  }

  // Send notification if threshold was violated
  if (title) {
    admin.messaging().send({
      notification: { title, body },
      topic: 'greenhouse_alerts',
    });
  }
}

/**
 * HTTP Endpoint
 * Simple health check endpoint to verify function is running
 */
exports.mqttSync = functions.https.onRequest((req, res) => res.send('MQTT Sync Running'));