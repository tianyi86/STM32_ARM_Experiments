#include <stdio.h>
#include <string.h>
#include "stm32f4xx.h"
#include "usart.h"
#include "delay.h"
#include "can.h"
#include "dht11.h"
#include "key_led.h"
#include "beep.h"
#include "lcd_ili9341.h"
#include "nfc_pn532.h"

typedef struct
{
    uint8_t temp;
    uint8_t hum;
    uint8_t key_flag;
} SENSOR_DATA;

SENSOR_DATA sensor_buf;
DHT11_Data  dht_data;
int8_t      dht_ret;

/* ---- LCD display ---- */
static void LCD_ShowSensor(uint8_t card_detected, uint8_t *uid, uint8_t high_temp)
{
    char buf[32];

    LCD_SetBackColor(BLACK);
    LCD_SetTextColor(WHITE);
    LCD_Clear(0, 0, LCD_GetLenX(), LCD_GetLenY());

    /* title */
    LCD_SetTextColor(CYAN);
    LCD_DispStringEN(8, LINE_EN(0), 0, "Multi-Sensor Monitor");

    /* temperature */
    LCD_SetTextColor(YELLOW);
    LCD_DispStringEN(8, LINE_EN(2), 0, "Temperature:");
    sprintf(buf, "%d.%d C", dht_data.temp_int, dht_data.temp_deci);
    LCD_SetTextColor(high_temp ? RED : WHITE);
    LCD_DispStringEN(140, LINE_EN(2), 0, buf);

    /* humidity */
    LCD_SetTextColor(GREEN);
    LCD_DispStringEN(8, LINE_EN(3), 0, "Humidity:");
    sprintf(buf, "%d.%d %%RH", dht_data.humi_int, dht_data.humi_deci);
    LCD_SetTextColor(WHITE);
    LCD_DispStringEN(140, LINE_EN(3), 0, buf);

    /* ---- high temp alert (no NFC needed) ---- */
    if(high_temp)
    {
        LCD_SetTextColor(RED);
        LCD_DispStringEN(8, LINE_EN(5), 0, "!!! High Temp Alert !!!");
    }

    /* ---- NFC card info ---- */
    if(card_detected)
    {
        LCD_SetTextColor(GREEN);
        LCD_DispStringEN(8, LINE_EN(6), 0, "Card Detected");

        LCD_SetTextColor(YELLOW);
        sprintf(buf, "UID:%02X %02X %02X %02X",
                uid[0], uid[1], uid[2], uid[3]);
        LCD_DispStringEN(8, LINE_EN(7), 0, buf);
    }
    else if(!high_temp)
    {
        LCD_SetTextColor(GREY);
        LCD_DispStringEN(8, LINE_EN(6), 0, "Place NFC card...");
    }
}

/* ---- main ---- */
int main(void)
{
    uint8_t  nfc_uid[4];
    int8_t   nfc_ret;
    uint8_t  prev_card = 0;
    uint8_t  can_tx_buf[8];
    uint8_t  high_temp = 0;
    uint32_t scan_tick = 0;

    SysTick_Init();
    UART2_Init(115200);
    CAN1_Init();
    DHT11_Init();
    KEY_LED_Init();
    BEEP_Init();

    printf("\r\n==== Multi-Sensor Experiment ====\r\n");
    printf("Init LCD...\r\n");
    LCD_Init();
    LCD_SetFontEN(&ASCII_8x16);
    printf("  LCD OK\r\n");

    /* Welcome */
    LCD_SetBackColor(BLACK);
    LCD_SetTextColor(CYAN);
    LCD_Clear(0, 0, LCD_GetLenX(), LCD_GetLenY());
    LCD_SetTextColor(YELLOW);
    LCD_DispStringEN(20, LINE_EN(4), 0, "Multi-Sensor");
    LCD_DispStringEN(35, LINE_EN(5), 0, "Experiment");
    LCD_SetTextColor(WHITE);
    LCD_DispStringEN(25, LINE_EN(7), 0, "DHT11 + NFC + LCD");
    delay_ms(1500);

    /* ---- NFC Init (与参考代码一致) ---- */
    printf("Init NFC (UART1, 115200)...\r\n");
    NFC_Init(115200);
    {
        extern uint32_t MAX_TRY;
        MAX_TRY = 15;
        if(NFC_WakeUp() < 0)
        {
            printf("NFC Init FAILED\r\n");
            MAX_TRY = 0;
        }
        else
        {
            printf("NFC Init OK\r\n");
            MAX_TRY = 0;
        }
    }

    printf("==== Monitor Mode: DHT11+NFC+LCD ====\r\n");
    printf("NFC: Waiting for card...\r\n");
    LCD_ShowSensor(0, NULL, 0);

    while(1)
    {
        /* ---- DHT11 ---- */
        dht_ret = DHT11_ReadData(&dht_data);
        if(dht_ret == 0)
        {
            sensor_buf.temp = dht_data.temp_int;
            sensor_buf.hum  = dht_data.humi_int;
        }

        /* ---- Key ---- */
        sensor_buf.key_flag = KEY_Scan();

        /* ---- Temp threshold ---- */
        high_temp = (dht_data.temp_int >= 28 ||
                    (dht_data.temp_int == 27 && dht_data.temp_deci >= 5));

        /* ---- LED / Buzzer ---- */
        if(high_temp)
            GPIO_ResetBits(GPIOB, GPIO_Pin_5);
        else
            GPIO_SetBits(GPIOB, GPIO_Pin_5);

        if(high_temp)
            BEEP_ON();
        else
            BEEP_OFF();

        /* ---- Serial print ---- */
        printf("T:%d.%dC H:%d.%d%% Key:%s\r\n",
               dht_data.temp_int, dht_data.temp_deci,
               dht_data.humi_int, dht_data.humi_deci,
               sensor_buf.key_flag ? "ON" : "OFF");

        /* ---- CAN TX ---- */
        can_tx_buf[0] = sensor_buf.temp;
        can_tx_buf[1] = sensor_buf.hum;
        can_tx_buf[2] = sensor_buf.key_flag;
        can_tx_buf[3] = dht_data.temp_deci;
        can_tx_buf[4] = dht_data.humi_deci;
        can_tx_buf[5] = 0;
        can_tx_buf[6] = 0;
        can_tx_buf[7] = 0;
        CAN1_SendMsg(0x1234, can_tx_buf, 8);

        /* ---- NFC Scan (~300ms interval) ---- */
        scan_tick++;
        if(scan_tick >= 3)
        {
            scan_tick = 0;
            nfc_ret = NFC_ScanCard(nfc_uid);

            if(nfc_ret == 1)
            {
                if(!prev_card)
                {
                    printf("=== CARD! UID:%02X%02X%02X%02X ===\r\n",
                           nfc_uid[0],nfc_uid[1],nfc_uid[2],nfc_uid[3]);
                }
                LCD_ShowSensor(1, nfc_uid, high_temp);
                prev_card = 1;
            }
            else
            {
                if(prev_card)
                {
                    printf("NFC: card removed\r\n");
                    prev_card = 0;
                }
                LCD_ShowSensor(0, NULL, high_temp);
            }
        }

        delay_ms(100);
    }
}
