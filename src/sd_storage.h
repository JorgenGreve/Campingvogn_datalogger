#ifndef SD_STORAGE_H
#define SD_STORAGE_H

#include "dataTASK.h"
#include "gpsTASK.h"
#include "senseTASK.h"
#include "stdbool.h"
#include "global.h"

void initSD();
bool logSampleToSD(const GpsData &gps, const TempHumidData &temp);
String getOldestSample();
void deleteOldestSample();
void trySendOldestSample();
bool sendToServer(const String &jsonLine);

#endif
