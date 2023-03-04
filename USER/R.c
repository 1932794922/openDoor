#include "R.h"
#include <stdio.h>
#include <string.h>
#include "cJSON.h"
#include "R.h"
#include "usartMy.h"
#include "delay.h"


R RData(char *json_string)
{
    R r;
    cJSON *json = cJSON_Parse(json_string);

    if (json == NULL)
    {
        usart_printf(&huart1, "Failed to parse JSON: %s\r\n", cJSON_GetErrorPtr());
        return r;
    }

    r.code = cJSON_GetObjectItem(json, "code")->valueint;
    r.message = cJSON_GetObjectItem(json, "message")->valuestring;
    r.dataStr = cJSON_GetObjectItem(json, "data")->valuestring;
    usart_printf(&huart1, "code: %d\r\n", r.code);
    usart_printf(&huart1, "message: %s\r\n", r.message);
    usart_printf(&huart1, "data: %s\r\n",  r.dataStr);
    cJSON_Delete(json);
    return r;
}



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
void parseRX(uint8_t date, uint8_t hand, uint8_t end, RXBuffer *rBuffer)
{
    // ����1
    if(date == hand)   //֡ͷ
    {
        rBuffer->flagReceiveStart = 1;
        rBuffer->flagReceiveFinish = 0;
    }
    else if((rBuffer->flagReceiveStart == 1) && (rBuffer->flagReceiveFinish == 0))  //��ʼ����
    {
        if(date != end)   //֡β
        {
            rBuffer->buffer[rBuffer->buffer_length++] = date;
        }
        else
        {
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
void dateRX(uint8_t data)
{
    parseRX(data, 'M', 'Q', &rBuffer);

    if(rBuffer.flagReceiveFinish)
    {
        usart_printf(&huart1, "%s\r\n", rBuffer.buffer);
        //�������
        rBuffer.flagReceiveFinish = 0;
        RData((char *)rBuffer.buffer);
        memset(rBuffer.buffer, 0, sizeof(rBuffer.buffer));

    }

}

