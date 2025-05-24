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
QueueHandle_t gpsQueue;
QueueHandle_t gprsQueue;
QueueHandle_t dataQueue;
QueueHandle_t mainQueue;


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
    gpsQueue  = xQueueCreate(10, sizeof(int));
    //senseQueue = xQueueCreate(10, sizeof(int));
    dataQueue = xQueueCreate(10, sizeof(int));
    mainQueue = xQueueCreate(10, sizeof(int));
    
    

    if (!gpsQueue || !dataQueue || !mainQueue) 
    {
        Serial.println("Queue creation error!");
        while (1);
    }

    //------------------------------------------------------------------------------------------------
    // taskMODEM        check GPRS status and reinitialize if needed
    //------------------------------------------------------------------------------------------------
    //xTaskCreatePinnedToCore(taskMODEM,"GPS", 2000, NULL, 1, NULL, 1);

    //------------------------------------------------------------------------------------------------
    // taskGPS          handle all GPS related tasks
    //------------------------------------------------------------------------------------------------
    xTaskCreatePinnedToCore(taskGPS,"GPS", 2000, NULL, 2, NULL, 1);
    
    //------------------------------------------------------------------------------------------------
    // taskSENSE        handle all sensor related tasks
    //------------------------------------------------------------------------------------------------
    //xTaskCreatePinnedToCore(taskSENSE,"SENSE", 2000, NULL, 2, NULL, 1);
    
    //------------------------------------------------------------------------------------------------
    // taskDATA         collect all data, make it ready for transmission and store in a buffer
    //------------------------------------------------------------------------------------------------
    xTaskCreatePinnedToCore(taskDATA,"DATA", 2000, NULL, 3, NULL, 1);
    
    //------------------------------------------------------------------------------------------------
    // taskMAIN         transmit data to the database whenever data is ready
    //------------------------------------------------------------------------------------------------
    xTaskCreatePinnedToCore(taskMAIN,"MAIN", 2000, NULL, 1, NULL, 1);
    
    Serial.println("-----------------------------------------------------");
    Serial.println("|                  SETUP HAS RUN                    |");
    Serial.println("-----------------------------------------------------");
}




void taskMAIN(void *pvParameters) // taskMAIN is responsible for checking whenever data is ready to be transmitted and then transmit the data
{
    Serial.println("TaskMAIN  running");
    MainCommand cmd;
    GpsCommand cmdGPS;
    GprsCommand cmdGPRS;

    cmd = MAIN_IDLE;

    while(1)
    {
        MainCommand incomingCmd;
        if (xQueueReceive(mainQueue, &incomingCmd, 0) == pdPASS) {
            cmd = incomingCmd;
        }

        switch (cmd) 
        {
            case MAIN_IDLE:
            vTaskDelay(100 / portTICK_PERIOD_MS);
            
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
