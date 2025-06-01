#include <Arduino.h>
#include <TinyGsmClient.h>
#include "gps.h"
#include "data.h"
#include "modem.h"
#include "gprs.h"
#include "message_queue_cmd.h"
#include "message_queue.h"
#include "tempHumid.h"


















void taskSENSE(void *pvParameters)
{
    Serial.println("SenseTASK  running");
    SenseCommand cmd;
    DataCommand dataCmd;

    cmd = SENSE_RUN;

    while (1)
    {
        SenseCommand incomingCmd;
        if (xQueueReceive(senseQueue, &incomingCmd, 0) == pdPASS) {
            cmd = incomingCmd;
        }

        switch (cmd)
        {
            case SENSE_IDLE:
            // DO NOTHING
            vTaskDelay(500 / portTICK_PERIOD_MS);
            //Serial.println("SENSE: SENSE IDLE");
            break;

            case SENSE_RUN:
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            if(getTempHumid(0))
            {
                dataCmd = SENSE_DATA_READY;
                xQueueSend(dataQueue, &dataCmd, portMAX_DELAY);
            }
            //getTempHumid(1);
            
            //Serial.print("SENSE: SENSE_RUN");
            break;

            default:
            Serial.println("SENSE: Unknown command");
            break;
        }
        

    }
}
