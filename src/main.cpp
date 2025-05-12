#define SerialMon Serial        // Set serial for debug console

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Ticker.h>
#include "gps.h"
#include "data.h"
#include "gprs.h"
#include "message_queue_cmd.h"

#define UART_BAUD       9600
#define PIN_TX          27
#define PIN_RX          26

HardwareSerial SerialAT(1);           // UART1
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

// Message queue handlers
QueueHandle_t gprsQueue;
QueueHandle_t gpsQueue;

void taskMAIN(void *pvParameters);

void setup()
{
    // Start console communication
    SerialMon.begin(115200); delay(10);    // Set console baud rate
    SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);
    Serial.println("-----------------------------------------------------");
    Serial.println("|                  SYSTEM STARTED                   |");
    Serial.println("-----------------------------------------------------");
    
    // Create message queues
    gprsQueue = xQueueCreate(10, sizeof(int));
    gpsQueue  = xQueueCreate(5, sizeof(float));

    if (!gprsQueue || !gpsQueue) 
    {
        Serial.println("Fejl ved kø-oprettelse!");
        while (1);
    }

    Serial.println("|--------------------SETUP DONE---------------------|");
        
    xTaskCreate(taskGPS, "GPS", 2000, NULL, 1, NULL);
    delay(100);
    xTaskCreate(taskGPRS, "GPRS", 2000, NULL, 1, NULL);
    delay(100);
    xTaskCreate(taskMAIN, "MAIN", 2000, NULL, 1, NULL);
}


void taskMAIN(void *pvParameters)
{
    Serial.println("TaskMAIN running");
    MainCommand cmd;
    GpsCommand cmdGPS;
    GprsCommand cmdGPRS;

    cmd = MAIN_SETUP_GPS;

    while(1)
    {
        //if(gpsData.speed)

        switch (cmd) 
        {
            case MAIN_IDLE:
            // DO NOTHING
            vTaskDelay(100 / portTICK_PERIOD_MS);
            //Serial.println("MAIN: MAIN IDLE");
            break;

            case MAIN_SETUP_GPS:
            Serial.print("MAIN: Setting up GPS");
            cmdGPS = GPS_SETUP;
            xQueueSend(gpsQueue, &cmdGPS, portMAX_DELAY);
            Serial.println(" - OK");
            cmd = MAIN_FETCH_GPS;
            break;

            case MAIN_FETCH_GPS:
            Serial.print("MAIN: Fetching GPS data");
            cmdGPS = GPS_FETCH_DATA;
            xQueueSend(gpsQueue, &cmdGPS, portMAX_DELAY);
            Serial.println(" - OK");
            cmd = MAIN_IDLE;
            break;

            default:
            Serial.println("GPRS: Unknown command");
            Serial.println("GPS: NOT IMPLEMENTED");
            cmd = MAIN_IDLE;
            break;
        }        
    }
}


void loop()
{
    delay(10);
}
