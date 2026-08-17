// src/drivers/max30102.h
#ifndef MAX30102_H
#define MAX30102_H

#include <Arduino.h>

// Khởi tạo cảm biến MAX30102
void max_init();

// Đọc dữ liệu cảm biến
// ir_value    : giá trị IR thô
// bpm         : nhịp tim
// detect_wear : đang đeo hay không
//
// return:
// true  = đọc thành công
// false = cảm biến lỗi/chưa init
bool max_read(uint32_t &ir,
              uint32_t &red,
              bool &wearing);
// Kiểm tra cảm biến đã sẵn sàng chưa
bool max_is_ready();

void max_sleep();
void max_wakeup();
typedef void (*MaxSampleCallback)(
    uint32_t ir,
    uint32_t red,
    bool wearing
);

void max_process_fifo(MaxSampleCallback cb);

#endif