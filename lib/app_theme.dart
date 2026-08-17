import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

// ─── Color Palette ────────────────────────────────────────────────────────────
// Deep navy + electric teal + clinical white — professional medical aesthetic

class AppColors {
  AppColors._();

  static const bg         = Color(0xFF0D1B2A); // deep navy
  static const surface    = Color(0xFF1A2B3C); // card surface
  static const surfaceAlt = Color(0xFF243447); // elevated card
  static const border     = Color(0xFF2E4460);

  static const accent     = Color(0xFF00D4FF); // electric teal
  static const accentGlow = Color(0x2200D4FF);

  static const safe       = Color(0xFF00E676); // green
  static const warn       = Color(0xFFFFAB40); // amber
  static const danger     = Color(0xFFFF5252); // red
  static const neutral    = Color(0xFF78909C); // grey

  static const textPrimary   = Color(0xFFE8F4FD);
  static const textSecondary = Color(0xFF90A4B7);
  static const textMuted     = Color(0xFF4A6880);
}

// ─── Typography ──────────────────────────────────────────────────────────────

class AppText {
  AppText._();

  // Display numbers (BPM, SpO2) — mono feel
  static const TextStyle display = TextStyle(
    fontFamily: 'RobotoMono',
    fontSize: 48,
    fontWeight: FontWeight.w700,
    letterSpacing: -1,
    color: AppColors.textPrimary,
    height: 1.0,
  );

  static const TextStyle displaySm = TextStyle(
    fontFamily: 'RobotoMono',
    fontSize: 32,
    fontWeight: FontWeight.w600,
    color: AppColors.textPrimary,
    height: 1.1,
  );

  static const TextStyle heading = TextStyle(
    fontSize: 20,
    fontWeight: FontWeight.w700,
    color: AppColors.textPrimary,
    letterSpacing: 0.3,
  );

  static const TextStyle label = TextStyle(
    fontSize: 11,
    fontWeight: FontWeight.w600,
    color: AppColors.textSecondary,
    letterSpacing: 1.5,
  );

  static const TextStyle body = TextStyle(
    fontSize: 14,
    color: AppColors.textSecondary,
    height: 1.5,
  );

  static const TextStyle caption = TextStyle(
    fontSize: 12,
    color: AppColors.textMuted,
  );
}

// ─── Theme ───────────────────────────────────────────────────────────────────

class AppTheme {
  AppTheme._();

  static ThemeData get theme {
    SystemChrome.setSystemUIOverlayStyle(const SystemUiOverlayStyle(
      statusBarColor:            Colors.transparent,
      statusBarIconBrightness:   Brightness.light,
      systemNavigationBarColor:  AppColors.bg,
    ));

    return ThemeData(
      useMaterial3:  true,
      brightness:    Brightness.dark,
      scaffoldBackgroundColor: AppColors.bg,
      colorScheme: const ColorScheme.dark(
        primary:   AppColors.accent,
        surface:   AppColors.surface,
        onPrimary: AppColors.bg,
        onSurface: AppColors.textPrimary,
      ),
      inputDecorationTheme: InputDecorationTheme(
        filled:      true,
        fillColor:   AppColors.surface,
        labelStyle:  AppText.body.copyWith(color: AppColors.textSecondary),
        hintStyle:   AppText.caption,
        border:      OutlineInputBorder(
          borderRadius: BorderRadius.circular(14),
          borderSide:   const BorderSide(color: AppColors.border),
        ),
        enabledBorder: OutlineInputBorder(
          borderRadius: BorderRadius.circular(14),
          borderSide:   const BorderSide(color: AppColors.border),
        ),
        focusedBorder: OutlineInputBorder(
          borderRadius: BorderRadius.circular(14),
          borderSide:   const BorderSide(color: AppColors.accent, width: 1.5),
        ),
        errorBorder: OutlineInputBorder(
          borderRadius: BorderRadius.circular(14),
          borderSide:   const BorderSide(color: AppColors.danger),
        ),
        contentPadding: const EdgeInsets.symmetric(horizontal: 18, vertical: 16),
        prefixIconColor: AppColors.textMuted,
        suffixIconColor: AppColors.textMuted,
      ),
      elevatedButtonTheme: ElevatedButtonThemeData(
        style: ElevatedButton.styleFrom(
          backgroundColor: AppColors.accent,
          foregroundColor: AppColors.bg,
          minimumSize:    const Size(double.infinity, 52),
          shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(14)),
          textStyle: const TextStyle(
            fontSize:   16,
            fontWeight: FontWeight.w700,
            letterSpacing: 0.5,
          ),
          elevation: 0,
        ),
      ),
      appBarTheme: const AppBarTheme(
        backgroundColor:  AppColors.bg,
        foregroundColor:  AppColors.textPrimary,
        elevation:        0,
        centerTitle:      false,
        titleTextStyle:   TextStyle(
          fontSize:   18,
          fontWeight: FontWeight.w700,
          color:      AppColors.textPrimary,
          letterSpacing: 0.2,
        ),
        systemOverlayStyle: SystemUiOverlayStyle.light,
      ),
    );
  }
}

// ─── Shared Decorations ──────────────────────────────────────────────────────

BoxDecoration cardDecoration({Color? borderColor}) => BoxDecoration(
      color:        AppColors.surface,
      borderRadius: BorderRadius.circular(20),
      border:       Border.all(color: borderColor ?? AppColors.border, width: 1),
    );

BoxDecoration glowDecoration(Color color) => BoxDecoration(
      color:        AppColors.surface,
      borderRadius: BorderRadius.circular(20),
      border:       Border.all(color: color.withOpacity(0.5), width: 1.5),
      boxShadow:    [BoxShadow(color: color.withOpacity(0.15), blurRadius: 20, spreadRadius: 2)],
    );
