#ifndef GSM_H
#define GSM_H

#pragma once

#include "data.h"  // ← Tilføj denne (eller hvad filen med structs hedder)

void setupGPRS();
void checkGPRS();
bool connectGPRS();
bool isDataConnected();
bool postDataToServer(const GpsData& gpsData, const TempHumidData& tempHumidData);
void taskGPRS(void *pvParameters);
#endif
