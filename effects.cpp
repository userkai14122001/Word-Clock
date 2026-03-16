#include "effects.h"

// ---------------------------------------------------------
// WORD STRUCT
// ---------------------------------------------------------
struct Word {
    int x;
    int y;
    int len;
};

// ---------------------------------------------------------
// WORD DEFINITIONS (aus deinem bisherigen Code extrahiert)
// ---------------------------------------------------------

// Grundwörter   x   y len
Word W_ES      = {0, 0, 2};
Word W_IST     = {3, 0, 3};
Word W_FUENF   = {7, 0, 4};
Word W_ZEHN    = {0, 1, 4};
Word W_ZWANZIG = {4, 1, 7};
Word W_VIERTEL = {4, 2, 7};
Word W_VOR     = {0, 3, 3};
Word W_NACH    = {7, 3, 4};
Word W_HALB    = {0, 4, 4};

// Stundenwörter
Word H_ZWOELF  = {5, 8, 5};
Word H_EINS    = {0, 9, 4};
Word H_EIN     = {0, 9, 3};
Word H_ZWEI    = {7, 7, 4};
Word H_DREI    = {0, 5, 4};
Word H_VIER    = {7, 5, 4};
Word H_FUENF_H = {7, 4, 4};
Word H_SECHS   = {0, 6, 5};
Word H_SIEBEN  = {5, 6, 6};
Word H_ACHT    = {1, 8, 4};
Word H_NEUN    = {3, 7, 4};
Word H_ZEHN_H  = {0, 7, 4};   // <- bitte später korrigieren
Word H_ELF     = {5, 4, 3};

// UHR (Platzhalter)
Word W_UHR     = {8, 9, 3};   // <- bitte später korrigieren

// Extraminuten
Word M1 = {7, 9, 1};   // Minute 1
Word M2 = {8, 9, 1};   // Minute 2
Word M3 = {9, 9, 1};   // Minute 3
Word M4 = {10, 9, 1};   // Minute 4


uint32_t makeColor(uint8_t r, uint8_t g, uint8_t b) {
    // globale Helligkeit
    r = (r * brightness) / 255;
    g = (g * brightness) / 255;
    b = (b * brightness) / 255;

    // GRB + Gamma
    return strip.gamma32(strip.Color(r, g, b));
}


// ---------------------------------------------------------
// DRAW WORD
// ---------------------------------------------------------
void drawWord(const Word& w) {
    for (int i = 0; i < w.len; i++) {
        setPixelXY(w.x + i, w.y);
    }
}

// ---------------------------------------------------------
// FIRE2D HEATMAP
// ---------------------------------------------------------
uint8_t heat2D[HEIGHT][WIDTH];

// ---------------------------------------------------------
// Startup
// ---------------------------------------------------------
void showStartupWave(uint32_t col) {
    const float cx = (WIDTH  - 1) / 2.0f;
    const float cy = (HEIGHT - 1) / 2.0f;
    const float maxDist = sqrt(cx*cx + cy*cy);

    float radius = 0.0f;

    while (radius <= maxDist + 3) {

        clearMatrix();

        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {

                float dx = x - cx;
                float dy = y - cy;
                float dist = sqrt(dx*dx + dy*dy);

                float diff = fabs(dist - radius);

                const float waveWidth = 3.0f; // weich & smooth

                if (diff < waveWidth) {
                    float fade = 1.0f - (diff / waveWidth);

                    uint8_t r = ((col >> 16) & 0xFF) * fade;
                    uint8_t g = ((col >> 8)  & 0xFF) * fade;
                    uint8_t b = ( col        & 0xFF) * fade;

                    strip.setPixelColor(XY(x, y), strip.Color(r, g, b));
                }
            }
        }

        strip.show();

        radius += 0.17f;   // Geschwindigkeit (kleiner = langsamer)
        delay(70);         // Frame‑Rate (größer = langsamer)
    }
}

void showWifiRingAnimation(uint32_t col) {
    static int headPos = 0;
    static unsigned long lastFrame = 0;

    const int frameDelay = 60; // smooth speed
    if (millis() - lastFrame < frameDelay) return;
    lastFrame = millis();

    clearMatrix();

    const int trail = 18;
    const int total = (WIDTH * 2 + HEIGHT * 2) - 4;

    // Farbe dimmen + brightness berücksichtigen
    auto dimColor = [&](uint32_t c, float f) {
        uint8_t r = ((c >> 16) & 0xFF) * f;
        uint8_t g = ((c >> 8)  & 0xFF) * f;
        uint8_t b = ( c        & 0xFF) * f;

        // brightness anwenden
        r = (r * brightness) / 255;
        g = (g * brightness) / 255;
        b = (b * brightness) / 255;

        // gamma anwenden
        return strip.gamma32(strip.Color(r, g, b));
    };

    // Pixel am Ring setzen
    auto setRingPixel = [&](int index, uint32_t c) {
        int count = 0;

        for (int x = 0; x < WIDTH; x++) {
            if (count == index) { strip.setPixelColor(XY(x, 0), c); return; }
            count++;
        }
        for (int y = 1; y < HEIGHT; y++) {
            if (count == index) { strip.setPixelColor(XY(WIDTH - 1, y), c); return; }
            count++;
        }
        for (int x = WIDTH - 2; x >= 0; x--) {
            if (count == index) { strip.setPixelColor(XY(x, HEIGHT - 1), c); return; }
            count++;
        }
        for (int y = HEIGHT - 2; y > 0; y--) {
            if (count == index) { strip.setPixelColor(XY(0, y), c); return; }
            count++;
        }
    };

    // Trail zeichnen
    for (int i = 0; i < trail; i++) {
        int pos = (headPos - i + total) % total;
        float fade = 1.0f - (float)i / trail;
        uint32_t c = dimColor(col, fade);
        setRingPixel(pos, c);
    }

    headPos = (headPos + 1) % total;

    strip.show();
}






// ---------------------------------------------------------
// LOVE YOU
// ---------------------------------------------------------
void showLoveYou() {
    clearMatrix();
    for (int i = 3; i < 7; i++) setPixelXY(i, 3);
    for (int i = 4; i < 7; i++) setPixelXY(i, 5);
    strip.show();
}

// ---------------------------------------------------------
// COLORLOOP
// ---------------------------------------------------------
void showColorloop() {
    static uint16_t baseHue = 0;
    baseHue += 3;

    for (int i = 0; i < LED_PIXEL_AMOUNT; i++) {
        uint16_t pixelHue = baseHue + (i * 300);
        uint32_t c = strip.gamma32(strip.ColorHSV(pixelHue));

        uint8_t r = ((c >> 16) & 0xFF) * brightness / 255;
        uint8_t g = ((c >> 8)  & 0xFF) * brightness / 255;
        uint8_t b = ( c        & 0xFF) * brightness / 255;

        strip.setPixelColor(i, makeColor(r, g, b));    
    }

    strip.show();
}

// ---------------------------------------------------------
// COLORWIPE
// ---------------------------------------------------------
void showColorwipe() {
    static int pos = 0;
    static uint16_t hue = 0;

    hue += 20;
    uint32_t c = strip.gamma32(strip.ColorHSV(hue));

    uint8_t r = ((c >> 16) & 0xFF) * brightness / 255;
    uint8_t g = ((c >> 8)  & 0xFF) * brightness / 255;
    uint8_t b = ( c        & 0xFF) * brightness / 255;

    strip.setPixelColor(pos, makeColor(r, g, b));

    strip.show();

    pos++;
    if (pos >= LED_PIXEL_AMOUNT) pos = 0;
}

// ---------------------------------------------------------
// FIRE 2D
// ---------------------------------------------------------
void showFire2D() {
    static unsigned long lastFrame = 0;
    const int frameDelay = 35;
    if (millis() - lastFrame < frameDelay) return;
    lastFrame = millis();

    // 1) Unterste Zeile warm anheizen (nicht zu hell)
    for (int x = 0; x < WIDTH; x++) {
        heat2D[HEIGHT - 1][x] = random(120, 200);
    }

    // 2) Hitze nach oben ziehen + verwirbeln
    for (int y = 0; y < HEIGHT - 1; y++) {
        for (int x = 0; x < WIDTH; x++) {

            int below      = heat2D[y + 1][x];
            int belowLeft  = (x > 0)         ? heat2D[y + 1][x - 1] : below;
            int belowRight = (x < WIDTH - 1) ? heat2D[y + 1][x + 1] : below;

            int avg = (below + belowLeft + belowRight) / 3;

            // leichte Abkühlung
            avg -= random(0, 10);
            if (avg < 0) avg = 0;

            heat2D[y][x] = avg;
        }
    }

    // 3) Funken erzeugen (selten, hell, steigen nach oben)
    if (random(0, 6) == 0) {  // Wahrscheinlichkeit
        int sx = random(0, WIDTH);
        int sy = HEIGHT - 2;
        heat2D[sy][sx] = 255;  // heller Funke
    }

    // Funken nach oben bewegen
    for (int y = 1; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (heat2D[y][x] > 220 && random(0, 3) == 0) {
                // nach oben wandern
                heat2D[y - 1][x] = heat2D[y][x];
                heat2D[y][x] = 0;
            }
        }
    }

    // 4) Heatmap → warme Farben
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {

            uint8_t h = heat2D[y][x];
            uint32_t c;

            // warme Palette: Rot → Orange → sanftes Gelb
            if (h < 80) {
                c = strip.Color(h * 3, h / 4, 0);  // dunkles Rot
            }
            else if (h < 150) {
                c = strip.Color(255, (h - 80) * 3, 0); // Rot → Orange
            }
            else if (h < 220) {
                c = strip.Color(255, 180 + (h - 150), (h - 150) * 2); // Orange → warmes Gelb
            }
            else {
                c = strip.Color(255, 230, 180); // Funken: warmes Weißgelb
            }

            // Transparenz: oben weniger dicht
            uint8_t fade = map(y, 0, HEIGHT - 1, 120, 255);
            uint8_t r = ((c >> 16) & 0xFF) * fade / 255;
            uint8_t g = ((c >> 8)  & 0xFF) * fade / 255;
            uint8_t b = ( c        & 0xFF) * fade / 255;

            // globale Helligkeit + Gamma
            r = (r * brightness) / 255;
            g = (g * brightness) / 255;
            b = (b * brightness) / 255;

            strip.setPixelColor(XY(x, y), strip.gamma32(strip.Color(g, r, b))   // GRB statt RGB
);

        }
    }

    strip.show();
}



// ---------------------------------------------------------
// EXTRA MINUTES
// ---------------------------------------------------------
static void showExtraMinutes(int minute) {
    int extra = minute % 5;

    if (extra >= 1) drawWord(M1);
    if (extra >= 2) drawWord(M2);
    if (extra >= 3) drawWord(M3);
    if (extra >= 4) drawWord(M4);
}


// ---------------------------------------------------------
// ZEIT-ANZEIGE
// ---------------------------------------------------------
void showTime(int hour, int minute) {
    clearMatrix();

    drawWord(W_ES);
    drawWord(W_IST);

    int m = minute / 5;
    bool fullHour = (m == 0);

    // Minuten
    switch (m) {
        case 0: break;

        case 1: drawWord(W_FUENF);   drawWord(W_NACH); break;                               // 05
        case 2: drawWord(W_ZEHN);    drawWord(W_NACH); break;                               // 10
        case 3: drawWord(W_VIERTEL); drawWord(W_NACH); break;                               // 15
        case 4: drawWord(W_ZWANZIG); drawWord(W_NACH); break;                               // 20

        case 5: drawWord(W_FUENF);   drawWord(W_VOR);  drawWord(W_HALB); hour++; break;     // 25
        case 6: drawWord(W_HALB);    hour++; break;                                         // 30
        case 7: drawWord(W_FUENF);   drawWord(W_NACH); drawWord(W_HALB); hour++; break;     // 35

        case 8: drawWord(W_ZWANZIG); drawWord(W_VOR);  hour++; break;                       // 40
        case 9: drawWord(W_VIERTEL); drawWord(W_VOR);  hour++; break;                       // 45
        case 10:drawWord(W_ZEHN);    drawWord(W_VOR);  hour++; break;                       // 50
        case 11:drawWord(W_FUENF);   drawWord(W_VOR);  hour++; break;                       // 55
    }


    // Stunden
    hour = hour % 12;

    switch (hour) {
        case 0:  drawWord(H_ZWOELF); break;
        case 1:  fullHour ? drawWord(H_EIN) : drawWord(H_EINS); break;
        case 2:  drawWord(H_ZWEI); break;
        case 3:  drawWord(H_DREI); break;
        case 4:  drawWord(H_VIER); break;
        case 5:  drawWord(H_FUENF_H); break;
        case 6:  drawWord(H_SECHS); break;
        case 7:  drawWord(H_SIEBEN); break;
        case 8:  drawWord(H_ACHT); break;
        case 9:  drawWord(H_NEUN); break;
        case 10: drawWord(H_ZEHN_H); break;
        case 11: drawWord(H_ELF); break;
    }

    if (fullHour) {
        drawWord(W_UHR);
    }

    showExtraMinutes(minute);
    strip.show();
}

void testTime(int hour, int minute) {
    Serial.printf("Testzeit gesetzt: %02d:%02d\n", hour, minute);
    showTime(hour, minute);
}
