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
static uint8_t auto_fan_state = 0;    // 风扇自动控制状态
static uint8_t auto_pump_state = 0;   // 水泵自动控制状态
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
    OLED_ShowChinese(0, 0, "火:");
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
         
	// 按键处理
if(key)
{
    switch(key)
    {
        case KEY0_PRES:  // 按键1：同时控制风扇和窗户
            auto_fan_state = 0;  // 清除自动控制状态
            feng = (feng == '0') ? '1' : '0';
            fa = feng;  // 窗户状态跟随风扇状态
            if(feng == '1')
            {
                fengkai();           // 开启风扇
                Servo_SetAngle(180); // 打开窗户
            }
            else
            {
                fengguan();         // 关闭风扇
                Servo_SetAngle(0);  // 关闭窗户
            }
            break;
            
        case KEY1_PRES:  // 按键2：控制水泵
            auto_pump_state = 0;  // 清除自动控制状态
            shui = (shui == '0') ? '1' : '0';
            if(shui == '1')
                shuikai();
            else
                shuiguan();
            break;
						
				case KEY2_PRES:  // 按键3：切换远程控制模式
            flag3 = (flag3 == 4) ? 0 : 4;  // 在普通模式和远程模式之间切换
            break;
            
        case KEY3_PRES:  // 按键4：翻页显示
            if(flag3 != 4)  // 非远程模式下才切换页面
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

        
// 主循环中的显示控制部分
if(flag3 == 4)  // 远程控制模式
{
    OLED_Clear();
    OLED_ShowChinese(0, 0, "远程");
    
    // 显示当前阈值
    OLED_ShowChinese(0, 16, "温度:");
    OLED_ShowNum(40, 16, tem, 2, OLED_8X16);
    OLED_ShowString(64, 16, "C", OLED_8X16);
    
    OLED_ShowChinese(0, 32, "烟雾:");
    OLED_ShowNum(40, 32, yan, 2, OLED_8X16);
    OLED_ShowString(64, 32, "%", OLED_8X16);
    
    OLED_ShowString(0, 48, "CO:", OLED_8X16);
    OLED_ShowNum(32, 48, ran, 2, OLED_8X16);
    OLED_ShowString(56, 48, "%", OLED_8X16);
}
else  // 普通显示模式
{
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
}

// 远程控制处理（放在主循环中）
if(flag3 == 4)
{
    // 温度阈值远程调节
    if(wena == '1')
    {
        tem = tem + 1;
        if(tem > 40) tem = 40;
    }
    if(wenb == '1')
    {
        tem = tem - 1;
        if(tem < 20) tem = 20;
    }
    
    // 烟雾阈值远程调节
    if(yan1 == '1')
    {
        yan = yan + 1;
        if(yan > 80) yan = 80;
    }
    if(yan2 == '1')
    {
        yan = yan - 1;
        if(yan < 20) yan = 20;
    }
    
    // CO阈值远程调节
    if(rana == '1')
    {
        ran = ran + 1;
        if(ran > 80) ran = 80;
    }
    if(ranb == '1')
    {
        ran = ran - 1;
        if(ran < 20) ran = 20;
    }
    
    // 远程设备控制（保持原有功能）
    if(feng == '1')
        fengkai();
    else if(feng == '0')
        fengguan();
        
    if(shui == '1')
        shuikai();
    else if(shui == '0')
        shuiguan();
        
    if(bao == '1')
        BEEP = 1;
    else if(bao == '0')
        BEEP = 0;
        
    if(fa == '1' && flag6 == 0)
    {
        Servo_SetAngle(180);
        flag6 = 1;
    }
    else if(fa == '0' && flag6 == 1)
    {
        Servo_SetAngle(0);
        flag6 = 0;
    }
}
       
/// 报警检测和自动控制
// 火灾检测控制水泵
if(fire) 
{
    BEEP = 1;           // 开启蜂鸣器
    if(!auto_pump_state)  // 如果不是已经自动开启
    {
        shuikai();        // 开启水泵
        shui = '1';       // 更新水泵状态
        auto_pump_state = 1; // 标记为自动开启
    }
}
else  // 火灾解除
{
    if(auto_pump_state)  // 如果是自动开启的水泵
    {
        shuiguan();       // 关闭水泵
        shui = '0';       // 更新水泵状态
        auto_pump_state = 0; // 清除自动标记
    }
}

// 烟雾、CO浓度或温度检测控制风扇和窗户
if(MQ2_Value > yan || MQ7_Value > ran || DHT11_Data.temp_int > tem)
{
    BEEP = 1;             // 开启蜂鸣器
    if(!auto_fan_state)   // 如果不是已经自动开启
    {
        fengkai();           // 开启风扇
        feng = '1';          // 更新风扇状态
        Servo_SetAngle(180); // 打开窗户
        fa = '1';            // 更新窗户状态
        auto_fan_state = 1;  // 标记为自动开启
    }
}
else  // 参数恢复正常
{
    if(auto_fan_state)    // 如果是自动开启的风扇和窗户
    {
        fengguan();          // 关闭风扇
        feng = '0';          // 更新风扇状态
        Servo_SetAngle(0);   // 关闭窗户
        fa = '0';            // 更新窗户状态
        auto_fan_state = 0;  // 清除自动标记
        BEEP = 0;            // 关闭蜂鸣器
    }
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
