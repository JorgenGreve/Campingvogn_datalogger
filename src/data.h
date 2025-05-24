#ifndef DATA_H
#define DATA_H

void taskDATA(void *pvParameters);




struct GpsData {
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
};

struct TempHumidData{
    bool tempHumidStructInUse;
    bool tempHumidDataReady;
    float tempCaravan;
    float humidCaravan;
    float tempOutside;
    float humidOutside;
};



extern GpsData gpsData; // Global instans-erklæring (extern = den findes et andet sted)
extern TempHumidData tempHumidData;

#endif
