#include "effects.h"

// Heatmap für Fire2D
uint8_t heat2D[HEIGHT][WIDTH];

// -------------------------------
// ATTENTUION
// -------------------------------
void showAttentionAnimation(uint32_t col) {
    static uint16_t phase = 0;
    phase++;

    clearMatrix();

    // Langsamer Puls (phase * Geschwindigkeit)
    float pulse = (sin(phase * 0.01) * 0.5f + 0.5f);

    // Dunkler machen (max 10% Helligkeit)
    pulse *= 0.9f;

    uint8_t r = ((col >> 16) & 0xFF) * pulse;
    uint8_t g = ((col >> 8)  & 0xFF) * pulse;
    uint8_t b = ( col        & 0xFF) * pulse;

    uint32_t c = strip.Color(r, g, b);

    // Oberer Rand
    for (int x = 0; x < WIDTH; x++)
        strip.setPixelColor(XY(x, 0), c);

    // Unterer Rand
    for (int x = 0; x < WIDTH; x++)
        strip.setPixelColor(XY(x, HEIGHT - 1), c);

    // Linker Rand
    for (int y = 0; y < HEIGHT; y++)
        strip.setPixelColor(XY(0, y), c);

    // Rechter Rand
    for (int y = 0; y < HEIGHT; y++)
        strip.setPixelColor(XY(WIDTH - 1, y), c);

    strip.show();
}

// -------------------------------
// LOVE YOU
// -------------------------------
void showLoveYou() {
    clearMatrix();
    // Einfaches Herz / LOVE-Muster
    for (int i = 3; i < 7; i++) setPixelXY(i, 3);
    for (int i = 4; i < 7; i++) setPixelXY(i, 5);
    strip.show();
}

// -------------------------------
// COLORLOOP
// -------------------------------
void showColorloop() {
    static uint16_t baseHue = 0;
    baseHue += 15;

    for (int i = 0; i < LED_PIXEL_AMOUNT; i++) {
        uint16_t pixelHue = baseHue + (i * 300);
        uint32_t c = strip.gamma32(strip.ColorHSV(pixelHue));

        uint8_t r = ((c >> 16) & 0xFF) * brightness / 255;
        uint8_t g = ((c >> 8)  & 0xFF) * brightness / 255;
        uint8_t b = ( c        & 0xFF) * brightness / 255;

        strip.setPixelColor(i, strip.Color(r, g, b));
    }

    strip.show();
}

// -------------------------------
// COLORWIPE
// -------------------------------
void showColorwipe() {
    static int pos = 0;
    static uint16_t hue = 0;

    hue += 20;
    uint32_t c = strip.gamma32(strip.ColorHSV(hue));

    uint8_t r = ((c >> 16) & 0xFF) * brightness / 255;
    uint8_t g = ((c >> 8)  & 0xFF) * brightness / 255;
    uint8_t b = ( c        & 0xFF) * brightness / 255;

    strip.setPixelColor(pos, strip.Color(r, g, b));
    strip.show();

    pos++;
    if (pos >= LED_PIXEL_AMOUNT) pos = 0;
}

// -------------------------------
// FIRE 2D
// -------------------------------
void showFire2D() {

    // 1. Abkühlen
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int cooldown = random(0, 4);
            heat2D[y][x] = (heat2D[y][x] > cooldown) ? heat2D[y][x] - cooldown : 0;
        }
    }

    // 2. Funken unten
    for (int x = 0; x < WIDTH; x++) {
        if (random(0, 4) == 0) {
            int add = random(120, 255);
            int sum = heat2D[HEIGHT - 1][x] + add;
            heat2D[HEIGHT - 1][x] = (sum > 255) ? 255 : sum;
        }
    }

    // 3. Hitze steigt nach oben
    for (int y = 0; y < HEIGHT - 1; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int below = heat2D[y + 1][x];
            if (x > 0) below += heat2D[y + 1][x - 1];
            if (x < WIDTH - 1) below += heat2D[y + 1][x + 1];
            heat2D[y][x] = below / 3;
        }
    }

    // 4. Heat → Farbe
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {

            uint8_t h = heat2D[y][x];
            float damp = 1.0 - (float)y / (HEIGHT * 1.4);
            h = h * damp;

            uint8_t r, g, b;

            if (h > 200) {
                r = 255;
                g = 200 + (h - 200) * 0.55;
                b = 50;
            } else if (h > 120) {
                r = 255;
                g = 100 + (h - 120) * 0.8;
                b = 0;
            } else {
                r = h * 2;
                g = h * 0.3;
                b = 0;
            }

            r = (r * brightness) / 255;
            g = (g * brightness) / 255;

            int px = WIDTH - 1 - x;
            strip.setPixelColor(XY(px, y), strip.Color(g, r, b));
        }
    }

    strip.show();
}

// -------------------------------
// EXTRA MINUTES
// -------------------------------
static void showExtraMinutes(int minute) {
    int extra = minute % 5;

    uint8_t r = ((color >> 16) & 0xFF) * brightness / 255;
    uint8_t g = ((color >> 8)  & 0xFF) * brightness / 255;
    uint8_t b = ( color        & 0xFF) * brightness / 255;

    for (int i = 0; i < extra; i++) {
        strip.setPixelColor(106 + i, strip.Color(r, b, g));
    }
}

// -------------------------------
// ZEIT-ANZEIGE
// -------------------------------
static void word(int x, int y, int len) {
    for (int i = 0; i < len; i++) {
        setPixelXY(x + i, y);
    }
}

void showTime(int hour, int minute) {
    clearMatrix();

    word(0, 0, 2);
    word(3, 0, 3);

    int m = minute / 5;

    switch (m) {
        case 1: word(7, 0, 4); word(7, 3, 4); break;
        case 2: word(0, 1, 4); word(7, 3, 4); break;
        case 3: word(4, 2, 7); word(7, 3, 4); break;
        case 4: word(4, 1, 7); word(7, 3, 4); break;
        case 5: word(7, 0, 4); word(0, 3, 3); word(0, 4, 4); hour++; break;
        case 6: word(0, 4, 4); hour++; break;
        case 7: word(4, 2, 7); word(0, 3, 3); word(0, 4, 4); hour++; break;
        case 8: word(4, 1, 7); word(0, 3, 3); hour++; break;
        case 9: word(0, 1, 4); word(0, 3, 3); hour++; break;
        case 10: word(7, 0, 4); word(0, 3, 3); hour++; break;
    }

    hour = hour % 12;

    switch (hour) {
        case 0:  word(5, 8, 5); break;
        case 1:  word(0, 9, 4); break;
        case 2:  word(7, 7, 4); break;
        case 3:  word(0, 5, 4); break;
        case 4:  word(7, 5, 4); break;
        case 5:  word(7, 4, 4); break;
        case 6:  word(0, 6, 5); break;
        case 7:  word(5, 6, 6); break;
        case 8:  word(1, 8, 4); break;
        case 9:  word(0, 7, 4); break;
        case 10: word(0, 7, 4); break;
        case 11: word(5, 4, 3); break;
    }

    showExtraMinutes(minute);
    strip.show();
}
