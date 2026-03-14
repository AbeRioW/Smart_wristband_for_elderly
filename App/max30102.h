#ifndef __MAX30102_H
#define __MAX30102_H

#include "gpio.h"

// MAX30102 寄存器地址
#define MAX30102_INT_STATUS        0x00
#define MAX30102_INT_ENABLE        0x01
#define MAX30102_FIFO_WR_PTR       0x02
#define MAX30102_OVF_COUNTER       0x03
#define MAX30102_FIFO_RD_PTR       0x04
#define MAX30102_FIFO_DATA         0x05
#define MAX30102_FIFO_CONFIG       0x08
#define MAX30102_MODE_CONFIG       0x09
#define MAX30102_SPO2_CONFIG       0x0A
#define MAX30102_LED1_PA           0x0C
#define MAX30102_LED2_PA           0x0D
#define MAX30102_LED3_PA           0x0E
#define MAX30102_LED_PROX_AMP      0x10
#define MAX30102_MULTI_LED_CTRL1   0x11
#define MAX30102_MULTI_LED_CTRL2   0x12
#define MAX30102_TEMP_INTG         0x1F
#define MAX30102_TEMP_FRAC         0x20
#define MAX30102_TEMP_CONFIG       0x21
#define MAX30102_PROX_INT_THRESH   0x30
#define MAX30102_REV_ID            0xFE
#define MAX30102_PART_ID           0xFF

// I2C 地址
#define MAX30102_I2C_ADDR          0x57

// 模式定义
#define MAX30102_MODE_HR           0x02    // 心率模式
#define MAX30102_MODE_SPO2         0x03    // 血氧模式
#define MAX30102_MODE_MULTI_LED    0x07    // 多LED模式

// 结构体定义
typedef struct {
    uint32_t ir_value;
    uint32_t red_value;
    float heart_rate;
    float spo2;
} MAX30102_Data;

// 函数声明
void MAX30102_Init(void);
void MAX30102_Reset(void);
void MAX30102_SetMode(uint8_t mode);
void MAX30102_ReadFIFO(MAX30102_Data *data);
float MAX30102_GetTemperature(void);
void MAX30102_CalculateHeartRateAndSpO2(MAX30102_Data *data);
uint8_t MAX30102_ReadPartID(void);
uint8_t MAX30102_ReadReg(uint8_t reg);

#endif