#ifndef MQTT_H
#define MQTT_H

#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include "global.h"

extern TinyGsmClient gsmClient;
extern PubSubClient mqttClient;

extern const char *mqtt_server;
extern const int mqtt_port;
extern const char *mqtt_topic;

void connectToMQTT();
bool sendDataMQTT(const GpsData &gpsData, const TempHumidData &tempHumidData);

#endif