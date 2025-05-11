#ifndef GPS_H
#define GPS_H

#include <TinyGsmClient.h>

void setupGPS();
void fetchGPS();
void taskGPS(void *pvParameters);
void printGPS();


#endif
