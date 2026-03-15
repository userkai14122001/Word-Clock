#include "mqtt_manager.h"
#include "effects.h"
#include "matrix.h"

extern PubSubClient mqtt;
extern WiFiClient espClient;

extern String mqtt_server;
extern String mqtt_user;
extern String mqtt_pass;

extern String device_id;
extern String command_topic;
extern String state_topic;

extern bool powerState;
extern String currentEffect;
extern uint32_t color;
extern uint8_t brightness;

static void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String msg;
    for (unsigned int i = 0; i < length; i++) {
        msg += (char)payload[i];
    }

    DynamicJsonDocument doc(256);
    if (deserializeJson(doc, msg)) return;

    if (doc.containsKey("state"))
        powerState = (String(doc["state"]) == "ON");

    if (doc.containsKey("brightness"))
        brightness = doc["brightness"];

    if (doc.containsKey("color")) {
        int r = doc["color"]["r"];
        int g = doc["color"]["g"];
        int b = doc["color"]["b"];
        color = ( (r << 16) | (g << 8) | b );
    }

    if (doc.containsKey("effect"))
        currentEffect = String(doc["effect"]);

    if (!powerState) {
        clearMatrix();
        strip.show();
    } else if (currentEffect == "love") {
        showLoveYou();
    } else if (currentEffect == "fire2d") {
        showFire2D();
    } else if (currentEffect == "colorloop") {
        showColorloop();
    } else if (currentEffect == "colorwipe") {
        showColorwipe();
    } else {
        struct tm timeinfo;
        if (getLocalTime(&timeinfo))
            showTime(timeinfo.tm_hour, timeinfo.tm_min);
    }

    publishState();
}

void sendDiscovery() {
    DynamicJsonDocument doc(1024);

    doc["name"]      = "WordClock";
    doc["unique_id"] = device_id;
    doc["cmd_t"]     = command_topic;
    doc["stat_t"]    = state_topic;
    doc["schema"]    = "json";

    doc["brightness"] = true;
    doc["effect"]     = true;

    JsonArray modes = doc.createNestedArray("supported_color_modes");
    modes.add("rgb");
    doc["color_mode"] = "rgb";

    JsonArray fx = doc.createNestedArray("effect_list");
    fx.add("clock");
    fx.add("love");
    fx.add("fire2d");
    fx.add("colorloop");
    fx.add("colorwipe");

    JsonObject dev = doc.createNestedObject("device");
    dev["identifiers"][0] = device_id;
    dev["name"]           = "WordClock";
    dev["manufacturer"]   = "Kai";
    dev["model"]          = "XIAO ESP32-C3";

    String payload;
    serializeJson(doc, payload);

    String discovery_topic =
        "homeassistant/light/" + device_id + "/config";

    mqtt.publish(discovery_topic.c_str(), payload.c_str(), true);
}

void publishState() {
    DynamicJsonDocument doc(256);

    doc["state"]      = powerState ? "ON" : "OFF";
    doc["brightness"] = brightness;
    doc["effect"]     = currentEffect;

    JsonObject col = doc.createNestedObject("color");
    col["r"] = (uint8_t)(color >> 16);
    col["g"] = (uint8_t)(color >> 8);
    col["b"] = (uint8_t)(color);

    String payload;
    serializeJson(doc, payload);

    mqtt.publish(state_topic.c_str(), payload.c_str(), true);
}

void mqttReconnect() {
    if (mqtt_server.length() == 0) {
        // MQTT ist deaktiviert
        return;
    }

    if (mqtt.connected()) return;

    while (!mqtt.connected()) {
        if (mqtt.connect("WordClockClient",
                         mqtt_user.length() ? mqtt_user.c_str() : nullptr,
                         mqtt_pass.length() ? mqtt_pass.c_str() : nullptr,
                         "homeassistant/status", 0, true, "offline")) {

            mqtt.publish("homeassistant/status", "online", true);
            mqtt.subscribe(command_topic.c_str());

            delay(300);
            sendDiscovery();
            delay(200);
            publishState();

        } else {
            delay(2000);
        }
    }
}


void initMQTT() {
    if (mqtt_server.length() == 0) {
        Serial.println("MQTT deaktiviert (kein Server eingetragen)");
        return;
    }

    mqtt.setServer(mqtt_server.c_str(), 1883);
    mqtt.setCallback(mqttCallback);
}


void mqttLoop() {
    if (mqtt_server.length() == 0) {
        // MQTT deaktiviert
        return;
    }

    if (!mqtt.connected()) {
        showAttentionAnimation(0xFFFF00);
        mqttReconnect();
    }

    mqtt.loop();
}
