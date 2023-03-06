#ifndef __USARTMY_H
#define __USARTMY_H

#include "main.h"
#include "usart.h"

extern uint8_t huart1_buf;	    //串口1接收中断缓冲
extern uint8_t huart2_buf;	    //串口2接收中断缓冲
extern uint8_t huart3_buf;	    //串口3接收中断缓冲


void usart_printf(UART_HandleTypeDef *handleTypeDef, const char *format, ...);
uint8_t* parse(uint8_t date, uint8_t dataBuffer[], uint8_t *length, uint8_t hand, uint8_t end, uint8_t *flagReceiveStart, uint8_t *flagReceiveFinish);
uint8_t* communication(uint8_t date, uint8_t dataBuffer[], uint8_t *length, uint8_t hand, uint8_t end, uint8_t *flagReceiveStart, uint8_t *flagReceiveFinish);
void usart_printf_dma(UART_HandleTypeDef *handleTypeDef, const char *format, ...);


#endif



