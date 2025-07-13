#ifndef MODEM_H
#define MODEM_H

#pragma once
#include <TinyGsmClient.h>
#include <HardwareSerial.h>

//Deklarér dem som eksisterende variabler (defineret i main.cpp)
extern HardwareSerial SerialAT;
//extern TinyGsm modem;
//extern TinyGsmClient client;

void initModemSerial();

#endif