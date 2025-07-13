#include <Arduino.h>
//#include <TinyGsmClient.h>
#include <HTTPClient.h>
#include "modem.h"
#include "dataTASK.h"
#include "gprsTASK.h"
#include "database.h"
#include "global.h"

//const char* serverName = "http://caravan.jorgre.dk/submit.php"; // Server URL

/*
bool sendDataMQTT(const GpsData& gpsData, const TempHumidData& tempHumidData)
{
    int cnt = 0;
    while (PSRAM_buffer->psramBufferInUse) {
        Serial.println("");
        Serial.print("Struct is in use "); Serial.println(cnt++);
        Serial.println("");
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    Serial.println("");
    Serial.println("Post data to server start...");

    char timestamp[20];
    sprintf(timestamp, "%04d-%02d-%02d %02d:%02d:%02d",
            gpsData.year, gpsData.month, gpsData.day,
            gpsData.hour, gpsData.minute, gpsData.second);

    String payload = "{";
    payload += "\"timestamp\":\"" + String(timestamp) + "\",";
    payload += "\"lat\":" + String(gpsData.lat, 6) + ",";
    payload += "\"lon\":" + String(gpsData.lon, 6) + ",";
    payload += "\"speed\":" + String(gpsData.speed, 2) + ",";
    payload += "\"alt\":" + String(gpsData.alt, 1) + ",";
    payload += "\"temp_in\":" + String(tempHumidData.tempCaravan, 1) + ",";
    payload += "\"hum_in\":" + String(tempHumidData.humidCaravan, 1) + ",";
    payload += "\"temp_out\":" + String(tempHumidData.tempOutside, 1) + ",";
    payload += "\"hum_out\":" + String(tempHumidData.humidOutside, 1);
    payload += "}";

}
*/


/*
bool postDataToServer(const GpsData& gpsData, const TempHumidData& tempHumidData)
{
    //if(gpsData.gpsDataReady== false)
    //{
    //    return false;
    //}
    int cnt = 0;
    while (PSRAM_buffer->psramBufferInUse) {
        Serial.println("");
        Serial.print("Struct is in use "); Serial.println(cnt++);
        Serial.println("");
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    Serial.println("");
    Serial.println("Post data to server start...");

    char timestamp[20];
    sprintf(timestamp, "%04d-%02d-%02d %02d:%02d:%02d",
            gpsData.year, gpsData.month, gpsData.day,
            gpsData.hour, gpsData.minute, gpsData.second);

    String postData = "timestamp=" + String(timestamp) +
                    "&lat=" + String(gpsData.lat, 6) +
                    "&lon=" + String(gpsData.lon, 6) +
                    "&speed=" + String(gpsData.speed, 2) +
                    "&alt=" + String(gpsData.alt, 1) +
                    "&temp_in=" + String(tempHumidData.tempCaravan, 1) +
                    "&hum_in=" + String(tempHumidData.humidCaravan, 1) +
                    "&temp_out=" + String(tempHumidData.tempOutside, 1) +
                    "&hum_out=" + String(tempHumidData.humidOutside, 1);

    //Serial.println("POSTing: " + postData);

    SerialAT.println("AT+HTTPTERM"); vTaskDelay(100 / portTICK_PERIOD_MS);
    SerialAT.println("AT+HTTPINIT"); vTaskDelay(200 / portTICK_PERIOD_MS);
    SerialAT.println("AT+HTTPPARA=\"CID\",1"); vTaskDelay(100 / portTICK_PERIOD_MS);
    SerialAT.println("AT+HTTPPARA=\"URL\",\"http://caravan.jorgre.dk/submit.php\""); vTaskDelay(200 / portTICK_PERIOD_MS);
    SerialAT.println("AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\""); vTaskDelay(100 / portTICK_PERIOD_MS);

    SerialAT.print("AT+HTTPDATA=");
    SerialAT.print(postData.length());
    SerialAT.println(",10000");

    bool downloadPromptReceived = false;
    unsigned long startWait = millis();
    while (millis() - startWait < 5000) {
        if (SerialAT.find("DOWNLOAD")) {
            downloadPromptReceived = true;
            break;
        }
        yield();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    if (!downloadPromptReceived) {
        Serial.println("Failed to get DOWNLOAD prompt.");
        Serial.println("Raw response:");
        while (SerialAT.available()) {
            Serial.write(SerialAT.read());
        }
        return false;
    }

    SerialAT.print(postData);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    SerialAT.println("AT+HTTPACTION=1");

    // Vent op til 10 sekunder på +HTTPACTION-svar
    bool httpActionFound = false;
    startWait = millis();
    while (millis() - startWait < 10000) {
        if (SerialAT.find("+HTTPACTION: 1,")) {
            httpActionFound = true;
            break;
        }
        yield();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    if (!httpActionFound) {
        Serial.println("POST failed.");
        Serial.println("Raw response:");
        while (SerialAT.available()) {
            Serial.write(SerialAT.read());
        }
        return false;
    }

    int statusCode = SerialAT.parseInt();
    int dataLen = SerialAT.parseInt();
    Serial.print("HTTP Status: "); Serial.println(statusCode);

    if (statusCode == 200) {
        SerialAT.println("AT+HTTPREAD");
        if (SerialAT.find("+HTTPREAD:")) {
            String response = SerialAT.readStringUntil('\n');
            Serial.println("Server reply: " + response);
            return response.indexOf("OK") >= 0;
        }
    }

    Serial.println("Server did not return success.");
    return false;
}
*/