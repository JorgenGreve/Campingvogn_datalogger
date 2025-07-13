#ifndef GPS_H
#define GPS_H

//#include <TinyGsmClient.h>

bool initGPS(void);
bool fetchGPSsetup();
bool fetchGPSrunning();
void recoverGps(void);
void printGPS();
void taskGPS(void *pvParameters);

#endif
