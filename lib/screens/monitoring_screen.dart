import 'dart:async';
import 'package:flutter/material.dart';
import 'package:intl/intl.dart';
import 'package:provider/provider.dart';

import '../app_theme.dart';
import '../models/health_data.dart';
import '../services/auth_service.dart';
import '../services/firebase_service.dart';
import '../widgets/heart_rate_graph.dart';
import '../widgets/map_widget.dart';
import '../widgets/metric_card.dart';
import '../widgets/status_banner.dart';
import '../widgets/patient_header.dart';

class MonitoringScreen extends StatefulWidget {
  const MonitoringScreen({super.key});
  @override
  State<MonitoringScreen> createState() => _MonitoringScreenState();
}

class _MonitoringScreenState extends State<MonitoringScreen> {
  final _firebaseService = FirebaseService();
  String _currentTime    = '';
  Timer? _timer;

  @override
  void initState() {
    super.initState();
    _updateTime();
    _timer = Timer.periodic(const Duration(seconds: 1), (_) => _updateTime());
  }

  void _updateTime() => setState(() => _currentTime = DateFormat('HH:mm:ss').format(DateTime.now()));

  @override
  void dispose() { _timer?.cancel(); super.dispose(); }

  void _confirmLogout() {
    showDialog(
      context: context,
      builder: (ctx) => AlertDialog(
        backgroundColor: AppColors.surfaceAlt,
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(20)),
        title: const Text('Đăng xuất?', style: TextStyle(color: AppColors.textPrimary)),
        content: const Text('Bạn sẽ phải đăng nhập lại để tiếp tục.', style: AppText.body),
        actions: [
          TextButton(onPressed: () => Navigator.pop(ctx), child: const Text('Hủy', style: TextStyle(color: AppColors.textSecondary))),
          TextButton(
            onPressed: () { Navigator.pop(ctx); Provider.of<AuthService>(context, listen: false).logout(); },
            child: const Text('Đăng xuất', style: TextStyle(color: AppColors.danger, fontWeight: FontWeight.w700)),
          ),
        ],
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: StreamBuilder<HealthData>(
        stream: _firebaseService.getRealtimeHealthData(),
        builder: (context, snapshot) {
          final data = snapshot.data;
          final connected = data != null && DateTime.now().difference(data.timestamp).inMinutes < 3;
          final isWearing = data?.wearing ?? false;

          final hrValue = isWearing ? data!.heartRate : 0;

          // 0 BPM khi đeo cũng được tính là Danger
          final isHrDanger = hrValue < 40 || hrValue > 110;
          final int? spo2Value = (isWearing && !isHrDanger) ? data!.spo2 : null;

          return CustomScrollView(
            slivers: [
              SliverAppBar(
                pinned: true, backgroundColor: AppColors.bg,
                title: Row(children: [
                  Container(
                    width: 8, height: 8,
                    decoration: BoxDecoration(
                      shape: BoxShape.circle, color: connected ? AppColors.safe : AppColors.neutral,
                      boxShadow: connected ? [BoxShadow(color: AppColors.safe.withOpacity(0.5), blurRadius: 6)] : null,
                    ),
                  ),
                  const SizedBox(width: 10), const Text('HealthWatch'), const SizedBox(width: 8),
                  Text(connected ? 'LIVE' : 'OFFLINE', style: TextStyle(fontSize: 10, fontWeight: FontWeight.w700, color: connected ? AppColors.safe : AppColors.neutral, letterSpacing: 1.2)),
                ]),
                actions: [
                  PopupMenuButton<String>(
                    icon: const Icon(Icons.more_vert, color: AppColors.textSecondary), color: AppColors.surfaceAlt,
                    shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(16)),
                    onSelected: (val) { if (val == 'logout') _confirmLogout(); },
                    itemBuilder: (context) => [
                      PopupMenuItem(
                        value: 'logout',
                        child: Row(children: const [
                          Icon(Icons.logout_rounded, color: AppColors.danger, size: 20), SizedBox(width: 12),
                          Text('Đăng xuất', style: TextStyle(color: AppColors.danger, fontSize: 14)),
                        ]),
                      ),
                    ],
                  ),
                ],
              ),
              SliverPadding(
                padding: const EdgeInsets.all(16),
                sliver: SliverList(
                  delegate: SliverChildListDelegate([
                    PatientHeader(time: _currentTime), const SizedBox(height: 20),
                    StatusBanner(status: data?.status ?? 'Đang chờ dữ liệu...'), const SizedBox(height: 20),

                    // ── Hàng 1: Nhịp tim và SpO₂ ──
                    Row(children: [
                      Expanded(
                        child: MetricCard(
                          label: 'NHỊP TIM',
                          value: hrValue,
                          unit: 'BPM',
                          icon: Icons.favorite_rounded,
                          color: _heartColor(hrValue, isWearing), // Thêm truyền biến isWearing vào đây
                          isWearing: isWearing,
                        ),
                      ),
                      const SizedBox(width: 12),
                      Expanded(
                        child: MetricCard(
                          label: 'SPO₂',
                          value: spo2Value,
                          unit: '%',
                          icon: Icons.water_drop_rounded,
                          color: _spo2Color(spo2Value ?? 0),
                          isWearing: isWearing,
                        ),
                      ),
                    ]),
                    const SizedBox(height: 12),

                    // ── Hàng 2: Trạng thái đeo và Té ngã ──
                    Row(children: [
                      Expanded(child: _WearingCard(isWearing: isWearing)),
                      const SizedBox(width: 12),
                      Expanded(child: _FallCard(detected: data?.fallDetected ?? false, isWearing: isWearing)),
                    ]),
                    const SizedBox(height: 20),

                    MapWidget(data: data), const SizedBox(height: 24),

                    // ── Lịch sử nhịp tim ──
                    const Text('Lịch sử nhịp tim', style: AppText.heading), const SizedBox(height: 12),
                    Container(
                      height: 260, padding: const EdgeInsets.all(16),
                      decoration: BoxDecoration(color: AppColors.surface, borderRadius: BorderRadius.circular(24), border: Border.all(color: AppColors.border)),
                      child: StreamBuilder<List<HealthData>>(
                        stream: _firebaseService.getHistoryData(),
                        builder: (context, histSnapshot) {
                          if (histSnapshot.connectionState == ConnectionState.waiting) return const Center(child: CircularProgressIndicator(color: AppColors.accent));
                          if (!histSnapshot.hasData || histSnapshot.data!.isEmpty) {
                            return Center(
                              child: Column(mainAxisSize: MainAxisSize.min, children: const [
                                Icon(Icons.history_toggle_off, color: AppColors.textMuted, size: 40), SizedBox(height: 8),
                                Text('Chưa có dữ liệu lịch sử', style: AppText.body),
                              ]),
                            );
                          }
                          return HeartRateGraph(historyData: histSnapshot.data!);
                        },
                      ),
                    ),
                    const SizedBox(height: 24),
                  ]),
                ),
              ),
            ],
          );
        },
      ),
    );
  }

  // Cập nhật hàm nhận diện màu sắc nhịp tim
  Color _heartColor(int hr, bool isWearing) {
    if (!isWearing) return AppColors.neutral; // Không đeo thì màu xám trung tính
    if (hr == 0) return AppColors.danger;     // Đang đeo mà bằng 0 BPM -> Báo đỏ (Danger)
    if (hr >= 50 && hr <= 100) return AppColors.safe;
    if ((hr >= 40 && hr <= 49) || (hr >= 101 && hr <= 110)) return AppColors.warn;
    return AppColors.danger;
  }

  Color _spo2Color(int spo2) {
    if (spo2 == 0) return AppColors.neutral;
    if (spo2 < 90) return AppColors.danger;
    if (spo2 < 95) return AppColors.warn;
    return AppColors.safe;
  }
}

// ─── Wearing Card ────────────────────────────────────────────────────────────
class _WearingCard extends StatelessWidget {
  final bool isWearing;
  const _WearingCard({required this.isWearing});

  @override
  Widget build(BuildContext context) {
    final color = isWearing ? AppColors.safe : AppColors.neutral;
    return Container(
      padding: const EdgeInsets.all(14), decoration: cardDecoration(),
      child: Column(children: [
        Container(width: 40, height: 40, decoration: BoxDecoration(shape: BoxShape.circle, color: color.withOpacity(0.15)), child: Icon(isWearing ? Icons.watch_rounded : Icons.watch_off_rounded, color: color, size: 22)),
        const SizedBox(height: 8), const Text('THIẾT BỊ', style: AppText.label), const SizedBox(height: 4),
        Text(isWearing ? 'Đang đeo' : 'Không đeo', style: TextStyle(color: color, fontWeight: FontWeight.w700, fontSize: 13), textAlign: TextAlign.center),
      ]),
    );
  }
}

// ─── Fall Card ───────────────────────────────────────────────────────────────
class _FallCard extends StatelessWidget {
  final bool detected;
  final bool isWearing;
  const _FallCard({required this.detected, required this.isWearing});

  @override
  Widget build(BuildContext context) {
    final color = !isWearing ? AppColors.neutral : (detected ? AppColors.danger : AppColors.safe);
    final icon  = !isWearing ? Icons.accessibility_new_rounded : (detected ? Icons.warning_rounded : Icons.accessibility_new_rounded);
    final label = !isWearing ? '--' : (detected ? 'Phát hiện' : 'An toàn');

    return Container(
      padding: const EdgeInsets.all(14), decoration: (detected && isWearing) ? glowDecoration(color) : cardDecoration(),
      child: Column(children: [
        Container(width: 40, height: 40, decoration: BoxDecoration(shape: BoxShape.circle, color: color.withOpacity(0.15)), child: Icon(icon, color: color, size: 22)),
        const SizedBox(height: 8), const Text('TÉ NGÃ', style: AppText.label), const SizedBox(height: 4),
        Text(label, style: TextStyle(color: color, fontWeight: FontWeight.w700, fontSize: 13), textAlign: TextAlign.center),
      ]),
    );
  }
}