import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import 'package:font_awesome_flutter/font_awesome_flutter.dart';

class SensorCard extends StatelessWidget {
  final String title, unit, value;
  final IconData icon;
  final Widget graph;

  SensorCard({required this.title, required this.icon, required this.value, required this.unit, required this.graph});

  @override
  Widget build(BuildContext context) {
    return Card(
      elevation: 4,
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
      child: Padding(
        padding: EdgeInsets.all(16),
        child: Column(
          children: [
            Row(
              children: [
                FaIcon(icon, color: Colors.green[700]),
                SizedBox(width: 8),
                Text(title, style: GoogleFonts.poppins(fontSize: 18, fontWeight: FontWeight.w500)),
                Spacer(),
                Text("$value $unit", style: GoogleFonts.poppins(fontSize: 20)),
              ],
            ),
            SizedBox(height: 16),
            Container(height: 100, child: graph),
          ],
        ),
      ),
    );
  }
}