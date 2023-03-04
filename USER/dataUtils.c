#include "dataUtils.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

/*****************************************
 * ��������	Split_Str	����ַ���
 * ��	 �ܣ�	��������ַ�����ֳɼ����������ַ���
 * ��	 �Σ�	*oldstr	->	��Ҫ��ֵ��ַ���
                len	->	��Ҫ��ֵ��ַ����ĳ���
                cha	->	�ο���Ŀ���ַ����Ը��ַ�����֣�
                newstr[10][20]	->	���ز�ֺ�����飨���ɽ���10���ַ�������ÿ���ַ���������20���ַ���
                *newnumb	->	��ֺ��ж��ٸ����ַ���
 * ����ֵ��	�β���ʽ���أ��趨���ά���������ղ�ֺ������
******************************************/

void Split_Str(uint8_t *oldstr, uint8_t len, uint8_t cha, uint8_t newstr[10][20], uint8_t *newnumb)			//����ַ���
{
    uint8_t i = 0;				//���ڱ�������
    uint8_t loca[10] = {0};		//λ�ã����ɱ�ʾ10λ�ã�
    uint8_t num = 0;			//Ŀ���ַ��ĸ���
    uint8_t n = 0;

    for(i = 0; i < len; i++)
    {
        if(oldstr[i] == cha)		//�ҵ���Ŀ���ַ�
            loca[num++] = i;
    }

    if(loca[0] != 0)				//���ָ��ַ����ڵ�һλ
    {
        memcpy(newstr[n++], oldstr, loca[0]);		//���Ŀ���ַ�ǰ���ַ���
        memset(newstr[n - 1] + loca[0], '\0', 1);
        *newnumb = num + 1;						//���ַ��ĸ���
    }
    else			//�ָ��ַ��ڵ�һλ
        *newnumb = num;

    loca[num++] = len - 1;			//���ַ������ȷ���λ����������һλ

    for(i = 1; i < num; i++)
    {
        if(loca[i + 1] > loca[i])	//�жϺ�һ��λ���Ƿ���ֵ
        {
            memcpy(newstr[n++], oldstr + loca[i - 1] + 1, loca[i] - loca[i - 1] - 1);		//����м�����
            memset(newstr[n - 1] + loca[i] - loca[i - 1] - 1, '\0', 1);
        }
        else
        {
            memcpy(newstr[n++], oldstr + loca[i - 1] + 1, len - loca[i - 1] - 1);			//�����������
            memset(newstr[n - 1] + len - loca[i - 1] - 1, '\0', 1);
        }
    }
}

/*****************************************
 * ��������	StrToIntFloatValue	���ַ���ת��Ϊ��ֵ���������Σ������ͣ�
 * ��	 �ܣ�	��������ַ���ת��Ϊ�����ŵ�int(float)����
 * ��	 �Σ�	*str	->	��Ҫת�����ַ���
                __STR *mystr	->	���ص����ݣ��ýṹ��������κ͸��������ݣ�
 * ����ֵ��	�β���ʽ���أ��趨��ṹ���������
******************************************/
void StrToIntFloatValue(uint8_t *str, __STR *mystr)	//���ַ���ת��Ϊ��ֵ������С����������
{
    uint8_t i = 0;
    uint8_t Int_num = 0;
    int32_t Intbuff = 0;			//��������������������
    float Float_Loca = 0;			//���������ݱ�ʶ����
    uint8_t len = strlen((char*)str);		//��ȡ����

    if(str[0] >= '0' && str[0] <= '9')				//����
    {
        for(i = 0; i < len; i++)
        {
            if(str[i] >= '0' && str[i] <= '9')		//��������
            {
                Intbuff += (str[i] - '0') * pow(10, len - Int_num - 1);		//ת������
                Int_num++;
            }
            else if(str[i] == '.')			//��������
            {
                Float_Loca = i;			//��¼λ��С����
            }
        }
    }
    else if(str[0] == '-')		//����
    {
        for(i = 1; i < len; i++)
        {
            if(str[i] >= '0' && str[i] <= '9')
            {
                Intbuff += (str[i] - '0') * pow(10, len - Int_num - 2);		//ת������
                Int_num++;
            }
            else if(str[i] == '.')			//��������
            {
                Float_Loca = i;			//��¼λ��С����
            }
        }

        Intbuff = -Intbuff;			//����ȡ��
    }

    if(Float_Loca)		//�ж��Ƿ��и���������
        mystr->Floatvalue = (float)Intbuff / (float)pow(10, len - Float_Loca);		//���㸡��������
    else
        mystr->Intvalue = (int)Intbuff;
}

