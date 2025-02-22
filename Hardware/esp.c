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
const char* WIFI = "1234";  // 保持不变
const char* WIFIASSWORD = "123456789";  // 保持不变
const char* ONENET_SERVER = "183.230.40.39";
const char* ONENET_PORT = "6002";
const char* ONENET_DEVICE_ID = "SmFONzEyRDhZNnBRd3lzUkc4ZG5hUW1nejVOOGZlRjE=" ; 
const char* ONENET_API_KEY = "tcWzS6+3L48hbxc/BT3dmytPeVs9Q/fVMbkLzSezQTU="  ;
const char* ONENET_PRODUCT_ID = "Ub99ifb8x7";
const char* func1="temperature";  // 温度
const char* func2="Humidity";  // 湿度
const char* func3="yan";  // 烟雾
const char* func4="feng"; // 风扇
const char* func5="yan1"; // 烟雾1
const char* func6="yan2";	 // 烟雾2
const char* func7="wen1";
const char* func8="wen2";
const char* func9="shui";
const char *func10="bao";
const char *func11="fa";
const char *func12="ran";
const char *func14="ran1";
const char *func15="ran2";


int fputc(int ch,FILE *f )   //printf重定向  
{
	USART_SendData(USART1,(uint8_t)ch);
	while(USART_GetFlagStatus (USART1,USART_FLAG_TC) == RESET);
	return ch;
}
char esp_Init(void)
{
    memset(RECS,0,sizeof(RECS));
    printf("AT+RST\r\n");  //重启
    Delay_ms(2000);
    
    memset(RECS,0,sizeof(RECS));
    printf("ATE0\r\n");    //关闭回显
    Delay_ms(10);
    if(strcmp(RECS,"OK"))
        return 1;
    
    printf("AT+CWMODE=1\r\n"); //Station模式
    Delay_ms(1000);
    if(strcmp(RECS,"OK"))
        return 2;
    
    memset(RECS,0,sizeof(RECS));
    printf("AT+CWJAP=\"%s\",\"%s\"\r\n",WIFI,WIFIASSWORD); //连接WiFi
    Delay_ms(2000);
    if(strcmp(RECS,"OK"))
        return 3;
    
    memset(RECS,0,sizeof(RECS));
    // 配置MQTT连接信息
    printf("AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n",
           ONENET_DEVICE_ID,
           ONENET_PRODUCT_ID,
           ONENET_API_KEY);
    Delay_ms(100);
    if(strcmp(RECS,"OK"))
        return 4;
    
    memset(RECS,0,sizeof(RECS));
    // 连接到OneNET MQTT服务器
    printf("AT+MQTTCONN=0,\"%s\",%s,1\r\n", ONENET_SERVER, ONENET_PORT);
    Delay_ms(1000);
    if(strcmp(RECS,"OK"))
        return 5;
    
    return 0;
}
//数据上报实现功能：esp发送消息
//参数：无
//返回值：0：发送成功；1：发送失败
char Esp_PUB(void)
{
    char dataBuf[512];
    memset(RECS,0,sizeof(RECS));
    
    // 构建JSON数据
    sprintf(dataBuf, 
        "{\"id\":\"123\","
        "\"params\":{"
            "\"temperature\":%d,"    // 使用温度整数值wen1
            "\"humidity\":%d,"       // 使用湿度值shi1
            "\"yan\":%.2f,"         // MQ2_Value
            "\"ran\":%d,"           // MQ7_Value
            "\"feng\":\"%c\","
            "\"shui\":\"%c\","
            "\"bao\":\"%c\","
            "\"fa\":\"%c\""
        "}}",
        wen1,           // 使用全局变量wen1替代WZ,WX
        (int)shi1,      // 使用全局变量shi1替代SZ,SX
        MQ2_Value,     // 烟雾值
        MQ7_Value,     // 燃气值
        feng,          // 风扇状态
        shui,          // 水泵状态
        bao,           // 报警器状态
        fa             // 窗户状态
    );
    
    // 发布数据到OneNET
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
	// 检查是否收到MQTT消息
	if(strncmp(RECS,"+MQTTSUBRECV:",13)==0)
	{
		uint8_t i=0;
		while(RECS[i++] != '\0')             
		{
			 // 处理风扇控制命令
            if(strncmp((RECS+i),func4,4)==0)
			{ 
				while(RECS[i++] != ':');
				feng=RECS[i];
			}
			 // 处理烟雾传感器1控制
			if(strncmp((RECS+i),func5,4)==0)
			{
				while(RECS[i++] != ':');       
				yan1=RECS[i];
			}
			 // 处理水浸传感器控制
			if(strncmp((RECS+i),func6,4)==0)
			{
				while(RECS[i++] != ':');
				yan2=RECS[i];
			}
			 // 处理报警器控制
			if(strncmp((RECS+i),func7,4)==0)
			{
				while(RECS[i++] != ':');
				wena=RECS[i];
			}
			// 处理窗户控制
			if(strncmp((RECS+i),func8,4)==0)
			{
				while(RECS[i++] != ':');
				wenb=RECS[i];
			}
			
			if(strncmp((RECS+i),func9,4)==0)
			{
				while(RECS[i++] != ':');
				shui=RECS[i];
			}
			if(strncmp((RECS+i),func10,3)==0)
			{
				while(RECS[i++] != ':');
				bao=RECS[i];
			}
				if(strncmp((RECS+i),func11,2)==0)
			{
				while(RECS[i++] != ':');
				fa=RECS[i];
			}
			if(strncmp((RECS+i),func14,4)==0)
			{
				while(RECS[i++] != ':');
				rana=RECS[i];
			}
			if(strncmp((RECS+i),func15,4)==0)
			{
				while(RECS[i++] != ':');
				ranb=RECS[i];
			}
		}
	}
}









