#ifndef _RTC_H_
#define _RTC_H_
#include "stm32f4xx.h"

//基础RTC函数
u8 BSP_RTC_Init(void);
ErrorStatus RTC_Set_Date(u8 year,u8 month,u8 date,u8 week);
ErrorStatus RTC_Set_Time(u8 hour,u8 min,u8 sec,u8 ampm);

//闹钟扩展
void RTC_Set_AlarmA(u8 hour, u8 min, u8 sec);
u8 RTC_Check_AlarmA(void);

//按键调时全局变量
extern u8 time_edit_flag;
extern u8 edit_sel;
void RTC_Time_Add(void);

#endif