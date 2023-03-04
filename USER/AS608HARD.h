
#ifndef __AS608HARD_H
#define __AS608HARD_H

#include "main.h"

extern uint8_t **kbd_tbl;

uint8_t AS608_get_keynum(uint16_t x, uint16_t y);
void AS608_key_staset(uint16_t x, uint16_t y, uint8_t keyx, uint8_t sta);
uint16_t GET_NUM(void);

#endif

