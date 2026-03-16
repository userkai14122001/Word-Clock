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
    delay(500);
    Serial.begin(115200);
    delay(5000);
    Serial.println();
    Serial.println("=== BOOT START ===");
    Serial.println("==================");
    Serial.println("Serial commands:");
    Serial.println("creds - - - - - - -  Ausgabe Credentials");
    Serial.println("reboot  - - - - - -  Neustart ");
    Serial.println("time **:**  - - - -  Setzte Zeit");
    Serial.println("debug layout- - - -  Zeigt Zeit in 5m Schritten");
    Serial.println("");
    Serial.println("==================");
    strip.begin();
    strip.show();
    showStartupWave(0x0090FF);


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
    handleSerialCommands();

    if (setupMode) {
        handleSetupWeb();
        showWifiRingAnimation(0x00A0FF);
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

void runFullTimeTest() {
    Serial.println("Starte kompletten WordClock-Testlauf...");

    for (int hour = 0; hour < 12; hour += 1) {

        Serial.printf("Test: %02d:%02d\n", hour, 0);

        showTime(hour, 0);
        strip.show();

        delay(250); // 4 Sekunden pro Anzeige
    }

    for (int minute = 0; minute < 60; minute += 1) {

        Serial.printf("Test: %02d:%02d\n", 5, minute);

        showTime(5, minute);
        strip.show();

        delay(500); // 4 Sekunden pro Anzeige
    }


    Serial.println("WordClock-Testlauf abgeschlossen.");
}

void handleSerialCommands() {
    if (!Serial.available()) return;

    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    // --- REBOOT ---
    if (cmd == "reboot") {
        Serial.println("Starte neu...");
        delay(200);
        ESP.restart();
        return;
    }

    // --- CREDENTIALS ---
    if (cmd == "creds") {
        Serial.println("===== WLAN Credentials =====");
        Serial.print("SSID: ");
        Serial.println(WiFi.SSID());
        Serial.print("Passwort: ");
        Serial.println(WiFi.psk());
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        
        return;
    }

    if (cmd == "creds flush") {

        Serial.println("===== WLAN Credentials löschen =====");

        prefs.begin("wifi", false);
        prefs.putString("ssid", "");
        prefs.putString("pass", "");
        prefs.end();

        Serial.println("Credentials gelöscht. Neustart...");
        delay(500);

        ESP.restart();
        return;
    }



    // --- Debug Layout ---
    if (cmd == "debug layout") {
        runFullTimeTest();
        return;
    }


    Serial.print("Unbekannter Befehl: ");
    Serial.println(cmd);
}
