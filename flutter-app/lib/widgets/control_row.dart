import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import 'package:font_awesome_flutter/font_awesome_flutter.dart';

class ControlRow extends StatelessWidget {
  final void Function(String, String) sendControl;

  ControlRow({required this.sendControl});

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceEvenly,
      children: [
        ControlButton(label: "Fan", icon: FontAwesomeIcons.fan, onTap: sendControl),
        ControlButton(label: "Vent", icon: FontAwesomeIcons.wind, onTap: sendControl),
        ControlButton(label: "Heater", icon: FontAwesomeIcons.fire, onTap: sendControl),
      ],
    );
  }
}

class ControlButton extends StatelessWidget {
  final String label;
  final IconData icon;
  final void Function(String, String) onTap;

  ControlButton({required this.label, required this.icon, required this.onTap});

  @override
  Widget build(BuildContext context) {
    return Column(
      children: [
        IconButton(
          icon: FaIcon(icon, size: 30),
          color: Colors.green[700],
          onPressed: () => onTap(label.toLowerCase(), "ON"), // Add toggle logic later
        ),
        Text(label, style: GoogleFonts.poppins()),
      ],
    );
  }
}