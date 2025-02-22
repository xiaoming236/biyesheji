#include "stm32f10x.h"                  // Device header
#include "MyUSART.h"
#include <stdio.h>
#include <string.h>
#include "delay.h"
#include "OLED.H"
#include "esp.h"

#define WZ DHT11_Data.temp_int	
#define WX DHT11_Data.temp_deci
#define SZ DHT11_Data.humi_int	
#define SX DHT11_Data.humi_deci
extern int wen1;
extern float shi1;

extern float MQ2_Value;
extern int MQ7_Value;
extern uint8_t yan1;
extern uint8_t yan2;
extern uint8_t feng;
extern uint8_t wena;
extern uint8_t wenb;
extern uint8_t shui;
extern uint8_t bao;
extern uint8_t fa;
extern uint8_t rana;
extern uint8_t ranb;
extern char RECS[250];
// 替换原来的阿里云配置为OneNET配置
// OneNET平台参数
// OneNET平台连接参数
const char* WIFI = "1234";  // 您的WiFi名称
const char* WIFIASSWORD = "123456789";  // WiFi密码
const char* ONENET_PRODUCT_ID = "Ub99ifb8x7";  // 产品ID
const char* ONENET_DEVICE_ID = "device1";  // 设备名称
const char* ONENET_DEVICE_KEY = "SmFONzEyRDhZNnBRd3lzUkc4ZG5hUW1nejVOOGZlRjE=";  // 设备密钥
const char* ONENET_SERVER = "Ub99ifb8x7.mqtts.acc.cmcconenet.cn";  // 接入域名
const char* ONENET_PORT = "1883";  // MQTT标准端口

// 各功能点标识符
const char* func1="temperature";  // 温度
const char* func2="humidity";     // 湿度
const char* func3="smoke";        // 烟雾浓度
const char* func4="fan";          // 风扇
const char* func5="flame";        // 火焰状态
const char* func6="flame_alarm";  // 火焰报警
const char* func7="co";           // CO浓度
const char* func8="co_alarm";     // CO报警
const char* func9="pump";         // 水泵
const char* func10="alarm";       // 报警器
const char* func11="window";      // 窗户
const char* func12="co_value";    // CO数值
const char* func14="flame_threshold1"; // 火焰阈值1
const char* func15="flame_threshold2"; // 火焰阈值2

int fputc(int ch,FILE *f )   //printf重定向  
{
	USART_SendData(USART1,(uint8_t)ch);
	while(USART_GetFlagStatus (USART1,USART_FLAG_TC) == RESET);
	return ch;
}
char esp_Init(void)
{
    memset(RECS,0,sizeof(RECS));
    printf("AT+RST\r\n");  // 重启模块
    Delay_ms(2000);
    
    memset(RECS,0,sizeof(RECS));
    printf("ATE0\r\n");    // 关闭回显
    Delay_ms(10);
    if(strcmp(RECS,"OK"))
        return 1;
    
    printf("AT+CWMODE=1\r\n"); // Station模式
    Delay_ms(1000);
    if(strcmp(RECS,"OK"))
        return 2;
    
    // 先尝试SmartConfig配网
    printf("AT+CWSTARTSMART=3\r\n"); // ESP-TOUCH + AirKiss双模式
    Delay_ms(1000);
    
    uint8_t timeout = 0;
    while(strstr(RECS, "smartconfig connected wifi") == NULL)
    {
        Delay_ms(1000);
        timeout++;
        if(timeout > 30) // 30秒超时后切换到AP配网
        {
            printf("AT+CWSTOPSMART\r\n");
            Delay_ms(1000);
            
            // 切换到AP+STA模式
            printf("AT+CWMODE=3\r\n");
            Delay_ms(1000);
            
            // 配置AP，SSID为"Safety_" + 产品ID后6位
            printf("AT+CWSAP=\"Safety_%s\",\"12345678\",11,3\r\n", 
                   ONENET_PRODUCT_ID + strlen(ONENET_PRODUCT_ID) - 6);
            Delay_ms(1000);
            break;
        }
    }
    
    // MQTT用户配置
    memset(RECS,0,sizeof(RECS));
    printf("AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n",
           ONENET_DEVICE_ID,
           ONENET_PRODUCT_ID,
           ONENET_DEVICE_KEY);
    Delay_ms(100);
    
    // 连接MQTT服务器
    memset(RECS,0,sizeof(RECS));
    printf("AT+MQTTCONN=0,\"%s\",%s,1\r\n", 
           ONENET_SERVER, 
           ONENET_PORT);
    Delay_ms(1000);
    
    // 建议添加订阅主题，接收下发命令
    printf("AT+MQTTSUB=0,\"$sys/%s/%s/thing/property/set\",1\r\n",
           ONENET_PRODUCT_ID,
           ONENET_DEVICE_ID);
    Delay_ms(100);
    
    return 0;
}
//数据上报实现功能：esp发送消息
//参数：无
//返回值：0：发送成功；1：发送失败
char Esp_PUB(void)
{
    char dataBuf[512];
    memset(RECS,0,sizeof(RECS));
    
    // 构建JSON数据，使用func变量作为键名
    sprintf(dataBuf, 
        "{\"id\":\"123\","
        "\"params\":{"
            "\"%s\":%d,"        // func1 (temperature)
            "\"%s\":%d,"        // func2 (Humidity)
            "\"%s\":%.2f,"      // func3 (yan)
            "\"%s\":%d,"        // func12 (ran)
            "\"%s\":\"%c\","    // func4 (feng)
            "\"%s\":\"%c\","    // func9 (shui)
            "\"%s\":\"%c\","    // func10 (bao)
            "\"%s\":\"%c\","    // func11 (fa)
            "\"%s\":\"%c\","    // func14 (ran1)
            "\"%s\":\"%c\","    // func15 (ran2)
            "\"%s\":\"%c\""     // func5 (yan1)
        "}}",
        func1, wen1,                // 温度
        func2, (int)shi1,          // 湿度
        func3, MQ2_Value,          // 烟雾
        func12, MQ7_Value,         // 燃气/CO值
        func4, feng,               // 风扇
        func9, shui,               // 水泵
        func10, bao,               // 报警
        func11, fa,                // 窗户
        func14, rana,              // 火焰阈值1
        func15, ranb,              // 火焰阈值2
        func5, yan1                // 火焰状态
    );
    
    // 发布到OneNET主题
    printf("AT+MQTTPUB=0,\"$sys/%s/%s/thing/property/post\",%s,1,0\r\n",
           ONENET_PRODUCT_ID,
           ONENET_DEVICE_ID,
           dataBuf);
    
    Delay_ms(200);
    if(strcmp(RECS,"ERROR")==0)
        return 1;
    return 0;
}
void CommandAnalyse(void)
{
    if(strncmp(RECS,"+MQTTSUBRECV:",13)==0)
    {
        char *json_start = strstr(RECS, "{");
        if(json_start != NULL)
        {
            // 解析各个控制命令
            if(strstr(json_start, "\"ventilation\""))
            {
                feng = strstr(json_start, "\"value\":1") ? '1' : '0';
            }
            if(strstr(json_start, "\"window\""))
            {
                fa = strstr(json_start, "\"value\":1") ? '1' : '0';
            }
            if(strstr(json_start, "\"alarm\""))
            {
                bao = strstr(json_start, "\"value\":1") ? '1' : '0';
            }
            // 火焰阈值设置
            if(strstr(json_start, "\"flame_threshold\""))
            {
                char *value_start = strstr(json_start, "\"value\":");
                if(value_start)
                {
                    value_start += 8; // 跳过"value":
                    rana = atoi(value_start); // 更新火焰阈值
                }
            }
        }
    }
}

