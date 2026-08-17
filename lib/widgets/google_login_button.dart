import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../app_theme.dart';
import '../services/auth_service.dart';

class GoogleLoginButton extends StatefulWidget {
  const GoogleLoginButton({super.key});

  @override
  State<GoogleLoginButton> createState() => _GoogleLoginButtonState();
}

class _GoogleLoginButtonState extends State<GoogleLoginButton> {
  bool _isLoading = false;

  Future<void> _handleSignIn() async {
    setState(() => _isLoading = true);
    try {
      final auth    = Provider.of<AuthService>(context, listen: false);
      final success = await auth.loginWithGoogle();
      if (!mounted) return;
      if (!success) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content:         Text(auth.lastErrorMessage.isNotEmpty
                ? auth.lastErrorMessage
                : 'Đăng nhập Google thất bại. Kiểm tra cấu hình SHA-1 và Firebase.'),
            backgroundColor: AppColors.danger,
            behavior:        SnackBarBehavior.floating,
            margin:          const EdgeInsets.all(16),
            shape:           RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
          ),
        );
      }
    } finally {
      if (mounted) setState(() => _isLoading = false);
    }
  }

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width:  double.infinity,
      height: 52,
      child: OutlinedButton(
        onPressed: _isLoading ? null : _handleSignIn,
        style: OutlinedButton.styleFrom(
          side:  const BorderSide(color: AppColors.border, width: 1.5),
          shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(14)),
          backgroundColor: AppColors.surface,
        ),
        child: _isLoading
            ? const SizedBox(
                width: 20, height: 20,
                child: CircularProgressIndicator(
                    color: AppColors.accent, strokeWidth: 2))
            : Row(mainAxisAlignment: MainAxisAlignment.center, children: [
                _GoogleLogo(),
                const SizedBox(width: 12),
                const Text(
                  'Đăng nhập bằng Google',
                  style: TextStyle(
                    fontSize:   15,
                    color:      AppColors.textPrimary,
                    fontWeight: FontWeight.w600,
                  ),
                ),
              ]),
      ),
    );
  }
}

/// Simple Google "G" painted with correct brand colors
class _GoogleLogo extends StatelessWidget {
  @override
  Widget build(BuildContext context) =>
      SizedBox(width: 20, height: 20, child: CustomPaint(painter: _GPainter()));
}

class _GPainter extends CustomPainter {
  @override
  void paint(Canvas canvas, Size size) {
    final cx = size.width / 2;
    final cy = size.height / 2;
    final r  = size.width / 2;

    // Draw full circle segments
    final segments = [
      (const Color(0xFF4285F4), -0.5,  1.0),  // blue
      (const Color(0xFF34A853),  0.5,  1.0),  // green
      (const Color(0xFFFBBC05),  1.5,  0.5),  // yellow
      (const Color(0xFFEA4335),  2.0,  1.5),  // red
    ];

    for (final (color, startAngle, sweep) in segments) {
      final paint = Paint()..color = color..style = PaintingStyle.fill;
      final path  = Path()
        ..moveTo(cx, cy)
        ..arcTo(
          Rect.fromCircle(center: Offset(cx, cy), radius: r),
          startAngle, sweep, false,
        )
        ..close();
      canvas.drawPath(path, paint);
    }

    // White center hole
    canvas.drawCircle(Offset(cx, cy), r * 0.6, Paint()..color = Colors.white);

    // Blue right bar of G
    final barPaint = Paint()..color = const Color(0xFF4285F4);
    canvas.drawRect(
      Rect.fromLTWH(cx, cy - r * 0.18, r, r * 0.36),
      barPaint,
    );
  }

  @override
  bool shouldRepaint(covariant CustomPainter _) => false;
}
