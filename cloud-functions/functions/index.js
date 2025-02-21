const functions = require('firebase-functions');
const admin = require('firebase-admin');
const mqtt = require('mqtt');

admin.initializeApp();
const db = admin.firestore();

const client = mqtt.connect('mqtts://your-cluster.hivemq.cloud:8883', {
  username: 'your-username',
  password: 'your-password',
});

client.on('connect', () => {
  client.subscribe('greenhouse/#');
});

client.on('message', (topic, message) => {
  const value = message.toString();
  const timestamp = admin.firestore.FieldValue.serverTimestamp();
  const key = topic.split('/').slice(1).join('_');
  
  if (topic.startsWith('greenhouse/ambient') || topic.startsWith('greenhouse/soil') || topic.startsWith('greenhouse/thermo')) {
    db.collection('sensor_data').add({ [key]: parseFloat(value), timestamp });
    checkThresholds(key, value);
  } else if (topic.startsWith('greenhouse/status')) {
    db.collection('status').doc(key.split('_')[1]).set({ state: value, timestamp });
  }
});

async function checkThresholds(key, value) {
  const settings = (await db.collection('settings').doc('thresholds').get()).data();
  const numValue = parseFloat(value);
  let title, body;

  if (settings[`${key}_high`] && numValue > settings[`${key}_high`]) {
    title = `High ${key.split('_').join(' ')} Alert`;
    body = `${key.split('_').join(' ')} is ${value}, above ${settings[`${key}_high`]}`;
  } else if (settings[`${key}_low`] && numValue < settings[`${key}_low`]) {
    title = `Low ${key.split('_').join(' ')} Alert`;
    body = `${key.split('_').join(' ')} is ${value}, below ${settings[`${key}_low`]}`;
  }

  if (title) {
    admin.messaging().send({
      notification: { title, body },
      topic: 'greenhouse_alerts',
    });
  }
}

exports.mqttSync = functions.https.onRequest((req, res) => res.send('MQTT Sync Running'));