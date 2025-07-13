
// Your GPRS credentials, if any
const char apn[]  = "internet.mtelia.dk";     //SET TO YOUR APN
const char gprsUser[] = "";
const char gprsPass[] = "";

#include <TinyGsmClient.h>
#include <SPI.h>
#include <SD.h>
#include <Ticker.h>
#include <HTTPClient.h>
#include "gprsTASK.h"
#include "modem.h"
#include "dataTASK.h"
#include "message_queue_cmd.h"
#include "message_queue.h"
#include "esp_task_wdt.h"
#include "database.h"
#include <HardwareSerial.h>
#include "mqtt.h"
#include "global.h"

#define GPRS_PWR_PIN     4
#define LED_PIN         12

int counter, lastIndex, numberOfPieces = 24;
String pieces[24], input;

bool waitForSimReady(uint32_t timeout_ms = 10000);

bool initGPRS(void)
{
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // LED off

    if (esp_reset_reason() == ESP_RST_POWERON)
    {
        Serial.println("Modem power-on pulse");
        pinMode(GPRS_PWR_PIN, OUTPUT);
        digitalWrite(GPRS_PWR_PIN, LOW);   // aktiv LOW = power ON
        delay(1000);
        digitalWrite(GPRS_PWR_PIN, HIGH);  // release PWRKEY
        delay(5000);                       // allow time for modem boot
    }
    else
    {
        Serial.println("Modem already powered");
    }

    Serial.println("Waiting for SIM...");
    if (!waitForSimReady(5000))
    {
        Serial.println("⚠️ No SIM detected – restarting modem");
        modem.poweroff(); // eller modem.restart();
        delay(2000);
        modem.restart();
        delay(3000);

        if (!waitForSimReady(2500))
        {
            Serial.println("❌ SIM still not ready – giving up");
            return false; // eller esp_restart();
        }
    }


    bool modemReady = false;
    for (int i = 0; i < 3; i++)
    {
        if (modem.init())
        {
            modemReady = true;
            break;
        }
        Serial.println("Modem init failed, retrying...");
        delay(1500);
    }

    if (!modemReady)
    {
        Serial.println("Fatal: Modem could not be initialized.");
        return false;
    }

    String name = modem.getModemName();
    String info = modem.getModemInfo();

    Serial.println("Modem Name: " + name);
    Serial.println("Modem Info: " + info);

    if (!name.startsWith("SIMCOM_Ltd SIMCOM_SIM7000E"))
    {
        Serial.println("Warning: Wrong modem identification, retrying...");
        modem.restart();
        delay(2000);
        name = modem.getModemName();
        Serial.println("Retry Modem Name: " + name);
    }

    return true;
}



bool waitForSimReady(uint32_t timeout_ms)
{
    uint32_t start = millis();
    bool atReady = false;

    while (millis() - start < timeout_ms)
    {
        if (!atReady)
        {
            if (modem.testAT())
            {
                Serial.println("AT OK");
                atReady = true;
            }
            else
            {
                Serial.print("x"); // ingen AT endnu
                delay(500);
                continue;
            }
        }

        SimStatus sim = modem.getSimStatus();
        if (sim == SIM_READY)
        {
            Serial.println("SIM ready.");
            return true;
        }
        Serial.print(".");
        delay(500);
    }

    Serial.println("SIM not ready after timeout");
    return false;
}



bool connectGPRS()
{
    Serial.println("Waiting for network...");
    modem.sendAT("+CFUN=0");
    if (modem.waitResponse(10000L) != 1) return false;
    vTaskDelay(200 / portTICK_PERIOD_MS);//delay(200);

    if (!modem.setNetworkMode(2)) return false;
    vTaskDelay(200 / portTICK_PERIOD_MS);//delay(200);

    if (!modem.setPreferredMode(3)) return false;
    vTaskDelay(200 / portTICK_PERIOD_MS);//delay(200);

    modem.sendAT("+CFUN=1");
    if (modem.waitResponse(10000L) != 1) return false;
    vTaskDelay(200 / portTICK_PERIOD_MS);//delay(200);

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
    vTaskDelay(500 / portTICK_PERIOD_MS);//delay(500);
    while (SerialAT.available()) {
        Serial.write(SerialAT.read());
    }

    return true;
}


bool reConnectGPRS()
{
    Serial.println("🔄 reConnectGPRS(): Starting...");

    // Nulstil modem
    modem.sendAT("+CFUN=0");
    if (modem.waitResponse(10000L) != 1) return false;
    vTaskDelay(pdMS_TO_TICKS(200));

    if (!modem.setNetworkMode(2)) return false;
    vTaskDelay(pdMS_TO_TICKS(200));

    if (!modem.setPreferredMode(3)) return false;
    vTaskDelay(pdMS_TO_TICKS(200));

    modem.sendAT("+CFUN=1");
    if (modem.waitResponse(10000L) != 1) return false;
    vTaskDelay(pdMS_TO_TICKS(200));

    // Vent på netværk
    if (!modem.waitForNetwork())
    {
        Serial.println("❌ Netværk ikke fundet");
        return false;
    }

    if (!modem.isNetworkConnected())
    {
        Serial.println("❌ Ikke forbundet til netværk");
        return false;
    }

    Serial.println("📶 Netværk fundet – forbinder til GPRS...");

    if (!modem.gprsConnect(apn, gprsUser, gprsPass))
    {
        Serial.println("❌ GPRS forbindelse mislykkedes");
        return false;
    }

    if (!modem.isGprsConnected())
    {
        Serial.println("❌ GPRS ikke tilsluttet");
        return false;
    }

    Serial.println("✅ GPRS forbundet!");

    // Debug info
    Serial.println("CCID: " + modem.getSimCCID());
    Serial.println("IMEI: " + modem.getIMEI());
    Serial.println("Operator: " + modem.getOperator());
    Serial.println("Local IP: " + modem.localIP().toString());
    Serial.println("Signal quality: " + String(modem.getSignalQuality()));

    // Ekstra info (kan også tage tid – derfor delays og watchdog fodring)
    SerialAT.println("AT+CPSI?");
    vTaskDelay(pdMS_TO_TICKS(500));
    while (SerialAT.available())
    {
        Serial.write(SerialAT.read());
        esp_task_wdt_reset(); // Undgå watchdog under læsning
    }

    return true;
}


bool checkGPRS(void)
{
    //Serial.print("GPRS connection check - ");
    if(xSemaphoreTake(modemMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        if(simulateGprsDrop || !modem.isNetworkConnected() || !modem.isGprsConnected())
        {
            Serial.println("❌ GPRS connection lost");
            gprsConnectionLostFlag = true;
            //modem.restart();  // eller modem.init()
            //modem.waitForNetwork();
            if(!modem.gprsConnect(apn, gprsUser, gprsPass))
            {
                xSemaphoreGive(modemMutex);
                Serial.println("❌ Could not reconnect GPRS - gprsConnectionLostFlag = true");
                gprsConnectionLostFlag = true;
                return false;
            }
            xSemaphoreGive(modemMutex);
        }
        else
        {
            xSemaphoreGive(modemMutex);
            //Serial.println("GPRS connection OK - gprsConnectionLostFlag = false");
            gprsConnectionLostFlag = false;
            return true;
        }
    }
    else
    {
        Serial.println("⚠️ Could not acquire modem mutex for GPS. checkGPRS()");
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

    PSRAM_buffer->transmitSampleIndex = 0;

    GprsCommand cmd;
    MainCommand cmdMAIN;
    GpsCommand cmdGPS;

    uint8_t timerCnt = 0;
    uint32_t lastTime = 0;
    uint32_t now = 0;
    uint32_t gprsTransmitCnt = 0;
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
                vTaskDelay(100 / portTICK_PERIOD_MS);
                
                gprsFreeStackPrint.printFreeStack();

                timerCnt++;

                now = millis();
                /*
                if(gprsConnectionLostFlag && now > (lastTime + 5000))
                {
                    //connectGPRS();
                    Serial.println("GPRS: connectGPRS() should run now");
                    Serial.print("PSRAM_buffer index = ["); Serial.print(saveSampleIndex); Serial.println("]");
                    lastTime = now;
                }
                */

                if(!gprsConnectionLostFlag && saveSampleIndex > 0 && timerCnt >= 5)   // vTaskDelay på 100 ms * 5 = 500 ms
                {
                    //Serial.println("GPRS: GPRS_TRANSMIT");
                    //Serial.print("GPRS: gprsConnectionLostFlag = "); Serial.println(gprsConnectionLostFlag);
                    //Serial.print("GPRS: saveSampleIndex        = "); Serial.println(saveSampleIndex);
                    //Serial.print("GPRS: timerCnt               = "); Serial.println(timerCnt);
                    cmd = GPRS_TRANSMIT;
                    xQueueSend(gprsQueue, &cmd, portMAX_DELAY);
                    timerCnt = 0;
                }
                
                //Serial.println("GPRS: GPRS IDLE");
                break;

            case GPRS_TRANSMIT:
                if(!gprsConnectionLostFlag)
                {
                    //Serial.print("GPRS: GPRS_TRANSMIT ["); Serial.print(gprsTransmitCnt); Serial.println("]");
                    gprsTransmitCnt++;
                    if(sendDataMQTT(PSRAM_buffer[txSampleIndex].psramGpsData, PSRAM_buffer[txSampleIndex].psramTempHumidData))
                    {
                        txSampleIndex++;
                    }

                    if(txSampleIndex == saveSampleIndex)    // If the transmission of samples has caught up with the sampling, reset indexes
                    {
                        saveSampleIndex = 0;
                        txSampleIndex = 0;
                    }
                    else if(txSampleIndex >> saveSampleIndex)
                    {
                        saveSampleIndex = 0;
                        txSampleIndex = 0;
                        Serial.print("GPRS: ERROR txSampleIndex is larger than saveSampleIndex");
                    }
                }
            break;
            
            case GPRS_RECONNECT:
                if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
                {
                    if (reConnectGPRS())
                    {
                        gprsConnectionLostFlag = false;
                        Serial.println("✅ GPRS reconnected successfully");
                    }
                    else
                    {
                        gprsConnectionLostFlag = true;
                        Serial.println("❌ GPRS reconnect failed");
                    }

                    xSemaphoreGive(modemMutex);
                    vTaskDelay(pdMS_TO_TICKS(500));
                }
                if(gprsConnectionLostFlag)
                {
                    GprsCommand gprsCmd = GPRS_RECONNECT;
                    xQueueSend(gprsQueue, &gprsCmd, pdMS_TO_TICKS(500));
                }
                break;
            default:
                Serial.println("GPRS: Unknown command");
                Serial.println("GPS: NOT IMPLEMENTED");
                break;
        }
        cmd = GPRS_IDLE;
    }
}
