import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import '../app_theme.dart';
import '../services/auth_service.dart';
import '../widgets/google_login_button.dart';
import 'register_screen.dart';

class LoginScreen extends StatefulWidget {
  const LoginScreen({super.key});

  @override
  State<LoginScreen> createState() => _LoginScreenState();
}

class _LoginScreenState extends State<LoginScreen>
    with SingleTickerProviderStateMixin {
  final _formKey          = GlobalKey<FormState>();
  final _emailCtrl        = TextEditingController();
  final _passwordCtrl     = TextEditingController();
  bool _isLoading         = false;
  bool _obscurePassword   = true;
  late AnimationController _animCtrl;
  late Animation<double>   _fadeAnim;

  @override
  void initState() {
    super.initState();
    _animCtrl = AnimationController(
      vsync:    this,
      duration: const Duration(milliseconds: 900),
    );
    _fadeAnim = CurvedAnimation(parent: _animCtrl, curve: Curves.easeOut);
    _animCtrl.forward();
  }

  @override
  void dispose() {
    _animCtrl.dispose();
    _emailCtrl.dispose();
    _passwordCtrl.dispose();
    super.dispose();
  }

Future<void> _handleLogin() async {
    if (!_formKey.currentState!.validate()) return;
    setState(() => _isLoading = true);

    final auth    = Provider.of<AuthService>(context, listen: false);
    final success = await auth.login(_emailCtrl.text, _passwordCtrl.text);

    if (!mounted) return;
    setState(() => _isLoading = false);

    if (success) {
      // Ví dụ: Tự động tách phần tên từ email để hiển thị tạm thời trước khi người dùng đổi tên
      final defaultName = _emailCtrl.text.split('@').first;
      auth.updateProfile(defaultName, '45 tuổi  •  Nam');
    } else {
      _showError(auth.lastErrorMessage);
    }
  }

  void _showError(String message) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Row(children: [
          const Icon(Icons.error_outline, color: Colors.white, size: 18),
          const SizedBox(width: 10),
          Expanded(child: Text(message, style: const TextStyle(fontSize: 13))),
        ]),
        backgroundColor: AppColors.danger,
        behavior:        SnackBarBehavior.floating,
        margin:          const EdgeInsets.all(16),
        shape:           RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
        duration:        const Duration(seconds: 3),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Stack(
        children: [
          // ── Background grid decoration ──
          Positioned.fill(child: _GridBackground()),

          SafeArea(
            child: FadeTransition(
              opacity: _fadeAnim,
              child: SingleChildScrollView(
                padding: const EdgeInsets.symmetric(horizontal: 28),
                child: Form(
                  key: _formKey,
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      const SizedBox(height: 56),

                      // ── Brand ──
                      Row(children: [
                        Container(
                          width: 48, height: 48,
                          decoration: BoxDecoration(
                            shape:  BoxShape.circle,
                            color:  AppColors.accentGlow,
                            border: Border.all(color: AppColors.accent, width: 1.5),
                          ),
                          child: const Icon(Icons.monitor_heart_outlined,
                              color: AppColors.accent, size: 24),
                        ),
                        const SizedBox(width: 12),
                        const Text('HealthWatch',
                            style: TextStyle(
                              fontSize:     20,
                              fontWeight:   FontWeight.w800,
                              color:        AppColors.accent,
                              letterSpacing: 0.5,
                            )),
                      ]),

                      const SizedBox(height: 52),

                      const Text('Chào mừng\ntrở lại', style: TextStyle(
                        fontSize:   36,
                        fontWeight: FontWeight.w800,
                        color:      AppColors.textPrimary,
                        height:     1.15,
                      )),

                      const SizedBox(height: 8),
                      const Text('Đăng nhập để giám sát sức khỏe',
                          style: AppText.body),

                      const SizedBox(height: 44),

                      // ── Email ──
                      _FieldLabel('EMAIL'),
                      const SizedBox(height: 8),
                      TextFormField(
                        controller:   _emailCtrl,
                        keyboardType: TextInputType.emailAddress,
                        style:        const TextStyle(color: AppColors.textPrimary),
                        decoration:   const InputDecoration(
                          hintText:   'ten@email.com',
                          prefixIcon: Icon(Icons.mail_outline_rounded),
                        ),
                        validator: (v) {
                          if (v == null || v.isEmpty) return 'Vui lòng nhập email';
                          if (!v.contains('@'))        return 'Email không hợp lệ';
                          return null;
                        },
                      ),

                      const SizedBox(height: 20),

                      // ── Password ──
                      _FieldLabel('MẬT KHẨU'),
                      const SizedBox(height: 8),
                      TextFormField(
                        controller:  _passwordCtrl,
                        obscureText: _obscurePassword,
                        style:       const TextStyle(color: AppColors.textPrimary),
                        decoration:  InputDecoration(
                          hintText:   '••••••••',
                          prefixIcon: const Icon(Icons.lock_outline_rounded),
                          suffixIcon: IconButton(
                            icon: Icon(_obscurePassword
                                ? Icons.visibility_off_outlined
                                : Icons.visibility_outlined),
                            onPressed: () => setState(
                                () => _obscurePassword = !_obscurePassword),
                          ),
                        ),
                        validator: (v) {
                          if (v == null || v.isEmpty) return 'Vui lòng nhập mật khẩu';
                          if (v.length < 6)           return 'Tối thiểu 6 ký tự';
                          return null;
                        },
                      ),

                      const SizedBox(height: 36),

                      // ── Login button ──
                      ElevatedButton(
                        onPressed: _isLoading ? null : _handleLogin,
                        child: _isLoading
                            ? const SizedBox(
                                width: 22, height: 22,
                                child: CircularProgressIndicator(
                                  color: AppColors.bg, strokeWidth: 2.5))
                            : const Text('Đăng nhập'),
                      ),

                      const SizedBox(height: 20),

                      // ── Divider ──
                      Row(children: [
                        Expanded(child: Divider(color: AppColors.border)),
                        Padding(
                          padding: const EdgeInsets.symmetric(horizontal: 14),
                          child: Text('hoặc', style: AppText.caption),
                        ),
                        Expanded(child: Divider(color: AppColors.border)),
                      ]),

                      const SizedBox(height: 20),

                      // ── Register ──
                      Center(
                        child: GestureDetector(
                          onTap: () => Navigator.push(context,
                              MaterialPageRoute(
                                  builder: (_) => const RegisterScreen())),
                          child: RichText(
                            text: const TextSpan(
                              text: 'Chưa có tài khoản? ',
                              style: AppText.body,
                              children: [
                                TextSpan(
                                  text:  'Đăng ký ngay',
                                  style: TextStyle(
                                    color:      AppColors.accent,
                                    fontWeight: FontWeight.w700,
                                  ),
                                ),
                              ],
                            ),
                          ),
                        ),
                      ),

                      const SizedBox(height: 40),
                    ],
                  ),
                ),
              ),
            ),
          ),
        ],
      ),
    );
  }
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

class _FieldLabel extends StatelessWidget {
  final String text;
  const _FieldLabel(this.text);
  @override
  Widget build(BuildContext context) =>
      Text(text, style: AppText.label);
}

class _GridBackground extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return CustomPaint(
      painter: _GridPainter(),
      child: Container(
        decoration: const BoxDecoration(
          gradient: LinearGradient(
            begin: Alignment.topRight,
            end:   Alignment.bottomLeft,
            colors: [Color(0xFF0D1B2A), Color(0xFF071018)],
          ),
        ),
      ),
    );
  }
}

class _GridPainter extends CustomPainter {
  @override
  void paint(Canvas canvas, Size size) {
    final paint = Paint()
      ..color       = AppColors.border.withOpacity(0.35)
      ..strokeWidth = 0.5;

    const step = 40.0;
    for (double x = 0; x < size.width; x += step) {
      canvas.drawLine(Offset(x, 0), Offset(x, size.height), paint);
    }
    for (double y = 0; y < size.height; y += step) {
      canvas.drawLine(Offset(0, y), Offset(size.width, y), paint);
    }

    // Accent glow top-right
    final glowPaint = Paint()
      ..shader = RadialGradient(
        colors: [AppColors.accent.withOpacity(0.12), Colors.transparent],
        radius: 0.7,
      ).createShader(Rect.fromCircle(
          center: Offset(size.width, 0), radius: size.width * 0.7));
    canvas.drawRect(Rect.fromLTWH(0, 0, size.width, size.height), glowPaint);
  }

  @override
  bool shouldRepaint(covariant CustomPainter _) => false;
}
