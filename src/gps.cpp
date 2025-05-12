// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

#include <Arduino.h>
#include <TinyGsmClient.h>
#include "gps.h"
#include "data.h"
#include "modem.h"
#include "gprs.h"
#include "message_queue_cmd.h"
#include "message_queue.h"
#include "esp_task_wdt.h"

#define PWR_PIN         4
#define LED_PIN         12

void enableGPS(void)
{
    // Set Modem GPS Power Control Pin to HIGH ,turn on GPS power
    // Only in version 20200415 is there a function to control GPS power
    modem.sendAT("+CGPIO=0,48,1,1");
    if (modem.waitResponse(10000L) != 1) {
        DBG("Set GPS Power HIGH Failed");
    }
    modem.enableGPS();
}

void disableGPS(void)
{
    // Set Modem GPS Power Control Pin to LOW ,turn off GPS power
    // Only in version 20200415 is there a function to control GPS power
    modem.sendAT("+CGPIO=0,48,1,0");
    if (modem.waitResponse(10000L) != 1) {
        DBG("Set GPS Power LOW Failed");
    }
    modem.disableGPS();
}

void modemPowerOn()
{
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, HIGH);
    delay(1000);    //Datasheet Ton mintues = 1S
    digitalWrite(PWR_PIN, LOW);
}

void modemPowerOff()
{
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, HIGH);
    delay(1500);    //Datasheet Ton mintues = 1.2S
    digitalWrite(PWR_PIN, LOW);
}


void modemRestart()
{
    modemPowerOff();
    delay(1000);
    modemPowerOn();
}


void setupGPS()
{
    gpsData.dataFetched = false;
    // Set LED OFF
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    modemPowerOn();

    delay(1000);
}


void fetchGPS()
{
    Serial.println("");
    Serial.print("Fetching GPS coordinates");
    
    if (!modem.testAT()) 
    {
        Serial.println("Failed to restart modem, attempting to continue without restarting");
        modemRestart();
        return;
    }

    enableGPS();
    
    while (1) 
    {
        gpsData.structInUse = true;

        if (modem.getGPS(&gpsData.lat, 
                         &gpsData.lon, 
                         &gpsData.speed,
                         &gpsData.alt,
                         &gpsData.vsat,
                         &gpsData.usat,
                         &gpsData.accuracy,
                         &gpsData.year,
                         &gpsData.month,
                         &gpsData.day,
                         &gpsData.hour,
                         &gpsData.minute,
                         &gpsData.second)) 
        {
            //Serial.println(" ");
            gpsData.structInUse = false;
            gpsData.dataFetched = true;
            Serial.println("OK");
            break;
        }
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        Serial.print(".");
        gpsData.structInUse = false;
        delay(1000);    // Wait a second before fetching coordinates again
    }

    disableGPS();
}


void printGPS()
{
    Serial.println("");
    Serial.println("Fetched data from GPS");
    Serial.print("latitude:              "); Serial.println(gpsData.lat, 6);
    Serial.print("longitude:             "); Serial.println(gpsData.lon, 6);
    Serial.print("Speed:                 "); Serial.print(gpsData.speed, 2); Serial.println(" km/h");
    Serial.print("Altitude:              "); Serial.print(gpsData.alt); Serial.println(" moh");
    Serial.print("VSAT:                  "); Serial.println(gpsData.vsat);
    Serial.print("USAT:                  "); Serial.println(gpsData.usat);
    Serial.print("Accuracy:              "); Serial.print(gpsData.accuracy); Serial.println(" m");
    Serial.print("Year:                  "); Serial.println(gpsData.year);
    Serial.print("Month:                 "); Serial.println(gpsData.month);
    Serial.print("Day:                   "); Serial.println(gpsData.day);
    Serial.print("Hour:                  "); Serial.print(gpsData.hour); Serial.println(" hours");
    Serial.print("Minute:                "); Serial.print(gpsData.minute); Serial.println(" minutes");
    Serial.print("Second:                "); Serial.print(gpsData.second); Serial.println(" seconds");
    Serial.print("Has data been fetched? "); Serial.println(gpsData.dataFetched);
    Serial.print("Struct in use?         "); Serial.println(gpsData.structInUse);
    //vTaskDelay(6000 / portTICK_PERIOD_MS);
}


void taskGPS(void *pvParameters)
{
    Serial.println("TaskGPS running");

    GpsCommand cmd;
    cmd = GPS_IDLE;

    while (1) 
    {
        xQueueReceive(gpsQueue, &cmd, portMAX_DELAY);

        switch (cmd) 
        {
            case GPS_IDLE:
            // DO NOTHING
            vTaskDelay(100 / portTICK_PERIOD_MS);
            Serial.println("GPS: GPS IDLE");
            break;
            
            case GPS_SETUP:
            setupGPS();
            Serial.println("GPS: Setup done");
            break;
            
            case GPS_FETCH_DATA:
            Serial.println("GPS: Start fetchGPS()");
            fetchGPS();
            Serial.println("GPS: Done fetchGPS()");
            Serial.println("GPS: Fetching data");
            break;

            case GPS_CHECK_SIGNAL:
            Serial.println("GPS: Checking signal strength");
            Serial.println("GPS: NOT IMPLEMENTED");
            break;

            case GPS_RESET:
            Serial.println("GPS: Resetting");
            Serial.println("GPS: NOT IMPLEMENTED");
            break;

            default:
            Serial.println("GPS: Unknown command");
            break;
        }
        cmd = GPS_IDLE;

    }
}
