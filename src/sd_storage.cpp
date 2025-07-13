// sd_storage.cpp

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <ArduinoJson.h>
#include "sd_storage.h"
#include "database.h"
#include "global.h"

#define SD_CS 13

SPIClass sdSPI(HSPI);

void initSD()
{
    sdSPI.begin(14, 2, 15); // SCK, MISO, MOSI
    if (!SD.begin(SD_CS, sdSPI)) {
        Serial.println("[SD] Initialization failed!");
    } else {
        Serial.println("[SD] Initialization successful.");
    }
}

bool logSampleToSD(const GpsData &gps, const TempHumidData &temp)
{
    if(gps.gpsStructInUse || temp.tempHumidStructInUse)
    {
        return false;
    }
    char timestamp[20];
    sprintf(timestamp, "%04d-%02d-%02d %02d:%02d:%02d",
            gps.year, gps.month, gps.day,
            gps.hour, gps.minute, gps.second);

    String json = "{";
    json += "\"timestamp\":\"" + String(timestamp) + "\",";
    json += "\"lat\":" + String(gps.lat, 6) + ",";
    json += "\"lon\":" + String(gps.lon, 6) + ",";
    json += "\"speed\":" + String(gps.speed, 2) + ",";
    json += "\"alt\":" + String(gps.alt, 1) + ",";
    json += "\"temp_in\":" + String(temp.tempCaravan, 1) + ",";
    json += "\"hum_in\":" + String(temp.humidCaravan, 1) + ",";
    json += "\"temp_out\":" + String(temp.tempOutside, 1) + ",";
    json += "\"hum_out\":" + String(temp.humidOutside, 1);
    json += "}";

    File logFile = SD.open("/samples.txt", FILE_APPEND);
    if (logFile) {
        logFile.println(json);
        logFile.close();
        Serial.println("[SD] Sample logged: " + json);
    } else {
        Serial.println("[SD] Failed to open samples.txt for writing.");
    }
    
    return true;
}

String getOldestSample()
{
    File logFile = SD.open("/samples.txt", FILE_READ);
    if (!logFile) return "";

    String firstLine = logFile.readStringUntil('\n');
    logFile.close();
    return firstLine;
}

void deleteOldestSample()
{
    File logFile = SD.open("/samples.txt", FILE_READ);
    File tempFile = SD.open("/temp.txt", FILE_WRITE);
    if (!logFile || !tempFile) return;

    logFile.readStringUntil('\n'); // skip first line

    while (logFile.available()) {
        String line = logFile.readStringUntil('\n');
        tempFile.println(line);
    }

    logFile.close();
    tempFile.close();

    SD.remove("/samples.txt");
    SD.rename("/temp.txt", "/samples.txt");
    Serial.println("[SD] Oldest sample deleted.");
}

/*
void trySendOldestSample()
{
    // Hurtig check: findes filen, og er den ikke tom?
    File f = SD.open("/samples.txt", FILE_READ);
    if (!f || !f.available()) {
        f.close();
        return;
    }
    f.close();

    String line = getOldestSample();
    if (line == "") return;

    bool success = sendToServer(line);
    if (success) {
        deleteOldestSample();
    }
}


bool sendToServer(const String &jsonLine)
{
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, jsonLine);
    if (error) {
        Serial.println("[SD] JSON parse failed: " + String(error.c_str()));
        return false;
    }

    GpsData gps;
    TempHumidData temp;

    String ts = doc["timestamp"];
    sscanf(ts.c_str(), "%d-%d-%d %d:%d:%d",
           &gps.year, &gps.month, &gps.day,
           &gps.hour, &gps.minute, &gps.second);

    gps.lat = doc["lat"] | 0.0;
    gps.lon = doc["lon"] | 0.0;
    gps.speed = doc["speed"] | 0.0;
    gps.alt = doc["alt"] | 0.0;
    gps.gpsDataReady = true;
    gps.gpsStructInUse = false;

    temp.tempCaravan = doc["temp_in"] | 0.0;
    temp.humidCaravan = doc["hum_in"] | 0.0;
    temp.tempOutside = doc["temp_out"] | 0.0;
    temp.humidOutside = doc["hum_out"] | 0.0;
    temp.tempHumidStructInUse = false;

    return true; // postDataToServer(gps, temp);
}
*/



//************************************************************
// Name:        saveCombinedDataAsJSONtoSD
// Description: Take all samples from combinedData and save
//              them to SD card as JSON
//************************************************************
//extern CombinedData combinedData[MAX_MEASUREMENTS]; // Hvis defineret andetsteds
void saveCombinedDataAsJSONtoSD()
{
    File file = SD.open("/data.json", FILE_APPEND);
    if (!file)
    {
        Serial.println("Failed to open file for writing.");
        return;
    }

    for (int i = 0; i < MAX_MEASUREMENTS; i++)
    {
        /*
        GpsData &gps = combinedData[i].gpsData;
        TempHumidData &th = combinedData[i].tempHumidData;
        
        file.print("{");

        file.print("\"lat\":"); file.print(gps.lat, 6); file.print(",");
        file.print("\"lon\":"); file.print(gps.lon, 6); file.print(",");
        file.print("\"speed\":"); file.print(gps.speed, 2); file.print(",");
        file.print("\"alt\":"); file.print(gps.alt, 2); file.print(",");
        file.print("\"vsat\":"); file.print(gps.vsat); file.print(",");
        file.print("\"usat\":"); file.print(gps.usat); file.print(",");
        file.print("\"accuracy\":"); file.print(gps.accuracy, 2); file.print(",");
        file.print("\"year\":"); file.print(gps.year); file.print(",");
        file.print("\"month\":"); file.print(gps.month); file.print(",");
        file.print("\"day\":"); file.print(gps.day); file.print(",");
        file.print("\"hour\":"); file.print(gps.hour); file.print(",");
        file.print("\"minute\":"); file.print(gps.minute); file.print(",");
        file.print("\"second\":"); file.print(gps.second); file.print(",");

        file.print("\"tempCaravan\":"); file.print(th.tempCaravan, 2); file.print(",");
        file.print("\"humidCaravan\":"); file.print(th.humidCaravan, 2); file.print(",");
        file.print("\"tempOutside\":"); file.print(th.tempOutside, 2); file.print(",");
        file.print("\"humidOutside\":"); file.print(th.humidOutside, 2);
        */
        file.println("}");
    }

    file.close();
    Serial.println("Combined data written to SD as JSON.");
}
