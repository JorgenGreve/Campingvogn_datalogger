#include "Arduino.h"
#include "mqtt.h"
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include "global.h"




void connectToMQTT()
{
    if(!gprsConnectionLostFlag)
    {
        Serial.print("Connecting to MQTT broker: ");
        Serial.println(mqtt_server);

        mqttClient.setServer(mqtt_server, mqtt_port);

        int retry = 0;
        while (!mqttClient.connected() && retry++ < 10)
        {
            Serial.print("Attempting MQTT connection... ");
            if (mqttClient.connect("LILYGOClient"))  // client ID
            {
                Serial.println("connected!");
            }
            else
            {
                Serial.print("failed, rc=");
                Serial.print(mqttClient.state());
                Serial.println(" try again in 5 seconds");
                delay(5000);
            }
            taskYIELD();
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}


bool sendDataMQTT(const GpsData& gpsData, const TempHumidData& tempHumidData)
{
    int cnt = 0;
    while (PSRAM_buffer->psramBufferInUse)
    {
        Serial.println("Struct is in use " + String(cnt++));
        taskYIELD();
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    //Serial.println("Post data to server start...");

    char timestamp[32];
    snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02d %02d:%02d:%02d",
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

    static String lastPayload;

    if(payload == lastPayload)
    {
        Serial.println("🔄 Duplicate payload – skipping send.");
        return false;
    }

    lastPayload = payload;

    //Serial.println("MQTT Payload:");
    //Serial.println(payload);

    if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(5000)) == pdTRUE)
    {
        if (!mqttClient.connected())
        {
            connectToMQTT();
            if (!mqttClient.connected())
            {
                Serial.println("MQTT connection failed – aborting send.");
                xSemaphoreGive(modemMutex);
                return false;
            }
        }

        //mqttClient.loop();  // kræves for PubSubClient
        bool success = mqttClient.publish(mqtt_topic, payload.c_str());

        //Serial.print(success ? "MQTT publish OK - " : "MQTT publish FAILED - ");
        //Serial.println(String(timestamp));
        xSemaphoreGive(modemMutex);
        return success;
    }
    else
    {
        Serial.println("⚠️ Could not acquire modem mutex for MQTT publish.");
        vTaskDelay(pdMS_TO_TICKS(1000));
        return false;
    }
}





