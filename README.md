# HealthWatch

HealthWatch là hệ thống giám sát sức khỏe thông minh sử dụng thiết bị đeo kết hợp ESP32, cảm biến sức khỏe, cảm biến chuyển động, GPS, GSM và ứng dụng Flutter.

Hệ thống có khả năng theo dõi nhịp tim (BPM), SpO2 và trạng thái đeo thiết bị theo thời gian thực. Dữ liệu được xử lý trên ESP32 và đồng bộ lên Firebase để ứng dụng Flutter giám sát từ xa.

## Linh kiện
* ESP32-C3 DevKit
* Cảm biến đo nhịp tim và SpO2 MAX30102
* Cảm biến gia tốc và con quay hồi chuyển MPU6500
* Module GPS ATGM336H
* Module GSM A7680C
* Màn hình OLED SSD1306 0.96 inch
* Button
* Buzzer

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

<img width="886" height="394" alt="image" src="https://github.com/user-attachments/assets/1b025f76-d283-4271-b7c5-84d51bd647b6" />


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

## PCB Proteus
<img width="607" height="567" alt="image" src="https://github.com/user-attachments/assets/b6c6a226-ea84-46f5-b8bd-1259653686b7" />
<img width="680" height="445" alt="image" src="https://github.com/user-attachments/assets/9cc75895-2388-41f3-a37e-4a16b2482853" />

## APP Flutter
<img width="380" height="771" alt="image" src="https://github.com/user-attachments/assets/07ca6e42-e20f-476c-80a6-619921e6d5c3" />

## Firebase
<img width="515" height="311" alt="image" src="https://github.com/user-attachments/assets/631d9a7f-1367-4bb8-912a-ec75f25f4275" />


## Mục tiêu

Đề tài hướng đến xây dựng một thiết bị đeo IoT có khả năng giám sát sức khỏe, phát hiện té ngã và cảnh báo khẩn cấp theo thời gian thực, đồng thời cho phép người thân hoặc người quản lý theo dõi trạng thái thiết bị thông qua ứng dụng Flutter.
