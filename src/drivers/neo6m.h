// src/drivers/neo6m.h
#ifndef NEO6M_H
#define NEO6M_H

#include <Arduino.h>

// Khởi tạo GPS Neo-6M
void neo_init();

// Đọc dữ liệu GPS
//
// lat       : vĩ độ
// lon       : kinh độ
// datetime  : ngày giờ GPS
// hasFix    : có tín hiệu GPS hay không
//
// return:
// true  = đọc thành công
// false = timeout / lỗi

void neo_update();

bool neo_read(
    double &lat,
    double &lon,
    String &datetime,
    bool &hasFix
);

#endif