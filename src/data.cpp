
#include <Arduino.h>
#include <TinyGsmClient.h>
#include "gps.h"
#include "data.h"
#include "modem.h"
#include "gprs.h"
#include "message_queue_cmd.h"
#include "message_queue.h"
#include "data.h"

// Her allokeres den globale instans
GpsData gpsData;
TempHumidData tempHumidData;

void taskDATA(void *pvParameters)
{
    Serial.println("TaskDATA running");
    DataCommand cmd = DATA_IDLE;

    bool gpsDataReadyFlag = false;
    bool tempHumidDataReadyFlag = false;

    while (1) 
    {
        DataCommand incomingCmd;
        if (xQueueReceive(dataQueue, &incomingCmd, 0) == pdPASS) 
        {
            cmd = incomingCmd;
        }

        //if(gpsData.gpsDataReady); postDataToServer(gpsData, tempHumidData); //NÅR DATA ER KLAR SKAL DET GEMMES I EN BUFFER OG NÅR DER ER DATA I BUFFEREN SKAL DET TRANSMITTERES MED DET SAMME
        
        switch (cmd) 
        {
            case DATA_IDLE:
                vTaskDelay(100 / portTICK_PERIOD_MS);

                if(gpsDataReadyFlag && tempHumidDataReadyFlag)
                {
                    // Send message to taskMAIN to transmit data
                    gpsDataReadyFlag = false;
                    tempHumidDataReadyFlag = false;
                }
                break;

            case GPS_DATA_READY:
                Serial.println("DATA: GPS data ready!");
                gpsDataReadyFlag = true;
                cmd = DATA_IDLE;
                break;
                
            case TEMP_HUMID_DATA_READY:
                Serial.println("DATA: TEMP/HUMID data ready!");
                tempHumidDataReadyFlag = true;
                cmd = DATA_IDLE;
                break;

            default:
                Serial.println("DATA: Unknown command");
                break;
        }
    }
}
