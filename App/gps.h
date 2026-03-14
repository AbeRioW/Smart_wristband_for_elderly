#ifndef GPS_H
#define GPS_H

#include "stm32f1xx_hal.h"

// GPS 数据结构
typedef struct {
    float latitude;    // 纬度
    float longitude;   // 经度
    uint8_t hour;      // 时
    uint8_t minute;    // 分
    uint8_t second;    // 秒
    uint8_t fix;       // 定位状态 (0=无效, 1=有效)
    uint8_t satellites; // 卫星数量
    float hdop;        // 水平精度因子
    float altitude;    // 海拔高度
} GPS_Data;

// 函数声明
void GPS_Init(void);
void GPS_ProcessData(uint8_t data);
void GPS_GetData(GPS_Data *data);
void GPS_ShowData(GPS_Data *data);

#endif /* GPS_H */
