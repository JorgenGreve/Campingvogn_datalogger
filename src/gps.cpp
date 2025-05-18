#include <Arduino.h>
#include <TinyGsmClient.h>
#include "gps.h"
#include "data.h"
#include "modem.h"
#include "gprs.h"
#include "message_queue_cmd.h"
#include "message_queue.h"

#define PWR_PIN         4
#define LED_PIN         12

void enableGPS(void) {
    modem.sendAT("+CGPIO=0,48,1,1");
    if (modem.waitResponse(10000L) != 1) {
        Serial.println("⚠️ Set GPS Power HIGH Failed");
    }

    if (!modem.enableGPS()) {
        Serial.println("⚠️ modem.enableGPS() failed");
    } else {
        Serial.println("✅ GPS enabled");
    }
}

void disableGPS(void) {
    modem.sendAT("+CGPIO=0,48,1,0");
    if (modem.waitResponse(10000L) != 1) {
        Serial.println("⚠️ Set GPS Power LOW Failed");
    }
    modem.disableGPS();
    Serial.println("✅ GPS disabled");
}

void restartModem() {
    Serial.println("🔁 Restarting modem...");
    modem.poweroff();
    delay(2000);
    modem.init();
}

void initGPS() {
    gpsData.dataFetched = false;
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    fetchGPSsetup();
}

void fetchGPSsetup() {
    Serial.println("→ fetchGPSsetup");

    if (!modem.testAT()) {
        Serial.println("⚠️ testAT() failed — attempting restart");
        restartModem();
        delay(1000);
        if (!modem.init()) {
            Serial.println("❌ modem.init() failed");
            return;
        }
    }

    Serial.println("🔋 Enabling GPS...");
    enableGPS();

    const int maxTries = 1000;
    int tries = 0;

    while (tries++ < maxTries) {
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
                         &gpsData.second)) {
            gpsData.dataFetched = true;
            gpsData.structInUse = false;
            Serial.println("✅ GPS fix received!");
            //printGPS();
            break;
        } else {
            Serial.print(".");
        }

        gpsData.structInUse = false;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    if (!gpsData.dataFetched) {
        Serial.println("❌ No GPS fix received within time limit.");
    }

    Serial.println("🔌 Disabling GPS");
    disableGPS();
}

void printGPS() {
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
}

void taskGPS(void *pvParameters) {
    Serial.println("TaskGPS running");
    GpsCommand cmd = GPS_IDLE;
    MainCommand cmdMAIN;

    while (1) {
        GpsCommand incomingCmd;
        if (xQueueReceive(gpsQueue, &incomingCmd, 0) == pdPASS) {
            cmd = incomingCmd;
        }

        switch (cmd) {
            case GPS_IDLE:
                vTaskDelay(500 / portTICK_PERIOD_MS);
                break;

            case GPS_SETUP:
                Serial.print("GPS: GPS_SETUP");
                initGPS();
                cmdMAIN = MAIN_SETUP_GPRS;
                xQueueSend(mainQueue, &cmdMAIN, portMAX_DELAY);
                Serial.println(" - OK");
                cmd = GPS_IDLE;
                break;

            case GPS_RUN:
                // Her kan du evt. kalde fetchGPS() igen kontinuerligt
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                break;

            case GPS_FETCH_DATA:
                Serial.println("GPS: Fetching data");
                break;

            case GPS_CHECK_SIGNAL:
                Serial.println("GPS: Checking signal strength (not implemented)");
                break;

            case GPS_RESET:
                Serial.println("GPS: Resetting (not implemented)");
                break;

            default:
                Serial.println("GPS: Unknown command");
                break;
        }
    }
}
