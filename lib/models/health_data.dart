import 'package:intl/intl.dart';

class HealthData {
  final int heartRate;
  final int spo2;
  final bool fallDetected;
  final DateTime timestamp;
  final String status;
  final bool wearing;
  final double lat;
  final double lng;
  final String zone;

  HealthData({
    required this.heartRate,
    required this.spo2,
    required this.fallDetected,
    required this.timestamp,
    required this.status,
    this.wearing = false,
    this.lat = 0.0,
    this.lng = 0.0,
    this.zone = 'none',
  });

  /// Parse từ Firebase map, key là timestamp millisecond
  factory HealthData.fromMap(Map<dynamic, dynamic> map, String key) {
    final heartRate = _parseInt(map['bpm'] ?? map['heartRate']);
    final spo2      = _parseInt(map['spo2']);
    final fall      = map['fallState'] == true || map['fallState'].toString() == 'true';
    final wearing   = map['wearing']?.toString() == 'yes';

    double lat = 0.0, lng = 0.0;
    if (map['gps'] is Map) {
      final gps = Map<dynamic, dynamic>.from(map['gps']);
      lat = _parseDouble(gps['lat']);
      lng = _parseDouble(gps['lng']);
    }

    DateTime ts;
    try {
      ts = DateFormat('dd-MM-yyyy HH:mm:ss').parse(map['datetime'].toString());
    } catch (_) {
      ts = DateTime.fromMillisecondsSinceEpoch(
        int.tryParse(key) ?? DateTime.now().millisecondsSinceEpoch,
      );
    }

    return HealthData(
      heartRate:    heartRate,
      spo2:         spo2,
      fallDetected: fall,
      timestamp:    ts,
      status:       getStatusFromZone(map['zone']?.toString() ?? '', heartRate, fall),
      wearing:      wearing,
      lat:          lat,
      lng:          lng,
      zone:         map['zone']?.toString() ?? 'none',
    );
  }

  // ─── Helpers ────────────────────────────────────────────────────────────────

  static int _parseInt(dynamic raw) {
    if (raw == null || raw == '--') return 0;
    return int.tryParse(raw.toString()) ?? 0;
  }

  static double _parseDouble(dynamic raw) =>
      double.tryParse(raw?.toString() ?? '0') ?? 0.0;

  /// Single source of truth cho status — dùng chung mọi nơi
  static String getStatus(int hr) {
    if (hr == "--")   return 'Đang chờ dữ liệu...';
    if (hr <= 40)   return 'Nguy hiểm - Nhịp tim quá thấp';
    if (hr <= 50)   return 'Thấp - Cần theo dõi';
    if (hr <= 100) return 'Bình thường';
    if (hr <= 110) return 'Cao - Cần theo dõi';
    return 'Nguy hiểm - Nhịp tim quá cao';
  }

  static String getStatusFromZone(String zone, int bpm, bool fall) {
    if (fall) return 'PHÁT HIỆN TÉ NGÃ';
    switch (zone) {
      case 'danger':  return 'Nguy hiểm';
      case 'warning': return 'Cần theo dõi';
      case 'safe':    return 'Bình thường';
      default:        return getStatus(bpm);
    }
  }

  bool get hasLocation => lat != 0.0 && lng != 0.0;
}