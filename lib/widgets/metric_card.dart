import 'package:flutter/material.dart';
import '../app_theme.dart';

class MetricCard extends StatelessWidget {
  final String label;
  final int? value;
  final String unit;
  final IconData icon;
  final Color color;
  final bool isWearing;

  const MetricCard({
    super.key,
    required this.label,
    required this.value,
    required this.unit,
    required this.icon,
    required this.color,
    required this.isWearing,
  });

  @override
  Widget build(BuildContext context) {
    // Có dữ liệu để hiển thị khi đang đeo
    final showValue = isWearing && value != null;

    // Chỉ bật hiệu ứng glow khi giá trị > 0
    final glow = showValue && value! > 0;

    return Container(
      padding: const EdgeInsets.all(14),
      decoration: glow ? glowDecoration(color) : cardDecoration(),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          // Icon + Unit
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              Container(
                width: 32,
                height: 32,
                decoration: BoxDecoration(
                  shape: BoxShape.circle,
                  color: color.withOpacity(0.15),
                ),
                child: Icon(
                  icon,
                  color: color,
                  size: 18,
                ),
              ),
              Text(
                unit,
                style: AppText.caption.copyWith(
                  color: color.withOpacity(0.8),
                  fontWeight: FontWeight.w700,
                  fontSize: 10,
                ),
              ),
            ],
          ),

          const SizedBox(height: 10),

          // Giá trị
          Text(
            showValue ? value.toString() : '--',
            style: TextStyle(
              fontFamily: 'RobotoMono',
              fontSize: 28,
              fontWeight: FontWeight.w700,
              color: showValue ? color : AppColors.textMuted,
              letterSpacing: -0.5,
              height: 1.0,
            ),
          ),

          const SizedBox(height: 4),

          Text(
            label,
            style: AppText.label,
          ),
        ],
      ),
    );
  }
}