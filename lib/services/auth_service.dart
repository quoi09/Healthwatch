import 'package:flutter/material.dart';
import 'package:firebase_auth/firebase_auth.dart';
import 'package:firebase_database/firebase_database.dart';
import 'package:google_sign_in/google_sign_in.dart';

enum AuthError { none, wrongPassword, userNotFound, emailInUse, weakPassword, network, unknown }

class AuthService extends ChangeNotifier {
  final _auth         = FirebaseAuth.instance;
  final _googleSignIn = GoogleSignIn();

  bool _isAuthenticated = false;
  bool _isLoading       = true;
  AuthError _lastError  = AuthError.none;

  // ── Biến cũ (Giữ lại để tương thích ngược nếu các màn hình khác cần) ──
  String _userName = 'Nguyễn Văn A';
  String _userInfo = '45 tuổi  •  Nam';

  // ── Biến mới tương thích cấu trúc Firebase Realtime DB của bạn ──
  String _fullName = 'Nguyễn Văn A';
  String _gender   = 'Nam';
  String _dob      = '12/1/2000';

  bool      get isAuthenticated => _isAuthenticated;
  bool      get isLoading       => _isLoading;
  AuthError get lastError       => _lastError;
  
  // ── Getters ──
  String    get userName        => _userName;
  String    get userInfo        => _userInfo;
  String    get fullName        => _fullName;
  String    get gender          => _gender;
  String    get dob             => _dob;

  String get lastErrorMessage {
    switch (_lastError) {
      case AuthError.wrongPassword:  return 'Mật khẩu không đúng.';
      case AuthError.userNotFound:   return 'Tài khoản không tồn tại.';
      case AuthError.emailInUse:     return 'Email đã được sử dụng.';
      case AuthError.weakPassword:   return 'Mật khẩu quá yếu (tối thiểu 6 ký tự).';
      case AuthError.network:        return 'Lỗi kết nối mạng.';
      case AuthError.unknown:        return 'Đã xảy ra lỗi. Vui lòng thử lại.';
      case AuthError.none:           return '';
    }
  }

  AuthService() {
    _auth.authStateChanges().listen((user) async {
      _isAuthenticated = user != null;
      
      if (user != null) {
        if (user.displayName != null && user.displayName!.isNotEmpty) {
          _fullName = user.displayName!;
          _userName = user.displayName!;
        }

        // Tự động fetch dữ liệu từ node users/$uid về máy khi phát hiện trạng thái Login
        try {
          final uid = user.uid;
          final snapshot = await FirebaseDatabase.instance.ref('users/$uid').get();
          
          if (snapshot.exists && snapshot.value != null) {
            final data = Map<String, dynamic>.from(snapshot.value as Map);
            _fullName = data['fullName'] ?? _fullName;
            _userName = data['fullName'] ?? _userName;
            _gender   = data['gender'] ?? _gender;
            _dob      = data['dob'] ?? _dob;
            _userInfo = '$_dob  •  $_gender';
          }
        } catch (e) {
          debugPrint("Lỗi khi tải thông tin từ Realtime DB: $e");
        }
      } else {
        // Reset về mặc định khi Logout
        _userName = 'Nguyễn Văn A';
        _userInfo = '45 tuổi  •  Nam';
        _fullName = 'Nguyễn Văn A';
        _gender   = 'Nam';
        _dob      = '12/1/2000';
      }
      
      _isLoading = false;
      notifyListeners();
    });
  }

  // Hàm ép đồng bộ dữ liệu cục bộ ngay khi vừa đăng ký thành công
  void setUserData(Map<String, dynamic> data) {
    _fullName = data['fullName'] ?? _fullName;
    _userName = data['fullName'] ?? _userName;
    _gender   = data['gender'] ?? _gender;
    _dob      = data['dob'] ?? _dob;
    _userInfo = '$_dob  •  $_gender';
    notifyListeners();
  }

  // Hàm cập nhật dữ liệu từ Dialog chỉnh sửa lên cả bộ nhớ App lẫn Firebase
  Future<void> updateProfileDetail({
    required String name,
    required String gender,
    required String dob,
  }) async {
    if (name.trim().isNotEmpty) {
      _fullName = name.trim();
      _userName = name.trim();
    }
    if (gender.trim().isNotEmpty) _gender = gender.trim();
    if (dob.trim().isNotEmpty) _dob = dob.trim();
    _userInfo = '$_dob  •  $_gender';

    notifyListeners();

    try {
      final uid = _auth.currentUser?.uid;
      if (uid != null) {
        await FirebaseDatabase.instance.ref('users/$uid').update({
          'fullName': _fullName,
          'gender': _gender,
          'dob': _dob,
        });
      }
    } catch (e) {
      debugPrint("Lỗi cập nhật Firebase: $e");
    }
  }

  void updateProfile(String name, String info) {
    if (name.trim().isNotEmpty) _userName = name.trim();
    if (info.trim().isNotEmpty) _userInfo = info.trim();
    notifyListeners();
  }

  // ─── Email / Password ────────────────────────────────────────────────────────

  Future<bool> login(String email, String password) async {
    _lastError = AuthError.none;
    try {
      await _auth.signInWithEmailAndPassword(
        email:    email.trim(),
        password: password.trim(),
      );
      return true;
    } on FirebaseAuthException catch (e) {
      _lastError = _mapError(e.code);
      notifyListeners();
      return false;
    } catch (_) {
      _lastError = AuthError.unknown;
      notifyListeners();
      return false;
    }
  }

  Future<bool> register(String email, String password) async {
    _lastError = AuthError.none;
    try {
      await _auth.createUserWithEmailAndPassword(
        email:    email.trim(),
        password: password.trim(),
      );
      return true;
    } on FirebaseAuthException catch (e) {
      _lastError = _mapError(e.code);
      notifyListeners();
      return false;
    } catch (_) {
      _lastError = AuthError.unknown;
      notifyListeners();
      return false;
    }
  }

  // ─── Google ─────────────────────────────────────────────────────────────────

  Future<bool> loginWithGoogle() async {
    _lastError = AuthError.none;
    try {
      final googleUser = await _googleSignIn.signIn();
      if (googleUser == null) return false;

      final googleAuth = await googleUser.authentication;
      final credential = GoogleAuthProvider.credential(
        accessToken: googleAuth.accessToken,
        idToken:     googleAuth.idToken,
      );
      final result = await _auth.signInWithCredential(credential);
      
      if (result.user != null && result.user!.displayName != null) {
        _userName = result.user!.displayName!;
        _fullName = result.user!.displayName!;
      }
      
      return result.user != null;
    } on FirebaseAuthException catch (e) {
      _lastError = _mapError(e.code);
      notifyListeners();
      return false;
    } catch (_) {
      _lastError = AuthError.unknown;
      notifyListeners();
      return false;
    }
  }

  // ─── Logout ─────────────────────────────────────────────────────────────────

  Future<void> logout() async {
    try {
      await Future.wait([_auth.signOut(), _googleSignIn.signOut()]);
    } catch (e) {
      debugPrint('Logout error: $e');
    }
  }

  AuthError _mapError(String code) {
    switch (code) {
      case 'wrong-password':
      case 'invalid-credential': return AuthError.wrongPassword;
      case 'user-not-found':     return AuthError.userNotFound;
      case 'email-already-in-use': return AuthError.emailInUse;
      case 'weak-password':      return AuthError.weakPassword;
      case 'network-request-failed': return AuthError.network;
      default:                   return AuthError.unknown;
    }
  }
}