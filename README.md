# HealthWatch

HealthWatch là hệ thống giám sát sức khỏe thông minh sử dụng thiết bị đeo kết hợp ESP32, cảm biến sức khỏe, cảm biến chuyển động, GPS, GSM và ứng dụng Flutter.

Hệ thống có khả năng theo dõi nhịp tim (BPM), SpO2 và trạng thái đeo thiết bị theo thời gian thực. Dữ liệu được xử lý trên ESP32 và đồng bộ lên Firebase để ứng dụng Flutter giám sát từ xa.

## Chức năng chính

* Đo nhịp tim và SpO2 bằng MAX30102.
* Phát hiện chuyển động và té ngã bằng MPU6500.
* Phân loại trạng thái sức khỏe dựa trên nhịp tim.
* Cảnh báo bằng buzzer khi phát hiện té ngã hoặc trạng thái nguy hiểm.
* Gửi SMS và cuộc gọi khẩn cấp thông qua module GSM.
* Thu thập và theo dõi vị trí bằng GPS.
* Đồng bộ dữ liệu sức khỏe lên Firebase.
* Ứng dụng Flutter hỗ trợ đăng nhập và giám sát sức khỏe.
* Hiển thị thông tin trên màn hình OLED.

## Kiến trúc hệ thống

MAX30102 ─┐
          │
MPU6500 ──┤
          │
GPS ──────┤
          ▼
        ESP32
          │
    ┌─────┼─────┐
    ▼     ▼     ▼
  OLED  Firebase GSM
           │      │
           ▼      ▼
        Flutter  SMS/Call

## Công nghệ sử dụng

Phần cứng:

* ESP32
* MAX30102
* MPU6500
* GPS
* GSM
* OLED
* Buzzer

Phần mềm:

* Arduino/C++
* Flutter/Dart
* Firebase
  

## Cơ chế cảnh báo té ngã

Khi phát hiện khả năng té ngã, ESP32 chuyển sang trạng thái xác nhận và kích hoạt buzzer. Hệ thống thực hiện đếm ngược để người dùng có thể hủy cảnh báo bằng nút nhấn.

Nếu không hủy, trạng thái sự cố được đồng bộ lên Firebase, sau đó hệ thống kích hoạt GSM để gửi SMS/cuộc gọi khẩn cấp và sử dụng GPS để hỗ trợ theo dõi vị trí.

## Mục tiêu

Đề tài hướng đến xây dựng một thiết bị đeo IoT có khả năng giám sát sức khỏe, phát hiện té ngã và cảnh báo khẩn cấp theo thời gian thực, đồng thời cho phép người thân hoặc người quản lý theo dõi trạng thái thiết bị thông qua ứng dụng Flutter.
