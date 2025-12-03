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
      title: 'Greenhouse Tender',
      debugShowCheckedModeBanner: false,
      theme: ThemeData.dark().copyWith(
        scaffoldBackgroundColor: Color(0xFF121212),
        primaryColor: Colors.greenAccent,
        colorScheme: ColorScheme.dark(
          primary: Colors.greenAccent,
          secondary: Colors.tealAccent,
          surface: Color(0xFF1E1E1E),
        ),
        textTheme: GoogleFonts.outfitTextTheme(ThemeData.dark().textTheme),
        cardTheme: CardTheme(
          color: Color(0xFF1E1E1E),
          elevation: 8,
          shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(16)),
        ),
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
  Map<String, dynamic> thresholds = {};

  @override
  void initState() {
    super.initState();
    // Replace with your actual ESP32-CAM IP or DDNS URL
    _videoController = VlcPlayerController.network(
      'http://192.168.1.100/stream', 
      autoPlay: true,
      options: VlcPlayerOptions(),
    );
    
    setupMessaging();
    loadThresholds();
  }

  void setupMessaging() async {
    await FirebaseMessaging.instance.requestPermission();
    await FirebaseMessaging.instance.subscribeToTopic('greenhouse_alerts');
    FirebaseMessaging.onMessage.listen((msg) {
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text(msg.notification?.body ?? 'Alert received'),
            backgroundColor: Colors.redAccent,
            behavior: SnackBarBehavior.floating,
          )
        );
      }
    });
  }

  void loadThresholds() {
    db.collection('settings').doc('thresholds').snapshots().listen((snapshot) {
      if (snapshot.exists && mounted) {
        setState(() => thresholds = snapshot.data() ?? {});
      }
    });
  }

  void sendControl(String device, String command) {
    // Write to 'commands' collection to trigger Cloud Function -> MQTT
    db.collection('commands').add({
      'device': device,
      'command': command,
      'timestamp': FieldValue.serverTimestamp(),
    });
  }

  void updateThreshold(String key, double value) {
    db.collection('settings').doc('thresholds').set({key: value}, SetOptions(merge: true));
  }

  @override
  void dispose() {
    _videoController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: CustomScrollView(
        slivers: [
          SliverAppBar(
            expandedHeight: 250.0,
            floating: false,
            pinned: true,
            backgroundColor: Color(0xFF121212),
            flexibleSpace: FlexibleSpaceBar(
              title: Text("Greenhouse Tender", 
                style: GoogleFonts.outfit(fontWeight: FontWeight.bold, shadows: [Shadow(color: Colors.black, blurRadius: 10)])
              ),
              background: Stack(
                fit: StackFit.expand,
                children: [
                  VlcPlayer(
                    controller: _videoController,
                    aspectRatio: 16 / 9,
                    placeholder: Center(child: CircularProgressIndicator(color: Colors.greenAccent)),
                  ),
                  Container(
                    decoration: BoxDecoration(
                      gradient: LinearGradient(
                        begin: Alignment.topCenter,
                        end: Alignment.bottomCenter,
                        colors: [Colors.transparent, Color(0xFF121212)],
                        stops: [0.7, 1.0],
                      ),
                    ),
                  ),
                ],
              ),
            ),
            actions: [
              IconButton(
                icon: FaIcon(FontAwesomeIcons.slidersH),
                onPressed: () => showThresholdDialog(context),
              ),
            ],
          ),
          
          SliverToBoxAdapter(
            child: Padding(
              padding: EdgeInsets.all(16),
              child: StreamBuilder<DocumentSnapshot>(
                stream: db.collection('status').doc('latest').snapshots(),
                builder: (context, snapshot) {
                  if (!snapshot.hasData) return Center(child: CircularProgressIndicator());
                  if (!snapshot.data!.exists) return Center(child: Text("No Data Available"));

                  final data = snapshot.data!.data() as Map<String, dynamic>;

                  return Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text("Environment", style: Theme.of(context).textTheme.headlineSmall?.copyWith(fontWeight: FontWeight.bold, color: Colors.white70)),
                      SizedBox(height: 16),
                      GridView.count(
                        crossAxisCount: 2,
                        crossAxisSpacing: 16,
                        mainAxisSpacing: 16,
                        shrinkWrap: true,
                        physics: NeverScrollableScrollPhysics(),
                        childAspectRatio: 1.1,
                        children: [
                          SensorCard(
                            title: "Temperature",
                            icon: FontAwesomeIcons.temperatureHigh,
                            value: (data['ambient_temp'] ?? 0).toStringAsFixed(1),
                            unit: "°C",
                            color: Colors.orangeAccent,
                            thresholdHigh: thresholds['ambient_temp_high'],
                            thresholdLow: thresholds['ambient_temp_low'],
                          ),
                          SensorCard(
                            title: "Humidity",
                            icon: FontAwesomeIcons.tint,
                            value: (data['ambient_humidity'] ?? 0).toStringAsFixed(1),
                            unit: "%",
                            color: Colors.blueAccent,
                            thresholdHigh: thresholds['ambient_humidity_high'],
                            thresholdLow: thresholds['ambient_humidity_low'],
                          ),
                          SensorCard(
                            title: "Soil Moisture",
                            icon: FontAwesomeIcons.seedling,
                            value: (data['soil_moisture'] ?? 0).toStringAsFixed(0),
                            unit: "%",
                            color: Colors.greenAccent,
                            thresholdHigh: thresholds['soil_moisture_high'],
                            thresholdLow: thresholds['soil_moisture_low'],
                          ),
                          SensorCard(
                            title: "Soil pH",
                            icon: FontAwesomeIcons.flask,
                            value: (data['soil_ph'] ?? 0).toStringAsFixed(1),
                            unit: "pH",
                            color: Colors.purpleAccent,
                            thresholdHigh: thresholds['soil_ph_high'],
                            thresholdLow: thresholds['soil_ph_low'],
                          ),
                        ],
                      ),
                      SizedBox(height: 24),
                      Text("Controls", style: Theme.of(context).textTheme.headlineSmall?.copyWith(fontWeight: FontWeight.bold, color: Colors.white70)),
                      SizedBox(height: 16),
                      ControlRow(
                        data: data,
                        sendControl: sendControl,
                      ),
                      SizedBox(height: 80), // Bottom padding
                    ],
                  );
                },
              ),
            ),
          ),
        ],
      ),
    );
  }

  void showThresholdDialog(BuildContext context) {
    showDialog(
      context: context,
      builder: (context) => AlertDialog(
        backgroundColor: Color(0xFF2C2C2C),
        title: Text("Alert Thresholds", style: TextStyle(color: Colors.white)),
        content: SingleChildScrollView(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            children: [
              ThresholdField(
                label: "Max Temp (°C)", 
                keyName: "ambient_temp_high", 
                initial: thresholds['ambient_temp_high'],
                onChanged: updateThreshold,
              ),
              ThresholdField(
                label: "Min Temp (°C)", 
                keyName: "ambient_temp_low", 
                initial: thresholds['ambient_temp_low'],
                onChanged: updateThreshold,
              ),
              ThresholdField(
                label: "Min Humidity (%)", 
                keyName: "ambient_humidity_low", 
                initial: thresholds['ambient_humidity_low'],
                onChanged: updateThreshold,
              ),
              ThresholdField(
                label: "Min Soil Moisture (%)", 
                keyName: "soil_moisture_low", 
                initial: thresholds['soil_moisture_low'],
                onChanged: updateThreshold,
              ),
            ],
          ),
        ),
        actions: [
          TextButton(onPressed: () => Navigator.pop(context), child: Text("Close")),
        ],
      ),
    );
  }
}