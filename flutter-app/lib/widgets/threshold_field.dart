import 'package:flutter/material.dart';

class ThresholdField extends StatelessWidget {
  final String label, key;
  final double? initial;

  ThresholdField({required this.label, required this.key, this.initial});

  @override
  Widget build(BuildContext context) {
    return TextField(
      decoration: InputDecoration(labelText: label),
      keyboardType: TextInputType.number,
      controller: TextEditingController(text: initial?.toString() ?? ''),
      onSubmitted: (value) => (context.findAncestorStateOfType<_DashboardState>())?.updateThreshold(key, double.parse(value)),
    );
  }
}