
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb
//#define SerialAT Serial1


//   Tests enabled

#define TINY_GSM_TEST_GPRS    true
//#define TINY_GSM_TEST_GPS     true
//#define TINY_GSM_POWERDOWN    true

// set GSM PIN, if any
#define GSM_PIN ""

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

#define uS_TO_S_FACTOR 1000000ULL  // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  60          // Time ESP32 will go to sleep (in seconds)

#define UART_BAUD   115200
#define PIN_DTR     25
#define PIN_TX      27
#define PIN_RX      26
#define PWR_PIN     4

#define SD_MISO     2
#define SD_MOSI     15
#define SD_SCLK     14
#define SD_CS       13
#define LED_PIN     12


int counter, lastIndex, numberOfPieces = 24;
String pieces[24], input;


void setupGPRS()
{
    // Set LED OFF
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, HIGH);
    // Starting the machine requires at least 1 second of low level, and with a level conversion, the levels are opposite
    delay(1000);
    digitalWrite(PWR_PIN, LOW);

    SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);

    // Restart takes quite some time
    // To skip it, call init() instead of restart()
    Serial.println("Initializing modem...");
    if (!modem.init()) {
        Serial.println("Failed to restart modem, attempting to continue without restarting");
    }
    
    String name = modem.getModemName();
    delay(500);
    Serial.println("Modem Name: " + name);

    String modemInfo = modem.getModemInfo();
    delay(500);
    Serial.println("Modem Info: " + modemInfo);
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


bool postDataToServer(const GpsData& gpsData, const TempHumidData& tempHumidData)
{
    Serial.println("Post data to server start...");

    // Konstruér timestamp i SQL DATETIME-format
    char timestamp[20];
    sprintf(timestamp, "%04d-%02d-%02d %02d:%02d:%02d",
            gpsData.year, gpsData.month, gpsData.day,
            gpsData.hour, gpsData.minute, gpsData.second);

    // Konstruér POST payload
    String postData = "timestamp=" + String(timestamp) +
                      "&lat=" + String(gpsData.lat, 6) +
                      "&lon=" + String(gpsData.lon, 6) +
                      "&speed=" + String(gpsData.speed, 2) +
                      "&alt=" + String(gpsData.alt, 1) +
                      "&temp_in=" + String(tempHumidData.tempCaravan, 1) +
                      "&hum_in=" + String(tempHumidData.humidCaravan, 1) +
                      "&temp_out=" + String(tempHumidData.tempOutside, 1) +
                      "&hum_out=" + String(tempHumidData.humidOutside, 1);

    Serial.println("POSTing: " + postData);

    // AT-kommandoer
    SerialAT.println("AT+HTTPTERM"); delay(100); // Afslut tidligere session hvis nødvendigt
    SerialAT.println("AT+HTTPINIT"); delay(200);
    SerialAT.println("AT+HTTPPARA=\"CID\",1"); delay(100);
    SerialAT.println("AT+HTTPPARA=\"URL\",\"http://caravan.jorgre.dk/submit.php\""); delay(200);
    SerialAT.println("AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\""); delay(100);

    // Angiv længde og vent på prompt
    SerialAT.print("AT+HTTPDATA="); SerialAT.print(postData.length()); SerialAT.println(",10000");
    delay(100);
    if (SerialAT.find("DOWNLOAD")) {
        SerialAT.print(postData);  // Send data
        delay(500);
    } else {
        Serial.println("Failed to get DOWNLOAD prompt.");
        return false;
    }

    SerialAT.println("AT+HTTPACTION=1");  // 1 = POST
    if (!SerialAT.find("+HTTPACTION: 1,")) {
        Serial.println("POST failed.");
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


void taskGPRS(void *pvParameters)
{
    Serial.println("TaskGPRS running");
    GprsCommand cmd;
    cmd = GPRS_IDLE;

    while (1) 
    {
        xQueueReceive(gprsQueue, &cmd, portMAX_DELAY);

        switch (cmd) 
        {
            case GPRS_IDLE:
            // DO NOTHING
            vTaskDelay(100 / portTICK_PERIOD_MS);
            Serial.println("GPRS: GPRS IDLE");
            break;

            case GPRS_POST_DATA:
            postDataToServer(gpsData, tempHumidData);
            Serial.println("GPRS: Executing POST");
            break;

            case GPRS_CHECK_SIGNAL:
            Serial.println("GPRS: Checking signal strength");
            Serial.println("GPS: NOT IMPLEMENTED");
            break;

            case GPRS_RESET_MODEM:
            Serial.println("GPRS: Resetting modem");
            Serial.println("GPS: NOT IMPLEMENTED");
            break;

            default:
            Serial.println("GPRS: Unknown command");
            Serial.println("GPS: NOT IMPLEMENTED");
            break;
        }
        cmd = GPRS_IDLE;

    }
}
