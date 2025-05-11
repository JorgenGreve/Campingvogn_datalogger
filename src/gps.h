#ifndef GPS_H
#define GPS_H

#include <TinyGsmClient.h>


void enableGPS(void);
void disableGPS(void);
void modemPowerOn();
void modemPowerOff();
void modemRestart();
void setupGps();
void getGps();



#endif
