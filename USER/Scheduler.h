#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#define TICK_PER_SECOND	1000 //º∆À„ms

#include "main.h"
#include "stm32f4xx_hal.h"

typedef struct
{
    void (*task_func)(void);
    uint16_t task_hz;
    uint16_t interval;
    uint32_t last_run;
} task_t;

extern uint32_t Task_num_ms;

void Scheduler_Setup(void);
void Scheduler_Run(void);
#endif

