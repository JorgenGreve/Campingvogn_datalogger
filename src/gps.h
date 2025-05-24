#ifndef GPS_H
#define GPS_H

#include <TinyGsmClient.h>

void initGPS();
bool fetchGPSsetup();
bool fetchGPSrunning();
void printGPS();
void taskGPS(void *pvParameters);

#endif
