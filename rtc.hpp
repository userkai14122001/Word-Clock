#include "rtc.h"

// Deine I2C-Pins
#ifdef I2C_CUSTOM_PINS
#define SDA_PIN I2C_SDA_PIN
#define SCL_PIN I2C_SCL_PIN
#else
#define SDA_PIN 8
#define SCL_PIN 9
#endif

RTC_DS3231 rtc;
bool rtc_ok = false;

void rtcInit() {
    Serial.println("RTC: Initialisiere I2C...");

    Wire.setPins(SDA_PIN, SCL_PIN);
    Wire.begin(SDA_PIN, SCL_PIN);

    Serial.println("RTC: Prüfe DS3231...");

    if (!rtc.begin()) {
        Serial.println("RTC: NICHT gefunden!");
        rtc_ok = false;
        return;
    }

    rtc_ok = true;

    if (rtc.lostPower()) {
        Serial.println("RTC: Hat Strom verloren → setze Zeit auf Kompilierzeit");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    Serial.println("RTC: OK");
}

bool rtcAvailable() {
    return rtc_ok;
}

DateTime rtcGetTime() {
    if (rtc_ok) {
        return rtc.now();
    }

    // Fallback: interne millis-basierte Zeit
    uint32_t seconds = millis() / 1000;
    return DateTime(2024, 1, 1, 0, 0, seconds);
}

void rtcSetTime(const DateTime& dt) {
    if (!rtc_ok) return;
    rtc.adjust(dt);
}
