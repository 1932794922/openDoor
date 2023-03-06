#include "R.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "cJSON.h"
#include "usartMy.h"
#include "funHandle.h"
#include "ESP8266.h"
#include "lcd.h"
/*
    date --> ���ݽ���Դ
    hand --> ֡ͷ
    end  --> ֡β
    RXBuffer --> �ṹ��
        ----
            uint8_t buffer[LONG];//��������
            uint8_t buffer_length ;   //�������ݳ���
            uint8_t flagReceiveStart ; //����״̬��־  1��ʼ���� 0����
            uint8_t flagReceiveFinish;          // ���ݽ�����ɱ�־  0 δ��� 1 ���
    // #Э��:  ֡ͷ + ���� + ֡β  ������Ӧ��
    M{"code": 200,"message":"ok","data":"open"}Q
    rBuffer = parseRX(data, '{', '}', rBuffer);
*/
void parseRX(uint8_t date, uint8_t hand, uint8_t end, RXBuffer *rBuffer) {
    // ����1
    if (date == hand)   //֡ͷ
    {
        rBuffer->flagReceiveStart = 1;
        rBuffer->flagReceiveFinish = 0;
    } else if ((rBuffer->flagReceiveStart == 1) && (rBuffer->flagReceiveFinish == 0))  //��ʼ����
    {
        if (date != end)   //֡β
        {
            rBuffer->buffer[rBuffer->buffer_length++] = date;
        } else {
            rBuffer->flagReceiveFinish = 1;   //�������ok
            rBuffer->buffer_length = 0;    //���ó���
            rBuffer->flagReceiveStart = 0; //�����ȴ���һ֡����
        }
    }
}



//���ݽ���
/**
����: data ��������
����ֵ:��
*/

// ����mqtt����
RXBuffer rBuffer;
/**
 * ���ڽ�������
 * @param data ��������֡����
 */
void dateRX(uint8_t data) {
    parseRX(data, FRAME_HEAD, FRAME_TAIL, &rBuffer);
    // ���ݽ������
    if (rBuffer.flagReceiveFinish) {
        //usart_printf(&huart1, "%s\r\n", rBuffer.buffer);
        rBuffer.flagReceiveFinish = 0;
        cJSON *json = cJSON_Parse((char *) rBuffer.buffer);
        char *dataString = cJSON_GetObjectItem(json, "data")->valuestring;
        if (dataString != NULL) {
            openDoorForMQTT(dataString);
        }

        cJSON_Delete(json);
        //�������
        memset(rBuffer.buffer, 0, sizeof(rBuffer.buffer));

    }

}

