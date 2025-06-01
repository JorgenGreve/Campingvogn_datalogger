#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <Arduino.h>

// Kø til GPS-beskeder
extern QueueHandle_t gpsQueue;

// Kø til SENSE-beskeder
extern QueueHandle_t senseQueue;

// Kø til GPRS-beskeder
extern QueueHandle_t gprsQueue;

// Kø til DATA-beskeder
extern QueueHandle_t dataQueue;

// Kø til MAIN-beskeder
extern QueueHandle_t mainQueue;


#endif
