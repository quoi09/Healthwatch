import 'package:flutter/material.dart';
import '../app_theme.dart';

class ThresholdBar extends StatelessWidget {
  const ThresholdBar({super.key});

  static const _zones = [
    _Zone('< 40',    'Nguy hiểm', AppColors.danger),
    _Zone('40–49',   'Thấp',      AppColors.warn),
    _Zone('50–100',  'An toàn',   AppColors.safe),
    _Zone('101–110', 'Cao',       AppColors.warn),
    _Zone('> 110',   'Nguy hiểm', AppColors.danger),
  ];

  @override
  Widget build(BuildContext context) {
    return Container(
      padding:    const EdgeInsets.symmetric(horizontal: 16, vertical: 14),
      decoration: cardDecoration(),
      child: Column(children: [
        Row(children: [
          const Icon(Icons.bar_chart_rounded, color: AppColors.textMuted, size: 16),
          const SizedBox(width: 8),
          const Text('NGƯỠNG NHỊP TIM (BPM)', style: AppText.label),
        ]),
        const SizedBox(height: 14),

        // Gradient bar
        ClipRRect(
          borderRadius: BorderRadius.circular(4),
          child: SizedBox(
            height: 6,
            child: Row(children: [
              _BarSegment(AppColors.danger, 1),
              _BarSegment(AppColors.warn,   1),
              _BarSegment(AppColors.safe,   2),
              _BarSegment(AppColors.warn,   1),
              _BarSegment(AppColors.danger, 1),
            ]),
          ),
        ),

        const SizedBox(height: 12),

        Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: _zones.map((z) => _ZoneLabel(zone: z)).toList(),
        ),
      ]),
    );
  }
}

class _BarSegment extends StatelessWidget {
  final Color color;
  final int flex;
  const _BarSegment(this.color, this.flex);
  @override
  Widget build(BuildContext context) =>
      Expanded(flex: flex, child: Container(color: color));
}

class _ZoneLabel extends StatelessWidget {
  final _Zone zone;
  const _ZoneLabel({required this.zone});
  @override
  Widget build(BuildContext context) {
    return Column(children: [
      Text(zone.range,
          style: TextStyle(
              fontSize: 10, fontWeight: FontWeight.w700, color: zone.color,
              fontFamily: 'RobotoMono')),
      const SizedBox(height: 2),
      Text(zone.label,
          style: const TextStyle(fontSize: 9, color: AppColors.textMuted)),
    ]);
  }
}

class _Zone {
  final String range, label;
  final Color  color;
  const _Zone(this.range, this.label, this.color);
}
