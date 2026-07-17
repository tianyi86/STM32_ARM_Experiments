#include "rtc.h" 
#include "delay.h" 

// 全局变量：按键调时状态
u8 time_edit_flag = 0;
u8 edit_sel = 0;

u8 BSP_RTC_Init(void)
{
    RTC_InitTypeDef RTC_InitStructure;
    u16 retry = 0X1FFF; 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    PWR_BackupAccessCmd(ENABLE);
        
    if(RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x5050)    
    {
        RCC_LSEConfig(RCC_LSE_ON);    
        while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET && retry > 0)    
        {
            retry--;
            delay_ms(10);
        }
        if(retry == 0)
        {
            // LSE晶振失败，直接退出初始化，不阻塞程序
            return 1;
        }
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
        RCC_RTCCLKCmd(ENABLE);  

        RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
        RTC_InitStructure.RTC_SynchPrediv  = 0xFF;
        RTC_InitStructure.RTC_HourFormat  = RTC_HourFormat_24;
        RTC_Init(&RTC_InitStructure);
     
        RTC_Set_Time(13,00,00,RTC_H12_AM);
        RTC_Set_Date(21,12,15,3);
        RTC_WriteBackupRegister(RTC_BKP_DR0,0x5050);    
    } 
    return 0;
}

//原有：设置时间函数
ErrorStatus RTC_Set_Time(u8 hour,u8 min,u8 sec,u8 ampm)
{
    RTC_TimeTypeDef RTC_TimeTypeInitStructure;
    RTC_TimeTypeInitStructure.RTC_Hours = hour;
    RTC_TimeTypeInitStructure.RTC_Minutes = min;
    RTC_TimeTypeInitStructure.RTC_Seconds = sec;
    RTC_TimeTypeInitStructure.RTC_H12 = ampm;
    return RTC_SetTime(RTC_Format_BIN, &RTC_TimeTypeInitStructure);
}

//原有：设置日期函数
ErrorStatus RTC_Set_Date(u8 year,u8 month,u8 date,u8 week)
{
    RTC_DateTypeDef RTC_DateTypeInitStructure;
    RTC_DateTypeInitStructure.RTC_Date = date;
    RTC_DateTypeInitStructure.RTC_Month = month;
    RTC_DateTypeInitStructure.RTC_WeekDay = week;
    RTC_DateTypeInitStructure.RTC_Year = year;
    return RTC_SetDate(RTC_Format_BIN, &RTC_DateTypeInitStructure);
}

// ====================== 扩展1：闹钟功能（F4标准库正确写法） ======================
void RTC_Set_AlarmA(u8 hour, u8 min, u8 sec)
{
    RTC_AlarmTypeDef RTC_AlarmStruct;
    RTC_AlarmStruct.RTC_AlarmTime.RTC_Hours = hour;
    RTC_AlarmStruct.RTC_AlarmTime.RTC_Minutes = min;
    RTC_AlarmStruct.RTC_AlarmTime.RTC_Seconds = sec;
    RTC_SetAlarm(RTC_Alarm_A, RTC_Format_BIN, &RTC_AlarmStruct);
    RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
}
u8 RTC_Check_AlarmA(void)
{
    if(RTC_GetFlagStatus(RTC_FLAG_ALRAF) != RESET)
    {
        RTC_ClearFlag(RTC_FLAG_ALRAF);
        return 1;
    }
    return 0;
}

// ====================== 扩展2：按键修改时间 ======================
void RTC_Time_Add(void)
{
    RTC_TimeTypeDef tim;
    RTC_GetTime(RTC_Format_BIN, &tim);
    switch(edit_sel)
    {
        case 0:
            tim.RTC_Hours = (tim.RTC_Hours + 1) % 24;
            break;
        case 1:
            tim.RTC_Minutes = (tim.RTC_Minutes + 1) % 60;
            break;
        case 2:
            tim.RTC_Seconds = (tim.RTC_Seconds + 1) % 60;
            break;
    }
    RTC_Set_Time(tim.RTC_Hours, tim.RTC_Minutes, tim.RTC_Seconds, RTC_H12_AM);
}
