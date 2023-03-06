#include "usartMy.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "stdio.h"
#include "usart.h"
#include <stdint.h>
#include "R.h"

uint8_t huart1_buf;	    //串口1接收中断缓冲
uint8_t huart2_buf;	   //串口2接收中断缓冲
uint8_t huart3_buf;	    //串口3接收中断缓冲

//int fputc(int ch, FILE *f)
//{
//    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
//    return ch;
//}


//int fgetc(FILE *f)
//{
//    uint8_t ch = 0;
//    HAL_UART_Receive(&huart1, &ch, 1, 0xffff);
//    return ch;
//}

//函数名：usart_printf
//功能：串口1发送
//返回值：无
/*********************************************************************************/
void usart_printf(UART_HandleTypeDef *handleTypeDef, const char *format, ...)
{
    va_list arg;
    static char SendBuffer[255] = {0};

    while (__HAL_UART_GET_FLAG(handleTypeDef, UART_FLAG_TC) == RESET);

    va_start(arg, format);
    vsprintf(SendBuffer, format, arg);
    va_end(arg);
    HAL_UART_Transmit(handleTypeDef, (uint8_t *)SendBuffer, strlen(SendBuffer), 0xFFFF);
}

void usart_printf_dma(UART_HandleTypeDef *handleTypeDef, const char *format, ...)
{
    va_list arg;
    static char SendBuffer[255] = {0};
    while (__HAL_UART_GET_FLAG(handleTypeDef, UART_FLAG_TC) == RESET);
    va_start(arg, format);
    vsprintf(SendBuffer, format, arg);
    va_end(arg);
    HAL_UART_Transmit_DMA(handleTypeDef, (uint8_t *)SendBuffer, strlen(SendBuffer));
}


/*
    date --> 数据接收源
    dataBuffer -->数据存放缓存
    *length  --> 数据
    hand --> 帧头
    end  --> 帧尾
    *flagReceiveStart --> 开始接收标志
    *flagReceiveFinish --> 接收完成标志

    // #协议: N 帧头 + 方向 +状态(1成功 0失败) + B 帧尾
    communication(Date,number,&number_length,'N','B',&number_state,&number_ok);
*/
uint8_t* communication(uint8_t date, uint8_t dataBuffer[], uint8_t *length, uint8_t hand, uint8_t end, uint8_t *flagReceiveStart, uint8_t *flagReceiveFinish)
{
    // 数据1
    if(date == hand)   //帧头
    {
        *flagReceiveStart = 1;
    }
    else if((*flagReceiveStart == 1) && (*flagReceiveFinish == 0))  //开始接收
    {
        if(date != end)   //帧尾B
        {
            dataBuffer[(*length)++] = date;
        }
        else
        {
            *flagReceiveFinish = 1;   //接收完成ok
            *length = 0;    //重置长度
            *flagReceiveStart = 0; //消除等待下一帧数据
        }
    }

    return dataBuffer;
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

    if (huart->Instance == USART1)
    {
        //dateRX(huart1_buf);
        //usart_printf(&huart1, "%c\r\n", huart1_buf);
    }


    //串口2
    if (huart->Instance == USART2)
    {

        dateRX(huart2_buf);

    }


    //串口3
    if (huart->Instance == USART3)
    {

        //usart_printf(&huart1, "%s\r\n", USART3->DR);

    }


    HAL_UART_Receive_IT(&huart1, (uint8_t *)&huart1_buf, 1);
    HAL_UART_Receive_DMA(&huart2, (uint8_t *)&huart2_buf, 1);
    HAL_UART_Receive_IT(&huart3, (uint8_t *)&huart3_buf, 1);
}



