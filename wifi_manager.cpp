#include "esp_netif_types.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>

#include "wifi_manager.h"
#include "web_pages.h"
#include "effects.h"
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
    WiFi.scanDelete();
    Serial.println("Starte asynchronen WLAN-Scan...");
    WiFi.scanNetworks(true);   // true = async
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
        server.send(200, "text/plain", "scan_started");
    });


    // -----------------------------------------------------
    // Einstellungen speichern
    // -----------------------------------------------------
    server.on("/save", HTTP_POST, []() {
        Serial.println("Save");

        // WLAN speichern
        prefs.begin("wifi", false);

        if (server.hasArg("ssid"))
            prefs.putString("ssid", server.arg("ssid"));

        if (server.hasArg("wifipass"))
            prefs.putString("pass", server.arg("wifipass"));

        Serial.println("Wifi ssid: " + prefs.getString("ssid"));
        Serial.println("Wifi pass: " + prefs.getString("pass"));
        prefs.end();


        // MQTT separat speichern
        prefs.begin("mqtt", false);

        if (server.hasArg("mqtt_host"))
            prefs.putString("host", server.arg("mqtt_host"));

        if (server.hasArg("mqtt_user"))
            prefs.putString("user", server.arg("mqtt_user"));

        if (server.hasArg("mqtt_pass"))
            prefs.putString("pass", server.arg("mqtt_pass"));

        Serial.println("MQTT host: " + prefs.getString("host"));
        Serial.println("MQTT User: " + prefs.getString("user"));
        Serial.println("MQTT Pass: " + prefs.getString("pass"));
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
    // WLAN-Konfig aus NVS lesen
    prefs.begin("wifi", true);
    wifi_ssid = prefs.getString("ssid", "");
    wifi_pass = prefs.getString("pass", "");
    prefs.end();

    // Keine SSID gespeichert → direkt Setup-Mode
    if (wifi_ssid == "") {
        startSetupMode();
        return;
    }

    // Sofort Setup-Mode starten (AP + DNS + Webserver)
    startSetupMode();   // AP ist jetzt aktiv, Captive Portal läuft

    // Parallel dazu WLAN-Client versuchen
    Serial.println("Versuche WLAN-Verbindung mit gespeicherten Daten...");
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());

    unsigned long startAttempt = millis();
    const unsigned long timeout = 15000; // 15 Sekunden

    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < timeout) {
        // WLAN-Animation
        if (wifi_ssid == "") {
            showWifiRingAnimation(0x0000FF);   // z.B. Blau
        } else {
            showWifiRingAnimation(0xffff00);   // z.B. Gelb
        }

        // Captive Portal / Webserver weiter bedienen
        dnsServer.processNextRequest();
        server.handleClient();
    }

    // Wenn nach Timeout immer noch kein WLAN → im Setup-Mode bleiben
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WLAN fehlgeschlagen → bleibe im Setup-Mode (AP aktiv)");
        setupMode = true;
        return;
    }

    // WLAN erfolgreich
    Serial.println("WLAN verbunden: " + WiFi.localIP().toString());
    setupMode = false;

    // AP wieder abschalten, nur noch STA
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);

    // Webrouten für normalen Betrieb (falls getrennt nötig)
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
    
    // Asynchronen WLAN-Scan prüfen
    int n = WiFi.scanComplete();
    if (n >= 0) {
        Serial.printf("SCAN FOUND %d NETWORKS\n", n);

        DynamicJsonDocument doc(2048);
        JsonArray arr = doc.to<JsonArray>();

        for (int i = 0; i < n; i++) {
            arr.add(WiFi.SSID(i));
        }

        String json;
        serializeJson(arr, json);
        cachedScanResult = json;

        WiFi.scanDelete();   // Speicher freigeben
    }

    if (millis() - setupStartTime > 5UL * 60UL * 1000UL) {
        ESP.restart();
    }
}

