#ifndef GSM_H
#define GSM_H

#pragma once

#include "dataTASK.h"  // ← Tilføj denne (eller hvad filen med structs hedder)

bool initGPRS(void);
bool checkGPRS(void);
bool connectGPRS();
//bool postDataToServer(const GpsData& gpsData, const TempHumidData& tempHumidData);
void taskGPRS(void *pvParameters);
#endif
