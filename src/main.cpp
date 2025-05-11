#define SerialMon Serial        // Set serial for debug console

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Ticker.h>
#include "gps.h"
#include "data.h"
//#include "gprs.h"
//#include <TinyGsmClient.h>
//#include <HardwareSerial.h>

#define UART_BAUD       9600
#define PIN_TX          27
#define PIN_RX          26

HardwareSerial SerialAT(1);           // UART1
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

void setup()
{
    SerialMon.begin(115200); delay(10);    // Set console baud rate
    SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);

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
