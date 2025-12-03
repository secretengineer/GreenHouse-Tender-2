import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import 'package:font_awesome_flutter/font_awesome_flutter.dart';

class ControlRow extends StatelessWidget {
  final Map<String, dynamic> data;
  final void Function(String, String) sendControl;

  ControlRow({required this.data, required this.sendControl});

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceEvenly,
      children: [
        ControlButton(
          label: "Fan", 
          icon: FontAwesomeIcons.fan, 
          isActive: data['fan_state'] == 'ON',
          onTap: () => sendControl('fan', data['fan_state'] == 'ON' ? 'OFF' : 'ON'),
        ),
        ControlButton(
          label: "Vent", 
          icon: FontAwesomeIcons.wind, 
          isActive: data['vent_state'] == 'ON',
          onTap: () => sendControl('vent', data['vent_state'] == 'ON' ? 'OFF' : 'ON'),
        ),
        ControlButton(
          label: "Heater", 
          icon: FontAwesomeIcons.fire, 
          isActive: data['heater_state'] == 'ON',
          onTap: () => sendControl('heater', data['heater_state'] == 'ON' ? 'OFF' : 'ON'),
        ),
      ],
    );
  }
}

class ControlButton extends StatelessWidget {
  final String label;
  final IconData icon;
  final bool isActive;
  final VoidCallback onTap;

  ControlButton({
    required this.label, 
    required this.icon, 
    required this.isActive, 
    required this.onTap
  });

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTap: onTap,
      child: Container(
        width: 100,
        padding: EdgeInsets.symmetric(vertical: 16),
        decoration: BoxDecoration(
          color: isActive ? Colors.greenAccent.withOpacity(0.2) : Color(0xFF2C2C2C),
          borderRadius: BorderRadius.circular(16),
          border: Border.all(
            color: isActive ? Colors.greenAccent : Colors.transparent,
            width: 2,
          ),
          boxShadow: isActive ? [
            BoxShadow(color: Colors.greenAccent.withOpacity(0.4), blurRadius: 12, spreadRadius: 2)
          ] : [],
        ),
        child: Column(
          children: [
            FaIcon(
              icon, 
              size: 30, 
              color: isActive ? Colors.greenAccent : Colors.grey,
            ),
            SizedBox(height: 8),
            Text(label, 
              style: GoogleFonts.outfit(
                color: isActive ? Colors.white : Colors.grey,
                fontWeight: FontWeight.w600,
              )
            ),
            SizedBox(height: 4),
            Text(isActive ? "ON" : "OFF", 
              style: GoogleFonts.outfit(
                fontSize: 12,
                color: isActive ? Colors.greenAccent : Colors.grey[600],
              )
            ),
          ],
        ),
      ),
    );
  }
}