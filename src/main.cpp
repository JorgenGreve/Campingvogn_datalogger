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
#include "senseTASK.h"
#include "errorHandler.h"

#define MAX_RETRY_CNT       5

// Message queue handlers
QueueHandle_t gpsQueue;
QueueHandle_t senseQueue;
QueueHandle_t gprsQueue;
QueueHandle_t dataQueue;
QueueHandle_t mainQueue;

void taskMAIN(void *pvParameters);
bool isItTimeToTx(void);
bool theStructIsInUse(IsTheStructInUse structInUse);

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
    if(!initGPS()) ERRORhandler(GPS_INIT_ERROR);
    Serial.println("MAIN:     MAIN_INIT_GPS        OK");

    
    Serial.println("MAIN:     MAIN_INIT_GPRS       RUN");
    if(initGPRS()){connectGPRS();}
    else{ERRORhandler(GPRS_INIT_ERROR);}
    Serial.println("MAIN:     MAIN_INIT_GPRS       OK");
    
    // Create message queues
    gpsQueue  = xQueueCreate(10, sizeof(int));
    senseQueue = xQueueCreate(10, sizeof(int));
    dataQueue = xQueueCreate(10, sizeof(int));
    mainQueue = xQueueCreate(10, sizeof(int));
    
    

    if (!gpsQueue || !senseQueue || !dataQueue || !mainQueue)
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
    xTaskCreatePinnedToCore(taskSENSE,"SENSE", 2000, NULL, 2, NULL, 1);
    
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

    bool itsTimeToTx = false;

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
                if(isItTimeToTx())
                {
                    itsTimeToTx = true;
                }
                break;
            
            case MAIN_ALL_DATA_READY:
                Serial.println("MAIN: All data ready!");
                MainCommand cmdOut;
                cmdOut = MAIN_IS_IT_TIME_TO_TX;
                xQueueSend(mainQueue, &cmdOut, portMAX_DELAY);
                cmd = MAIN_IDLE;
                break;
            
            case MAIN_IS_IT_TIME_TO_TX:
                if(itsTimeToTx)
                {
                    MainCommand cmdOut;
                    cmdOut = MAIN_TRANSMIT_DATA;
                    xQueueSend(mainQueue, &cmdOut, portMAX_DELAY);
                    itsTimeToTx = false;
                }
                cmd = MAIN_IDLE;
                break;

            case MAIN_TRANSMIT_DATA:
                Serial.println("MAIN: MAIN_TRANSMIT_DATA");
                postDataToServer(gpsData, tempHumidData);
                cmd = MAIN_IDLE;
                break;
            
            default:
                Serial.println("MAIN: Unknown command");
                break;
        }
        cmd = MAIN_IDLE;
    }
}


void loop()
{
    delay(10);
}


// FREE RTOS configTICK_RATE_HZ findes ved at trykke F12 på pdMS_TO_TICKS og derefter F12 på configTICK_RATE_HZ i pdMS_TO_TICKS define
// configTICK_RATE_HZ er sat til 1000 Hz -> 1 tick = 1 ms
// xTaskGetTickCount(); returnerer antal milisekunder siden FreeRTOS startede hvis configTICK_RATE_HZ er sat til 1000 Hz

//   0 kmh = 900 s tx (15 min)
//  15 kmh = 1000 ms tx
// >15 kmh = 1000 ms
// y = -59933.333 x + 900000;

bool isItTimeToTx(void)
{
    static uint32_t nextTxTick = 0;
    static uint32_t currentTimeToTx_ms = 0;
    static uint32_t lastTimeToTx_ms = 0;

    uint32_t currentTick = xTaskGetTickCount();

    uint16_t retryCnt = 0;
    while (theStructIsInUse(GPS_DATA_STRUCT) && retryCnt <= MAX_RETRY_CNT)
    {
        retryCnt++;

        if (retryCnt == MAX_RETRY_CNT)
        {
            Serial.println("ERROR: isItTimeToTx() failed - too many struct retries");
            return false;
        }
    }

    if (!theStructIsInUse(GPS_DATA_STRUCT))
    {
        float currentSpeed = gpsData.speed;

        // Beregn nyt interval afhængigt af hastighed
        if (currentSpeed <= 15.0f)
        {
            currentTimeToTx_ms = (uint32_t)(-59933.333f * currentSpeed + 900000);
        }
        else
        {
            currentTimeToTx_ms = 1000;
        }

        //currentTimeToTx_ms = 10000;                   // DEBUG

        // Hvis intervallet er ændret, opdatér næste tidspunkt
        if (currentTimeToTx_ms != lastTimeToTx_ms)
        {
            lastTimeToTx_ms = currentTimeToTx_ms;
            nextTxTick = currentTick + currentTimeToTx_ms;
        }

        // Wrap-around sikker kontrol
        if ((int32_t)(currentTick - nextTxTick) >= 0)
        {
            nextTxTick = currentTick + currentTimeToTx_ms;
            return true;
        }

        // Debug
        /*
        Serial.print("Speed: "); Serial.println(currentSpeed);
        Serial.print("Tx interval (ms): "); Serial.println(currentTimeToTx_ms);
        Serial.print("Next Tx at tick: "); Serial.println(nextTxTick);
        Serial.print("Current tick: "); Serial.println(currentTick);
        Serial.println();
        */
    }

    return false;
}



bool theStructIsInUse(IsTheStructInUse structInUse)
{
    switch (structInUse)
    {
    case  GPS_DATA_STRUCT:
        return gpsData.gpsStructInUse;
    
    case  TEMP_HUMID_DATA_STRUCT:
        return tempHumidData.tempHumidStructInUse;

    default:
        return true;
        break;
    }
}
