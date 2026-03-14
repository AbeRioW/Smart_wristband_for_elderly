#include "gps.h"
#include "usart.h"
#include "oled.h"
#include <string.h>

// 全局变量
static GPS_Data gps_data;
static uint8_t gps_buf[100];
static uint8_t gps_buf_idx = 0;
static uint8_t gps_recv_flag = 0;

// 初始化 GPS 模块
void GPS_Init(void)
{
    // 初始化 GPS 数据结构
    memset(&gps_data, 0, sizeof(GPS_Data));
    memset(gps_buf, 0, sizeof(gps_buf));
    gps_buf_idx = 0;
    gps_recv_flag = 0;
}

// 处理接收到的 GPS 数据
void GPS_ProcessData(uint8_t data)
{
    if (data == '$')
    {
        gps_buf_idx = 0;
    }
    
    if (gps_buf_idx < sizeof(gps_buf) - 1)
    {
        gps_buf[gps_buf_idx++] = data;
    }
    
    if (data == '\n')
    {
        gps_buf[gps_buf_idx] = '\0';
        gps_recv_flag = 1;
    }
}

// 解析 NMEA GGA 语句
static void GPS_ParseGGA(void)
{
    char *token;
    char *ptr = (char *)gps_buf;
    
    // 检查是否是 GGA 语句
    if (strstr(ptr, "GPGGA") == NULL)
        return;
    
    // 解析各个字段
    token = strtok(ptr, ",");
    for (uint8_t i = 0; i < 15; i++)
    {
        if (token == NULL)
            break;
        
        switch (i)
        {
            case 1: // 时间
                if (strlen(token) >= 6)
                {
                    gps_data.hour = (token[0] - '0') * 10 + (token[1] - '0');
                    gps_data.minute = (token[2] - '0') * 10 + (token[3] - '0');
                    gps_data.second = (token[4] - '0') * 10 + (token[5] - '0');
                }
                break;
            case 2: // 纬度
                if (strlen(token) > 0)
                {
                    float deg = atof(token) / 100.0;
                    int int_deg = (int)deg;
                    gps_data.latitude = int_deg + (deg - int_deg) * 100 / 60;
                }
                break;
            case 3: // 纬度方向
                if (token[0] == 'S')
                    gps_data.latitude = -gps_data.latitude;
                break;
            case 4: // 经度
                if (strlen(token) > 0)
                {
                    float deg = atof(token) / 100.0;
                    int int_deg = (int)deg;
                    gps_data.longitude = int_deg + (deg - int_deg) * 100 / 60;
                }
                break;
            case 5: // 经度方向
                if (token[0] == 'W')
                    gps_data.longitude = -gps_data.longitude;
                break;
            case 6: // 定位状态
                gps_data.fix = atoi(token);
                break;
            case 7: // 卫星数量
                gps_data.satellites = atoi(token);
                break;
            case 8: // HDOP
                gps_data.hdop = atof(token);
                break;
            case 9: // 海拔高度
                gps_data.altitude = atof(token);
                break;
        }
        
        // 获取下一个 token
        token = strtok(NULL, ",");
    }
}

// 获取 GPS 数据
void GPS_GetData(GPS_Data *data)
{
    if (gps_recv_flag)
    {
        GPS_ParseGGA();
        gps_recv_flag = 0;
    }
    
    memcpy(data, &gps_data, sizeof(GPS_Data));
}

// 在 OLED 上显示 GPS 数据
void GPS_ShowData(GPS_Data *data)
{
    OLED_Clear();
    
    // 显示时间
    OLED_ShowString(0, 0, (uint8_t*)"Time:", 8, 1);
    if (data->hour < 10) OLED_ShowChar(48, 0, '0', 8, 1);
    OLED_ShowNum(48, 0, data->hour, 2, 8, 1);
    OLED_ShowChar(64, 0, ':', 8, 1);
    if (data->minute < 10) OLED_ShowChar(72, 0, '0', 8, 1);
    OLED_ShowNum(72, 0, data->minute, 2, 8, 1);
    OLED_ShowChar(88, 0, ':', 8, 1);
    if (data->second < 10) OLED_ShowChar(96, 0, '0', 8, 1);
    OLED_ShowNum(96, 0, data->second, 2, 8, 1);
    
    // 显示定位状态
    OLED_ShowString(0, 8, (uint8_t*)"Status:", 8, 1);
    if (data->fix)
        OLED_ShowString(56, 8, (uint8_t*)"Valid", 8, 1);
    else
        OLED_ShowString(56, 8, (uint8_t*)"Invalid", 8, 1);
    
    // 显示卫星数量
    OLED_ShowString(0, 16, (uint8_t*)"Sats:", 8, 1);
    OLED_ShowNum(48, 16, data->satellites, 2, 8, 1);
    
    // 显示纬度
    OLED_ShowString(0, 24, (uint8_t*)"Lat:", 8, 1);
    if (data->latitude >= 0)
    {
        OLED_ShowString(40, 24, (uint8_t*)"N ", 8, 1);
    }
    else
    {
        OLED_ShowString(40, 24, (uint8_t*)"S ", 8, 1);
        data->latitude = -data->latitude;
    }
    int lat_int = (int)data->latitude;
    int lat_dec = (int)((data->latitude - lat_int) * 10000);
    OLED_ShowNum(56, 24, lat_int, 2, 8, 1);
    OLED_ShowChar(72, 24, '.', 8, 1);
    if (lat_dec < 1000) OLED_ShowChar(80, 24, '0', 8, 1);
    if (lat_dec < 100) OLED_ShowChar(88, 24, '0', 8, 1);
    if (lat_dec < 10) OLED_ShowChar(96, 24, '0', 8, 1);
    OLED_ShowNum(80, 24, lat_dec, 4, 8, 1);
    
    // 显示经度
    OLED_ShowString(0, 32, (uint8_t*)"Lon:", 8, 1);
    if (data->longitude >= 0)
    {
        OLED_ShowString(40, 32, (uint8_t*)"E ", 8, 1);
    }
    else
    {
        OLED_ShowString(40, 32, (uint8_t*)"W ", 8, 1);
        data->longitude = -data->longitude;
    }
    int lon_int = (int)data->longitude;
    int lon_dec = (int)((data->longitude - lon_int) * 10000);
    OLED_ShowNum(56, 32, lon_int, 3, 8, 1);
    OLED_ShowChar(80, 32, '.', 8, 1);
    if (lon_dec < 1000) OLED_ShowChar(88, 32, '0', 8, 1);
    if (lon_dec < 100) OLED_ShowChar(96, 32, '0', 8, 1);
    if (lon_dec < 10) OLED_ShowChar(104, 32, '0', 8, 1);
    OLED_ShowNum(88, 32, lon_dec, 4, 8, 1);
    
    // 显示海拔高度
    OLED_ShowString(0, 40, (uint8_t*)"Alt:", 8, 1);
    int alt_int = (int)data->altitude;
    int alt_dec = (int)((data->altitude - alt_int) * 10);
    OLED_ShowNum(40, 40, alt_int, 3, 8, 1);
    OLED_ShowChar(64, 40, '.', 8, 1);
    OLED_ShowNum(72, 40, alt_dec, 1, 8, 1);
    OLED_ShowString(88, 40, (uint8_t*)"m", 8, 1);
    
    OLED_Refresh();
}
