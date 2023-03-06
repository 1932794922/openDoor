#ifndef __ESP8266_H
#define __ESP8266_H

#include "main.h"

#define ESP_UART    huart2 //串口2

//帧头
#define FRAME_HEAD 0x4D  //M
//帧尾
#define FRAME_TAIL  0x51 //Q
// 默认发送主题
#define  DEFAULT_PUB_TOPIC "esp8266/pub/door"
// 默认接收主题
#define  DEFAULT_SUB_TOPIC "esp8266/sub/door"

#define TYPE_COMMAND  "C"  //指令
#define TYPE_FINGERTIP "F"   //指纹
#define TYPE_RFID "R"   //刷卡
#define TYPE_PASSWORD "P"   //密码



void sendMQTT(char *data);
void sendMQTTHaveTopic(char *topic, char *data);

#endif