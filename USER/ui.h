#ifndef __UI_H
#define __UI_H

#include "stm32f4xx_hal.h"
#include "lcd.h"
#include "stmflash.h"
#include "touch.h"
#include "tim.h"


extern const char *kbd_menu[];
extern const char *kbd_delFR[];
extern uint8_t rC522Flag;


void LCD_show(void);
void LCD_AS608_UI(void);
void AS608_load_keyboard(uint16_t x, uint16_t y, uint8_t **kbtbl);
void LCD_RC522_UI(void);
void LCD_RC522_Scan(void);
#endif

