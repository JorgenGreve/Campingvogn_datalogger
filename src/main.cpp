#define SerialMon Serial        // Set serial for debug console

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Ticker.h>
#include "gps.h"



void setup()
{
    SerialMon.begin(115200); delay(10);    // Set console baud rate
    
    setupGps();
    
}

void loop()
{
   getGps();



}
