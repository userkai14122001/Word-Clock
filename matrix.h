#pragma once

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

extern Adafruit_NeoPixel strip;

uint16_t XY(int x, int y);
void setPixelXY(int x, int y);
void clearMatrix();
