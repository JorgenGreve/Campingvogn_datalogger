#include "Arduino.h"
#include "global.h"


GpsData gpsDataTemporary;
TempHumidData tempHumidDataTemporary;
CombinedData *PSRAM_buffer;

// UART'en til modem – justér hvis du bruger SoftwareSerial eller en anden port
HardwareSerial SerialAT(1);
TinyGsm modem(SerialAT);            // ← den centrale modeminstans
TinyGsmClient gsmClient(modem);
PubSubClient mqttClient(gsmClient);

SemaphoreHandle_t modemMutex = NULL;
SemaphoreHandle_t i2cMutex = NULL;


const char *mqtt_server = "212.237.182.132";  // ← din Raspberry Pi IP
const int mqtt_port = 1883;
const char *mqtt_topic = "caravan/logger";

bool gprsConnectionLostFlag = false;

bool simulateGprsDrop = false;

PrintStack gpsFreeStackPrint("GPS", 5);         // Calls the function every 2 seconds (2 x 5 = 10 seconds)
PrintStack senseFreeStackPrint("SENSE", 5);     // Calls the function every 2 seconds (2 x 5 = 10 seconds)
PrintStack dataFreeStackPrint("DATA", 5);       // Calls the function every 2 seconds (2 x 5 = 10 seconds)
PrintStack mainFreeStackPrint("MAIN", 100);     // Calls the function every 0.1 seconds (0.1 x 100 = 10 seconds)
PrintStack gprsFreeStackPrint("GPRS", 100);     // Calls the function every 0.1 seconds (0.1 x 100 = 10 seconds)

