#include "dataUtils.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

/*****************************************
 * 函数名：	Split_Str	拆分字符串
 * 功	 能：	把输入的字符串拆分成几个单独的字符串
 * 形	 参：	*oldstr	->	需要拆分的字符串
                len	->	需要拆分的字符串的长度
                cha	->	参考的目标字符（以该字符来拆分）
                newstr[10][20]	->	返回拆分后的数组（最大可接收10个字符串，且每个字符串最大可有20个字符）
                *newnumb	->	拆分后有多少个字字符串
 * 返回值：	形参形式返回，需定义二维数组来接收拆分后的数据
******************************************/

void Split_Str(uint8_t *oldstr, uint8_t len, uint8_t cha, uint8_t newstr[10][20], uint8_t *newnumb)			//拆分字符串
{
    uint8_t i = 0;				//用于遍历数组
    uint8_t loca[10] = {0};		//位置（最多可表示10位置）
    uint8_t num = 0;			//目标字符的个数
    uint8_t n = 0;

    for(i = 0; i < len; i++)
    {
        if(oldstr[i] == cha)		//找到了目标字符
            loca[num++] = i;
    }

    if(loca[0] != 0)				//若分割字符不在第一位
    {
        memcpy(newstr[n++], oldstr, loca[0]);		//拆分目标字符前的字符串
        memset(newstr[n - 1] + loca[0], '\0', 1);
        *newnumb = num + 1;						//新字符的个数
    }
    else			//分割字符在第一位
        *newnumb = num;

    loca[num++] = len - 1;			//把字符串长度放在位置数组的最后一位

    for(i = 1; i < num; i++)
    {
        if(loca[i + 1] > loca[i])	//判断后一个位置是否有值
        {
            memcpy(newstr[n++], oldstr + loca[i - 1] + 1, loca[i] - loca[i - 1] - 1);		//拆分中间数据
            memset(newstr[n - 1] + loca[i] - loca[i - 1] - 1, '\0', 1);
        }
        else
        {
            memcpy(newstr[n++], oldstr + loca[i - 1] + 1, len - loca[i - 1] - 1);			//拆分最后的数据
            memset(newstr[n - 1] + len - loca[i - 1] - 1, '\0', 1);
        }
    }
}

/*****************************************
 * 函数名：	StrToIntFloatValue	把字符串转换为数值（包括整形，浮点型）
 * 功	 能：	把输入的字符串转换为带符号的int(float)数据
 * 形	 参：	*str	->	需要转换的字符串
                __STR *mystr	->	返回的数据（用结构体接收整形和浮点型数据）
 * 返回值：	形参形式返回，需定义结构体接收数据
******************************************/
void StrToIntFloatValue(uint8_t *str, __STR *mystr)	//把字符串转换为数值（不分小数和整数）
{
    uint8_t i = 0;
    uint8_t Int_num = 0;
    int32_t Intbuff = 0;			//缓存计算出来的整形数据
    float Float_Loca = 0;			//浮点型数据标识计算
    uint8_t len = strlen((char*)str);		//获取长度

    if(str[0] >= '0' && str[0] <= '9')				//正数
    {
        for(i = 0; i < len; i++)
        {
            if(str[i] >= '0' && str[i] <= '9')		//整形数据
            {
                Intbuff += (str[i] - '0') * pow(10, len - Int_num - 1);		//转换数据
                Int_num++;
            }
            else if(str[i] == '.')			//浮点数据
            {
                Float_Loca = i;			//记录位置小数点
            }
        }
    }
    else if(str[0] == '-')		//负数
    {
        for(i = 1; i < len; i++)
        {
            if(str[i] >= '0' && str[i] <= '9')
            {
                Intbuff += (str[i] - '0') * pow(10, len - Int_num - 2);		//转换数据
                Int_num++;
            }
            else if(str[i] == '.')			//浮点数据
            {
                Float_Loca = i;			//记录位置小数点
            }
        }

        Intbuff = -Intbuff;			//负数取反
    }

    if(Float_Loca)		//判断是否有浮点型数据
        mystr->Floatvalue = (float)Intbuff / (float)pow(10, len - Float_Loca);		//计算浮点型数据
    else
        mystr->Intvalue = (int)Intbuff;
}

