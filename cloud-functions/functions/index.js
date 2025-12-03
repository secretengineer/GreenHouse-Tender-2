/**
 * Firebase Cloud Functions for Greenhouse Monitoring System
 * 
 * This module handles:
 * - MQTT message processing from ESP32 sensors
 * - Data storage in Firestore (Real-time & History)
 * - Automated alerts based on sensor thresholds
 * - Bi-directional control (App -> Firestore -> MQTT -> ESP32)
 */

const functions = require('firebase-functions');
const admin = require('firebase-admin');
const mqtt = require('mqtt');

// Initialize Firebase Admin SDK
admin.initializeApp();
const db = admin.firestore();

// MQTT Configuration
const MQTT_URL = functions.config().mqtt.url || 'mqtt://broker.hivemq.com'; // Fallback for dev
const MQTT_USER = functions.config().mqtt.username;
const MQTT_PASS = functions.config().mqtt.password;

const client = mqtt.connect(MQTT_URL, {
  username: MQTT_USER,
  password: MQTT_PASS,
});

/**
 * MQTT Connection Handler
 */
client.on('connect', () => {
  client.subscribe('greenhouse/#');
  console.log('Connected to MQTT broker');
});

/**
 * MQTT Message Handler
 * Processes incoming messages:
 * 1. Updates 'status/latest' for real-time UI.
 * 2. Logs to 'sensor_history' for graphs.
 * 3. Updates device status.
 */
client.on('message', async (topic, message) => {
  const value = message.toString();
  const timestamp = admin.firestore.FieldValue.serverTimestamp();

  // Topic: greenhouse/ambient/temp -> key: ambient_temp
  const key = topic.split('/').slice(1).join('_');

  // 1. Handle Sensor Data (ambient, soil, thermo)
  if (topic.startsWith('greenhouse/ambient') ||
    topic.startsWith('greenhouse/soil') ||
    topic.startsWith('greenhouse/thermo')) {

    const numValue = parseFloat(value);

    // A. Update Real-time Status (Merge into one doc)
    await db.collection('status').doc('latest').set({
      [key]: numValue,
      last_updated: timestamp
    }, { merge: true });

    // B. Log History (New doc)
    await db.collection('sensor_history').add({
      type: key,
      value: numValue,
      timestamp: timestamp
    });

    // C. Check Thresholds
    checkThresholds(key, value);
  }
  // 2. Handle Device Status Updates (feedback from ESP32)
  else if (topic.startsWith('greenhouse/status')) {
    const device = key.split('_')[2]; // greenhouse_status_fan -> fan
    await db.collection('status').doc('latest').set({
      [`${device}_state`]: value, // e.g., fan_state: "ON"
      last_updated: timestamp
    }, { merge: true });
  }
});

/**
 * Firestore Trigger: Send Control Commands
 * Listens for new documents in 'commands' collection and publishes to MQTT.
 * App creates doc: commands/{auto-id} { device: 'fan', command: 'ON' }
 */
exports.sendControl = functions.firestore
  .document('commands/{cmdId}')
  .onCreate((snap, context) => {
    const data = snap.data();
    const device = data.device;   // 'fan', 'vent', 'heater'
    const command = data.command; // 'ON', 'OFF'

    if (device && command) {
      const topic = `greenhouse/control/${device}`;
      client.publish(topic, command);
      console.log(`Published ${command} to ${topic}`);
    }
    return snap.ref.delete(); // Cleanup command doc
  });

/**
 * Threshold Checker
 */
async function checkThresholds(key, value) {
  const settingsDoc = await db.collection('settings').doc('thresholds').get();
  if (!settingsDoc.exists) return;

  const settings = settingsDoc.data();
  const numValue = parseFloat(value);
  let title, body;

  if (settings[`${key}_high`] && numValue > settings[`${key}_high`]) {
    title = `High Alert: ${key}`;
    body = `${key} is ${value}, above ${settings[`${key}_high`]}`;
  }
  else if (settings[`${key}_low`] && numValue < settings[`${key}_low`]) {
    title = `Low Alert: ${key}`;
    body = `${key} is ${value}, below ${settings[`${key}_low`]}`;
  }

  if (title) {
    admin.messaging().send({
      notification: { title, body },
      topic: 'greenhouse_alerts',
    });
  }
}

exports.mqttSync = functions.https.onRequest((req, res) => res.send('MQTT Sync Running'));