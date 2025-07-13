#include <Arduino.h>
#include "gpsTASK.h"
#include "dataTASK.h"
#include "modem.h"
#include "gprsTASK.h"
#include "message_queue_cmd.h"
#include "message_queue.h"
#include "errorHandler.h"
#include "config.h"
#include "global.h"
//#include "TinyGsm.h"


#define PWR_PIN         4
#define LED_PIN         12

#define GPS_MAX_RETRIES         10

void gpsPowerOn(void)
{
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, HIGH);
    delay(1000);    //Datasheet Ton mintues = 1s
    digitalWrite(PWR_PIN, LOW);
}





bool enableGPS(void)
{
    modem.sendAT("+CGPIO=0,48,1,1");
    if (modem.waitResponse(10000L) != 1)
    {
        Serial.println("⚠️ Set GPS Power HIGH Failed");
        return false;
    }

    if (!modem.enableGPS())
    {
        Serial.println("⚠️ modem.enableGPS() failed");
        return false;
    } else {
        Serial.println("✅ GPS enabled");
        return true;
    }
}


bool initGPS(void)
{
    gpsDataTemporary.gpsDataReady = false;
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    gpsPowerOn();
#if !DEBUG_GPS
    if(!fetchGPSsetup())
    {
        return false;
    }
    else
    {
        return true;
    }
#else
    Serial.println("GPS:      GPS DEBUG       ENABLED");
    return true;
#endif
}


int splitCSV(const String& input, String fields[], int maxFields)
{
    int fieldCount = 0;
    int start = 0;
    int end = 0;

    while ((end = input.indexOf(',', start)) != -1 && fieldCount < maxFields) {
        fields[fieldCount++] = input.substring(start, end);
        start = end + 1;
    }

    if (fieldCount < maxFields) {
        fields[fieldCount++] = input.substring(start);
    }

    return fieldCount;
}


void parseGpsStatus(const String& line, bool& gpsFix, int& gpsUsed, int& gpsVisible, float& accuracy)
{
    gpsFix = false;
    gpsUsed = -1;
    gpsVisible = -1;
    accuracy = -1.0;

    const int maxFields = 30;
    String fields[maxFields];
    int count = splitCSV(line, fields, maxFields);

    if (count > 1 && fields[1].length() > 0) {
        gpsFix = (fields[1].toInt() == 1);
    }

    if (count > 15 && fields[15].length() > 0) {
        gpsUsed = fields[15].toInt();  // Correct: used sats
    }

    if (count > 14 && fields[14].length() > 0) {
        gpsVisible = fields[14].toInt();  // Correct: visible sats
    }

    if (count > 10 && fields[10].length() > 0) {
        accuracy = fields[10].toFloat();
    }
}




bool fetchGPSsetup()
{
    //Serial.println("→ fetchGPSsetup()");

    if(!modem.testAT())
    {
        Serial.println("⚠️ testAT() failed");
        return false;
    }
    if(!enableGPS())
    {
        Serial.println("❌ enableGPS() failed");
        return false;
    }
    
    const int maxTries = 50;
    int tries = 0;
    bool gpsFix;
    int gpsConn;
    int gpsVisible;
    float accuracy;

    while (tries++ < maxTries)
    {
        String line = modem.getGPSraw();
        if (line.length() > 0)
        {
            parseGpsStatus(line, gpsFix, gpsConn, gpsVisible, accuracy);
            
            if(tries < 10) Serial.print("0");

            Serial.print(tries); Serial.print(" / "); Serial.print(maxTries);
            Serial.print(" - gpsFix: "); Serial.print(gpsFix ? "YES" : "NO");
            Serial.print(" - Satellites in use: "); Serial.print(gpsConn);
            Serial.print(" - Visible: "); Serial.print(gpsVisible);
            Serial.print(" - Accuracy: "); Serial.print(accuracy, 1); Serial.println(" m");
            
        }
        
        gpsDataTemporary.gpsStructInUse = true;     //Serial.println("                                  gpsStructInUse = true");

        if (gpsConn > 0 && accuracy > 0 && modem.getGPS(&gpsDataTemporary.lat,
                                                        &gpsDataTemporary.lon,
                                                        &gpsDataTemporary.speed,
                                                        &gpsDataTemporary.alt,
                                                        &gpsDataTemporary.vsat,
                                                        &gpsDataTemporary.usat,
                                                        &gpsDataTemporary.accuracy,
                                                        &gpsDataTemporary.year,
                                                        &gpsDataTemporary.month,
                                                        &gpsDataTemporary.day,
                                                        &gpsDataTemporary.hour,
                                                        &gpsDataTemporary.minute,
                                                        &gpsDataTemporary.second))
        {
            gpsDataTemporary.gpsDataReady = true;
            gpsDataTemporary.gpsStructInUse= false;     //Serial.println("                                  gpsStructInUse = false");
            Serial.print("✅ GPS fix received! Connected to "); Serial.print(gpsConn); Serial.println(" satellittes");
            //printGPS();
            break;
        }
        else
        {
            //Serial.print(".");

        }

        gpsDataTemporary.gpsStructInUse = false;     //Serial.println("                                  gpsStructInUse = false");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
    gpsDataTemporary.gpsStructInUse = false;     //Serial.println("                                  gpsStructInUse = false");

    if (!gpsDataTemporary.gpsDataReady) {
        Serial.println("❌ No GPS fix received within time limit.");
        return false;
    }
    
    return true;
}


bool fetchGPSrunning()
{
#if !DEBUG_GPS
    
    if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(5000)) == pdTRUE)
    {

        gpsDataTemporary.gpsStructInUse = true;     //Serial.println("                                  gpsStructInUse = true");
        unsigned long t0 = millis();

        if (modem.getGPS(&gpsDataTemporary.lat,
                        &gpsDataTemporary.lon,
                        &gpsDataTemporary.speed,
                        &gpsDataTemporary.alt,
                        &gpsDataTemporary.vsat,
                        &gpsDataTemporary.usat,
                        &gpsDataTemporary.accuracy,
                        &gpsDataTemporary.year,
                        &gpsDataTemporary.month,
                        &gpsDataTemporary.day,
                        &gpsDataTemporary.hour,
                        &gpsDataTemporary.minute,
                        &gpsDataTemporary.second))
        {
            gpsDataTemporary.gpsDataReady = true;
            unsigned long duration = millis() - t0;
            //Serial.print("getGPS duration                                    = "); Serial.println(duration);
            gpsDataTemporary.gpsStructInUse = false;     //Serial.println("                                  gpsStructInUse = false");
            xSemaphoreGive(modemMutex);
            return true;
        }
        else
        {
            gpsDataTemporary.gpsDataReady = false;
            unsigned long duration = millis() - t0;
            //Serial.print("getGPS duration                                    = "); Serial.println(duration);
            gpsDataTemporary.gpsStructInUse = false;     //Serial.println("                                  gpsStructInUse = false");
            Serial.println("⚠️ No GPS fix available.");
            xSemaphoreGive(modemMutex);
            return false;
        }
    }
    else
    {
        Serial.println("⚠️ Could not acquire modem mutex for GPS. fetchGPSrunning()");
        vTaskDelay(pdMS_TO_TICKS(1000));
        return false;
    }
#else
    return true;
#endif
}


void recoverGps(void)
{
    if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(3000)) == pdTRUE)
        {
            Serial.println("🔁 GPS recovery: re-enabling GPS...");
            modem.sendAT("+CGNSPWR=0");
            delay(500);
            modem.sendAT("+CGNSPWR=1");
            modem.enableGPS();
            xSemaphoreGive(modemMutex);
        }
        else
        {
            Serial.println("⚠️ Could not acquire mutex for GPS recovery");
        }

}




void printGPS()
{
    Serial.println("");
    Serial.println("Fetched data from GPS");
    Serial.print("latitude:              "); Serial.println(gpsDataTemporary.lat, 6);
    Serial.print("longitude:             "); Serial.println(gpsDataTemporary.lon, 6);
    Serial.print("Speed:                 "); Serial.print(gpsDataTemporary.speed, 2); Serial.println(" km/h");
    Serial.print("Altitude:              "); Serial.print(gpsDataTemporary.alt); Serial.println(" moh");
    Serial.print("VSAT:                  "); Serial.println(gpsDataTemporary.vsat);
    Serial.print("USAT:                  "); Serial.println(gpsDataTemporary.usat);
    Serial.print("Accuracy:              "); Serial.print(gpsDataTemporary.accuracy); Serial.println(" m");
    Serial.print("Year:                  "); Serial.println(gpsDataTemporary.year);
    Serial.print("Month:                 "); Serial.println(gpsDataTemporary.month);
    Serial.print("Day:                   "); Serial.println(gpsDataTemporary.day);
    Serial.print("Hour:                  "); Serial.print(gpsDataTemporary.hour); Serial.println(" hours");
    Serial.print("Minute:                "); Serial.print(gpsDataTemporary.minute); Serial.println(" minutes");
    Serial.print("Second:                "); Serial.print(gpsDataTemporary.second); Serial.println(" seconds");
    Serial.print("Has data been fetched? "); Serial.println(gpsDataTemporary.gpsDataReady);
    Serial.print("Struct in use?         "); Serial.println(gpsDataTemporary.gpsStructInUse);
}


void taskGPS(void *pvParameters)
{
    Serial.println("TaskGPS running");
    GpsCommand cmd = GPS_IDLE;
    DataCommand dataCmd;
    uint8_t gpsRetryCnt = 0;
    //uint16_t gpsRunCnt = 0;

    while (1)
    {
        GpsCommand incomingCmd;
        if (xQueueReceive(gpsQueue, &incomingCmd, portMAX_DELAY) == pdPASS)
        {
            cmd = incomingCmd;
        }

        switch (cmd)
        {
            case GPS_IDLE:
                Serial.println("GPS: idle");
                break;

            case GPS_SAMPLE:
                
                gpsFreeStackPrint.printFreeStack();
                
                if(fetchGPSrunning())                       // Sample taken, everything OK
                {
                    dataCmd = DATA_GPS_DATA_READY;
                    xQueueSend(dataQueue, &dataCmd, portMAX_DELAY);
                    //Serial.print("GPS: sample taken - "); Serial.println(gpsRunCnt);
                    //gpsRunCnt++;
                    gpsRetryCnt = 0;
                }
                else                                        // Sample NOT taken, an error has occured, try again
                {
                    Serial.print("GPS: gpsRetryCnt = "); Serial.println(gpsRetryCnt);
                    gpsRetryCnt++;
                    vTaskDelay(50 / portTICK_PERIOD_MS);
                    GpsCommand gpsCmd = GPS_SAMPLE;
                    xQueueSend(gpsQueue, &gpsCmd, pdMS_TO_TICKS(500));
                }

                if(gpsRetryCnt >= (GPS_MAX_RETRIES/2))      // Try to recover the GPS if half of max retries has been reached
                {
                    Serial.println("GPS: Recover GPS");
                    recoverGps();
                }

                if(gpsRetryCnt >= GPS_MAX_RETRIES)          // Error -> restart system
                {
                    ERRORhandler(GPS_RETRY_ERROR);
                    gpsRetryCnt = 0;        // Should not happend
                }

                break;

            default:
                Serial.println("GPS: Unknown command");
                break;
        }
        cmd = GPS_IDLE;
    }
}
