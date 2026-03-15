#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>

#include "wifi_manager.h"
#include "web_pages.h"

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

// ---------------------------------------------------------
// Cache für WLAN-Scan
// ---------------------------------------------------------
String cachedScanResult = "[]";


// ---------------------------------------------------------
// WLAN-Scan durchführen und Ergebnis cachen
// ---------------------------------------------------------
void performWifiScan() {
    Serial.println("Initialer WLAN-Scan...");

    WiFi.mode(WIFI_AP_STA);
    WiFi.disconnect();
    delay(150);

    int n = WiFi.scanNetworks();
    Serial.printf("SCAN FOUND %d NETWORKS\n", n);

    DynamicJsonDocument doc(2048);
    JsonArray arr = doc.to<JsonArray>();

    Serial.println("Gefundene Netzwerke");
    for (int i = 0; i < n; i++) {
        arr.add(WiFi.SSID(i));
        Serial.print(WiFi.SSID(i) + ", ");
    }
    
    String json;
    serializeJson(arr, json);
    cachedScanResult = json;
}



// ---------------------------------------------------------
// ALLE Webserver-Routen (AP + WLAN)
// ---------------------------------------------------------
void setupWebRoutes() {

    // Index
    server.on("/", []() {
        server.send(200, "text/html", pageIndex());
    });

    // WLAN-Seite
    server.on("/wifi", []() {
        server.send(200, "text/html", pageWifi());
    });

    // MQTT-Seite
    server.on("/mqtt", []() {
        server.send(200, "text/html", pageMqtt());
    });

    // Reboot-Seite
    server.on("/reboot", []() {
        server.send(200, "text/html", pageReboot());
    });


    // -----------------------------------------------------
    // DO REBOOT
    // -----------------------------------------------------
    server.on("/do_reboot", HTTP_POST, []() {
        server.send(200, "text/plain", "Rebooting...");
        ESP.restart();
    });

    // -----------------------------------------------------
    // WLAN-Scan (liefert gecachten Scan)
    // -----------------------------------------------------
    server.on("/scan", []() {
        server.send(200, "application/json", cachedScanResult);
    });

    // Optional: Live-Scan
    server.on("/scan_live", []() {
        performWifiScan();
        server.send(200, "application/json", cachedScanResult);
    });

    // -----------------------------------------------------
    // Einstellungen speichern
    // -----------------------------------------------------
    server.on("/save", HTTP_POST, []() {
        Serial.println("Save");

        prefs.begin("config", false);

        if (server.hasArg("ssid"))
            prefs.putString("wifi_ssid", server.arg("ssid"));

        if (server.hasArg("wifipass"))
            prefs.putString("wifi_pass", server.arg("wifipass"));

        if (server.hasArg("mqtt_host"))
            prefs.putString("mqtt_host", server.arg("mqtt_host"));

        if (server.hasArg("mqtt_user"))
            prefs.putString("mqtt_user", server.arg("mqtt_user"));

        if (server.hasArg("mqtt_pass"))
            prefs.putString("mqtt_pass", server.arg("mqtt_pass"));

        Serial.println("Wifi ssid: " + prefs.getString("wifi_ssid"));
        Serial.println("Wifi pass: " + prefs.getString("wifi_pass"));
        Serial.println("MQTT host: " + prefs.getString("mqtt_host"));
        Serial.println("MQTT User: " + prefs.getString("mqtt_user"));
        Serial.println("MQTT Pass: " + prefs.getString("mqtt_pass"));
        prefs.end();

        server.send(200, "text/html", pageReboot());

    });

    // -----------------------------------------------------
    // Factory Reset
    // -----------------------------------------------------
    server.on("/factoryreset", []() {
        Serial.println("Factory Reset!");

        prefs.begin("config", false);
        prefs.clear();
        prefs.end();

        server.send(200, "text/plain", "Reset OK");
        delay(300);
        ESP.restart();
    });

    // -----------------------------------------------------
    // Fallback / Captive Portal
    // -----------------------------------------------------
    server.onNotFound([]() {
        Serial.println("Not found: " + server.uri());
        server.sendHeader("Location", "/", true);
        server.send(302, "text/plain", "");
    });
}



// ---------------------------------------------------------
// Setup-Modus starten (AP + DNS + Webserver + Sofort-Scan)
// ---------------------------------------------------------
void startSetupMode() {
    setupMode = true;
    setupStartTime = millis();

    Serial.println("=== ENTERING SETUP MODE ===");

    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_OFF);
    delay(80);

    WiFi.mode(WIFI_AP);
    WiFi.softAP("WordClock-Setup", nullptr);

    IPAddress apIP = WiFi.softAPIP();
    Serial.println("AP gestartet: WordClock-Setup");
    Serial.println("AP IP: " + apIP.toString());

    dnsServer.start(53, "*", apIP);

    performWifiScan();

    setupWebRoutes();
    server.begin();
    Serial.println("Webserver im Setup-Mode gestartet");
}



// ---------------------------------------------------------
// WLAN verbinden oder Setup starten
// ---------------------------------------------------------
void tryConnectWiFi() {
    prefs.begin("config", true);
    wifi_ssid   = prefs.getString("wifi_ssid", "");
    wifi_pass   = prefs.getString("wifi_pass", "");
    mqtt_server = prefs.getString("mqtt_host", "");
    mqtt_user   = prefs.getString("mqtt_user", "");
    mqtt_pass   = prefs.getString("mqtt_pass", "");
    prefs.end();

    if (wifi_ssid == "") {
        Serial.println("Keine WLAN-Daten → Setup-Modus");
        startSetupMode();
        return;
    }

    Serial.println("Verbinde mit WLAN: " + wifi_ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());

    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED &&
           millis() - startAttempt < 15000) {
        delay(300);
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WLAN fehlgeschlagen → Setup-Modus");

        WiFi.disconnect(true, true);
        delay(200);
        WiFi.mode(WIFI_OFF);
        delay(200);

        startSetupMode();
        return;
    }

    Serial.println("WLAN verbunden: " + WiFi.localIP().toString());
    setupMode = false;

    setupWebRoutes();
    server.begin();
    Serial.println("Webserver im WLAN gestartet");
}



// ---------------------------------------------------------
// Setup-Webserver laufen lassen
// ---------------------------------------------------------
void handleSetupWeb() {
    if (!setupMode) return;

    dnsServer.processNextRequest();
    server.handleClient();

    if (millis() - setupStartTime > 5UL * 60UL * 1000UL) {
        ESP.restart();
    }
}
