import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import 'package:font_awesome_flutter/font_awesome_flutter.dart';

class SensorCard extends StatelessWidget {
  final String title, unit, value;
  final IconData icon;
  final Color color;
  final dynamic thresholdHigh;
  final dynamic thresholdLow;

  SensorCard({
    required this.title,
    required this.icon,
    required this.value,
    required this.unit,
    required this.color,
    this.thresholdHigh,
    this.thresholdLow,
  });

  @override
  Widget build(BuildContext context) {
    double? val = double.tryParse(value);
    bool isAlert = false;
    if (val != null) {
      if (thresholdHigh != null && val > (thresholdHigh as num).toDouble()) isAlert = true;
      if (thresholdLow != null && val < (thresholdLow as num).toDouble()) isAlert = true;
    }

    return Container(
      decoration: BoxDecoration(
        color: Color(0xFF2C2C2C),
        borderRadius: BorderRadius.circular(20),
        boxShadow: [
          BoxShadow(color: Colors.black45, blurRadius: 8, offset: Offset(0, 4)),
        ],
        border: isAlert ? Border.all(color: Colors.redAccent, width: 2) : null,
      ),
      child: Padding(
        padding: EdgeInsets.all(16),
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            FaIcon(icon, color: isAlert ? Colors.red : color, size: 32),
            SizedBox(height: 12),
            Text(title, 
              style: GoogleFonts.outfit(fontSize: 14, color: Colors.white70),
              textAlign: TextAlign.center,
            ),
            SizedBox(height: 8),
            RichText(
              text: TextSpan(
                children: [
                  TextSpan(
                    text: value,
                    style: GoogleFonts.outfit(fontSize: 28, fontWeight: FontWeight.bold, color: Colors.white),
                  ),
                  TextSpan(
                    text: unit,
                    style: GoogleFonts.outfit(fontSize: 16, color: Colors.white54),
                  ),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }
}