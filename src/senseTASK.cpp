#include <Arduino.h>
#include <TinyGsmClient.h>
#include "gpsTASK.h"
#include "dataTASK.h"
#include "modem.h"
#include "gprsTASK.h"
#include "message_queue_cmd.h"
#include "message_queue.h"
#include "tempHumid.h"
#include "global.h"
#include "errorHandler.h"

#define SENSE_MAX_RETRIES         10














void taskSENSE(void *pvParameters)
{
    Serial.println("SenseTASK  running");
    SenseCommand cmd;
    DataCommand dataCmd;
    uint8_t senseRetryCnt = 0;

    cmd = SENSE_IDLE;

    while (1)
    {
        SenseCommand incomingCmd;
        if (xQueueReceive(senseQueue, &incomingCmd, portMAX_DELAY) == pdPASS) {
            cmd = incomingCmd;
        }

        switch (cmd)
        {
            case SENSE_IDLE:
            Serial.println("SENSE: idle");
            break;

            case SENSE_SAMPLE:
            {
                senseFreeStackPrint.printFreeStack();

                if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(100)) == pdTRUE)
                {

                    getTempHumid(AHT10_IN_PIN);
                    getTempHumid(AHT10_OUT_PIN);
                    
                    xSemaphoreGive(i2cMutex);
                    
                    dataCmd = DATA_SENSE_DATA_READY;
                    xQueueSend(dataQueue, &dataCmd, portMAX_DELAY);
                }
                else                                        // Sample NOT taken, an error has occured, try again
                {
                    Serial.print("SENSE: senseRetryCnt = "); Serial.println(senseRetryCnt);
                    senseRetryCnt++;
                    vTaskDelay(50 / portTICK_PERIOD_MS);
                    SenseCommand senseCmd = SENSE_SAMPLE;
                    xQueueSend(senseQueue, &senseCmd, pdMS_TO_TICKS(500));
                }

                if(senseRetryCnt >= (SENSE_MAX_RETRIES/2))      // Try to recover the GPS if half of max retries has been reached
                {
                    Serial.println("SENSE: Recover temp/humid sensors");
                    Serial.println("SENSE: Recover NOT IMPLEMENTED YET");       // REMEMBER TO IMPLEMENT THIS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                }

                if(senseRetryCnt >= SENSE_MAX_RETRIES)          // Error -> restart system
                {
                    ERRORhandler(SENSE_RETRY_ERROR);
                    senseRetryCnt = 0;        // Should not happend
                }
                
                
            }
            break;

            default:
            Serial.println("SENSE: Unknown command");
            break;
        }
        

    }
    cmd = SENSE_IDLE;
}
