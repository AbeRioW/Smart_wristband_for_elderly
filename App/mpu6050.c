#include "mpu6050.h"
#include "stm32f1xx_hal.h"
#include "i2c.h"

// 动态 I2C 地址
static uint8_t mpu6050_i2c_addr = MPU6050_I2C_ADDR;

// 写入寄存器
void MPU6050_WriteReg(uint8_t reg, uint8_t data)
{
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = data;
    HAL_I2C_Master_Transmit(&hi2c1, (mpu6050_i2c_addr << 1), buf, 2, 100);
}

// 读取寄存器
uint8_t MPU6050_ReadReg(uint8_t reg)
{
    uint8_t data;
    HAL_I2C_Master_Transmit(&hi2c1, (mpu6050_i2c_addr << 1), &reg, 1, 100);
    HAL_I2C_Master_Receive(&hi2c1, (mpu6050_i2c_addr << 1) | 0x01, &data, 1, 100);
    return data;
}

// 读取多个字节
void MPU6050_ReadMulti(uint8_t reg, uint8_t *data, uint8_t len)
{
    HAL_I2C_Master_Transmit(&hi2c1, (mpu6050_i2c_addr << 1), &reg, 1, 100);
    HAL_I2C_Master_Receive(&hi2c1, (mpu6050_i2c_addr << 1) | 0x01, data, len, 100);
}

// 初始化 MPU6050
void MPU6050_Init(void)
{
    HAL_Delay(100);
    
    // 唤醒 MPU6050
    MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x00);
    HAL_Delay(100);
    
    // 配置采样率
    MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x07);
    
    // 配置低通滤波器
    MPU6050_WriteReg(MPU6050_CONFIG, 0x06);
    
    // 配置陀螺仪: ±2000dps
    MPU6050_WriteReg(MPU6050_GYRO_CONFIG, 0x18);
    
    // 配置加速度计: ±2g
    MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, 0x01);
    
    HAL_Delay(100);
}

// 读取 MPU6050 数据
void MPU6050_ReadData(MPU6050_Data *data)
{
    uint8_t buf[14];
    
    MPU6050_ReadMulti(MPU6050_ACCEL_XOUT_H, buf, 14);
    
    // 读取加速度计数据
    data->accel_x = (int16_t)((buf[0] << 8) | buf[1]);
    data->accel_y = (int16_t)((buf[2] << 8) | buf[3]);
    data->accel_z = (int16_t)((buf[4] << 8) | buf[5]);
    
    // 读取温度数据
    int16_t temp_raw = (int16_t)((buf[6] << 8) | buf[7]);
    data->temperature = (float)temp_raw / 340.0 + 36.53;
    
    // 读取陀螺仪数据
    data->gyro_x = (int16_t)((buf[8] << 8) | buf[9]);
    data->gyro_y = (int16_t)((buf[10] << 8) | buf[11]);
    data->gyro_z = (int16_t)((buf[12] << 8) | buf[13]);
}

// 读取 MPU6050 ID
uint8_t MPU6050_ReadID(void)
{
    return MPU6050_ReadReg(MPU6050_WHO_AM_I);
}

// 扫描 I2C 总线
uint8_t MPU6050_ScanI2C(void)
{
    uint8_t addr;
    
    for (addr = 0x00; addr < 0x80; addr++)
    {
        if (HAL_I2C_IsDeviceReady(&hi2c1, (addr << 1), 3, 100) == HAL_OK)
        {
            return addr;
        }
    }
    
    return 0;
}

// 设置 I2C 地址
void MPU6050_SetI2CAddr(uint8_t addr)
{
    mpu6050_i2c_addr = addr;
}

// 调试读取函数
void MPU6050_DebugRead(void)
{
    uint8_t regs[8];
    regs[0] = MPU6050_ReadReg(0x6B);
    regs[1] = MPU6050_ReadReg(0x6C);
    regs[2] = MPU6050_ReadReg(0x75);
    regs[3] = MPU6050_ReadReg(0x3B);
    regs[4] = MPU6050_ReadReg(0x3C);
    regs[5] = MPU6050_ReadReg(0x41);
    regs[6] = MPU6050_ReadReg(0x42);
    regs[7] = MPU6050_ReadReg(0x43);
}
