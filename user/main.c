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

u16 MQ7_Value;//adc值
uint16_t fire,y,r,w,h,Y,W,R;
float temp_one,shi1;//电压值
float MQ2_Value;              
DHT11_Data_TypeDef  DHT11_Data;
uint16_t key = 0, flag1 = 0, flag2 = 0, flag3 = 0,flag5 = 0,flag6 = 0,AD=0,wen1,A=0,tem=28,yan=70,ran=60,Judge,cnt;
uint8_t  temp_int;	 	
uint8_t  temp_deci;
uint8_t  humi_int;		
uint8_t  humi_deci;
uint8_t yan1='0';
uint8_t yan2='0';
uint8_t feng='0';
uint8_t wena='0';
uint8_t wenb='0';
uint8_t shui='0';
uint8_t bao='0';
uint8_t fa='0';
uint8_t rana='0';
uint8_t ranb='0';
#define WZ DHT11_Data.temp_int	
#define WX DHT11_Data.temp_deci
#define SZ DHT11_Data.humi_int	
#define SX DHT11_Data.humi_deci

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
        MQ7_Value=Get_Adc_Average1(ADC_Channel_1,10)*100/4095;//模拟烟雾浓度的值
        key = KEY_Scan(0);
        wen1=DHT11_Data.temp_int;
        shi1=DHT11_Data.humi_int;

        // 按键切换模式
        if (key == 1)
        {
            flag1 += 1;
            if (flag1 > 2)
                flag1 = 0;
        }
		
		if(flag1==0)
		{
		   OLED_Clear();
			OLED_ShowChinese(48, 0, "模式选择");
			OLED_ShowChinese(0, 0, "模式：");
			OLED_ShowChinese(0, 20, "烟雾：");
			OLED_ShowChinese(0,40,"温");
			OLED_ShowChinese(60,40, "湿");
			OLED_ShowNum(40,20,MQ2_Value,3,OLED_8X16);
			OLED_ShowNum(84, 40, SZ, 2,OLED_8X16);
			OLED_ShowString(100, 40 ,".",OLED_8X16);
			OLED_ShowNum(107, 40, SX, 2,OLED_8X16);
			OLED_ShowChinese(66, 20, "燃气：");
			OLED_ShowNum(105,20,MQ7_Value,2,OLED_8X16);
			OLED_ShowNum(27, 40, WZ, 2,OLED_8X16);
			OLED_ShowString(45, 40, ".",OLED_8X16);
			OLED_ShowNum(50, 40, WX, 1,OLED_8X16);
			
			if(Read_DHT11(&DHT11_Data) == SUCCESS)
		{
			OLED_ShowNum(84, 40, SZ, 2,OLED_8X16);
			OLED_ShowString(100, 40 ,".",OLED_8X16);
			OLED_ShowNum(107, 40, SX, 2,OLED_8X16);
			
			OLED_ShowNum(27, 40, WZ, 2,OLED_8X16);
			OLED_ShowString(45, 40, ".",OLED_8X16);
			OLED_ShowNum(50, 40, WX, 1,OLED_8X16);
			shi1=DHT11_Data.humi_int;
		    wen1=DHT11_Data.temp_int;
				OLED_Update();
		}
			
			
			
			OLED_Update();
		}
		
		if(flag1==1)
		{
		
			 OLED_Clear();
		    OLED_ShowChinese(48, 0, "监控模式");
			OLED_ShowChinese(0, 0, "模式：");
			OLED_ShowChinese(0, 20, "烟雾：");
			OLED_ShowChinese(66, 20, "燃气：");
			OLED_ShowNum(105,20,MQ7_Value,2,OLED_8X16);
			OLED_ShowChinese(0,40,"温");
			OLED_ShowChinese(60,40, "湿");
			OLED_ShowNum(40,20,MQ2_Value,3,OLED_8X16);
			OLED_ShowNum(84, 40, SZ, 2,OLED_8X16);
			OLED_ShowString(100, 40 ,".",OLED_8X16);
			OLED_ShowNum(107, 40, SX, 2,OLED_8X16);
			
			OLED_ShowNum(27, 40, WZ, 2,OLED_8X16);
			OLED_ShowString(45, 40, ".",OLED_8X16);
			OLED_ShowNum(50, 40, WX, 1,OLED_8X16);
			if(Read_DHT11(&DHT11_Data) == SUCCESS)
		{
			OLED_ShowNum(84, 40, SZ, 2,OLED_8X16);
			OLED_ShowString(100, 40 ,".",OLED_8X16);
			OLED_ShowNum(107, 40, SX, 2,OLED_8X16);
			
			OLED_ShowNum(27, 40, WZ, 2,OLED_8X16);
			OLED_ShowString(45, 40, ".",OLED_8X16);
			OLED_ShowNum(50, 40, WX, 1,OLED_8X16);

		}
		
		if(fire)
		{
	  OLED_ShowNum(115, 0, 1, 1,OLED_8X16);
		shuikai();
			
		h=1;
		}
		
		else 
		{	
		OLED_ShowNum(115, 0, 0, 1,OLED_8X16);
	    shuiguan();
			h=0;

		
		}

		if(MQ2_Value>yan)
		{
			Y=1;
			y=1;
		
		
		}
		else if(MQ2_Value<yan)
		{
			Y=0;
			y=0;
		
		}		
		if(MQ7_Value<ran&&flag5==0)
		{
			R=0;
			r=0;
			flag5=1;
		
		
		}
		else if(MQ7_Value>ran&&flag5==1)
		{
			R=1;
			flag5=0;

			r=1;
			
		
		}


		if(wen1>tem)
		{
			W=1;
			w=1;
		
		}
		else  if(wen1<tem)
		{
		
		W=0;
			w=0;
		}
		if(w==1||y==1||r==1||h==1)
		{
		BEEP=1;
		}
		else{
		
		BEEP=0;
		}
		if(W==1||R==1||Y==1)
		{
		fengkai();
		Servo_SetAngle(180);
			
		}
		else{
		
		fengguan();
			 Servo_SetAngle(0);
		}
		
		OLED_Update();
		
		}			
		
	
	if(flag1==2)
	{
		OLED_Clear();
		OLED_ShowChinese(40,0,"设置页面");
		OLED_ShowChinese(0,40,"温度");
		OLED_ShowChinese(70,40,"烟雾");
		OLED_ShowChinese(0,20,"进入设置:");
		OLED_ShowChinese(66, 20, "燃气：");
		OLED_ShowNum(105,20,ran,2,OLED_8X16);
		OLED_ShowNum(35,40,tem,2,OLED_8X16);
		OLED_ShowNum(110,40,yan,2,OLED_8X16);

		if (key == 2)
			
		{
		flag3++;
		if(flag3>4)
		flag3=0;
		
		}
		
		if(flag3==0)
		{
			
		OLED_ShowChinese(0,20,"进入设置:");
			
		}
			if(flag3==1)
		{
		OLED_ShowChinese(0,0,"温度");

	
			if(key == 3)
			{
			tem++;
			
			}
		if(key == 4)
					{
					tem--;
					
					}
		}
    
		
		 if(flag3==2)
		{
		OLED_ShowChinese(0,0,"烟雾");//烟雾设置

			if(key == 3)
			{
			yan++;
			
			}
		if(key == 4)
					{
					yan--;
					
					}
		
	}
		 if(flag3==3)
		{
		OLED_ShowChinese(0,0,"燃气");//烟雾设置

			if(key == 3)
			{
			ran++;
			
			}
		if(key == 4)
					{
					ran--;
					
					}
		
	}
		 if(flag3==4)
		{
		OLED_ShowChinese(0,0,"远程:");//远程设置
	
		
		if(wena=='1')
		{
		
		tem=tem+1;
		}
		else if(wena=='0')
		{
			
		}
		
		if(wenb=='1')
		{
			tem=tem-1;
		}
		else if(wenb=='0')
		{
			
		}
		
		if(yan1=='1')
		{
		yan=yan+1;
		}
		else if(yan1=='0')
		{
			
		}
		if(yan2=='1')
		{
			yan=yan-1;
		}
		else if(yan2=='0')
		{
			
		}
		if(rana=='1')
		{
		
		ran=ran+1;
		}
		else if(rana=='0')
		{
			
		}
		
		if(ranb=='1')
		{
			ran=ran-1;
		}
		else if(ranb=='0')
		{
			
		}
		
		if(feng=='1')
		{
		fengkai();
		}
		else if(feng=='0')
		{
		fengguan();
		
		}
		
		if(shui=='0')
		{
		shuiguan();
		}
		else if(shui=='1')
		{
		shuikai();
		
		}
		if(bao=='1')
		{
		BEEP=1;
		}
		else if(bao=='0')
		{
	    BEEP=0;
		
		} 
		if(fa=='1'&&flag6==0)
		{
		  Servo_SetAngle(180);
			flag6=1;
		}
		else if(fa=='0'&&flag6==1)
		{
	    Servo_SetAngle(0);
			flag6=0;
			
		
		}

	}		
OLED_Update();

}
	}
}

