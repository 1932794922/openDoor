#include "funHandle.h"
#include "string.h"
#include "main.h"
#include "ESP8266.h"
#include <stdio.h>
#include "lcd.h"
#include "usartMy.h"

//���Ŷ���
void openDoorForMQTT(char *data) {
    if (strcmp(data, "open") == 0) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
        uint8_t str[32];
        sprintf((char *) str, "ָ����ɹ�");
        LCD_Fill(0, 100, lcddev.width, 160, BLACK);
        Show_Str_Mid(0, 100, (uint8_t *) str, 16, 240);
        //����MQTT��������
        usart_printf_dma(&ESP_UART,
                         "%c{\"topic\":\"%s\",\"data\":{\"message\":\"open the door\",\"status\":1,\"type\":\"%s\"}}%c",
                         FRAME_HEAD, DEFAULT_PUB_TOPIC, TYPE_COMMAND,FRAME_TAIL);
    } else if (strcmp(data, "close") == 0) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
        uint8_t str[32];
        sprintf((char *) str, "ָ��رճɹ�");
        LCD_Fill(0, 100, lcddev.width, 160, BLACK);
        Show_Str_Mid(0, 100, (uint8_t *) str, 16, 240);
        //����MQTT��������
        usart_printf_dma(&ESP_UART,
                         "%c{\"topic\":\"%s\",\"data\":{\"message\":\"close the door\",\"status\":0,\"type\":\"%s\"}}%c",
                         FRAME_HEAD, DEFAULT_PUB_TOPIC, TYPE_COMMAND,FRAME_TAIL);
    }
}