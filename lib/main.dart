import 'package:flutter/material.dart';
import 'package:firebase_core/firebase_core.dart';
import 'package:provider/provider.dart';

import 'app_theme.dart';
import 'screens/login_screen.dart';
import 'screens/monitoring_screen.dart';
import 'services/auth_service.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await Firebase.initializeApp();
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MultiProvider(
      providers: [
        ChangeNotifierProvider(create: (_) => AuthService()),
      ],
      child: MaterialApp(
        debugShowCheckedModeBanner: false,
        title: 'HealthWatch',
        theme: AppTheme.theme,
        home: Consumer<AuthService>(
          builder: (context, auth, _) {
            if (auth.isLoading) {
              return const _SplashScreen();
            }
            return auth.isAuthenticated
                ? const MonitoringScreen()
                : const LoginScreen();
          },
        ),
      ),
    );
  }
}

class _SplashScreen extends StatelessWidget {
  const _SplashScreen();

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: AppColors.bg,
      body: Center(
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            Container(
              width: 72,
              height: 72,
              decoration: BoxDecoration(
                shape: BoxShape.circle,
                color: AppColors.accentGlow,
                border: Border.all(color: AppColors.accent, width: 1.5),
              ),
              child: const Icon(Icons.monitor_heart_outlined,
                  color: AppColors.accent, size: 36),
            ),
            const SizedBox(height: 24),
            const Text('HealthWatch',
                style: TextStyle(
                  fontSize: 24,
                  fontWeight: FontWeight.w700,
                  color: AppColors.textPrimary,
                  letterSpacing: 1,
                )),
            const SizedBox(height: 32),
            const SizedBox(
              width: 24,
              height: 24,
              child: CircularProgressIndicator(
                color: AppColors.accent,
                strokeWidth: 2,
              ),
            ),
          ],
        ),
      ),
    );
  }
}
