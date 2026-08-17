import 'package:flutter/material.dart';
import 'package:intl/intl.dart';
import 'package:provider/provider.dart';
import '../app_theme.dart';
import '../services/auth_service.dart';

class PatientHeader extends StatelessWidget {
  final String time;

  const PatientHeader({
    super.key,
    required this.time,
  });

  // Logic tự động phân tích cú pháp d/M/yyyy (ví dụ: 12/1/2000 hoặc 18/1/2000) để tính tuổi chính xác
  int _calculateAge(String dobString) {
    if (dobString.isEmpty) return 0;
    try {
      DateFormat format = DateFormat("d/M/yyyy");
      DateTime birthDate = format.parse(dobString);
      DateTime today = DateTime.now();

      int age = today.year - birthDate.year;

      if (today.month < birthDate.month ||
          (today.month == birthDate.month && today.day < birthDate.day)) {
        age--;
      }
      return age;
    } catch (e) {
      debugPrint("Lỗi xử lý chuỗi Ngày Sinh: $e");
      return 0;
    }
  }

  void _showEditDialog(BuildContext context, AuthService authService) {
    final nameCtrl = TextEditingController(text: authService.fullName);
    final genderCtrl = TextEditingController(text: authService.gender);
    final dobCtrl = TextEditingController(text: authService.dob);

    showDialog(
      context: context,
      builder: (ctx) => AlertDialog(
        backgroundColor: AppColors.surfaceAlt,
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(20)),
        title: const Text('Chỉnh sửa thông tin', style: TextStyle(color: AppColors.textPrimary)),
        content: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            TextField(
              controller: nameCtrl,
              style: const TextStyle(color: AppColors.textPrimary),
              decoration: const InputDecoration(
                labelText: 'TÊN',
                labelStyle: AppText.label,
              ),
            ),
            const SizedBox(height: 16),
            TextField(
              controller: genderCtrl,
              style: const TextStyle(color: AppColors.textPrimary),
              decoration: const InputDecoration(
                labelText: 'GIỚI TÍNH',
                labelStyle: AppText.label,
              ),
            ),
            const SizedBox(height: 16),
            TextField(
              controller: dobCtrl,
              style: const TextStyle(color: AppColors.textPrimary),
              decoration: const InputDecoration(
                labelText: 'NGÀY SINH',
                hintText: 'Ví dụ: 12/1/2000',
                labelStyle: AppText.label,
              ),
            ),
          ],
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(ctx),
            child: const Text('Hủy', style: TextStyle(color: AppColors.textSecondary)),
          ),
          TextButton(
            onPressed: () {
              authService.updateProfileDetail(
                name: nameCtrl.text,
                gender: genderCtrl.text,
                dob: dobCtrl.text,
              );
              Navigator.pop(ctx);
            },
            child: const Text('Lưu', style: TextStyle(color: AppColors.accent, fontWeight: FontWeight.w700)),
          ),
        ],
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    final authService = Provider.of<AuthService>(context);

    // Tính toán số tuổi động từ thuộc tính dob
    final dobText = authService.dob;
    final genderText = authService.gender;
    final ageText = dobText.isNotEmpty ? '${_calculateAge(dobText)} tuổi' : '— tuổi';

    return GestureDetector(
      onTap: () => _showEditDialog(context, authService),
      child: Container(
        padding: const EdgeInsets.all(18),
        decoration: cardDecoration(),
        child: Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Row(children: [
              Container(
                width: 44,
                height: 44,
                decoration: BoxDecoration(
                  shape: BoxShape.circle,
                  color: AppColors.accentGlow,
                  border: Border.all(color: AppColors.accent, width: 1),
                ),
                child: const Icon(Icons.person, color: AppColors.accent, size: 22),
              ),
              const SizedBox(width: 14),
              Column(crossAxisAlignment: CrossAxisAlignment.start, children: [
                const Text('', style: AppText.label),
                const SizedBox(height: 2),
                Text(
                  authService.fullName,
                  style: const TextStyle(
                    color: AppColors.textPrimary,
                    fontWeight: FontWeight.w700,
                    fontSize: 16,
                  ),
                ),
                Text('$ageText  •  $genderText', style: AppText.caption),
              ]),
            ]),
            Column(crossAxisAlignment: CrossAxisAlignment.end, children: [
              const Text('THỜI GIAN', style: AppText.label),
              const SizedBox(height: 4),
              Text(
                time,
                style: const TextStyle(
                  fontFamily: 'RobotoMono',
                  color: AppColors.accent,
                  fontSize: 16,
                  fontWeight: FontWeight.w700,
                ),
              ),
              Text(
                DateFormat('dd/MM/yyyy').format(DateTime.now()),
                style: AppText.caption,
              ),
            ]),
          ],
        ),
      ),
    );
  }
}