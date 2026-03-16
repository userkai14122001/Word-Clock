#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
// Funktionen aus wifi_manager.cpp
void setupWebRoutes();
void startSetupMode();
void tryConnectWiFi();
void handleSetupWeb();
void performWifiScan();

// Globale Objekte aus WordClock.ino
extern WebServer server;
extern DNSServer dnsServer;
extern Preferences prefs;

extern bool setupMode;
extern unsigned long setupStartTime;

extern String wifi_ssid;
extern String wifi_pass;

extern String mqtt_server;
extern String mqtt_user;
extern String mqtt_pass;

// MQTT-Variablen (kommen aus WordClock.ino)
extern WiFiClient espClient;
extern PubSubClient mqtt;

extern String device_id;
extern String command_topic;
extern String state_topic;

#endif
