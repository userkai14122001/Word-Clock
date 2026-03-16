#include "matrix.h"
#include "config.h"

extern uint32_t color;
extern uint8_t brightness;

uint32_t makeColor(uint8_t r, uint8_t g, uint8_t b);

// ---------------------------------------------------------
// XY-Mapping (SERPENTINE – WordClock-Standard)
// ---------------------------------------------------------
uint16_t XY(int x, int y) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return 0;

    // OBERSTE ZEILE BEGINNT RECHTS → LINKS
    if (y % 2 == 0) {
        // gerade Zeile: rechts → links
        return y * WIDTH + (WIDTH - 1 - x);
    } else {
        // ungerade Zeile: links → rechts
        return y * WIDTH + x;
    }
}


// ---------------------------------------------------------
void setPixelXY(int x, int y) {
    uint16_t index = XY(x, y);

    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8)  & 0xFF;
    uint8_t b =  color        & 0xFF;

    strip.setPixelColor(index, makeColor(g, r, b));
}

// ---------------------------------------------------------
void clearMatrix() {
    for (uint16_t i = 0; i < LED_PIXEL_AMOUNT; i++) {
        strip.setPixelColor(i, 0);
    }
}
