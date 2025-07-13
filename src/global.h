#ifndef GLOBAL_H
#define GLOBAL_H

#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include "printFreeStack.h"

// GPS
#define GPS_SAMPLE_SPEED_MS             1000

// SENSE
#define SENSE_SAMPLE_SPEED_MS           1000

// PSRAM
#define MAX_PSRAM_SAMPLES               45000


#define txSampleIndex                   PSRAM_buffer->transmitSampleIndex
#define saveSampleIndex                 PSRAM_buffer->storeSampleIndex

#define MAX_MEASUREMENTS                50


extern HardwareSerial SerialAT;
extern TinyGsm modem;
extern TinyGsmClient gsmClient;
extern PubSubClient mqttClient;

extern SemaphoreHandle_t modemMutex;
extern SemaphoreHandle_t i2cMutex;


extern const char *mqtt_server;
extern const int mqtt_port;
extern const char *mqtt_topic;

extern bool gprsConnectionLostFlag;

///////////////////////////////////////// GPS DATA STRUCT /////////////////////////////////////////
// Struct for saving GPS data
typedef struct
{
    bool gpsStructInUse;
    bool gpsDataReady;
    float lat;        // Latitude: Geographical latitude (positive for north, negative for south), typically with a precision of up to 6 decimal places (~0.0001°), accurate to about 1 meter
    float lon;        // Longitude: Geographical longitude (positive for east, negative for west), typically with a precision of up to 6 decimal places (~0.0001°), accurate to about 1 meter
    float speed;      // Speed: Speed in km/h, calculated from GPS data based on movement over time (accuracy can vary with signal quality, but generally within 0.1-0.5 km/h)
    float alt;        // Altitude: Height above sea level in meters (GPS modules like UBLOX NEO-M8N provide altitude accuracy of ~1-2 meters under good conditions)
    int vsat;         // Vertical Satellites: Number of satellites used to calculate vertical position (usually 3-12 satellites, indicates the quality of the vertical fix)
    int usat;         // User Satellites: Total number of satellites used for position fix (typically 3-20 satellites, reflects the overall GPS signal quality)
    float accuracy;   // Accuracy: Estimated GPS position accuracy in meters, often reported as Horizontal Dilution of Precision (HDOP) or a similar value (higher value indicates lower precision)
    int year;         // Year: Current year (4 digits, as reported by GPS system)
    int month;        // Month: Current month (1-12, as reported by GPS system)
    int day;          // Day: Current day of the month (1-31, as reported by GPS system)
    int hour;         // Hour: Current hour of the day (0-23, based on UTC time from GPS satellites)
    int minute;       // Minute: Current minute of the hour (0-59, based on UTC time from GPS satellites)
    int second;       // Second: Current second of the minute (0-59, based on UTC time from GPS satellites)
}GpsData;
extern GpsData gpsDataTemporary;


///////////////////////////////////////// TEMPERATURE & HUMIDITY DATA STRUCT /////////////////////////////////////////
// Struct for saving temperature and humidity data
typedef struct
{
    bool tempHumidStructInUse;
    bool tempHumidDataReadyCaravan;
    bool tempHumidDataReadyOutside;
    float tempCaravan;
    float humidCaravan;
    float tempOutside;
    float humidOutside;
}TempHumidData;
extern TempHumidData tempHumidDataTemporary;


///////////////////////////////////////// COMBINED GPS & TEMPHUMID DATA STRUCT AND PSRAM BUFFER /////////////////////////////////////////
// Struct that combines all sample data into one sample and the PSRAM buffer that stores all samples before transmission
typedef struct
{
    GpsData psramGpsData;
    TempHumidData psramTempHumidData;
    bool psramBufferInUse;
    uint16_t storeSampleIndex;
    uint16_t transmitSampleIndex;
}CombinedData;
extern CombinedData *PSRAM_buffer;
//extern CombinedData combinedData[MAX_MEASUREMENTS];



///////////////////////////////////////// OS TIMER ID'S /////////////////////////////////////////
// Enum for the osTimer ID's
typedef enum
{
    OS_TIMER_GPS_SAMPLE_TIMER = 0,
    OS_TIMER_SENSE_SAMPLE_TIMER,
    OS_TIMER_GPRS_CHECK_TIMER,
    OS_TIMER_GPRS_TEST_LOST_CONN,
    OS_TIMER_GPS_TEST_LOST_SIG,
} OsTimerEnum;
extern OsTimerEnum osTimerID;

extern bool simulateGprsDrop;

class PrintStack;
extern PrintStack gpsFreeStackPrint;
extern PrintStack senseFreeStackPrint;
extern PrintStack dataFreeStackPrint;
extern PrintStack mainFreeStackPrint;
extern PrintStack gprsFreeStackPrint;








#endif