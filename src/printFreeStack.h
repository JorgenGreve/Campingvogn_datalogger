#ifndef PRINTSTACK_H
#define PRINTSTACK_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "config.h"

class PrintStack
{
public:
    PrintStack(const char* name = "Task", uint16_t interval = 5)
        : taskName(name), printInterval(interval)
    {}

    void printFreeStack()
    {
#if PRINT_FREE_TASK_STACK
        if (++cnt >= printInterval)
        {
            UBaseType_t highWaterMark = uxTaskGetStackHighWaterMark(NULL);
            Serial.print(taskName);
            Serial.print(" stack free: ");
            Serial.println(highWaterMark);
            cnt = 0;
        }
#endif
    }

private:
    const char* taskName;
    uint16_t printInterval;
    uint16_t cnt = 0;
};


#endif // PRINTSTACK_H
