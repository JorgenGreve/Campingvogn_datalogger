#ifndef OS_TIMER_H
#define OS_TIMER_H

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "gprsTASK.h"

//extern bool gprsConnectionLostFlag;

void createOsTimers(void);
void startOsTimer(TimerHandle_t xTimer);

void osTimerCallback(TimerHandle_t xTimer);

#endif