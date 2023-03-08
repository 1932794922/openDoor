#ifndef __FUNHANDLE_H
#define __FUNHANDLE_H

#include "main.h"


void openDoorForMQTT(char *data);

void Read_IDcard(void);

uint8_t authStateNFC(const uint8_t *pKey);

void errorRFIDMQTT(uint8_t *id,  uint8_t *err);

uint8_t entryCard(void) ;

uint8_t delCard(void);
#endif

