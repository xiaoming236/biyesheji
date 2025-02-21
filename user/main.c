#include "stm32f10x.h"                  // Device header
#include "delay.h"
#include "OLED.h"
#include "OELD_Data.h"
#include "FMQ.h"
#include "MQ2.h"
#include "dht11.h"
#include "key.h"
#include "ad.h"
#include "PWM.h"
#include "beep.h"
#include "MyUSART.H"
#include "esp.h"
#include "Servo.h"
#include "MQ7.h"
#include "stepmotor.h"
#include <stdio.h>
#include <string.h>

// 变量定义
u16 MQ7_Value;//adc值
uint16_t fire,y,r,w,h,Y,W,R;
float temp_one,shi1;//电压值
float MQ2_Value;              
DHT11_Data_TypeDef  DHT11_Data;
// 控制标志位

//uint16_t key = 0, flag1 = 0, flag2 = 0, flag3 = 0,flag5 = 0,flag6 = 0,AD=0,wen1,A=0,tem=28,yan=70,ran=60,Judge,cnt;
// 各种标志位和控制变量
uint16_t key = 0;     // 按键值存储变量
uint16_t flag1 = 0;   // 模式切换标志位
uint16_t flag2 = 0;   // 通用标志位
uint16_t flag3 = 0;   // 设置模式标志位
uint16_t flag5 = 0;   // 燃气检测标志位
uint16_t flag6 = 0;   // 舵机控制标志位
uint16_t AD = 0;      // AD转换值存储
uint16_t wen1;        // 温度值存储
uint16_t A = 0;       // 通用计数器
uint16_t tem = 28;    // 温度阈值（28度）
uint16_t yan = 70;    // 烟雾阈值（70%）
uint16_t ran = 60;    // 燃气阈值（60%）
uint16_t Judge;       // WiFi连接判断标志
uint16_t cnt;         // 计数器
// DHT11温湿度传感器相关变量
uint8_t temp_int;     // 温度整数部分
uint8_t temp_deci;    // 温度小数部分
uint8_t humi_int;     // 湿度整数部分
uint8_t humi_deci;    // 湿度小数部分
// 远程控制标志位（'0'表示关闭，'1'表示开启）
uint8_t yan1='0';// 烟雾阈值增加控制位
uint8_t yan2='0';// 烟雾阈值减少控制位
uint8_t feng='0';// 风扇控制位
uint8_t wena='0';// 温度阈值增加控制位
uint8_t wenb='0';// 温度阈值减少控制位
uint8_t shui='0';// 水泵控制位
uint8_t bao='0'; // 报警器控制位
uint8_t fa='0';// 窗户（舵机）控制位
uint8_t rana='0';// 燃气co阈值增加控制位
uint8_t ranb='0';  // 燃气co阈值减少控制位
uint8_t display_page = 0;

#define WZ DHT11_Data.temp_int	
#define WX DHT11_Data.temp_deci
#define SZ DHT11_Data.humi_int	
#define SX DHT11_Data.humi_deci

// 显示第一页
void Display_Page1(void)
{
    OLED_Clear();
    // 温度显示
    OLED_ShowChinese(0, 0, "温度:");
    OLED_ShowNum(40, 0, DHT11_Data.temp_int, 2, OLED_8X16);
    OLED_ShowString(56, 0, ".", OLED_8X16);
    OLED_ShowNum(64, 0, DHT11_Data.temp_deci, 1, OLED_8X16);
    OLED_ShowString(72, 0, "C", OLED_8X16);
    
    // 湿度显示
    OLED_ShowChinese(0, 16, "湿度:");
    OLED_ShowNum(40, 16, DHT11_Data.humi_int, 2, OLED_8X16);
    OLED_ShowString(56, 16, ".", OLED_8X16);
    OLED_ShowNum(64, 16, DHT11_Data.humi_deci, 1, OLED_8X16);
    OLED_ShowString(72, 16, "%", OLED_8X16);

    // 页面指示
    OLED_ShowString(95, 0, "P:1/3", OLED_8X16);
}

// 显示第二页
void Display_Page2(void)
{
    OLED_Clear();
    // 烟雾值显示
    OLED_ShowChinese(0, 0, "烟雾:");
    OLED_ShowNum(40, 0, MQ2_Value, 3, OLED_8X16);
    
    // CO值显示
    OLED_ShowString(0, 16, "CO:", OLED_8X16);
    OLED_ShowNum(32, 16, MQ7_Value, 3, OLED_8X16);

    // 页面指示
    OLED_ShowString(95, 0, "P:2/3", OLED_8X16);
}

// 显示第三页
void Display_Page3(void)
{
    OLED_Clear();
    // 火焰状态
    OLED_ShowChinese(0, 0, "火焰:");
    OLED_ShowString(40, 0, fire ? "WARNING" : "NORMAL", OLED_8X16);
    
    // 控制状态
    OLED_ShowString(0, 16, feng=='1' ? "Fan:ON" : "Fan:OFF", OLED_8X16);
    OLED_ShowString(64, 16, shui=='1' ? "Pump:ON" : "Pump:OFF", OLED_8X16);
    
    // 页面指示
    OLED_ShowString(95, 0, "P:3/3", OLED_8X16);
}
int main(void)
{   
	    // 初始化外设
    MyUSART_Init();
    AD_Init();
    Adc_Init();
    OLED_Init();
    mfq_Init();
    BEEP_Init();
    KEY_Init();
    DHT11_GPIO_Config();
    Adc_Init1();
    MOTOR_Init();
    Servo_Init();

    // 尝试连接WiFi，但不阻塞程序
    Judge = esp_Init();

    while(1)
    {    
        // WiFi连接成功才执行数据上报
        if(!Judge)
        {
            cnt++;
            if(cnt==6) //约每6s执行一次数据上报
            {
                if(Esp_PUB() == 1)
                {
                    Delay_ms(200);
                }
                cnt=0;                     
            }
        }

        // 获取传感器数据
        fire = IR_FireData();
        MQ2_Value=Get_Adc_Average1(ADC_Channel_3,10)*100/4095;//模拟烟雾浓度的值
        MQ7_Value=Get_Adc_Average1(ADC_Channel_1,10)*100/4095;//模拟co浓度的值
        key = KEY_Scan(0);
        Read_DHT11(&DHT11_Data);

        // 按键切换模式
         if(key)
        {
            switch(key)
            {
                case KEY0_PRES:  // 按键1：风扇控制
                    feng = (feng == '0') ? '1' : '0';
                    if(feng == '1')
                        fengkai();
                    else
                        fengguan();
                    break;
                    
                case KEY1_PRES:  // 按键2：水泵控制
                    shui = (shui == '0') ? '1' : '0';
                    if(shui == '1')
                        shuikai();
                    else
                        shuiguan();
                    break;
                    
                case KEY2_PRES:  // 按键3：窗户控制
                    fa = (fa == '0') ? '1' : '0';
                    if(fa == '1')
                        Servo_SetAngle(180);
                    else
                        Servo_SetAngle(0);
                    break;
                    
                case KEY3_PRES:  // 按键4：翻页
                    display_page = (display_page + 1) % 3;
                    break;
            }
        }
        
        // 根据页面显示不同内容
        switch(display_page)
        {
            case 0:
                Display_Page1();
                break;
            case 1:
                Display_Page2();
                break;
            case 2:
                Display_Page3();
                break;
        }
        
        // 报警检测和自动控制
        if(fire || MQ2_Value > yan || MQ7_Value > ran || DHT11_Data.temp_int > tem)
        {
            BEEP = 1;
            if(fire || MQ2_Value > yan)
            {
                shuikai();
                shui = '1';
            }
            if(MQ7_Value > ran || DHT11_Data.temp_int > tem)
            {
                fengkai();
                Servo_SetAngle(180);
                feng = '1';
                fa = '1';
            }
        }
        else
        {
            BEEP = 0;
        }
        
        // WiFi数据上报
        if(!Judge)
        {
            cnt++;
            if(cnt == 6)
            {
                if(Esp_PUB() == 1)
                {
                    Delay_ms(200);
                }
                cnt = 0;
            }
        }
        
        OLED_Update();
        Delay_ms(100);
    }
}
