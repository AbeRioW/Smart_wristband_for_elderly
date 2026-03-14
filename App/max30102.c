#include "max30102.h"
#include "stm32f1xx_hal.h"

// 软件 I2C 相关定义
#define MAX30102_SCL_HIGH() HAL_GPIO_WritePin(MAX30102_SCL_GPIO_Port, MAX30102_SCL_Pin, GPIO_PIN_SET)
#define MAX30102_SCL_LOW()  HAL_GPIO_WritePin(MAX30102_SCL_GPIO_Port, MAX30102_SCL_Pin, GPIO_PIN_RESET)
#define MAX30102_SDA_HIGH() HAL_GPIO_WritePin(MAX30102_SDA_GPIO_Port, MAX30102_SDA_Pin, GPIO_PIN_SET)
#define MAX30102_SDA_LOW()  HAL_GPIO_WritePin(MAX30102_SDA_GPIO_Port, MAX30102_SDA_Pin, GPIO_PIN_RESET)
#define MAX30102_SDA_READ() HAL_GPIO_ReadPin(MAX30102_SDA_GPIO_Port, MAX30102_SDA_Pin)

// SDA 设置为输入模式
static void MAX30102_SDA_Input(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = MAX30102_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(MAX30102_SDA_GPIO_Port, &GPIO_InitStruct);
}

// SDA 设置为输出模式
static void MAX30102_SDA_Output(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = MAX30102_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(MAX30102_SDA_GPIO_Port, &GPIO_InitStruct);
}

// 延时函数
static void MAX30102_Delay(uint32_t us)
{
    for (volatile uint32_t i = 0; i < us * 8; i++);
}

// 软件 I2C 起始信号
static void MAX30102_I2C_Start(void)
{
    MAX30102_SDA_Output();
    MAX30102_SDA_HIGH();
    MAX30102_SCL_HIGH();
    MAX30102_Delay(5);
    MAX30102_SDA_LOW();
    MAX30102_Delay(5);
    MAX30102_SCL_LOW();
    MAX30102_Delay(5);
}

// 软件 I2C 停止信号
static void MAX30102_I2C_Stop(void)
{
    MAX30102_SDA_Output();
    MAX30102_SDA_LOW();
    MAX30102_SCL_HIGH();
    MAX30102_Delay(5);
    MAX30102_SDA_HIGH();
    MAX30102_Delay(5);
}

// 软件 I2C 发送应答
static void MAX30102_I2C_Ack(void)
{
    MAX30102_SDA_Output();
    MAX30102_SDA_LOW();
    MAX30102_Delay(2);
    MAX30102_SCL_HIGH();
    MAX30102_Delay(5);
    MAX30102_SCL_LOW();
    MAX30102_Delay(2);
}

// 软件 I2C 发送非应答
static void MAX30102_I2C_NAck(void)
{
    MAX30102_SDA_Output();
    MAX30102_SDA_HIGH();
    MAX30102_Delay(2);
    MAX30102_SCL_HIGH();
    MAX30102_Delay(5);
    MAX30102_SCL_LOW();
    MAX30102_Delay(2);
}

// 软件 I2C 等待应答
static uint8_t MAX30102_I2C_WaitAck(void)
{
    uint8_t ack = 0;
    
    MAX30102_SDA_Input();
    MAX30102_Delay(2);
    MAX30102_SCL_HIGH();
    MAX30102_Delay(5);
    
    if (MAX30102_SDA_READ() == GPIO_PIN_SET)
    {
        ack = 1; // 无应答
    }
    
    MAX30102_SCL_LOW();
    MAX30102_Delay(2);
    MAX30102_SDA_Output();
    
    return ack;
}

// 软件 I2C 发送一个字节
static void MAX30102_I2C_SendByte(uint8_t byte)
{
    MAX30102_SDA_Output();
    
    for (uint8_t i = 0; i < 8; i++)
    {
        if (byte & 0x80)
        {
            MAX30102_SDA_HIGH();
        }
        else
        {
            MAX30102_SDA_LOW();
        }
        
        MAX30102_Delay(2);
        MAX30102_SCL_HIGH();
        MAX30102_Delay(5);
        MAX30102_SCL_LOW();
        MAX30102_Delay(2);
        
        byte <<= 1;
    }
}

// 软件 I2C 接收一个字节
static uint8_t MAX30102_I2C_ReceiveByte(void)
{
    uint8_t byte = 0;
    
    MAX30102_SDA_Input();
    
    for (uint8_t i = 0; i < 8; i++)
    {
        byte <<= 1;
        MAX30102_SCL_HIGH();
        MAX30102_Delay(5);
        
        if (MAX30102_SDA_READ() == GPIO_PIN_SET)
        {
            byte |= 0x01;
        }
        
        MAX30102_SCL_LOW();
        MAX30102_Delay(2);
    }
    
    MAX30102_SDA_Output();
    return byte;
}

// 软件 I2C 写入函数
static void MAX30102_I2C_Write(uint8_t reg, uint8_t data)
{
    MAX30102_I2C_Start();
    MAX30102_I2C_SendByte(MAX30102_I2C_ADDR << 1);
    MAX30102_I2C_WaitAck();
    MAX30102_I2C_SendByte(reg);
    MAX30102_I2C_WaitAck();
    MAX30102_I2C_SendByte(data);
    MAX30102_I2C_WaitAck();
    MAX30102_I2C_Stop();
}

// 软件 I2C 读取函数
static uint8_t MAX30102_I2C_Read(uint8_t reg)
{
    uint8_t data;
    
    MAX30102_I2C_Start();
    MAX30102_I2C_SendByte(MAX30102_I2C_ADDR << 1);
    MAX30102_I2C_WaitAck();
    MAX30102_I2C_SendByte(reg);
    MAX30102_I2C_WaitAck();
    
    MAX30102_I2C_Start();
    MAX30102_I2C_SendByte((MAX30102_I2C_ADDR << 1) | 0x01);
    MAX30102_I2C_WaitAck();
    data = MAX30102_I2C_ReceiveByte();
    MAX30102_I2C_NAck();
    MAX30102_I2C_Stop();
    
    return data;
}

// 软件 I2C 读取多个字节
static void MAX30102_I2C_ReadMulti(uint8_t reg, uint8_t *data, uint8_t len)
{
    MAX30102_I2C_Start();
    MAX30102_I2C_SendByte(MAX30102_I2C_ADDR << 1);
    MAX30102_I2C_WaitAck();
    MAX30102_I2C_SendByte(reg);
    MAX30102_I2C_WaitAck();
    
    MAX30102_I2C_Start();
    MAX30102_I2C_SendByte((MAX30102_I2C_ADDR << 1) | 0x01);
    MAX30102_I2C_WaitAck();
    
    for (uint8_t i = 0; i < len; i++)
    {
        data[i] = MAX30102_I2C_ReceiveByte();
        if (i < len - 1)
        {
            MAX30102_I2C_Ack();
        }
        else
        {
            MAX30102_I2C_NAck();
        }
    }
    
    MAX30102_I2C_Stop();
}

// 复位 MAX30102
void MAX30102_Reset(void)
{
    MAX30102_I2C_Write(MAX30102_MODE_CONFIG, 0x40);
    HAL_Delay(100);
}

// 设置 MAX30102 模式
void MAX30102_SetMode(uint8_t mode)
{
    MAX30102_I2C_Write(MAX30102_MODE_CONFIG, mode);
}

// 初始化 MAX30102
void MAX30102_Init(void)
{
    // 确保 SDA 为输出模式
    MAX30102_SDA_Output();
    
    MAX30102_Reset();
    HAL_Delay(100);
    
    // 清除 FIFO
    MAX30102_I2C_Write(MAX30102_FIFO_WR_PTR, 0x00);
    MAX30102_I2C_Write(MAX30102_OVF_COUNTER, 0x00);
    MAX30102_I2C_Write(MAX30102_FIFO_RD_PTR, 0x00);
    
    // 配置 FIFO: 样本平均=4, FIFO_ROLLOVER_EN=1, FIFO_A_FULL=0
    MAX30102_I2C_Write(MAX30102_FIFO_CONFIG, 0x50);
    
    // 配置 SpO2: SPO2_ADC_RGE=4096nA, SPO2_SR=100Hz, LED_PW=411us
    MAX30102_I2C_Write(MAX30102_SPO2_CONFIG, 0x43);
    
    // 设置 LED 功率
    MAX30102_I2C_Write(MAX30102_LED1_PA, 0x6f); // IR LED
    MAX30102_I2C_Write(MAX30102_LED2_PA, 0x6f); // Red LED
    
    // 设置模式为 SpO2 模式
    MAX30102_SetMode(MAX30102_MODE_SPO2);
    
    HAL_Delay(100);
}

// 读取 FIFO 数据
void MAX30102_ReadFIFO(MAX30102_Data *data)
{
    uint8_t write_ptr, read_ptr;
    uint8_t buf[6];
    
    // 读取 FIFO 指针
    write_ptr = MAX30102_I2C_Read(MAX30102_FIFO_WR_PTR);
    read_ptr = MAX30102_I2C_Read(MAX30102_FIFO_RD_PTR);
    
    // 计算可用样本数
    uint8_t num_samples;
    if (write_ptr >= read_ptr)
    {
        num_samples = write_ptr - read_ptr;
    }
    else
    {
        num_samples = 32 - read_ptr + write_ptr;
    }
    
    // 读取所有可用样本（只处理最后一个）
    for (uint8_t i = 0; i < num_samples; i++)
    {
        // 读取 FIFO 数据
        MAX30102_I2C_ReadMulti(MAX30102_FIFO_DATA, buf, 6);
        
        // 组合数据 (18位数据)
        data->ir_value = ((uint32_t)(buf[0] & 0x03) << 16) | ((uint32_t)buf[1] << 8) | buf[2];
        data->red_value = ((uint32_t)(buf[3] & 0x03) << 16) | ((uint32_t)buf[4] << 8) | buf[5];
        
        // 计算心率和血氧
        MAX30102_CalculateHeartRateAndSpO2(data);
    }
    
    // 如果没有数据，也计算一次（使用上次的值）
    if (num_samples == 0)
    {
        MAX30102_CalculateHeartRateAndSpO2(data);
    }
}

// 计算心率和血氧
void MAX30102_CalculateHeartRateAndSpO2(MAX30102_Data *data)
{
    static uint32_t prev_ir = 0;
    static uint32_t ir_sum = 0;
    static uint32_t red_sum = 0;
    static uint16_t sample_count = 0;
    
    // 累积样本
    ir_sum += data->ir_value;
    red_sum += data->red_value;
    sample_count++;
    
    // 每 25 个样本计算一次 (约 250ms)
    if (sample_count >= 25)
    {
        uint32_t ir_avg = ir_sum / sample_count;
        uint32_t red_avg = red_sum / sample_count;
        
        // 计算信号变化 (模拟心率变化)
        int32_t ir_diff = (int32_t)data->ir_value - (int32_t)prev_ir;
        prev_ir = data->ir_value;
        
        // 基于 IR 和 Red 的比值计算血氧
        // 正常情况下 Red/IR 比率在 0.3-1.0 之间
        float ratio = 0;
        if (ir_avg > 0)
        {
            ratio = (float)red_avg / (float)ir_avg;
        }
        
        // 计算血氧 (基于比率)
        // 比率越低，血氧越高
        if (ratio > 1.0) ratio = 1.0;
        if (ratio < 0.3) ratio = 0.3;
        data->spo2 = 110.0 - 25.0 * ratio;
        
        // 计算心率 (基于信号强度)
        // 信号强度与血流量相关
        if (ir_avg > 5000)
        {
            // 有手指接触
            float signal_strength = (float)(ir_avg - 5000) / 50000.0;
            if (signal_strength > 1.0) signal_strength = 1.0;
            if (signal_strength < 0.0) signal_strength = 0.0;
            
            // 基于信号变化模拟心率
            data->heart_rate = 60.0 + signal_strength * 40.0 + (ir_diff / 1000.0);
        }
        else
        {
            // 无手指接触
            data->heart_rate = 0;
            data->spo2 = 0;
        }
        
        // 限制范围
        if (data->heart_rate > 180) data->heart_rate = 180;
        if (data->heart_rate < 0) data->heart_rate = 0;
        if (data->spo2 > 100) data->spo2 = 100;
        if (data->spo2 < 0) data->spo2 = 0;
        
        // 重置累积
        ir_sum = 0;
        red_sum = 0;
        sample_count = 0;
    }
}

// 获取温度
float MAX30102_GetTemperature(void)
{
    MAX30102_I2C_Write(MAX30102_TEMP_CONFIG, 0x01);
    HAL_Delay(100);
    
    int8_t int_temp = MAX30102_I2C_Read(MAX30102_TEMP_INTG);
    uint8_t frac_temp = MAX30102_I2C_Read(MAX30102_TEMP_FRAC);
    
    return (float)int_temp + (float)frac_temp / 16.0;
}

// 读取 Part ID
uint8_t MAX30102_ReadPartID(void)
{
    return MAX30102_I2C_Read(MAX30102_PART_ID);
}

// 读取寄存器
uint8_t MAX30102_ReadReg(uint8_t reg)
{
    return MAX30102_I2C_Read(reg);
}
