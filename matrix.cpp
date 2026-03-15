#include "matrix.h"

int XY(int x, int y) {
    if (y % 2 == 0) {
        return (y * WIDTH) + (WIDTH - 1 - x);
    } else {
        return (y * WIDTH) + x;
    }
}

void setPixelXY(int x, int y) {
    uint8_t r = ((color >> 16) & 0xFF) * brightness / 255;
    uint8_t g = ((color >> 8)  & 0xFF) * brightness / 255;
    uint8_t b = ( color        & 0xFF) * brightness / 255;

    strip.setPixelColor(XY(x, y), strip.Color(g, r, b));
}

void clearMatrix() {
    for (int i = 0; i < LED_PIXEL_AMOUNT; i++) {
        strip.setPixelColor(i, 0);
    }
}
