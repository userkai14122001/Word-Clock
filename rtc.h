#ifndef RTC_H
#define RTC_H

#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>

// Initialisiert RTC (DS3231/DS1307)
void rtcInit();

// Gibt aktuelle Zeit zurück (RTC oder Fallback)
DateTime rtcGetTime();

// Setzt RTC-Zeit (z. B. nach NTP)
void rtcSetTime(const DateTime& dt);

// Prüft, ob RTC verfügbar ist
bool rtcAvailable();

#endif
