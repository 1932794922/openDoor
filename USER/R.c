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
void parseRX(uint8_t date, uint8_t hand, uint8_t end, RXBuffer *rBuffer) {
    // 数据1
    if (date == hand)   //帧头
    {
        rBuffer->flagReceiveStart = 1;
        rBuffer->flagReceiveFinish = 0;
    } else if ((rBuffer->flagReceiveStart == 1) && (rBuffer->flagReceiveFinish == 0))  //开始接收
    {
        if (date != end)   //帧尾
        {
            rBuffer->buffer[rBuffer->buffer_length++] = date;
        } else {
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
/**
 * 串口接收数据
 * @param data 串口数据帧数据
 */
void dateRX(uint8_t data) {
    parseRX(data, FRAME_HEAD, FRAME_TAIL, &rBuffer);
    // 数据接收完成
    if (rBuffer.flagReceiveFinish) {
        //usart_printf(&huart1, "%s\r\n", rBuffer.buffer);
        rBuffer.flagReceiveFinish = 0;
        cJSON *json = cJSON_Parse((char *) rBuffer.buffer);
        char *dataString = cJSON_GetObjectItem(json, "data")->valuestring;
        if (dataString != NULL) {
            openDoorForMQTT(dataString);
        }

        cJSON_Delete(json);
        //清空数组
        memset(rBuffer.buffer, 0, sizeof(rBuffer.buffer));

    }

}

