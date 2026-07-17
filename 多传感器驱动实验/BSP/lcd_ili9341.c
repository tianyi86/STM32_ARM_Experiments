#include <stddef.h>
#include "lcd_ili9341.h"
#include "lcd_fonts.h"
#include "delay.h"

/* FSMC Bank1 NORSRAM1: 0x6000 0000 ~ 0x63FF FFFF
   FSMC_A16 connected to LCD DC (Data/Command) pin
   CMD address = 0x60000000 (A16=0)
   DATA address = 0x60020000 (A16=1) */

#define FSMC_Addr_ILI9341_CMD   ((uint32_t)0x60000000)
#define FSMC_Addr_ILI9341_DATA  ((uint32_t)0x60020000)

#define ILI9341_RST_PORT    GPIOE
#define ILI9341_RST_PIN     GPIO_Pin_1
#define ILI9341_BK_PORT     GPIOD
#define ILI9341_BK_PIN      GPIO_Pin_12

#define ILI9341_LESS_PIXEL  240
#define ILI9341_MORE_PIXEL  320

/* Commands */
#define CMD_RESET           0x0001
#define CMD_SLEEP_OUT       0x0011
#define CMD_GAMMA           0x0026
#define CMD_DISPLAY_OFF     0x0028
#define CMD_DISPLAY_ON      0x0029
#define CMD_COLUMN_ADDR     0x002A
#define CMD_PAGE_ADDR       0x002B
#define CMD_GRAM            0x002C
#define CMD_MAC             0x0036
#define CMD_PIXEL_FORMAT    0x003A
#define CMD_WDB             0x0051
#define CMD_WCD             0x0053
#define CMD_RGB_INTERFACE   0x00B0
#define CMD_FRC             0x00B1
#define CMD_BPC             0x00B5
#define CMD_DFC             0x00B6
#define CMD_POWER1          0x00C0
#define CMD_POWER2          0x00C1
#define CMD_VCOM1           0x00C5
#define CMD_VCOM2           0x00C7
#define CMD_POWERA          0x00CB
#define CMD_POWERB          0x00CF
#define CMD_PGAMMA          0x00E0
#define CMD_NGAMMA          0x00E1
#define CMD_DTCA            0x00E8
#define CMD_DTCB            0x00EA
#define CMD_POWER_SEQ       0x00ED
#define CMD_3GAMMA_EN       0x00F2
#define CMD_INTERFACE       0x00F6
#define CMD_PRC             0x00F7
#define CMD_VERTICAL_SCROLL 0x0033
#define CMD_SetCoordinateX  0x2A
#define CMD_SetCoordinateY  0x2B
#define CMD_SetPixel        0x2C

#define ILI9341_DELAY_MS(x)          delay_ms(x)
#define ILI9341_WriteCmd(usCmd)      (*(__IO uint16_t*)(FSMC_Addr_ILI9341_CMD) = (usCmd))
#define ILI9341_WriteData(usData)    (*(__IO uint16_t*)(FSMC_Addr_ILI9341_DATA) = (usData))
#define ILI9341_ReadData()           (*(__IO uint16_t*)(FSMC_Addr_ILI9341_DATA))

uint8_t  g_LCD_ScanMode = 2;
static uint16_t g_LCD_TextColor   = BLACK;
static uint16_t g_LCD_BackColor   = WHITE;
uint16_t g_LCD_LenX = ILI9341_LESS_PIXEL;
uint16_t g_LCD_LenY = ILI9341_MORE_PIXEL;

static void ILI9341_GPIO_Config(void);
static void ILI9341_FSMC_Config(void);
static void ILI9341_Rst(void);
static void ILI9341_REG_Config(void);
static void ILI9341_FillColor(uint32_t ulAmout_Point, uint16_t usColor);
static void ILI9341_BackLight(uint8_t uOnOff);
static void ILI9341_GramScan(uint8_t ucOption);
static void ILI9341_OpenWindow(uint16_t usX, uint16_t usY,
                               uint16_t usWidth, uint16_t usHeight);

uint16_t LCD_GetLenX(void) { return g_LCD_LenX; }
uint16_t LCD_GetLenY(void) { return g_LCD_LenY; }

/* ===================== GPIO Config ===================== */
static void ILI9341_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE, ENABLE);

    /* GPIOD FSMC pins */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 |
                                  GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 |
                                  GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* GPIOE FSMC pins (E7~E15 only, NOT E6 to avoid DHT11 conflict) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
                                  GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 |
                                  GPIO_Pin_15;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    /* AF mapping GPIOD */
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource0,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource1,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource4,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource5,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource7,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);

    /* AF mapping GPIOE */
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource7,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource8,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource9,  GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource10, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource12, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource15, GPIO_AF_FSMC);

    /* PD12 = Backlight (GPIO output) */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* PE1 = Reset (GPIO output) */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
}

/* ===================== Backlight ===================== */
static void ILI9341_BackLight(uint8_t uOnOff)
{
    if(uOnOff)
        GPIO_ResetBits(ILI9341_BK_PORT, ILI9341_BK_PIN);
    else
        GPIO_SetBits(ILI9341_BK_PORT, ILI9341_BK_PIN);
}

/* ===================== Reset ===================== */
static void ILI9341_Rst(void)
{
    GPIO_ResetBits(ILI9341_RST_PORT, ILI9341_RST_PIN);
    ILI9341_DELAY_MS(5);
    GPIO_SetBits(ILI9341_RST_PORT, ILI9341_RST_PIN);
    ILI9341_DELAY_MS(5);
}

/* ===================== FSMC Config ===================== */
static void ILI9341_FSMC_Config(void)
{
    FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
    FSMC_NORSRAMTimingInitTypeDef  p;

    RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);

    p.FSMC_AddressSetupTime = 0x6;
    p.FSMC_AddressHoldTime  = 0;
    p.FSMC_DataSetupTime    = 0x6;
    p.FSMC_BusTurnAroundDuration = 0;
    p.FSMC_CLKDivision = 0;
    p.FSMC_DataLatency = 0;
    p.FSMC_AccessMode = FSMC_AccessMode_A;

    FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;
    FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
    FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
    FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst   = FSMC_WriteBurst_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct     = &p;
    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);

    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);
}

/* ===================== REG Config ===================== */
static void ILI9341_REG_Config(void)
{
    ILI9341_WriteCmd(CMD_RESET);
    ILI9341_DELAY_MS(120);
    ILI9341_WriteCmd(CMD_DISPLAY_OFF);

    /* Power control */
    ILI9341_WriteCmd(CMD_POWER1);
    ILI9341_WriteData(0x23);
    ILI9341_WriteCmd(CMD_POWER2);
    ILI9341_WriteData(0x10);
    ILI9341_WriteCmd(CMD_VCOM1);
    ILI9341_WriteData(0x2B);
    ILI9341_WriteData(0x2B);
    ILI9341_WriteCmd(CMD_VCOM2);
    ILI9341_WriteData(0xC0);

    ILI9341_WriteCmd(CMD_PIXEL_FORMAT);
    ILI9341_WriteData(0x55); /* 16bit/pixel */

    ILI9341_WriteCmd(CMD_FRC);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x1B);

    ILI9341_WriteCmd(CMD_DFC);
    ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x82);
    ILI9341_WriteData(0x27);

    ILI9341_WriteCmd(CMD_3GAMMA_EN);
    ILI9341_WriteData(0x00);

    ILI9341_WriteCmd(CMD_GAMMA);
    ILI9341_WriteData(0x01);

    ILI9341_WriteCmd(CMD_PGAMMA);
    ILI9341_WriteData(0x0F); ILI9341_WriteData(0x31); ILI9341_WriteData(0x2B);
    ILI9341_WriteData(0x0C); ILI9341_WriteData(0x0E); ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x4E); ILI9341_WriteData(0xF1); ILI9341_WriteData(0x37);
    ILI9341_WriteData(0x07); ILI9341_WriteData(0x10); ILI9341_WriteData(0x03);
    ILI9341_WriteData(0x0E); ILI9341_WriteData(0x09); ILI9341_WriteData(0x00);

    ILI9341_WriteCmd(CMD_NGAMMA);
    ILI9341_WriteData(0x00); ILI9341_WriteData(0x0E); ILI9341_WriteData(0x14);
    ILI9341_WriteData(0x03); ILI9341_WriteData(0x11); ILI9341_WriteData(0x07);
    ILI9341_WriteData(0x31); ILI9341_WriteData(0xC1); ILI9341_WriteData(0x48);
    ILI9341_WriteData(0x08); ILI9341_WriteData(0x0F); ILI9341_WriteData(0x0C);
    ILI9341_WriteData(0x31); ILI9341_WriteData(0x36); ILI9341_WriteData(0x0F);

    ILI9341_WriteCmd(CMD_SLEEP_OUT);
    ILI9341_DELAY_MS(100);
    ILI9341_WriteCmd(CMD_DISPLAY_ON);
    ILI9341_DELAY_MS(100);
    ILI9341_WriteCmd(CMD_GRAM);
    ILI9341_DELAY_MS(5);
}

/* ===================== GramScan ===================== */
static void ILI9341_GramScan(uint8_t ucOption)
{
    if(ucOption > 7) return;
    g_LCD_ScanMode = ucOption;

    if(ucOption % 2 == 0) {
        g_LCD_LenX = ILI9341_LESS_PIXEL;
        g_LCD_LenY = ILI9341_MORE_PIXEL;
    } else {
        g_LCD_LenX = ILI9341_MORE_PIXEL;
        g_LCD_LenY = ILI9341_LESS_PIXEL;
    }

    ILI9341_WriteCmd(CMD_MAC);
    ILI9341_WriteData(0x08 | (ucOption << 5));
    ILI9341_OpenWindow(0, 0, g_LCD_LenX, g_LCD_LenY);
    ILI9341_WriteCmd(CMD_SetPixel);
}

/* ===================== OpenWindow ===================== */
static void ILI9341_OpenWindow(uint16_t usX, uint16_t usY,
                               uint16_t usWidth, uint16_t usHeight)
{
    ILI9341_WriteCmd(CMD_SetCoordinateX);
    ILI9341_WriteData(usX >> 8);
    ILI9341_WriteData(usX & 0xff);
    ILI9341_WriteData((usX + usWidth - 1) >> 8);
    ILI9341_WriteData((usX + usWidth - 1) & 0xff);

    ILI9341_WriteCmd(CMD_SetCoordinateY);
    ILI9341_WriteData(usY >> 8);
    ILI9341_WriteData(usY & 0xff);
    ILI9341_WriteData((usY + usHeight - 1) >> 8);
    ILI9341_WriteData((usY + usHeight - 1) & 0xff);
}

/* ===================== FillColor ===================== */
static void ILI9341_FillColor(uint32_t ulAmout_Point, uint16_t usColor)
{
    uint32_t i = 0;
    ILI9341_WriteCmd(CMD_SetPixel);
    for(i = 0; i < ulAmout_Point; i++)
        ILI9341_WriteData(usColor);
}

/* ===================== LCD Init ===================== */
void LCD_Init(void)
{
    ILI9341_GPIO_Config();
    ILI9341_FSMC_Config();
    ILI9341_Rst();
    ILI9341_BackLight(ENABLE);
    ILI9341_REG_Config();
    ILI9341_GramScan(g_LCD_ScanMode);
}

/* ===================== Colors ===================== */
void LCD_SetColors(uint16_t TextColor, uint16_t BackColor)
{
    g_LCD_TextColor = TextColor;
    g_LCD_BackColor = BackColor;
}

void LCD_GetColors(uint16_t *TextColor, uint16_t *BackColor)
{
    *TextColor = g_LCD_TextColor;
    *BackColor = g_LCD_BackColor;
}

void LCD_SetTextColor(uint16_t Color) { g_LCD_TextColor = Color; }
void LCD_SetBackColor(uint16_t Color) { g_LCD_BackColor = Color; }

/* ===================== Clear ===================== */
void LCD_Clear(uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight)
{
    ILI9341_OpenWindow(usX, usY, usWidth, usHeight);
    ILI9341_FillColor(usWidth * usHeight, g_LCD_BackColor);
}

/* ===================== SetPixel ===================== */
void LCD_SetPixel(uint16_t uX, uint16_t uY)
{
    if((uX < g_LCD_LenX) && (uY < g_LCD_LenY)) {
        ILI9341_OpenWindow(uX, uY, 1, 1);
        ILI9341_FillColor(1, g_LCD_TextColor);
    }
}

/* ===================== GetPixel ===================== */
uint16_t LCD_GetPixel(uint16_t usX, uint16_t usY)
{
    ILI9341_OpenWindow(usX, usY, 1, 1);
    ILI9341_WriteCmd(CMD_GRAM);
    (void)ILI9341_ReadData();
    return ILI9341_ReadData();
}

/* ===================== DrawLine ===================== */
void LCD_DrawLine(uint16_t usX1, uint16_t usY1, uint16_t usX2, uint16_t usY2)
{
    uint16_t us;
    uint16_t xpos, ypos;
    int32_t dx, dy, incx, incy, pdx, pdy, ddx, ddy, es, el, err;

    dx = (int32_t)usX2 - (int32_t)usX1;
    dy = (int32_t)usY2 - (int32_t)usY1;
    incx = (dx > 0) ? 1 : ((dx < 0) ? -1 : 0);
    incy = (dy > 0) ? 1 : ((dy < 0) ? -1 : 0);
    dx = (dx > 0) ? dx : -dx;
    dy = (dy > 0) ? dy : -dy;

    if(dx > dy) { pdx = incx; pdy = 0; ddx = incx; ddy = incy; es = dy; el = dx; }
    else        { pdx = 0; pdy = incy; ddx = incx; ddy = incy; es = dx; el = dy; }

    xpos = usX1; ypos = usY1;
    err = el / 2;
    LCD_SetPixel(xpos, ypos);

    for(us = 0; us < el; us++) {
        err -= es;
        if(err < 0) { err += el; xpos += ddx; ypos += ddy; }
        else        { xpos += pdx; ypos += pdy; }
        LCD_SetPixel(xpos, ypos);
    }
}

/* ===================== DrawRectangle ===================== */
void LCD_DrawRectangle(uint16_t usX_Start, uint16_t usY_Start,
                       uint16_t usWidth, uint16_t usHeight, uint8_t ucFilled)
{
    if(ucFilled) {
        ILI9341_OpenWindow(usX_Start, usY_Start, usWidth, usHeight);
        ILI9341_FillColor(usWidth * usHeight, g_LCD_TextColor);
    } else {
        LCD_DrawLine(usX_Start, usY_Start,
                     usX_Start + usWidth - 1, usY_Start);
        LCD_DrawLine(usX_Start, usY_Start + usHeight - 1,
                     usX_Start + usWidth - 1, usY_Start + usHeight - 1);
        LCD_DrawLine(usX_Start, usY_Start, usX_Start, usY_Start + usHeight - 1);
        LCD_DrawLine(usX_Start + usWidth - 1, usY_Start,
                     usX_Start + usWidth - 1, usY_Start + usHeight - 1);
    }
}

/* ===================== DrawCircle ===================== */
void LCD_DrawCircle(uint16_t usX_Center, uint16_t usY_Center,
                    uint16_t usRadius, uint8_t ucFilled)
{
    int32_t x, y, d;
    x = 0;
    y = (int32_t)usRadius;
    d = 3 - 2 * (int32_t)usRadius;

    while(x <= y) {
        if(ucFilled) {
            LCD_DrawLine(usX_Center - x, usY_Center - y,
                         usX_Center + x, usY_Center - y);
            LCD_DrawLine(usX_Center - x, usY_Center + y,
                         usX_Center + x, usY_Center + y);
            LCD_DrawLine(usX_Center - y, usY_Center - x,
                         usX_Center + y, usY_Center - x);
            LCD_DrawLine(usX_Center - y, usY_Center + x,
                         usX_Center + y, usY_Center + x);
        } else {
            LCD_SetPixel(usX_Center + x, usY_Center + y);
            LCD_SetPixel(usX_Center - x, usY_Center + y);
            LCD_SetPixel(usX_Center + x, usY_Center - y);
            LCD_SetPixel(usX_Center - x, usY_Center - y);
            LCD_SetPixel(usX_Center + y, usY_Center + x);
            LCD_SetPixel(usX_Center - y, usY_Center + x);
            LCD_SetPixel(usX_Center + y, usY_Center - x);
            LCD_SetPixel(usX_Center - y, usY_Center - x);
        }
        x++;
        if(d < 0) { d += 4 * x + 6; }
        else      { d += 4 * (x - y) + 10; y--; }
    }
}

/* ===================== DispMask ===================== */
void LCD_DispMask(uint16_t uX, uint16_t uY, uint16_t uWidth,
                  int16_t uHeight, const uint8_t *pMask)
{
    uint16_t i, j = 0;
    uint16_t uFontLength = uWidth * uHeight / 8;

    ILI9341_OpenWindow(uX, uY, uWidth, uHeight);
    ILI9341_WriteCmd(CMD_SetPixel);

    for(i = 0; i < uFontLength; i++) {
        for(j = 0; j < 8; j++) {
            if(pMask[i] & (0x80 >> j))
                ILI9341_WriteData(g_LCD_TextColor);
            else
                ILI9341_WriteData(g_LCD_BackColor);
        }
    }
}

/* ===================== DispStringEN ===================== */
void LCD_DispStringEN(uint16_t uX, uint16_t uY, uint8_t uDir, char *pStr)
{
    uint8_t *pMask = NULL;
    uint16_t uCharWidth  = LCD_GetFontEN()->Width;
    uint16_t uCharHeight = LCD_GetFontEN()->Height;

    while(*pStr != '\0') {
        pMask = LCD_GetMaskEN(*pStr);
        pStr++;

        if(g_LCD_LenX - uX < uCharWidth) {
            uX = 0;
            uY += uCharHeight;
        }
        if(g_LCD_LenY - uY < uCharHeight) {
            uY = 0;
        }

        LCD_DispMask(uX, uY, uCharWidth, uCharHeight, pMask);
        if(uDir == 0)
            uX += uCharWidth;
        else
            uY += uCharHeight;
    }
}

/* ===================== DrawFlame ===================== */
void LCD_DrawFlame(void)
{
    uint16_t cx = g_LCD_LenX / 2;       /* center X */
    uint16_t cy = g_LCD_LenY * 3 / 4;   /* flame base Y */

    /* ---- clear screen black ---- */
    LCD_SetBackColor(BLACK);
    LCD_Clear(0, 0, g_LCD_LenX, g_LCD_LenY);

    /* ---- outer flame (dark red) ---- */
    LCD_SetTextColor(DARKRED);
    LCD_DrawCircle(cx,     cy - 50,  55, 1);
    LCD_DrawCircle(cx - 30, cy - 55,  50, 1);
    LCD_DrawCircle(cx + 30, cy - 55,  50, 1);
    LCD_DrawRectangle(cx - 45, cy - 70, 90, 80, 1);

    /* ---- mid flame (red-orange) ---- */
    LCD_SetTextColor(RED);
    LCD_DrawCircle(cx,     cy - 35,  45, 1);
    LCD_DrawCircle(cx - 25, cy - 40,  40, 1);
    LCD_DrawCircle(cx + 25, cy - 40,  40, 1);
    LCD_DrawRectangle(cx - 35, cy - 55, 70, 65, 1);

    /* ---- inner flame (orange) ---- */
    LCD_SetTextColor(ORANGE);
    LCD_DrawCircle(cx,     cy - 20,  32, 1);
    LCD_DrawCircle(cx - 18, cy - 25,  28, 1);
    LCD_DrawCircle(cx + 18, cy - 25,  28, 1);
    LCD_DrawRectangle(cx - 22, cy - 38, 44, 50, 1);

    /* ---- bright core (yellow) ---- */
    LCD_SetTextColor(YELLOW);
    LCD_DrawCircle(cx,     cy - 8,   18, 1);
    LCD_DrawCircle(cx - 10, cy - 12,  15, 1);
    LCD_DrawCircle(cx + 10, cy - 12,  15, 1);
    LCD_DrawRectangle(cx - 12, cy - 22, 24, 32, 1);

    /* ---- white-hot center ---- */
    LCD_SetTextColor(WHITE);
    LCD_DrawCircle(cx,     cy - 4,   8, 1);
    LCD_DrawCircle(cx - 5,  cy - 6,   6, 1);
    LCD_DrawCircle(cx + 5,  cy - 6,   6, 1);

    /* ---- spark particles ---- */
    LCD_SetTextColor(YELLOW);
    LCD_DrawCircle(cx + 30, cy - 90,  2, 1);
    LCD_DrawCircle(cx - 25, cy - 100, 2, 1);
    LCD_DrawCircle(cx + 15, cy - 110, 1, 1);
    LCD_DrawCircle(cx - 40, cy - 80,  1, 1);
    LCD_DrawCircle(cx + 45, cy - 75,  2, 1);
    LCD_DrawCircle(cx - 10, cy - 120, 2, 1);
    LCD_DrawCircle(cx + 35, cy - 105, 1, 1);
    LCD_DrawCircle(cx - 35, cy - 95,  2, 1);

    /* ---- ember glow at base ---- */
    LCD_SetTextColor(RED);
    LCD_DrawCircle(cx - 50, cy + 5,  12, 1);
    LCD_DrawCircle(cx + 50, cy + 5,  12, 1);
    LCD_DrawCircle(cx,     cy + 10, 15, 1);

    LCD_SetTextColor(ORANGE);
    LCD_DrawCircle(cx - 50, cy + 5,  6, 1);
    LCD_DrawCircle(cx + 50, cy + 5,  6, 1);
}
