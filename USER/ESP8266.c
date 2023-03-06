#include "ESP8266.h"
#include "usart.h"
#include "stdio.h"
#include "stdlib.h"
#include "usartMy.h"
#include "as608.h"
#include "delay.h"

/**
 * ����MQTT���� JSON�ַ�����ʽ �Զ�������
 * @param topic
 * @param data
 */
void sendMQTTHaveTopic(char *topic, char *data) {
    // �ж�data�Ĵ�С�Ƿ���Ҫ�ְ�
    usart_printf_dma(&ESP_UART, "%c{\"topic\":\"%s\",\"code\":200", FRAME_HEAD, topic);
    delay_ms(10);
    usart_printf_dma(&ESP_UART, ",\"data\":%s }%c", data, FRAME_TAIL);

}

/**
 * ����MQTT���� JSON�ַ�����ʽ Ĭ������
 * @param data
 */
void sendMQTT(char *data) {
    usart_printf_dma(&ESP_UART, "%c{%s}%c", FRAME_HEAD, DEFAULT_PUB_TOPIC, data, FRAME_TAIL);
}