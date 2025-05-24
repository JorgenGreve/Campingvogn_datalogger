
// Your GPRS credentials, if any
const char apn[]  = "internet.mtelia.dk";     //SET TO YOUR APN
const char gprsUser[] = "";
const char gprsPass[] = "";

#include <TinyGsmClient.h>
#include <SPI.h>
#include <SD.h>
#include <Ticker.h>
#include <HTTPClient.h>
#include "gprs.h"
#include "modem.h"
#include "data.h"
#include "message_queue_cmd.h"
#include "message_queue.h"
#include "esp_task_wdt.h"

#include <HardwareSerial.h>

#define GPRS_PWR_PIN     4
#define LED_PIN         12

int counter, lastIndex, numberOfPieces = 24;
String pieces[24], input;

void initGPRS()
{
    // Sluk LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    // Tænd for modemet med et "power pulse"
    pinMode(GPRS_PWR_PIN, OUTPUT);
    digitalWrite(GPRS_PWR_PIN, HIGH);
    delay(1000);                    // Minimum 1 sekund "lav" for at tænde
    digitalWrite(GPRS_PWR_PIN, LOW);    // Slut med lav (modsat logik pga. konvertering)
    delay(100);

    SerialAT.println("AT");

    // Initialiser modem (ikke restart – hurtigere og bevarer SIM mm.)
    bool modemReady = false;
    for (int i = 0; i < 3; i++) {
        if (modem.init()) {
            modemReady = true;
            break;
        }
        Serial.println("Modem init failed, retrying...");
        delay(1500);
    }

    if (!modemReady) {
        Serial.println("Fatal: Modem could not be initialized.");
        return;
    }

    // Hent navn og info
    String name = modem.getModemName();
    String info = modem.getModemInfo();

    Serial.println("Modem Name: " + name);
    Serial.println("Modem Info: " + info);

    // Tjek om modem rapporterer korrekt navn
    if (!name.startsWith("SIMCOM_Ltd SIMCOM_SIM7000E")) {
        Serial.println("Warning: Wrong modem identication, retrying...");
        modem.restart();  // fallback
        delay(2000);
        name = modem.getModemName();
        Serial.println("Retry Modem Name: " + name);
    }
}



bool connectGPRS()
{
    Serial.println("Waiting for network...");
    modem.sendAT("+CFUN=0");
    if (modem.waitResponse(10000L) != 1) return false;
    delay(200);

    if (!modem.setNetworkMode(2)) return false;
    delay(200);

    if (!modem.setPreferredMode(3)) return false;
    delay(200);

    modem.sendAT("+CFUN=1");
    if (modem.waitResponse(10000L) != 1) return false;
    delay(200);

    if (!modem.waitForNetwork()) return false;

    if (!modem.isNetworkConnected()) {
        Serial.println("Network not connected");
        return false;
    }

    Serial.println("Network connected");

    Serial.println("Connecting to APN: " + String(apn));
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) return false;

    if (!modem.isGprsConnected()) {
        Serial.println("GPRS not connected");
        return false;
    }

    Serial.println("GPRS connected");

    // Ekstra information
    String ccid = modem.getSimCCID();
    Serial.println("CCID: " + ccid);

    String imei = modem.getIMEI();
    Serial.println("IMEI: " + imei);

    String cop = modem.getOperator();
    Serial.println("Operator: " + cop);

    IPAddress local = modem.localIP();
    Serial.println("Local IP: " + String(local));

    int csq = modem.getSignalQuality();
    Serial.println("Signal quality: " + String(csq));

    // Eventuelt band og teknologi-type
    SerialAT.println("AT+CPSI?");
    delay(500);
    while (SerialAT.available()) {
        Serial.write(SerialAT.read());
    }

    return true;
}

void checkGPRS()
{
    if (!modem.isNetworkConnected() || !modem.isGprsConnected()) 
    {
        connectGPRS();    
        //modem.restart();  // eller modem.init()
        //modem.waitForNetwork();
        //modem.gprsConnect(apn, gprsUser, gprsPass);
    }
}

bool isDataConnected()
{
    bool net = modem.isNetworkConnected();
    bool gprs = modem.isGprsConnected();

    if (!net) {
        Serial.println("❌ Ikke forbundet til mobilnetværk");
    }
    if (!gprs) {
        Serial.println("❌ Ikke forbundet til GPRS/data");
    }

    if (net && gprs) {
        Serial.println("✅ Dataforbindelse OK");
        return true;
    }

    return false;
}


bool waitForResponse(const String& target, int timeoutMs) {
    unsigned long start = millis();
    String buffer;
    while (millis() - start < timeoutMs) {
        while (SerialAT.available()) {
            char c = SerialAT.read();
            buffer += c;
            if (buffer.endsWith(target)) {
                return true;
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS); // Giv CPU-tid til andre tasks
    }
    return false;
}



void taskGPRS(void *pvParameters)
{
    Serial.println("TaskGPRS  running");
    GprsCommand cmd;
    MainCommand cmdMAIN;
    GpsCommand cmdGPS;

    cmd = GPRS_IDLE;

    while (1) 
    {
        GprsCommand incomingCmd;
        if (xQueueReceive(gprsQueue, &incomingCmd, 0) == pdPASS) {
            cmd = incomingCmd;
        }

        switch (cmd) 
        {
            case GPRS_IDLE:
            // DO NOTHING
            vTaskDelay(500 / portTICK_PERIOD_MS);
            //Serial.println("GPRS: GPRS IDLE");
            break;

            case GPRS_SETUP:
            Serial.println("GPRS:     GPRS_SETUP");
            initGPRS();
            connectGPRS();
            Serial.println("");
            Serial.println("GPRS:     GPRS_SETUP            - OK");
            //cmdMAIN = MAIN_SETUP_DONE;
            xQueueSend(mainQueue, &cmdMAIN, portMAX_DELAY);
            cmd = GPRS_IDLE;
            break;

            case GPRS_RUN:
            //postDataToServer(gpsData, tempHumidData);
            vTaskDelay(20000 / portTICK_PERIOD_MS);
            Serial.print("GPRS: GPRS_RUN");
            break;

            default:
            Serial.println("GPRS: Unknown command");
            Serial.println("GPS: NOT IMPLEMENTED");
            break;
        }
        

    }
}
