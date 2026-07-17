#ifndef __XPT2046_H__
#define __XPT2046_H__

#include "stm32f4xx.h"

// 液晶坐标结构体
typedef struct {
    int16_t x;
    int16_t y;
    int16_t pre_x;
    int16_t pre_y;
} XPT2046_Coord;

// 校准因子结构体（重点，必须有 typedef 和分号）
typedef struct {
    long double A;
    long double B;
    long double C;
    long double D;
    long double E;
    long double F;
} XPT2046_Factor;

// 触摸状态枚举
typedef enum {
    XPT2046_STATE_RELEASE = 0,
    XPT2046_STATE_WAITING,
    XPT2046_STATE_PRESSED
} enumTouchState;

// 函数声明
void    XPT2046_Init(void);
uint8_t XPT2046_DetectTouch(void);
int8_t  XPT2046_GetTouchPoint(XPT2046_Coord *displayPtr);
int8_t  XPT2046_CalibrationFactor(XPT2046_Coord *pLCD, XPT2046_Coord *pTouch, XPT2046_Factor *pFactor);

#endif /* __XPT2046_H__ */