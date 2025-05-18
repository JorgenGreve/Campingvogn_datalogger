#define SerialMon Serial        // Set serial for debug console

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Ticker.h>
#include "gps.h"
#include "data.h"
#include "gprs.h"
#include "message_queue_cmd.h"
#include "modem.h"
#include "database.h"
#include "data.h"

// Message queue handlers
QueueHandle_t mainQueue;
QueueHandle_t gprsQueue;
QueueHandle_t gpsQueue;

void taskMAIN(void *pvParameters);

void setup()
{
    // Start console communication
    SerialMon.begin(115200); delay(10);     // Set console baud rate
    initModemSerial();                      // Initialize modem for both GPS and GPRS
    Serial.println("-----------------------------------------------------");
    Serial.println("|                  SYSTEM STARTED                   |");
    Serial.println("-----------------------------------------------------");
    
    //initSensors(); MANGLER IMPLEMENTERING
    Serial.println("MAIN:     MAIN_INIT_GPS        RUN");
    initGPS();
    Serial.println("MAIN:     MAIN_INIT_GPS        OK");

    
    Serial.println("MAIN:     MAIN_INIT_GPRS       RUN");
    initGPRS();
    connectGPRS();
    Serial.println("MAIN:     MAIN_INIT_GPRS       OK");
    
    postDataToServer(gpsData, tempHumidData);

    // Create message queues
    mainQueue = xQueueCreate(10, sizeof(int));
    gprsQueue = xQueueCreate(10, sizeof(int));
    gpsQueue  = xQueueCreate(10, sizeof(int));

    if (!gprsQueue || !gpsQueue) 
    {
        Serial.println("Fejl ved kø-oprettelse!");
        while (1);
    }

    /*
    xTaskCreate(taskGPS, "GPS", 2000, NULL, 1, NULL);
    delay(100);
    xTaskCreate(taskGPRS, "GPRS", 2000, NULL, 1, NULL);
    delay(100);
    xTaskCreate(taskMAIN, "MAIN", 2000, NULL, 1, NULL);
    */
   Serial.println("-----------------------------------------------------");
   Serial.println("|                  SETUP HAS RUN                    |");
   Serial.println("-----------------------------------------------------");
}


void taskMAIN(void *pvParameters)
{
    Serial.println("TaskMAIN  running");
    MainCommand cmd;
    GpsCommand cmdGPS;
    GprsCommand cmdGPRS;

    cmd = MAIN_SETUP_GPS;

    while(1)
    {
        MainCommand incomingCmd;
        if (xQueueReceive(mainQueue, &incomingCmd, 0) == pdPASS) {
            cmd = incomingCmd;
        }

        switch (cmd) 
        {
            case MAIN_IDLE:
            // DO NOTHING
            vTaskDelay(500 / portTICK_PERIOD_MS);
            //Serial.println("MAIN: MAIN IDLE");
            break;

            case MAIN_SETUP_GPS:                        
            Serial.print("MAIN:     MAIN_SETUP_GPS");
            cmdGPS = GPS_SETUP;
            xQueueSend(gpsQueue, &cmdGPS, portMAX_DELAY);
            Serial.println("        - OK");
            vTaskDelay(10000 / portTICK_PERIOD_MS);
            break;
            
            case MAIN_SETUP_GPRS:
            Serial.print("MAIN:     MAIN_SETUP_GPRS");
            cmdGPRS = GPRS_SETUP;
            xQueueSend(gprsQueue, &cmdGPRS, portMAX_DELAY);
            Serial.println("       - OK");
            break;
            
            case MAIN_SETUP_DONE:
            Serial.println("MAIN:     MAIN_SETUP            - DONE");
            cmd = MAIN_RUN_GPS;
            xQueueSend(mainQueue, &cmd, portMAX_DELAY);
            break;

            case MAIN_RUN_GPS:
            Serial.print("MAIN:     MAIN_RUN_GPS");
            cmdGPS = GPS_RUN;
            xQueueSend(gpsQueue, &cmdGPS, portMAX_DELAY);
            Serial.println("          - OK");
            cmd = MAIN_RUN_TEMP_HUMID;
            xQueueSend(mainQueue, &cmd, portMAX_DELAY);
            break;
            
            case MAIN_RUN_TEMP_HUMID:
            Serial.print("MAIN:     MAIN_RUN_TEMP_HUMID");
            //cmdTEMP = TEMP_RUN;
            //xQueueSend(tempQueue, &cmdTEMP, portMAX_DELAY);
            Serial.println("   - OK");
            cmd = MAIN_RUN_GPRS;
            xQueueSend(mainQueue, &cmd, portMAX_DELAY);
            break;

            case MAIN_RUN_GPRS:
            Serial.print("MAIN:     MAIN_RUN_GPRS");
            cmdGPRS = GPRS_RUN;
            xQueueSend(gprsQueue, &cmdGPRS, portMAX_DELAY);
            Serial.println("         - OK");
            break;
            
            default:
            Serial.println("GPRS: Unknown command");
            Serial.println("GPS: NOT IMPLEMENTED");
            break;
        }
        cmd = MAIN_IDLE;
    }
}


void loop()
{
    delay(10);
}
