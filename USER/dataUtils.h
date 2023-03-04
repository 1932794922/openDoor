#ifndef __STRUTILSL_H
#define __STRUTILSL_H

#include "usart.h"

#define m 	0		//两个整形数据
#define mn 	0		//两个浮点型数据
#define mc 	0		//一个整形一个浮点型数据

typedef struct
{
    uint8_t NewStrNum;				//拆分后的字符串个数
    uint8_t newstr[10][20];		    //储存拆分后的字符串
    int32_t Intvalue;					//转换后的整形数据
    float 	Floatvalue;				//转换后的浮点型数据
} __STR;


void Split_Str(uint8_t *oldstr, uint8_t len, uint8_t cha, uint8_t newstr[10][20], uint8_t *newnumb);			//拆分字符串
void StrToIntFloatValue(uint8_t *str, __STR *mystr);	//把字符串转换为数值（不分小数和整数）

#endif
