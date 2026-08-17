import 'package:flutter/material.dart';
import '../app_theme.dart';

class StatusBanner extends StatelessWidget {
  final String status;

  const StatusBanner({super.key, required this.status});

  @override
  Widget build(BuildContext context) {
    final cfg = _config(status);

    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 18, vertical: 14),
      decoration: BoxDecoration(
        color:        cfg.color.withOpacity(0.08),
        borderRadius: BorderRadius.circular(16),
        border:       Border.all(color: cfg.color.withOpacity(0.4), width: 1.5),
        boxShadow: [
          BoxShadow(
            color:      cfg.color.withOpacity(0.12),
            blurRadius: 16,
            spreadRadius: 0,
          ),
        ],
      ),
      child: Row(children: [
        Container(
          width: 44, height: 44,
          decoration: BoxDecoration(
            shape: BoxShape.circle,
            color: cfg.color.withOpacity(0.15),
          ),
          child: Icon(cfg.icon, color: cfg.color, size: 24),
        ),
        const SizedBox(width: 16),
        Expanded(
          child: Column(crossAxisAlignment: CrossAxisAlignment.start, children: [
            Text('TRẠNG THÁI', style: AppText.label),
            const SizedBox(height: 2),
            Text(status,
                style: TextStyle(
                    color:      cfg.color,
                    fontSize:   15,
                    fontWeight: FontWeight.w700)),
            if (cfg.sub.isNotEmpty) ...[
              const SizedBox(height: 2),
              Text(cfg.sub, style: AppText.caption),
            ],
          ]),
        ),
        // Pulse dot for danger
        if (cfg.pulse) _PulseDot(color: cfg.color),
      ]),
    );
  }

  _StatusConfig _config(String status) {
    if (status.contains('Nguy kịch') || status.contains('Khẩn cấp')) {
      return _StatusConfig(
        color: AppColors.danger,
        icon:  Icons.emergency_rounded,
        sub:   'Liên hệ bác sĩ ngay lập tức',
        pulse: true,
      );
    }
    if (status.contains('Nguy hiểm')) {
      return _StatusConfig(
        color: AppColors.danger,
        icon:  Icons.warning_rounded,
        sub:   'Nhịp tim trong ngưỡng nguy hiểm',
        pulse: true,
      );
    }
    if (status.contains('Cao') || status.contains('Thấp')) {
      return _StatusConfig(
        color: AppColors.warn,
        icon:  Icons.monitor_heart_rounded,
        sub:   'Nhịp tim trong ngưỡng cần theo dõi thêm',
        pulse: false,
      );
    }
    if (status.contains('Bình thường')) {
      return _StatusConfig(
        color: AppColors.safe,
        icon:  Icons.check_circle_rounded,
        sub:   'Nhịp tim trong ngưỡng an toàn',
        pulse: false,
      );
    }
    return _StatusConfig(
      color: AppColors.neutral,
      icon:  Icons.hourglass_top_rounded,
      sub:   '',
      pulse: false,
    );
  }
}

class _StatusConfig {
  final Color   color;
  final IconData icon;
  final String  sub;
  final bool    pulse;
  const _StatusConfig({required this.color, required this.icon, required this.sub, required this.pulse});
}

class _PulseDot extends StatefulWidget {
  final Color color;
  const _PulseDot({required this.color});
  @override
  State<_PulseDot> createState() => _PulseDotState();
}

class _PulseDotState extends State<_PulseDot> with SingleTickerProviderStateMixin {
  late AnimationController _ctrl;
  late Animation<double>   _scale;

  @override
  void initState() {
    super.initState();
    _ctrl  = AnimationController(vsync: this, duration: const Duration(milliseconds: 900))
      ..repeat(reverse: true);
    _scale = Tween<double>(begin: 0.8, end: 1.4)
        .animate(CurvedAnimation(parent: _ctrl, curve: Curves.easeInOut));
  }

  @override
  void dispose() {
    _ctrl.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return ScaleTransition(
      scale: _scale,
      child: Container(
        width: 12, height: 12,
        decoration: BoxDecoration(
          shape:     BoxShape.circle,
          color:     widget.color,
          boxShadow: [BoxShadow(color: widget.color.withOpacity(0.6), blurRadius: 8)],
        ),
      ),
    );
  }
}
