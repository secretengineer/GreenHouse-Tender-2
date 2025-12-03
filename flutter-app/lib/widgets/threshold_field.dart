import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';

class ThresholdField extends StatelessWidget {
  final String label, keyName;
  final dynamic initial;
  final Function(String, double) onChanged;

  ThresholdField({
    required this.label, 
    required this.keyName, 
    this.initial, 
    required this.onChanged
  });

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 8.0),
      child: TextField(
        style: GoogleFonts.outfit(color: Colors.white),
        decoration: InputDecoration(
          labelText: label,
          labelStyle: TextStyle(color: Colors.white70),
          enabledBorder: OutlineInputBorder(
            borderSide: BorderSide(color: Colors.white24),
            borderRadius: BorderRadius.circular(12),
          ),
          focusedBorder: OutlineInputBorder(
            borderSide: BorderSide(color: Colors.greenAccent),
            borderRadius: BorderRadius.circular(12),
          ),
          filled: true,
          fillColor: Colors.black12,
        ),
        keyboardType: TextInputType.numberWithOptions(decimal: true),
        controller: TextEditingController(text: initial?.toString() ?? ''),
        onSubmitted: (value) {
          if (value.isNotEmpty) {
            onChanged(keyName, double.parse(value));
          }
        },
      ),
    );
  }
}