#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include "config.h"

void initMQTT();
void mqttReconnect();
void publishState();
void sendDiscovery();
void mqttLoop();

#endif
