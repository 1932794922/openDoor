#ifndef __USARTMY_H
#define __USARTMY_H

#include "main.h"
#include "usart.h"

extern uint8_t huart1_buf;	    //����1�����жϻ���
extern uint8_t huart2_buf;	    //����2�����жϻ���
extern uint8_t huart3_buf;	    //����3�����жϻ���


void usart_printf(UART_HandleTypeDef *handleTypeDef, const char *format, ...);
uint8_t* parse(uint8_t date, uint8_t dataBuffer[], uint8_t *length, uint8_t hand, uint8_t end, uint8_t *flagReceiveStart, uint8_t *flagReceiveFinish);
uint8_t* communication(uint8_t date, uint8_t dataBuffer[], uint8_t *length, uint8_t hand, uint8_t end, uint8_t *flagReceiveStart, uint8_t *flagReceiveFinish);
void usart_printf_dma(UART_HandleTypeDef *handleTypeDef, const char *format, ...);


#endif



