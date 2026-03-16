#ifndef EFFECTS_H
#define EFFECTS_H

#include <Arduino.h>
#include "config.h"
#include "matrix.h"

extern uint8_t heat2D[HEIGHT][WIDTH];

void showFire2D();
void showColorloop();
void showColorwipe();
void showLoveYou();
void showTime(int hour, int minute);
void showStartupWave(uint32_t col);
void testTime(int hour, int minute);
void showWifiRingAnimation(uint32_t col);
uint32_t makeColor(uint8_t r, uint8_t g, uint8_t b);

#endif
