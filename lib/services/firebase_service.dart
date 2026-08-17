import 'package:firebase_database/firebase_database.dart';
import '../models/health_data.dart';

class FirebaseService {
  static const String _deviceId = 'device_01';
  final DatabaseReference _db = FirebaseDatabase.instance.ref(_deviceId);

  // ─── Realtime ────────────────────────────────────────────────────────────────
  Stream<HealthData> getRealtimeHealthData() {
    return _db.child('realtime').onValue.map((event) {
      final raw = event.snapshot.value;
      if (raw == null) return _empty();
      final map = Map<dynamic, dynamic>.from(raw as Map);
      return HealthData.fromMap(map, 'realtime');
    });
  }

  // ─── History ─────────────────────────────────────────────────────────────────
  Stream<List<HealthData>> getHistoryData() {
    return _db.child('history').onValue.map((event) {
      final raw = event.snapshot.value;
      if (raw == null || raw is! Map) return <HealthData>[];
      final list = Map<dynamic, dynamic>.from(raw)
          .entries
          .where((e) => e.value is Map)
          .map((e) => HealthData.fromMap(
        Map<dynamic, dynamic>.from(e.value as Map),
        e.key.toString(),
      ))
          .toList()
        ..sort((a, b) => a.timestamp.compareTo(b.timestamp));
      return list;
    });
  }

  // ─── Helper ─────────────────────────────────────────────────────────────────
  HealthData _empty() => HealthData(
    heartRate: 0,
    spo2: 0,
    fallDetected: false,
    timestamp: DateTime.now(),
    status: 'Đang chờ dữ liệu...',
    wearing: false,
  );
}
