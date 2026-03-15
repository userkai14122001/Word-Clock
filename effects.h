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
void showAttentionAnimation(uint32_t col);

#endif
