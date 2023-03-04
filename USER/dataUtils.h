#ifndef __STRUTILSL_H
#define __STRUTILSL_H

#include "usart.h"

#define m 	0		//������������
#define mn 	0		//��������������
#define mc 	0		//һ������һ������������

typedef struct
{
    uint8_t NewStrNum;				//��ֺ���ַ�������
    uint8_t newstr[10][20];		    //�����ֺ���ַ���
    int32_t Intvalue;					//ת�������������
    float 	Floatvalue;				//ת����ĸ���������
} __STR;


void Split_Str(uint8_t *oldstr, uint8_t len, uint8_t cha, uint8_t newstr[10][20], uint8_t *newnumb);			//����ַ���
void StrToIntFloatValue(uint8_t *str, __STR *mystr);	//���ַ���ת��Ϊ��ֵ������С����������

#endif
