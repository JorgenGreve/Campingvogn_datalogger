#ifndef GPS_H
#define GPS_H

#include <TinyGsmClient.h>

void initGPS();
void fetchGPSsetup();
void printGPS();
void taskGPS(void *pvParameters);

#endif
