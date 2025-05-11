#define SerialMon Serial        // Set serial for debug console

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Ticker.h>
#include "gps.h"
#include "data.h"


void setup()
{
    SerialMon.begin(115200); delay(10);    // Set console baud rate
    
    Serial.println("-----------------------------------------------------");
    Serial.println("|                  SYSTEM STARTED                   |");
    Serial.println("-----------------------------------------------------");
    
    setupGPS();

    xTaskCreate(taskGPS, "GPS", 2000, NULL, 1, NULL);

    Serial.println("|--------------------SETUP DONE---------------------|");
}

void loop()
{
    
}
