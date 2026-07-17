#ifndef __LCD_ILI9341_H__
#define __LCD_ILI9341_H__
#include "stm32f4xx.h"
#include "lcd_fonts.h"

/******************************* ILI9341 常用颜色 ********************************/
#define   WHITE         0xFFFF
#define   BLACK         0x0000
#define   GREY          0xF7DE
#define   BLUE          0x001F
#define   BLUE2         0x051F
#define   RED           0xF800
#define   MAGENTA       0xF81F
#define   GREEN         0x07E0
#define   CYAN          0x7FFF
#define   YELLOW        0xFFE0
#define   ORANGE        0xFD20
#define   DARKRED       0xA800
#define   BRED          0xF81F
#define   GRED          0xFFE0
#define   GBLUE         0x07FF

void LCD_Init(void);

uint16_t LCD_GetLenX(void);
uint16_t LCD_GetLenY(void);
void LCD_SetColors(uint16_t TextColor, uint16_t BackColor);
void LCD_GetColors(uint16_t *TextColor, uint16_t *BackColor);
void LCD_SetTextColor(uint16_t Color);
void LCD_SetBackColor(uint16_t Color);

void LCD_SetPixel(uint16_t usX, uint16_t usY);
uint16_t LCD_GetPixel(uint16_t usX, uint16_t usY);
void LCD_Clear(uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight);

void LCD_DrawLine(uint16_t usX1, uint16_t usY1, uint16_t usX2, uint16_t usY2);
void LCD_DrawRectangle(uint16_t usX_Start, uint16_t usY_Start,
                       uint16_t usWidth, uint16_t usHeight, uint8_t ucFilled);
void LCD_DrawCircle(uint16_t usX_Center, uint16_t usY_Center,
                    uint16_t usRadius, uint8_t ucFilled);

void LCD_DispStringEN(uint16_t usX, uint16_t usY, uint8_t uDir, char *pStr);
void LCD_DispMask(uint16_t uX, uint16_t uY, uint16_t uWidth,
                  int16_t uHeight, const uint8_t *pMask);

void LCD_DrawFlame(void);

#endif /* __LCD_ILI9341_H__ */
