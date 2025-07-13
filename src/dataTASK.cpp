
#include <Arduino.h>
#include "gpsTASK.h"
#include "dataTASK.h"
#include "modem.h"
#include "gprsTASK.h"
#include "message_queue_cmd.h"
#include "message_queue.h"
#include "global.h"
#include "database.h"


void taskDATA(void *pvParameters)
{
    Serial.println("TaskDATA running");
    DataCommand cmd = DATA_IDLE;

    saveSampleIndex = 0;

    uint16_t transmitSampleIndex = 0;
    bool gpsDataReadyFlag = false;
    bool tempHumidDataReadyFlag = false;
    bool timeToSave = false;
    bool samplesWaitingInBuffer = false;

    uint16_t gpsSampleCnt = 0;
    uint16_t senseSampleCnt = 0;

    while (1)
    {
        DataCommand incomingCmd;
        if (xQueueReceive(dataQueue, &incomingCmd, 0) == pdPASS)
        {
            cmd = incomingCmd;
        }

        switch (cmd)
        {
            case DATA_IDLE:
                vTaskDelay(100 / portTICK_PERIOD_MS);

                if(gpsDataReadyFlag && tempHumidDataReadyFlag && timeToSave)
                {
                    dataFreeStackPrint.printFreeStack();
                    
                    cmd = DATA_SAVE_TO_RAM;
                    xQueueSend(dataQueue, &cmd, portMAX_DELAY);
                    gpsDataReadyFlag = false;
                    tempHumidDataReadyFlag = false;
                    timeToSave = false;
                }
                break;

            case DATA_GPS_DATA_READY:
                //Serial.print("DATA: GPS   data ready - ("); Serial.print(gpsSampleCnt); Serial.print(")"); Serial.print(" Sec  = "); Serial.print(gpsDataTemporary.second); Serial.println("s");
                gpsSampleCnt++;
                gpsDataReadyFlag = true;
                break;

            case DATA_SENSE_DATA_READY:
                //Serial.print("DATA: SENSE data ready - ("); Serial.print(gpsSampleCnt); Serial.print(")"); Serial.print(" Temp = "); Serial.print(tempHumidDataTemporary.tempCaravan); Serial.println("C");
                senseSampleCnt++;
                tempHumidDataReadyFlag = true;
                break;
            
            case DATA_TIME_TO_SAVE:
                timeToSave = true;
                break;

            case DATA_SAVE_TO_RAM:
                if(saveSampleIndex >= MAX_PSRAM_SAMPLES) Serial.println("DATA ERROR: PSRAM_buffer FULL -> Sample NOT saved!");
                else
                {
                    PSRAM_buffer->psramBufferInUse = true;
                    PSRAM_buffer[saveSampleIndex].psramGpsData = gpsDataTemporary;
                    PSRAM_buffer[saveSampleIndex].psramTempHumidData = tempHumidDataTemporary;
                    saveSampleIndex++;
                    PSRAM_buffer->psramBufferInUse = false;

                    //Serial.print("DATA: Sample saved in PSRAM_buffer["); Serial.print(saveSampleIndex); Serial.println("]");
                }
                break;

            default:
                Serial.println("DATA: Unknown command");
                break;
        }
        cmd = DATA_IDLE;
    }
}
