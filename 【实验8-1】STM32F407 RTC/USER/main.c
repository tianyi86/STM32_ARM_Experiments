#include "stm32f4xx.h"
#include "delay.h"
#include "oled.h"
#include "rtc.h"
#include "stdio.h"

//按键、闹钟LED引脚定义
#define KEY_ADD_PIN    GPIO_Pin_0
#define KEY_SW_PIN     GPIO_Pin_1
#define KEY_GPIO_PORT  GPIOA
#define LED_ALARM_PIN  GPIO_Pin_2
#define LED_GPIO_PORT  GPIOD

//RTC缓存结构体、显示数组
RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;
u8  Rtc_Data[50];

//按键初始化 PA0 PA1 上拉输入
void KEY_Init(void)
{
    GPIO_InitTypeDef gpio;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    gpio.GPIO_Pin = KEY_ADD_PIN | KEY_SW_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IN;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(KEY_GPIO_PORT, &gpio);
}

//闹钟提示LED初始化 PD2推挽输出
void LED_Init(void)
{
    GPIO_InitTypeDef gpio;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    gpio.GPIO_Pin = LED_ALARM_PIN;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(LED_GPIO_PORT, &gpio);
    GPIO_ResetBits(LED_GPIO_PORT, LED_ALARM_PIN);
}

//按键扫描函数 消抖
u8 KEY_Scan(void)
{
    if(GPIO_ReadInputDataBit(KEY_GPIO_PORT, KEY_ADD_PIN) == 0)
    {
        delay_ms(20);
        while(GPIO_ReadInputDataBit(KEY_GPIO_PORT, KEY_ADD_PIN) == 0);
        delay_ms(20);
        return 1;
    }
    if(GPIO_ReadInputDataBit(KEY_GPIO_PORT, KEY_SW_PIN) == 0)
    {
        delay_ms(20);
        while(GPIO_ReadInputDataBit(KEY_GPIO_PORT, KEY_SW_PIN) == 0);
        delay_ms(20);
        return 2;
    }
    return 0;
}

int main(void)
{
    u8 key_val = 0;
    u8 alarm_flag = 0;
    delay_ms(100); 

    //外设初始化
    OLED_Init();   
    OLED_Clear();  
    KEY_Init();
    LED_Init();
    
    //开机显示“青软集团”
    OLED_ShowCHinese(28,0,0);    //青
    delay_ms(200);
    OLED_ShowCHinese(46,0,1);    //软
    delay_ms(200);
    OLED_ShowCHinese(64,0,2);    //集
    delay_ms(200);
    OLED_ShowCHinese(82,0,3);    //团
    
    //RTC初始化，删掉原来阻塞的delay_ms(1000);
    BSP_RTC_Init();
    RTC_Set_AlarmA(13, 5, 0); //预设闹钟13:05
    
    while(1)
    {
        key_val = KEY_Scan();
        //切换按键：进入/退出调时
        if(key_val == 2)
        {
            time_edit_flag = !time_edit_flag;
            if(time_edit_flag == 1)
                edit_sel = 0;
        }
        //加按键：仅调时模式生效，数值+1
        if(key_val == 1 && time_edit_flag == 1)
        {
            RTC_Time_Add();
            edit_sel = (edit_sel + 1) % 3;
        }

        //读取当前时间、日期
        RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);
        RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);

        //分模式显示时间：正常/调时高亮标记
        if(time_edit_flag == 0)
        {
            sprintf((char*)Rtc_Data," Time:%02d:%02d:%02d  ", 
                    RTC_TimeStruct.RTC_Hours, 
                    RTC_TimeStruct.RTC_Minutes, 
                    RTC_TimeStruct.RTC_Seconds); 
        }
        else
        {
            switch(edit_sel)
            {
                case 0:sprintf((char*)Rtc_Data,"TIME:>%02d<:%02d:%02d",RTC_TimeStruct.RTC_Hours,RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds);break;
                case 1:sprintf((char*)Rtc_Data,"TIME:%02d:>%02d<:%02d",RTC_TimeStruct.RTC_Hours,RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds);break;
                case 2:sprintf((char*)Rtc_Data,"TIME:%02d:%02d:>%02d<",RTC_TimeStruct.RTC_Hours,RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds);break;
            }
        }
        OLED_ShowString(0,0,Rtc_Data,16);  

        //第二行显示日期+闹钟提示
        sprintf((char*)Rtc_Data,"Date:20%02d-%02d-%02d ALARM:13:05", 
                RTC_DateStruct.RTC_Year, 
                RTC_DateStruct.RTC_Month, 
                RTC_DateStruct.RTC_Date); 
        OLED_ShowString(0,2,Rtc_Data,16);  

        //闹钟触发LED闪烁
        alarm_flag = RTC_Check_AlarmA();
        if(alarm_flag == 1)
        {
            GPIO_SetBits(LED_GPIO_PORT, LED_ALARM_PIN);
            delay_ms(200);
            GPIO_ResetBits(LED_GPIO_PORT, LED_ALARM_PIN);
        }
        delay_ms(400);
    }
}