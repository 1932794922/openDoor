#include "ESP8266.h"
#include "usart.h"
#include "stdio.h"
#include "stdlib.h"
#include "usartMy.h"
#include "as608.h"
#include "delay.h"

/**
 * 发送MQTT数据 JSON字符串格式 自定义主题
 * @param topic
 * @param data
 */
void sendMQTTHaveTopic(char *topic, char *data) {
    // 判断data的大小是否需要分包
    usart_printf_dma(&ESP_UART, "%c{\"topic\":\"%s\",\"code\":200", FRAME_HEAD, topic);
    delay_ms(10);
    usart_printf_dma(&ESP_UART, ",\"data\":%s }%c", data, FRAME_TAIL);

}

/**
 * 发送MQTT数据 JSON字符串格式 默认主题
 * @param data
 */
void sendMQTT(char *data) {
    usart_printf_dma(&ESP_UART, "%c{%s}%c", FRAME_HEAD, DEFAULT_PUB_TOPIC, data, FRAME_TAIL);
}