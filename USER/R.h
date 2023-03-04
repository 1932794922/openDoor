#ifndef __R_H
#define __R_H

#include "stm32f4xx_hal.h"


#define LONG 255

typedef struct R
{
    int code;

    //��Ϣ
    char* message;

    //����
    char* dataStr;

    int dataInt;

} R;


typedef struct RXBuffer
{
    uint8_t buffer[LONG];//��������
    uint8_t buffer_length ;   //�������ݳ���
    uint8_t flagReceiveStart ; //����״̬��־  1��ʼ���� 0����
    uint8_t flagReceiveFinish;          // ���ݽ�����ɱ�־  0 δ��� 1 ���
} RXBuffer;

R RData(char *json_string);
void parseRX(uint8_t date, uint8_t hand, uint8_t end, RXBuffer *rBuffer);
void dateRX(uint8_t data);
#endif

