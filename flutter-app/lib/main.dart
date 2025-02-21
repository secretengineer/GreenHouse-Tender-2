import 'package:flutter/material.dart';
import 'package:firebase_core/firebase_core.dart';
import 'package:cloud_firestore/cloud_firestore.dart';
import 'package:firebase_messaging/firebase_messaging.dart';
import 'package:flutter_vlc_player/flutter_vlc_player.dart';
import 'package:fl_chart/fl_chart.dart';
import 'package:google_fonts/google_fonts.dart';
import 'package:font_awesome_flutter/font_awesome_flutter.dart';
import 'widgets/sensor_card.dart';
import 'widgets/control_row.dart';
import 'widgets/threshold_field.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await Firebase.initializeApp();
  runApp(GreenhouseApp());
}

class GreenhouseApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      theme: ThemeData(
        textTheme: GoogleFonts.poppinsTextTheme(),
        primaryColor: Colors.green[700],
        scaffoldBackgroundColor: Colors.grey[100],
      ),
      home: Dashboard(),
    );
  }
}

class Dashboard extends StatefulWidget {
  @override
  _DashboardState createState() => _DashboardState();
}

class _DashboardState extends State<Dashboard> {
  late VlcPlayerController _videoController;
  final db = FirebaseFirestore.instance;
  Map<String, double> thresholds = {};

  @override
  void initState() {
    super.initState();
    _videoController = VlcPlayerController.network('http://<esp32-cam-ip>/stream', autoPlay: true);
    FirebaseMessaging.instance.subscribeToTopic('greenhouse_alerts');
    FirebaseMessaging.onMessage.listen((msg) {
      ScaffoldMessenger.of(context).showSnackBar(SnackBar(content: Text(msg.notification?.body ?? '')));
    });
    loadThresholds();
  }

  void loadThresholds() async {
    final doc = await db.collection('settings').doc('thresholds').get();
    setState(() => thresholds = Map<String, double>.from(doc.data() ?? {}));
  }

  void updateThreshold(String key, double value) {
    thresholds[key] = value;
    db.collection('settings').doc('thresholds').set(thresholds);
  }

  void sendControl(String device, String state) {
    db.collection('status').doc(device).set({'state': state, 'timestamp': FieldValue.serverTimestamp()});
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text("Greenhouse", style: GoogleFonts.poppins(fontWeight: FontWeight.bold)),
        backgroundColor: Colors.green[700],
        actions: [
          IconButton(
            icon: FaIcon(FontAwesomeIcons.cog),
            onPressed: () => showThresholdDialog(context),
          ),
        ],
      ),
      body: SingleChildScrollView(
        child: Column(
          children: [
            Container(
              height: 200,
              child: VlcPlayer(controller: _videoController, aspectRatio: 16 / 9),
            ),
            Padding(
              padding: EdgeInsets.all(16),
              child: StreamBuilder<QuerySnapshot>(
                stream: db.collection('sensor_data').orderBy('timestamp', descending: true).limit(50).snapshots(),
                builder: (context, snapshot) {
                  if (!snapshot.hasData) return CircularProgressIndicator();
                  final data = snapshot.data!.docs;
                  return Column(
                    children: [
                      SensorCard(
                        title: "Temperature",
                        icon: FontAwesomeIcons.thermometerHalf,
                        value: data.first['ambient_temp']?.toString() ?? '--',
                        unit: "°C",
                        graph: LineChart(LineChartData(
                          titlesData: FlTitlesData(show: false),
                          borderData: FlBorderData(show: false),
                          lineBarsData: [
                            LineChartBarData(
                              spots: data.reversed.map((d) => FlSpot(data.indexOf(d).toDouble(), d['ambient_temp'] ?? 0)).toList(),
                              color: Colors.red,
                            ),
                          ],
                        )),
                      ),
                      SensorCard(
                        title: "Humidity",
                        icon: FontAwesomeIcons.tint,
                        value: data.first['ambient_humidity']?.toString() ?? '--',
                        unit: "%",
                        graph: LineChart(LineChartData(
                          titlesData: FlTitlesData(show: false),
                          borderData: FlBorderData(show: false),
                          lineBarsData: [
                            LineChartBarData(
                              spots: data.reversed.map((d) => FlSpot(data.indexOf(d).toDouble(), d['ambient_humidity'] ?? 0)).toList(),
                              color: Colors.blue,
                            ),
                          ],
                        )),
                      ),
                      // Add more SensorCards for soil_moisture, ph, thermo_temp
                      ControlRow(sendControl: sendControl),
                    ],
                  );
                },
              ),
            ),
          ],
        ),
      ),
    );
  }

  void showThresholdDialog(BuildContext context) {
    showDialog(
      context: context,
      builder: (context) => AlertDialog(
        title: Text("Set Thresholds"),
        content: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            ThresholdField(label: "Temp High", key: "ambient_temp_high", initial: thresholds['ambient_temp_high']),
            ThresholdField(label: "Temp Low", key: "ambient_temp_low", initial: thresholds['ambient_temp_low']),
            // Add more fields
          ],
        ),
        actions: [
          TextButton(onPressed: () => Navigator.pop(context), child: Text("Cancel")),
          TextButton(onPressed: () => Navigator.pop(context), child: Text("Save")),
        ],
      ),
    );
  }
}