#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Ticker.h>
#include "gpsTASK.h"
#include "dataTASK.h"
#include "gprsTASK.h"
#include "message_queue_cmd.h"
#include "modem.h"
#include "database.h"
#include "senseTASK.h"
#include "errorHandler.h"
#include "osTimer.h"
#include "esp_heap_caps.h"
#include "main.h"
#include "global.h"
#include "mqtt.h"
#include "tempHumid.h"

#define MAX_RETRY_CNT       5
#define SerialMon           Serial // Set serial for debug console

// Message queue handlers
QueueHandle_t gpsQueue;
QueueHandle_t senseQueue;
QueueHandle_t gprsQueue;
QueueHandle_t dataQueue;
QueueHandle_t mainQueue;

void taskMAIN(void *pvParameters);
bool isItTimeToSaveData(void);
void printFreeRAM();
void printFreePSRAM(String beforeOrAfterRamAllocation);
void allocateSpaceForPSRAM_buffer();

void setup()
{
    // Start console communication
    SerialMon.begin(115200); delay(10);     // Set console baud rate
    initModemSerial();                      // Initialize modem for both GPS and GPRS
    Serial.println("-----------------------------------------------------");
    Serial.println("|                  SYSTEM STARTED                   |");
    Serial.println("-----------------------------------------------------");

    printFreeRAM();
    allocateSpaceForPSRAM_buffer();


    Serial.println("MAIN:     MAIN_INIT_GPS        RUN");
    if(!initGPS()) ERRORhandler(GPS_INIT_ERROR);
    Serial.println("MAIN:     MAIN_INIT_GPS        OK");

    
    Serial.println("MAIN:     MAIN_INIT_GPRS       RUN");
    if(initGPRS()){connectGPRS();}
    else{ERRORhandler(GPRS_INIT_ERROR);}
    Serial.println("MAIN:     MAIN_INIT_GPRS       OK");

    Serial.println("MAIN:     MAIN_INIT_AHT10      RUN");
    initAHT10();
    Serial.println("MAIN:     MAIN_INIT_AHT10      OK");

    Serial.println("MAIN:     MAIN_INIT_MQTT       RUN");
    mqttClient.setServer(mqtt_server, mqtt_port);
    // Optionelt: mqttClient.setCallback(...) hvis du skal modtage
    Serial.println("MAIN:     MAIN_INIT_MQTT       OK");

    
    // Create message queues
    gprsQueue = xQueueCreate(10, sizeof(int));
    gpsQueue  = xQueueCreate(10, sizeof(int));
    senseQueue = xQueueCreate(10, sizeof(int));
    dataQueue = xQueueCreate(10, sizeof(int));
    mainQueue = xQueueCreate(10, sizeof(int));
    
    if (!gpsQueue || !senseQueue || !dataQueue || !mainQueue || !gprsQueue)
    {
        Serial.println("Queue creation error!");
        while (1);
    }

    modemMutex = xSemaphoreCreateMutex();
    i2cMutex = xSemaphoreCreateMutex();

    if (modemMutex == NULL)
    {
        Serial.println("❌ Failed to create modem mutex");
    }

    //------------------------------------------------------------------------------------------------
    // taskGPS          handle all GPS related tasks
    //------------------------------------------------------------------------------------------------
    xTaskCreatePinnedToCore(taskGPS,"GPS", 2560, NULL, 1, NULL, 1); // Prioritet 2

    //------------------------------------------------------------------------------------------------
    // taskSENSE        handle all sensor related tasks
    //------------------------------------------------------------------------------------------------
    xTaskCreatePinnedToCore(taskSENSE,"SENSE", 3072, NULL, 1, NULL, 1); // Prioritet 2
    
    //------------------------------------------------------------------------------------------------
    // taskDATA         collect all data, make it ready for transmission and store in a buffer
    //------------------------------------------------------------------------------------------------
    xTaskCreatePinnedToCore(taskDATA,"DATA", 2048, NULL, 1, NULL, 1); // Prioritet 1
    
    //------------------------------------------------------------------------------------------------
    // taskMAIN         handle sampling timing
    //------------------------------------------------------------------------------------------------
    xTaskCreatePinnedToCore(taskMAIN,"MAIN", 2560, NULL, 1, NULL, 1); // Prioritet 1
    
    //------------------------------------------------------------------------------------------------
    // taskGPRS        check GPRS status and reinitialize if needed
    //------------------------------------------------------------------------------------------------
    xTaskCreatePinnedToCore(taskGPRS,"GPRS", 3072, NULL, 1, NULL, 1); // Prioritet 2

    delay(500);

    createOsTimers();

    Serial.println("-----------------------------------------------------");
    Serial.println("|                  SETUP HAS RUN                    |");
    Serial.println("-----------------------------------------------------");
}



void taskMAIN(void *pvParameters)
{
    Serial.println("TaskMAIN running");
    MainCommand cmd = MAIN_IDLE;

    while (1)
    {
        MainCommand incomingCmd;
        if (xQueueReceive(mainQueue, &incomingCmd, 0) == pdPASS) {
            cmd = incomingCmd;
        }

        switch (cmd)
        {
            case MAIN_IDLE:
                mainFreeStackPrint.printFreeStack();

                if(isItTimeToSaveData())
                {
                    
                    DataCommand cmdOut = DATA_TIME_TO_SAVE;
                    xQueueSend(dataQueue, &cmdOut, portMAX_DELAY);
                }
                
                if(!gprsConnectionLostFlag && xSemaphoreTake(modemMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
                {
                    mqttClient.loop();
                    xSemaphoreGive(modemMutex);
                }
                else if(!gprsConnectionLostFlag)
                {
                    //Serial.println("⚠️ Could not acquire modem mutex for GPS. taskMAIN()");
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
                else if(gprsConnectionLostFlag)
                {

                }
                vTaskDelay(100 / portTICK_PERIOD_MS);
                break;
            
            default:
                break;
        }
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

bool isItTimeToSaveData(void)
{
    static uint32_t nextTxTick = 0;
    static uint32_t currentTimeToTx_ms = 0;
    static uint32_t lastTimeToTx_ms = 0;

    uint32_t currentTick = xTaskGetTickCount();

    float currentSpeed = gpsDataTemporary.speed;

    // Beregn nyt interval afhængigt af hastighed
    // Hvor ofte der transmitteres bestemmes også af sample hastigheden for sense og gps, se osTimer.cpp
    if (currentSpeed <= 15.0f)
    {
        currentTimeToTx_ms = (uint32_t)(-59933.333f * currentSpeed + 900000);
    }
    else
    {
        currentTimeToTx_ms = 1000;
    }

    //currentTimeToTx_ms = 5000;                   // DEBUG

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

    return false;
}


void printFreeRAM()
{
    Serial.println("");
    Serial.print("Free heap:           ");
    Serial.print(ESP.getFreeHeap()/1000);
    Serial.println(" kB");

    Serial.print("Largest free block:  ");
    Serial.print(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT)/1000);
    Serial.println(" kB");
    Serial.println("");
}

void printFreePSRAM(String beforeOrAfterRamAllocation)
{
    size_t free = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t largest = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
    
    Serial.println("");
    Serial.println(beforeOrAfterRamAllocation);
    Serial.print("Free PSRAM:          ");
    Serial.print(free / 1024);
    Serial.println(" kB");

    Serial.print("Largest PSRAM block: ");
    Serial.print(largest / 1024);
    Serial.println(" kB");
    Serial.println("");
}


void allocateSpaceForPSRAM_buffer()
{
    printFreePSRAM("Before PSRAM allocation");
    PSRAM_buffer = (CombinedData *)ps_malloc(sizeof(CombinedData) * MAX_PSRAM_SAMPLES);
    
    if (PSRAM_buffer == NULL)
    {
        Serial.println("PSRAM allocation failed!");
    }
    else
    {
        Serial.print("Allocated PSRAM buffer for ");
        Serial.print(MAX_PSRAM_SAMPLES);
        Serial.print(" samples (");
        Serial.print(sizeof(CombinedData) * MAX_PSRAM_SAMPLES / 1024);
        Serial.println(" kB)");
    }
    printFreePSRAM("After PSRAM allocation");
}