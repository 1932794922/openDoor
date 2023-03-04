#include "AS608HARD.h"
#include "usart.h"
#include "delay.h"
#include "as608.h"
#include "lcd.h"
#include "touch.h"


uint8_t **kbd_tbl;

//����״̬����
//x,y:��������
//key:��ֵ(0~14)
//sta:״̬��0���ɿ���1�����£�
void AS608_key_staset(uint16_t x, uint16_t y, uint8_t keyx, uint8_t sta)
{
	uint16_t i = keyx / 3, j = keyx % 3;
	if (keyx > 16)
		return;
	if (sta && keyx != 1) //�������2����:������Ҫ���
		LCD_Fill(x + j * 80 + 1, y + i * 30 + 1, x + j * 80 + 78, y + i * 30 + 28, GREEN);
	else if (keyx != 1)
		LCD_Fill(x + j * 80 + 1, y + i * 30 + 1, x + j * 80 + 78, y + i * 30 + 28, BLACK);
	if (keyx != 1) //���ǡ�����
		Show_Str_Mid(x + j * 80, y + 7 + 30 * i, (uint8_t *)kbd_tbl[keyx], 16, 80);
}

//��ȡ������ֵ
uint16_t GET_NUM(void)
{
	uint8_t key_num = 0;
	uint16_t num = 0;
	while (1)
	{
		key_num = AS608_get_keynum(0, 170);
		if (key_num)
		{
			if (key_num == 1)
				return 0xFFFF; //�����ء���
			if (key_num == 3)
				return 0xFF00;							 //
			if (key_num > 3 && key_num < 13 && num < 99) //��1-9����(��������3λ��)
				num = num * 10 + key_num - 3;
			if (key_num == 13)
				num = num / 10; //��Del����
			if (key_num == 14 && num < 99)
				num = num * 10; //��0����
			if (key_num == 15)
				return num; //��Enter����
		}
		LCD_ShowNum(80 + 15, 170 + 7, num, 6, 16, RED, BLACK);
	}
}


//�õ�������������
//x,y:��������
//����ֵ:��1~15,��Ӧ������
uint8_t AS608_get_keynum(uint16_t x, uint16_t y)
{
	uint16_t i, j;
	uint8_t key_x = 0; //0,û���κΰ�������
	uint8_t key = 0;
	tp_dev.scan(0);
	if (tp_dev.sta & TP_PRES_DOWN) //������������
		for (i = 0; i < 5; i++)
		{
			for (j = 0; j < 3; j++)
			{
				if (tp_dev.x[0] < (x + j * 80 + 80) && tp_dev.x[0] > (x + j * 80) && tp_dev.y[0] < (y + i * 30 + 30) && tp_dev.y[0] > (y + i * 30))
				{
					key = i * 3 + j + 1; //�ڼ�������
					while (tp_dev.x[0] < (x + j * 80 + 80) && tp_dev.x[0] > (x + j * 80) && tp_dev.y[0] < (y + i * 30 + 30) && tp_dev.y[0] > (y + i * 30)) //���ּ��
					{
						tp_dev.scan(0);
					}
					break;
				}
			}
			if (key)
			{
				if (key_x == key)
					key = 0;
				else
				{
					AS608_key_staset(x, y, key_x - 1, 0);
					key_x = key;
					AS608_key_staset(x, y, key_x - 1, 1);
				}
				break;
			}
		}
	{
		AS608_key_staset(x, y, key_x - 1, 0);
		key_x = 0;
	}
	return key;
}
