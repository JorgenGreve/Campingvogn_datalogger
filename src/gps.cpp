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

void gpsPowerOn(void)
{
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, HIGH);
    delay(1000);    //Datasheet Ton mintues = 1S
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
    gpsData.gpsDataReady= false;
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    gpsPowerOn();

    while(!fetchGPSsetup())
    {
        delay(500);
    }
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


void parseGpsStatus(const String& line, bool& gpsFix, int& gpsConn, int& gpsVisible)
{
    gpsFix = false;     // Does the GPS have a location fix
    gpsConn = -1;       // Hov many satellittes does the GPS have a fix on
    gpsVisible = -1;    // How many satellittes can the GPS "see"

    const int maxFields = 30;
    String fields[maxFields];
    int count = splitCSV(line, fields, maxFields);

    if (count > 1 && fields[1].length() > 0) {
        gpsFix = (fields[1].toInt() == 1);
    }

    if (count > 14 && fields[14].length() > 0) {
        gpsConn = fields[14].toInt();
    }

    if (count > 18 && fields[18].length() > 0) {
        gpsVisible = fields[18].toInt();
    }
}


bool fetchGPSsetup()
{
    Serial.println("→ fetchGPSsetup");

    if(!modem.testAT()) 
    {
        Serial.println("⚠️ testAT() failed");
        return false;
    }

    if(!modem.init())
    {
        Serial.println("❌ modem.init() failed");
        return false;
    }

    if(!enableGPS())
    {
        Serial.println("❌ enableGPS() failed");
        return false;
    }

    const int maxTries = 1000;
    int tries = 0;
    bool gpsFix;
    int gpsConn;
    int gpsVisible;
    
    while (tries++ < maxTries) 
    {
        String line = modem.getGPSraw();
        if (line.length() > 0) 
        {
            parseGpsStatus(line, gpsFix, gpsConn, gpsVisible);

            Serial.print("gpsFix: "); Serial.print(gpsFix); Serial.print(" - "); Serial.print("Satellittes in use: "); Serial.print(gpsConn); Serial.print(" - "); Serial.print("Visible: "); Serial.println(gpsVisible);
        }

        gpsData.gpsStructInUse = true;

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
            gpsData.gpsDataReady = true;
            gpsData.gpsStructInUse= false;
            Serial.print("✅ GPS fix received! Connected to "); Serial.print(gpsConn); Serial.println(" satellittes");
            //printGPS();
            break;
        } 
        else
        {
            //Serial.print(".");

        }

        gpsData.gpsStructInUse = false;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    if (!gpsData.gpsDataReady) {
        Serial.println("❌ No GPS fix received within time limit.");
        return false;
    }

    return true;
}

bool fetchGPSrunning()
{
    if(modem.sendAT("+CGPIO=0,48,1,1"), modem.waitResponse(200) != 1)   // Activate GPS
    {
        Serial.println("⚠️ GPS power on failed.");
        return false;
    }
    
    modem.enableGPS();

    gpsData.gpsStructInUse = true;

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
        gpsData.gpsDataReady = true;        // Måske der bare skulle bruges et lokalt flag til dette
        gpsData.gpsStructInUse = false;
        //Serial.print("📡 GPS fix: ");
        //Serial.print(gpsData.lat, 6);
        //Serial.print(", ");
        //Serial.println(gpsData.lon, 6);
        return true;
    }
    else
    {
        gpsData.gpsDataReady = false;
        gpsData.gpsStructInUse = false;
        Serial.println("⚠️ No GPS fix available.");
        return false;
    }
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
    Serial.print("Has data been fetched? "); Serial.println(gpsData.gpsDataReady);
    Serial.print("Struct in use?         "); Serial.println(gpsData.gpsStructInUse);
}


void taskGPS(void *pvParameters)
{
    Serial.println("TaskGPS running");
    GpsCommand cmd = GPS_RUN;
    DataCommand dataCmd;

    while (1) 
    {
        GpsCommand incomingCmd;
        if (xQueueReceive(gpsQueue, &incomingCmd, 0) == pdPASS) 
        {
            cmd = incomingCmd;
        }

        switch (cmd) 
        {
            case GPS_IDLE:
                vTaskDelay(100 / portTICK_PERIOD_MS);
                break;

            case GPS_RUN:
                if(fetchGPSrunning())
                {
                    vTaskDelay(1000 / portTICK_PERIOD_MS);      // Sample speed
                    dataCmd = GPS_DATA_READY;
                    xQueueSend(dataQueue, &dataCmd, portMAX_DELAY);
                }
                else
                {
                    vTaskDelay(50 / portTICK_PERIOD_MS);      // Small delay if fetch GPS fail -> try again
                }
                break;

            default:
                Serial.println("GPS: Unknown command");
                break;
        }
    }
}
