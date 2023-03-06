#ifndef __ESP8266_H
#define __ESP8266_H

#include "main.h"

#define ESP_UART    huart2 //����2

//֡ͷ
#define FRAME_HEAD 0x4D  //M
//֡β
#define FRAME_TAIL  0x51 //Q
// Ĭ�Ϸ�������
#define  DEFAULT_PUB_TOPIC "esp8266/pub/door"
// Ĭ�Ͻ�������
#define  DEFAULT_SUB_TOPIC "esp8266/sub/door"

#define TYPE_COMMAND  "C"  //ָ��
#define TYPE_FINGERTIP "F"   //ָ��
#define TYPE_RFID "R"   //ˢ��
#define TYPE_PASSWORD "P"   //����



void sendMQTT(char *data);
void sendMQTTHaveTopic(char *topic, char *data);

#endif