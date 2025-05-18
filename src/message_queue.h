#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <Arduino.h>

// Kø til MAIN-beskeder
extern QueueHandle_t mainQueue;

// Kø til GPRS-beskeder
extern QueueHandle_t gprsQueue;

// Kø til GPS-beskeder
extern QueueHandle_t gpsQueue;

#endif
