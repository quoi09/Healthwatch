import 'package:flutter/material.dart';
import 'package:flutter_map/flutter_map.dart';
import 'package:latlong2/latlong.dart';
import '../app_theme.dart';
import '../models/health_data.dart';

// Ho Chi Minh City default fallback
const _kDefaultLat = 0.000000;
const _kDefaultLng = 0.000000;

class MapWidget extends StatefulWidget {
  final HealthData? data;
  const MapWidget({super.key, this.data});

  @override
  State<MapWidget> createState() => _MapWidgetState();
}

class _MapWidgetState extends State<MapWidget> {
  final MapController _mapController = MapController();

  @override
  void didUpdateWidget(covariant MapWidget oldWidget) {
    super.didUpdateWidget(oldWidget);
    final d = widget.data;
    if (d != null && d.hasLocation) {
      if (oldWidget.data?.lat != d.lat || oldWidget.data?.lng != d.lng) {
        WidgetsBinding.instance.addPostFrameCallback((_) {
          _mapController.move(LatLng(d.lat, d.lng), 16.0);
        });
      }
    }
  }

  @override
  void dispose() { _mapController.dispose(); super.dispose(); }

  @override
  Widget build(BuildContext context) {
    final data = widget.data;
    final hasLoc = data != null && data.hasLocation;
    final lat  = hasLoc ? data.lat : _kDefaultLat;
    final lng  = hasLoc ? data.lng : _kDefaultLng;
    final pos  = LatLng(lat, lng);

    return Container(
      decoration: cardDecoration(), padding: const EdgeInsets.all(16),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          // ── Header ──
          Row(children: const [
            Icon(Icons.location_on_rounded, color: AppColors.accent, size: 18),
            SizedBox(width: 8), Text('VỊ TRÍ', style: AppText.label),
          ]),
          const SizedBox(height: 14),

          // ── Map Container với Stack nút GPS ──
          ClipRRect(
            borderRadius: BorderRadius.circular(14),
            child: SizedBox(
              height: 230,
              child: Stack(
                children: [
                  FlutterMap(
                    mapController: _mapController,
                    options: MapOptions(initialCenter: pos, initialZoom: 16.0),
                    children: [
                      TileLayer(
                        urlTemplate: 'https://tile.openstreetmap.org/{z}/{x}/{y}.png',
                        userAgentPackageName: 'com.example.health_monitoring_app',
                      ),
                      MarkerLayer(markers: [
                        Marker(
                          point: pos, width: 44, height: 44,
                          child: _MarkerIcon(fall: data?.fallDetected ?? false, hasData: hasLoc),
                        ),
                      ]),
                    ],
                  ),
                  // Nút GPS định vị nhanh về vị trí thiết bị
                  Positioned(
                    right: 12, bottom: 12,
                    child: FloatingActionButton.small(
                      heroTag: "gps", backgroundColor: Colors.white, elevation: 2,
                      onPressed: () { if (hasLoc) _mapController.move(LatLng(data.lat, data.lng), 16.0); },
                      child: const Icon(Icons.my_location, color: AppColors.accent),
                    ),
                  ),
                ],
              ),
            ),
          ),

          // ── GPS coords ──
          const SizedBox(height: 10),
          Row(children: [
            Icon(hasLoc ? Icons.my_location : Icons.gps_not_fixed, color: AppColors.textMuted, size: 13),
            const SizedBox(width: 6),
            Text(
              hasLoc ? '${data.lat.toStringAsFixed(6)}, ${data.lng.toStringAsFixed(6)}' : 'Chưa có dữ liệu GPS',
              style: hasLoc ? AppText.caption.copyWith(fontFamily: 'RobotoMono') : AppText.caption,
            ),
          ]),
        ],
      ),
    );
  }
}

class _MarkerIcon extends StatelessWidget {
  final bool fall, hasData;
  const _MarkerIcon({required this.fall, required this.hasData});

  @override
  Widget build(BuildContext context) {
    final color = fall ? AppColors.danger : AppColors.accent;
    return Container(
      decoration: BoxDecoration(
        shape: BoxShape.circle, color: color.withOpacity(0.2),
        border: Border.all(color: color, width: 2),
        boxShadow: [BoxShadow(color: color.withOpacity(0.4), blurRadius: 10)],
      ),
      child: Icon(fall ? Icons.warning_rounded : Icons.person_pin_circle_rounded, color: color, size: 22),
    );
  }
}