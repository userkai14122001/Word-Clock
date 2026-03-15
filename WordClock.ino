#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>

#include "wifi_manager.h"
#include "web_pages.h"
#include "matrix.h"
#include "effects.h"
#include "mqtt_manager.h"
#include "config.h"

// ---------------------------------------------------------
// Globale Objekte
// ---------------------------------------------------------
WebServer server(80);
DNSServer dnsServer;
Preferences prefs;

bool setupMode = false;
unsigned long setupStartTime = 0;

String wifi_ssid = "";
String wifi_pass = "";

String mqtt_server = "";
String mqtt_user   = "";
String mqtt_pass   = "";

// MQTT – Definitionen kommen aus config.h
WiFiClient espClient;
PubSubClient mqtt(espClient);

String device_id     = "wordclock";
String command_topic = "wordclock/set";
String state_topic   = "wordclock/state";

// WordClock Variablen
bool powerState      = true;
String currentEffect = "clock";
uint32_t color       = 0xFF7800;
uint8_t brightness   = 120;

int lastMinute = -1;

// LED-Strip
Adafruit_NeoPixel strip(LED_PIXEL_AMOUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


// ---------------------------------------------------------
// SETUP
// ---------------------------------------------------------
void setup() {
    delay(2000);
    Serial.begin(115200);
    Serial.println();
    Serial.println("=== BOOT START ===");

    strip.begin();
    strip.show();

    tryConnectWiFi();

    if (!setupMode) {
        configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org");

        ArduinoOTA.setHostname("WordClock");
        ArduinoOTA.setPassword("update123");
        ArduinoOTA.begin();

        initMQTT();
    }
}


// ---------------------------------------------------------
// LOOP
// ---------------------------------------------------------
void loop() {

    if (setupMode) {
        handleSetupWeb();
        showAttentionAnimation(0x0000FF);
        return;
    }

    server.handleClient();
    ArduinoOTA.handle();
    mqttLoop();

    if (!powerState) {
        clearMatrix();
        strip.show();
        return;
    }

    if (currentEffect == "love") {
        showLoveYou();
        return;
    }

    if (currentEffect == "fire2d") {
        showFire2D();
        return;
    }

    if (currentEffect == "colorloop") {
        showColorloop();
        return;
    }

    if (currentEffect == "colorwipe") {
        showColorwipe();
        return;
    }

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return;

    int hour   = timeinfo.tm_hour;
    int minute = timeinfo.tm_min;

    if (minute != lastMinute) {
        showTime(hour, minute);
        lastMinute = minute;
        publishState();
    }
}
