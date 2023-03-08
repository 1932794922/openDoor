#include "ui.h"
#include "as608.h"
#include "stdio.h"
#include "lcd.h"
#include "AS608HARD.h"
#include "funHandle.h"

const char *kbd_menu[] =
        {
                "ɾָ��",
                "ID:",
                "¼ָ��",
                "1",
                "2",
                "3",
                "4",
                "5",
                "6",
                "7",
                "8",
                "9",
                "DEL",
                "0",
                "Enter",
        }; //������

const char *kbd_delFR[] = {
        "����",
        "ID:",
        "���ָ��",
        "1",
        "2",
        "3",
        "4",
        "5",
        "6",
        "7",
        "8",
        "9",
        "DEL",
        "0",
        "Enter",
}; //������

uint8_t rC522Flag = 3;

void LCD_RC522_Scan(void) {
    tp_dev.scan(0);
    if (tp_dev.sta & TP_PRES_DOWN)            //������������
    {
        if (tp_dev.x[0] < 240 && tp_dev.y[0] < 320)    //�жϰ��������Ƿ�����ʾ����
        {
            if (tp_dev.x[0] > 180 && tp_dev.x[0] < 240 && tp_dev.y[0] > 20 && tp_dev.y[0] < 60) {
                //�����¼��
                LCD_Clear(BLACK);//����
                LCD_ShowString(54, 0, (uint8_t *) "----����¼��----", RED, BLACK);
                LCD_DrawRectangle(100, 20, 160, 60, RED);
                LCD_ShowString(115, 32, (uint8_t *) "HOME", RED, BLACK);
                rC522Flag = 1; //ִ��¼��
                while (tp_dev.x[0] > 180 && tp_dev.x[0] < 240 && tp_dev.y[0] > 20 && tp_dev.y[0] < 60) //���ּ��
                {
                    tp_dev.scan(0);
                }
            }

            if (tp_dev.x[0] > 0 && tp_dev.x[0] < 60 && tp_dev.y[0] > 20 && tp_dev.y[0] < 60) {
                //�����ɾ��
                LCD_Clear(BLACK);//����
                LCD_ShowString(54, 0, (uint8_t *) "----����ɾ��----", RED, BLACK);
                LCD_DrawRectangle(100, 20, 160, 60, RED);
                LCD_ShowString(115, 32, (uint8_t *) "HOME", RED, BLACK);
                rC522Flag = 2; //ִ��¼��
                while (tp_dev.x[0] > 0 && tp_dev.x[0] < 60 && tp_dev.y[0] > 20 && tp_dev.y[0] < 60) //���ּ��
                {
                    tp_dev.scan(0);
                }
            }

            if (tp_dev.x[0] > 100 && tp_dev.x[0] < 160 && tp_dev.y[0] > 20 && tp_dev.y[0] < 60) {
                //�����Home
                LCD_Clear(BLACK);//����
                AS608_load_keyboard(0, 170, (uint8_t **) kbd_menu); //�����������
                LCD_RC522_UI();
                LCD_AS608_UI();
                rC522Flag = 3; //ִ��¼��
                while (tp_dev.x[0] > 20 && tp_dev.x[0] < 220 && tp_dev.y[0] > 20 && tp_dev.y[0] < 60) //���ּ��
                {
                    tp_dev.scan(0);
                }
            }
        }

    }


}

/**
 * RFIDģ��UI
 */
void LCD_RC522_UI(void) {
    LCD_DrawRectangle(180, 20, lcddev.width, 50, RED);
    LCD_DrawRectangle(0, 20, 60, 50, RED);
    LCD_ShowString(200, 28, (uint8_t *) "¼��", RED, BLACK);
    LCD_ShowString(10, 28, (uint8_t *) "ɾ��", RED, BLACK);
}

/**
 * ָ��ģ��UI
 */
void LCD_AS608_UI(void) {
    uint8_t str[32];
    uint8_t ensure;
    uint16_t ValidN;
    //ģ������Чָ�Ƹ���
    LCD_ShowString(54, 0, (uint8_t *) "----�Ž�ϵͳ----", RED, BLACK);
    ensure = PS_ValidTempleteNum(&ValidN); //����ָ�Ƹ���
    if (ensure != 0x00) {
        //ShowErrMessage(ensure);             //��ʾȷ���������Ϣ
    }
    ensure = PS_ReadSysPara(&AS608Para); //������
    if (ensure == 0x00) {
        sprintf((char *) str, "������:%d", AS608Para.PS_max - ValidN);
        LCD_ShowString(8, 60, (uint8_t *) str, RED, BLACK);
    } else {
        //ShowErrMessage(ensure);
    }
}


//���ذ������棨�ߴ�x,yΪ240*150��
//x,y:������ʼ���꣨240*320�ֱ��ʵ�ʱ��x����Ϊ0��
void AS608_load_keyboard(uint16_t x, uint16_t y, uint8_t **kbtbl) {
    uint16_t i;
    POINT_COLOR = RED;
    kbd_tbl = kbtbl;
    LCD_Fill(x, y, x + 240, y + 150, BLACK);
    LCD_DrawRectangle(x, y, x + 240, y + 150, RED);
    LCD_DrawRectangle(x + 80, y, x + 160, y + 150, RED);
    LCD_DrawRectangle(x, y + 30, x + 240, y + 60, RED);
    LCD_DrawRectangle(x, y + 90, x + 240, y + 120, RED);
    for (i = 0; i < 15; i++) {
        if (i == 1) //�������2����:������Ҫ�м���ʾ
            LCD_ShowString(x + (i % 3) * 80 + 2, y + 7 + 30 * (i / 3), (uint8_t *) kbd_tbl[i], RED, BLACK);
        else
            Show_Str_Mid(x + (i % 3) * 80, y + 7 + 30 * (i / 3), (uint8_t *) kbd_tbl[i], 16, 80);
    }
}
