import 'package:flutter/material.dart';
import 'package:fl_chart/fl_chart.dart';
import '../app_theme.dart';
import '../models/health_data.dart';

class HeartRateGraph extends StatelessWidget {
  final List<HealthData> historyData;

  const HeartRateGraph({super.key, required this.historyData});

  @override
  Widget build(BuildContext context) {
    if (historyData.isEmpty) {
      return const Center(child: Text('Không có dữ liệu', style: AppText.body));
    }

    // Compute min/max for better Y axis
    final rates  = historyData.map((d) => d.heartRate.toDouble()).toList();
    final minY   = (rates.reduce((a, b) => a < b ? a : b) - 10).clamp(0.0, 200.0);
    final maxY   = (rates.reduce((a, b) => a > b ? a : b) + 10).clamp(0.0, 250.0);

    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Row(children: [
          Container(width: 10, height: 10,
              decoration: const BoxDecoration(
                  shape: BoxShape.circle, color: AppColors.danger)),
          const SizedBox(width: 8),
          Text('${historyData.length} lần đo',
              style: AppText.body.copyWith(color: AppColors.textSecondary)),
        ]),
        const SizedBox(height: 16),

        SizedBox(
          height: 260,
          child: LineChart(
            LineChartData(
              minY: minY,
              maxY: maxY,
              backgroundColor: Colors.transparent,

              gridData: FlGridData(
                show:             true,
                drawVerticalLine: false,
                horizontalInterval: 20,
                getDrawingHorizontalLine: (_) => FlLine(
                  color:       AppColors.border,
                  strokeWidth: 0.8,
                ),
              ),

              borderData: FlBorderData(
                show:  true,
                border: Border(
                  bottom: BorderSide(color: AppColors.border),
                  left:   BorderSide(color: AppColors.border),
                ),
              ),

              titlesData: FlTitlesData(
                leftTitles: AxisTitles(
                  sideTitles: SideTitles(
                    showTitles:   true,
                    reservedSize: 36,
                    interval:     20,
                    getTitlesWidget: (v, _) => Text(
                      v.toInt().toString(),
                      style: AppText.caption.copyWith(fontSize: 10),
                    ),
                  ),
                ),
                bottomTitles: AxisTitles(
                  sideTitles: SideTitles(
                    showTitles: true,
                    interval:   (historyData.length / 5).ceilToDouble().clamp(1, 100),
                    getTitlesWidget: (v, _) {
                      final i = v.toInt();
                      if (i < 0 || i >= historyData.length) return const SizedBox();
                      return Text(
                        '${i + 1}',
                        style: AppText.caption.copyWith(fontSize: 10),
                      );
                    },
                  ),
                ),
                rightTitles: const AxisTitles(sideTitles: SideTitles(showTitles: false)),
                topTitles:   const AxisTitles(sideTitles: SideTitles(showTitles: false)),
              ),

              lineBarsData: [
                LineChartBarData(
                  spots: historyData.asMap().entries.map((e) =>
                      FlSpot(e.key.toDouble(), e.value.heartRate.toDouble())
                  ).toList(),
                  isCurved:          true,
                  curveSmoothness:   0.3,
                  color:             AppColors.danger,
                  barWidth:          2.5,
                  isStrokeCapRound:  true,
                  dotData: FlDotData(
                    show: historyData.length <= 30,
                    getDotPainter: (_, __, ___, ____) => FlDotCirclePainter(
                      radius:    3,
                      color:     AppColors.danger,
                      strokeWidth: 1.5,
                      strokeColor: AppColors.surface,
                    ),
                  ),
                  belowBarData: BarAreaData(
                    show: true,
                    gradient: LinearGradient(
                      begin:  Alignment.topCenter,
                      end:    Alignment.bottomCenter,
                      colors: [
                        AppColors.danger.withOpacity(0.2),
                        AppColors.danger.withOpacity(0.0),
                      ],
                    ),
                  ),
                ),
              ],

              lineTouchData: LineTouchData(
                handleBuiltInTouches: true,
                touchTooltipData: LineTouchTooltipData(
                  getTooltipColor: (_) => AppColors.surfaceAlt,
                  tooltipRoundedRadius: 10,
                  getTooltipItems: (spots) => spots.map((spot) =>
                    LineTooltipItem(
                      '❤ ${spot.y.toInt()} BPM\nLần ${spot.x.toInt() + 1}',
                      const TextStyle(
                        color:      AppColors.textPrimary,
                        fontSize:   12,
                        fontWeight: FontWeight.w600,
                        fontFamily: 'RobotoMono',
                      ),
                    ),
                  ).toList(),
                ),
              ),
            ),
          ),
        ),
      ],
    );
  }
}
