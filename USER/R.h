#ifndef __R_H
#define __R_H

#include "stm32f4xx_hal.h"


#define LONG 255

typedef struct R
{
    int code;

    //消息
    char* message;

    //数据
    char* dataStr;

    int dataInt;

} R;


typedef struct RXBuffer
{
    uint8_t buffer[LONG];//数据容器
    uint8_t buffer_length ;   //缓存数据长度
    uint8_t flagReceiveStart ; //数据状态标志  1开始接收 0结束
    uint8_t flagReceiveFinish;          // 数据接收完成标志  0 未完成 1 完成
} RXBuffer;

R RData(char *json_string);
void parseRX(uint8_t date, uint8_t hand, uint8_t end, RXBuffer *rBuffer);
void dateRX(uint8_t data);
#endif

