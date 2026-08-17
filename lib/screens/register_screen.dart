import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:firebase_database/firebase_database.dart';
import 'package:firebase_auth/firebase_auth.dart';

import '../app_theme.dart';
import '../services/auth_service.dart';

class RegisterScreen extends StatefulWidget {
  const RegisterScreen({super.key});

  @override
  State<RegisterScreen> createState() => _RegisterScreenState();
}

class _RegisterScreenState extends State<RegisterScreen> {
  final _formKey             = GlobalKey<FormState>();
  final _emailCtrl            = TextEditingController();
  final _passwordCtrl         = TextEditingController();
  final _confirmCtrl          = TextEditingController();
  final _fullNameCtrl         = TextEditingController();
  final _phoneCtrl            = TextEditingController();
  final _addressCtrl          = TextEditingController();
  final _dobCtrl              = TextEditingController();

  String? _gender;
  bool    _isLoading          = false;
  bool    _hidePassword       = true;
  bool    _hideConfirmPassword = true;

  int _step = 0;

  @override
  void dispose() {
    _emailCtrl.dispose();
    _passwordCtrl.dispose();
    _confirmCtrl.dispose();
    _fullNameCtrl.dispose();
    _phoneCtrl.dispose();
    _addressCtrl.dispose();
    _dobCtrl.dispose();
    super.dispose();
  }

  Future<void> _pickDate() async {
    final picked = await showDatePicker(
      context:     context,
      initialDate: DateTime(2000),
      firstDate:   DateTime(1900),
      lastDate:    DateTime.now(),
      builder:     (context, child) => Theme(
        data: Theme.of(context).copyWith(
          colorScheme: const ColorScheme.dark(
            primary:   AppColors.accent,
            surface:   AppColors.surface,
            onSurface: AppColors.textPrimary,
          ),
        ),
        child: child!,
      ),
    );
    if (picked != null) {
      setState(() =>
          _dobCtrl.text = '${picked.day}/${picked.month}/${picked.year}');
    }
  }

  Future<void> _register() async {
    if (!_formKey.currentState!.validate()) return;
    setState(() => _isLoading = true);

    final auth    = Provider.of<AuthService>(context, listen: false);
    final success = await auth.register(_emailCtrl.text, _passwordCtrl.text);

    if (!mounted) return;

    if (success) {
      final uid = FirebaseAuth.instance.currentUser?.uid;
      if (uid != null) {
        final userData = {
          'fullName': _fullNameCtrl.text.trim(),
          'gender':   _gender ?? 'Chưa rõ',
          'phone':    _phoneCtrl.text.trim(),
          'address':  _addressCtrl.text.trim(),
          'dob':      _dobCtrl.text,
          'email':    _emailCtrl.text.trim(),
          'createdAt': DateTime.now().toIso8601String(),
        };

        // 1. Ghi dữ liệu lên Cloud Firebase Realtime Database
        await FirebaseDatabase.instance.ref('users/$uid').set(userData);
        
        // 2. Ép đồng bộ luôn dữ liệu mới vào RAM của AuthService để Header ăn tên mới ngay lập tức
        auth.setUserData(userData);
      }

      setState(() => _isLoading = false);
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content:         const Text('Đăng ký thành công!'),
          backgroundColor: AppColors.safe,
          behavior:        SnackBarBehavior.floating,
          margin:          const EdgeInsets.all(16),
          shape:           RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
        ),
      );
      Navigator.pop(context);
    } else {
      setState(() => _isLoading = false);
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content:         Text(auth.lastErrorMessage),
          backgroundColor: AppColors.danger,
          behavior:        SnackBarBehavior.floating,
          margin:          const EdgeInsets.all(16),
          shape:           RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
        ),
      );
    }
  }

  void _onNext() {
    if (_step == 0) {
      if (!_formKey.currentState!.validate()) return;
      setState(() => _step = 1);
    } else {
      _register();
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Tạo tài khoản'),
        leading: BackButton(color: AppColors.textSecondary),
      ),
      body: SafeArea(
        child: Column(
          children: [
            Padding(
              padding: const EdgeInsets.symmetric(horizontal: 28, vertical: 16),
              child: Row(children: [
                _StepDot(index: 0, current: _step, label: 'Tài khoản'),
                Expanded(child: Divider(
                    color: _step > 0 ? AppColors.accent : AppColors.border,
                    thickness: 1.5)),
                _StepDot(index: 1, current: _step, label: 'Hồ sơ'),
              ]),
            ),

            Expanded(
              child: Form(
                key: _formKey,
                child: SingleChildScrollView(
                  padding: const EdgeInsets.symmetric(horizontal: 28),
                  child: AnimatedSwitcher(
                    duration: const Duration(milliseconds: 300),
                    child:     _step == 0 ? _buildStep0() : _buildStep1(),
                  ),
                ),
              ),
            ),

            Padding(
              padding: const EdgeInsets.all(28),
              child: ElevatedButton(
                onPressed: _isLoading ? null : _onNext,
                child: _isLoading
                    ? const SizedBox(
                        width: 22, height: 22,
                        child: CircularProgressIndicator(
                            color: AppColors.bg, strokeWidth: 2.5))
                    : Text(_step == 0 ? 'Tiếp theo' : 'Tạo tài khoản'),
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildStep0() {
    return Column(
      key: const ValueKey(0),
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const Text('Thông tin tài khoản', style: AppText.heading),
        const SizedBox(height: 4),
        const Text('Email và mật khẩu để đăng nhập', style: AppText.body),
        const SizedBox(height: 28),

        _Label('EMAIL'),
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
            if (v == null || v.isEmpty) return 'Nhập email';
            if (!v.contains('@'))        return 'Email không hợp lệ';
            return null;
          },
        ),

        const SizedBox(height: 20),
        _Label('MẬT KHẨU'),
        const SizedBox(height: 8),
        TextFormField(
          controller:  _passwordCtrl,
          obscureText: _hidePassword,
          style:       const TextStyle(color: AppColors.textPrimary),
          decoration:  InputDecoration(
            hintText:   '••••••••',
            prefixIcon: const Icon(Icons.lock_outline_rounded),
            suffixIcon: IconButton(
              icon: Icon(_hidePassword
                  ? Icons.visibility_off_outlined
                  : Icons.visibility_outlined),
              onPressed: () => setState(() => _hidePassword = !_hidePassword),
            ),
          ),
          validator: (v) {
            if (v == null || v.isEmpty) return 'Nhập mật khẩu';
            if (v.length < 6)           return 'Tối thiểu 6 ký tự';
            return null;
          },
        ),

        const SizedBox(height: 20),
        _Label('XÁC NHẬN MẬT KHẨU'),
        const SizedBox(height: 8),
        TextFormField(
          controller:  _confirmCtrl,
          obscureText: _hideConfirmPassword,
          style:       const TextStyle(color: AppColors.textPrimary),
          decoration:  InputDecoration(
            hintText:   '••••••••',
            prefixIcon: const Icon(Icons.lock_outline_rounded),
            suffixIcon: IconButton(
              icon: Icon(_hideConfirmPassword
                  ? Icons.visibility_off_outlined
                  : Icons.visibility_outlined),
              onPressed: () => setState(
                  () => _hideConfirmPassword = !_hideConfirmPassword),
            ),
          ),
          validator: (v) {
            if (v != _passwordCtrl.text) return 'Tài khoản hoặc mật khẩu không khớp';
            return null;
          },
        ),
        const SizedBox(height: 24),
      ],
    );
  }

  Widget _buildStep1() {
    return Column(
      key: const ValueKey(1),
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const Text('Hồ sơ cá nhân', style: AppText.heading),
        const SizedBox(height: 4),
        const Text('Thông tin này sẽ được lưu vào hệ thống', style: AppText.body),
        const SizedBox(height: 28),

        _Label('HỌ VÀ TÊN'),
        const SizedBox(height: 8),
        TextFormField(
          controller: _fullNameCtrl,
          style:      const TextStyle(color: AppColors.textPrimary),
          decoration: const InputDecoration(
              hintText:   '',
              prefixIcon: Icon(Icons.person_outline_rounded)),
          validator: (v) => v == null || v.isEmpty ? 'Nhập họ tên' : null,
        ),

        const SizedBox(height: 20),
        _Label('GIỚI TÍNH'),
        const SizedBox(height: 8),
        DropdownButtonFormField<String>(
          value:       _gender,
          dropdownColor: AppColors.surfaceAlt,
          style:       const TextStyle(color: AppColors.textPrimary),
          decoration:  const InputDecoration(
              prefixIcon: Icon(Icons.people_outline_rounded)),
          items: const [
            DropdownMenuItem(value: 'Nam',  child: Text('Nam')),
            DropdownMenuItem(value: 'Nữ',   child: Text('Nữ')),
            DropdownMenuItem(value: 'Khác', child: Text('Khác')),
          ],
          onChanged: (v) => setState(() => _gender = v),
          validator: (v) => v == null ? 'Chọn giới tính' : null,
        ),

        const SizedBox(height: 20),
        _Label('SỐ ĐIỆN THOẠI'),
        const SizedBox(height: 8),
        TextFormField(
          controller:   _phoneCtrl,
          keyboardType: TextInputType.phone,
          style:        const TextStyle(color: AppColors.textPrimary),
          decoration:   const InputDecoration(
              hintText:   '0912 345 678',
              prefixIcon: Icon(Icons.phone_outlined)),
        ),

        const SizedBox(height: 20),
        _Label('ĐỊA CHỈ'),
        const SizedBox(height: 8),
        TextFormField(
          controller: _addressCtrl,
          style:      const TextStyle(color: AppColors.textPrimary),
          decoration: const InputDecoration(
              hintText:   'TP. Hồ Chí Minh',
              prefixIcon: Icon(Icons.home_outlined)),
        ),

        const SizedBox(height: 20),
        _Label('NGÀY SINH'),
        const SizedBox(height: 8),
        TextFormField(
          controller: _dobCtrl,
          readOnly:   true,
          onTap:      _pickDate,
          style:      const TextStyle(color: AppColors.textPrimary),
          decoration: const InputDecoration(
              hintText:   'DD/MM/YYYY',
              prefixIcon: Icon(Icons.cake_outlined)),
          validator: (v) => v == null || v.isEmpty ? 'Chọn ngày sinh' : null,
        ),
        const SizedBox(height: 24),
      ],
    );
  }
}

class _Label extends StatelessWidget {
  final String text;
  const _Label(this.text);
  @override
  Widget build(BuildContext context) => Text(text, style: AppText.label);
}

class _StepDot extends StatelessWidget {
  final int index, current;
  final String label;
  const _StepDot({required this.index, required this.current, required this.label});

  @override
  Widget build(BuildContext context) {
    final done   = index < current;
    final active = index == current;
    final color  = done || active ? AppColors.accent : AppColors.textMuted;

    return Column(
      children: [
        Container(
          width: 32, height: 32,
          decoration: BoxDecoration(
            shape:  BoxShape.circle,
            color:  done ? AppColors.accent : (active ? AppColors.accentGlow : AppColors.surface),
            border: Border.all(color: color, width: 1.5),
          ),
          child: Center(
            child: done
                ? const Icon(Icons.check, color: AppColors.bg, size: 16)
                : Text('${index + 1}',
                    style: TextStyle(
                        color:      active ? AppColors.accent : AppColors.textMuted,
                        fontSize:   13,
                        fontWeight: FontWeight.w700)),
          ),
        ),
        const SizedBox(height: 4),
        Text(label, style: AppText.caption.copyWith(
            color: active ? AppColors.accent : AppColors.textMuted)),
      ],
    );
  }
}