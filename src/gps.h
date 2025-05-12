#ifndef GPS_H
#define GPS_H

#include <TinyGsmClient.h>

void setupGPS();
void fetchGPS();
void printGPS();
void taskGPS(void *pvParameters);

#endif
