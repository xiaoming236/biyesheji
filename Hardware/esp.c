#include "stm32f10x.h"                  // Device header
#include "MyUSART.h"
#include <stdio.h>
#include <string.h>
#include "delay.h"
#include "OLED.H"


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
const char* WIFI ="1234";
const char* WIFIASSWORD="123456789";
const char* ClintID="a1jzeEAuGe0.1234|securemode=2\\,signmethod=hmacsha256\\,timestamp=1728745199404|";
const char* username="1234&a1jzeEAuGe0";
const char* passwd="8d45f0ddd31033057d547e6bcd0f29b63ab00d9aee714c8b096f2d31c6f3fa3e";
const char* Url="a1jzeEAuGe0.iot-as-mqtt.cn-shanghai.aliyuncs.com";
const char* pubtopic="/sys/a1jzeEAuGe0/1234/thing/event/property/post";
const char* subtopic="/sys/a1jzeEAuGe0/1234/thing/event/property/post_reply";
const char* func1="temperature";
const char* func2="Humidity";
const char* func3="yan";
const char* func4="feng";
const char* func5="yan1";
const char* func6="yan2";	
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
	printf("AT+CWJAP=\"%s\",\"%s\"\r\n",WIFI,WIFIASSWORD); //连接热点
	Delay_ms(2000);
	if(strcmp(RECS,"OK"))
		return 3;
	
	memset(RECS,0,sizeof(RECS));
	printf("AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n",ClintID,username,passwd);//用户信息配置
	Delay_ms(10);
	if(strcmp(RECS,"OK"))
		return 4;
	
	memset(RECS,0,sizeof(RECS));
	printf("AT+MQTTCONN=0,\"%s\",1883,1\r\n",Url); //连接服务器
	Delay_ms(1000);
	if(strcmp(RECS,"OK"))
		return 5;
	
	printf("AT+MQTTSUB=0,\"%s\",1\r\n",subtopic); //订阅消息
	Delay_ms(500);
	if(strcmp(RECS,"OK"))			
		return 5;
	memset(RECS,0,sizeof(RECS));
	return 0;
}
//功能：esp发送消息
//参数：无
//返回值：0：发送成功；1：发送失败
char Esp_PUB(void)
{
	memset(RECS,0,sizeof(RECS));
//	printf("AT+MQTTPUB=0,\"%s\",\"{\\\"method\\\":\\\"thing.event.property.post\\\"\\,\\\"params\\\":{\\\"%s\\\":%d\\,\\\"%s\\\":%f\\,\\\"%s\\\":%d\\,\\\"%s\\\":%d\\,\\\"%s\\\":%d\\,\\\"%s\\\":%d\\,\\\"%s\\\":%d\\,\\\"%s\\\":%d\\,\\\"%s\\\":%d\\}}\",0,0\r\n",
//	pubtopic,func1,wen1,func2,shi1,func3,MQ2_Value,func4,deng1,func5,guang1,func6,guang2,func7,shi11,func8,shi22,func9,duoji);
	printf("AT+MQTTPUB=0,\"%s\",\"{\\\"method\\\":\\\"thing.event.property.post\\\"\\,\\\"params\\\":{\\\"%s\\\":%d\\,\\\"%s\\\":%f\\,\\\"%s\\\":%f\\,\\\"%s\\\":%d\\}}\",0,0\r\n",
	pubtopic,func1,wen1,func2,shi1,func3,MQ2_Value,func12,MQ7_Value);
	printf("AT+MQTTPUB=0,\"%s\",\"{\\\"method\\\":\\\"thing.event.property.post\\\"\\,\\\"params\\\":{\\\"%s\\\":%d\\,\\\"%s\\\":%d\\,\\\"%s\\\":%d\\,\\\"%s\\\":%d\\,\\\"%s\\\":%d\\,\\\"%s\\\":%d\\,\\\"%s\\\":%d\\,\\\"%s\\\":%d\\,\\\"%s\\\":%d\\,\\\"%s\\\":%d\\}}\",0,0\r\n",
	pubtopic,func4,feng,func5,yan1,func6,yan2,func7,wena,func8,wenb,func9,shui,func10,bao,func11,fa,func14,rana,func15,ranb);
	//while(RECS[0]);//等待ESP返回数据
////	printf("AT+MQTTPUB=0,\"%s\",/thing/event/property/post","{\"params\":{\"Temperature\":27}}",0,0);
	Delay_ms(200);//延时等待数据接收完成
	if(strcmp(RECS,"ERROR")==0)
		return 1;
	return 0;
}
void CommandAnalyse(void)
{
	if(strncmp(RECS,"+MQTTSUBRECV:",13)==0)
	{
		uint8_t i=0;
		while(RECS[i++] != '\0')             
		{
            if(strncmp((RECS+i),func4,4)==0)
			{ 
				while(RECS[i++] != ':');
				feng=RECS[i];
			}
			if(strncmp((RECS+i),func5,4)==0)
			{
				while(RECS[i++] != ':');       
				yan1=RECS[i];
			}
			if(strncmp((RECS+i),func6,4)==0)
			{
				while(RECS[i++] != ':');
				yan2=RECS[i];
			}
			if(strncmp((RECS+i),func7,4)==0)
			{
				while(RECS[i++] != ':');
				wena=RECS[i];
			}
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









