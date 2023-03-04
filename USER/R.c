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
    date --> 数据接收源
    hand --> 帧头
    end  --> 帧尾
    RXBuffer --> 结构体
        ----
            uint8_t buffer[LONG];//数据容器
            uint8_t buffer_length ;   //缓存数据长度
            uint8_t flagReceiveStart ; //数据状态标志  1开始接收 0结束
            uint8_t flagReceiveFinish;          // 数据接收完成标志  0 未完成 1 完成
    // #协议:  帧头 + 数据 + 帧尾  解析响应体
    M{"code": 200,"message":"ok","data":"open"}Q
    rBuffer = parseRX(data, '{', '}', rBuffer);
*/
void parseRX(uint8_t date, uint8_t hand, uint8_t end, RXBuffer *rBuffer)
{
    // 数据1
    if(date == hand)   //帧头
    {
        rBuffer->flagReceiveStart = 1;
        rBuffer->flagReceiveFinish = 0;
    }
    else if((rBuffer->flagReceiveStart == 1) && (rBuffer->flagReceiveFinish == 0))  //开始接收
    {
        if(date != end)   //帧尾
        {
            rBuffer->buffer[rBuffer->buffer_length++] = date;
        }
        else
        {
            rBuffer->flagReceiveFinish = 1;   //接收完成ok
            rBuffer->buffer_length = 0;    //重置长度
            rBuffer->flagReceiveStart = 0; //消除等待下一帧数据
        }
    }
}



//数据接收
/**
参数: data 串口数据
返回值:无
*/

// 接收mqtt数据
RXBuffer rBuffer;
void dateRX(uint8_t data)
{
    parseRX(data, 'M', 'Q', &rBuffer);

    if(rBuffer.flagReceiveFinish)
    {
        usart_printf(&huart1, "%s\r\n", rBuffer.buffer);
        //清空数组
        rBuffer.flagReceiveFinish = 0;
        RData((char *)rBuffer.buffer);
        memset(rBuffer.buffer, 0, sizeof(rBuffer.buffer));

    }

}

