#include "Screen.h"
#include "lcd.h"
#include "touch.h"

void Screen_scan(void)	//ÆÁÄ»É¨Ãè
{
    tp_dev.scan(0);

    if(tp_dev.x[0] > 100 && tp_dev.y[0] > 30 && tp_dev.x[0] < 220 && tp_dev.y[0] < 80) // function 1----- BC¶Î
    {
        //´úÂëĞ´Õâ
        LCD_Clear(WHITE);//ÇåÆÁ
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
        while (tp_dev.x[0] > 100 && tp_dev.y[0] > 30 && tp_dev.x[0] < 220 && tp_dev.y[0] < 80) //ËÉÊÖ¼ì²â
        {
            tp_dev.scan(0);
        }
    }
}


