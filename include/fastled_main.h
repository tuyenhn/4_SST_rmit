#ifndef FASTLED_MAIN_H
#define FASTLED_MAIN_H

#include <FastLED.h>

#include "gamma.h"
#include "matrix_mapping.h"

#define NUM_LEDS 3163

const uint8_t MWIDTH = 152;
const uint8_t MHEIGHT = 37;

const uint8_t MATRIX_CENTER_X = MWIDTH / 2;
const uint8_t MATRIX_CENTER_Y = MHEIGHT / 2;

const uint8_t MATRIX_CENTRE_X = MATRIX_CENTER_X - 1;
const uint8_t MATRIX_CENTRE_Y = MATRIX_CENTER_Y - 1;

const uint8_t brightness = 50;
uint8_t rotation = 0;

CRGB leds_plus_safety_pixel[NUM_LEDS + 1];
CRGB* const leds(leds_plus_safety_pixel + 1);

// scale the brightness of the screenbuffer down
void fadeAll(byte value) {
    for (int i = 0; i < NUM_LEDS; i++) leds[i].nscale8(value);
}

inline uint16_t XYsafe(uint16_t x, uint16_t y) {
    if (x < 1 || x > 151)
        return -1;
    if (-1 < y && y < 21)
        return topArr[x + y * MWIDTH];
    else if (20 < y && y < 37)
        return btmArr[x + (y - 21) * MWIDTH];
    else
        return -1;
}

// ---------------------------------------------------------------------

CRGB leds2[NUM_LEDS];

uint32_t noise_x;
uint32_t noise_y;
uint32_t noise_z;
uint32_t noise_scale_x;
uint32_t noise_scale_y;

uint8_t noise[MWIDTH][MHEIGHT];
uint8_t noisesmoothing;

uint8_t beatcos8(accum88 beats_per_minute, uint8_t lowest = 0, uint8_t highest = 255, uint32_t timebase = 0, uint8_t phase_offset = 0) {
    uint8_t beat = beat8(beats_per_minute, timebase);
    uint8_t beatcos = cos8(beat + phase_offset);
    uint8_t rangewidth = highest - lowest;
    uint8_t scaledbeat = scale8(beatcos, rangewidth);
    uint8_t result = lowest + scaledbeat;
    return result;
}

void BresenhamLine(int x0, int y0, int x1, int y1, byte colorIndex) {
    BresenhamLine(x0, y0, x1, y1, ColorFromPalette(PaletteCtrl::curPalette, colorIndex));
}

void BresenhamLine(int x0, int y0, int x1, int y1, CRGB color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    while (true) {
        leds[XYsafe(x0, y0)] += color;
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 > dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void Pixel(int x, int y, uint8_t colorIndex) {
    leds[XYsafe(x, y)] = ColorFromPalette(PaletteCtrl::curPalette, colorIndex);
}

// create a square twister to the left or counter-clockwise
// x and y for center, r for radius
void SpiralStream(int x, int y, int r, byte dimm) {
    for (int d = r; d >= 0; d--) {  // from the outside to the inside
        for (int i = x - d; i <= x + d; i++) {
            leds[XYsafe(i, y - d)] += leds[XYsafe(i + 1, y - d)];  // lowest row to the right
            leds[XYsafe(i, y - d)].nscale8(dimm);
        }
        for (int i = y - d; i <= y + d; i++) {
            leds[XYsafe(x + d, i)] += leds[XYsafe(x + d, i + 1)];  // right colum up
            leds[XYsafe(x + d, i)].nscale8(dimm);
        }
        for (int i = x + d; i >= x - d; i--) {
            leds[XYsafe(i, y + d)] += leds[XYsafe(i - 1, y + d)];  // upper row to the left
            leds[XYsafe(i, y + d)].nscale8(dimm);
        }
        for (int i = y + d; i >= y - d; i--) {
            leds[XYsafe(x - d, i)] += leds[XYsafe(x - d, i - 1)];  // left colum down
            leds[XYsafe(x - d, i)].nscale8(dimm);
        }
    }
}

void NoiseVariablesSetup() {
    noisesmoothing = 200;

    noise_x = random16();
    noise_y = random16();
    noise_z = random16();
    noise_scale_x = 6000;
    noise_scale_y = 6000;
}

void FillNoise() {
    for (uint16_t i = 0; i < MWIDTH; i++) {
        uint32_t ioffset = noise_scale_x * (i - MATRIX_CENTRE_Y);

        for (uint16_t j = 0; j < MHEIGHT; j++) {
            uint32_t joffset = noise_scale_y * (j - MATRIX_CENTRE_Y);

            byte data = inoise16(noise_x + ioffset, noise_y + joffset, noise_z) >> 8;

            uint8_t olddata = noise[i][j];
            uint8_t newdata = scale8(olddata, noisesmoothing) + scale8(data, 256 - noisesmoothing);
            data = newdata;

            noise[i][j] = data;
        }
    }
}

void MoveFractionalNoiseX(byte amt = 16) {
    // move delta pixelwise
    for (int y = 0; y < MHEIGHT; y++) {
        uint16_t amount = noise[0][y] * amt;
        byte delta = 31 - (amount / 256);

        for (int x = 0; x < MWIDTH - delta; x++) {
            leds2[XYsafe(x, y)] = leds[XYsafe(x + delta, y)];
        }
        for (int x = MWIDTH - delta; x < MWIDTH; x++) {
            leds2[XYsafe(x, y)] = leds[XYsafe(x + delta - MWIDTH, y)];
        }
    }

    //move fractions
    CRGB PixelA;
    CRGB PixelB;

    for (uint16_t y = 0; y < MHEIGHT; y++) {
        uint16_t amount = noise[0][y] * amt;
        byte delta = 31 - (amount / 256);
        byte fractions = amount - (delta * 256);

        for (uint16_t x = 1; x < MWIDTH; x++) {
            PixelA = leds2[XYsafe(x, y)];
            PixelB = leds2[XYsafe(x - 1, y)];

            PixelA %= 255 - fractions;
            PixelB %= fractions;

            leds[XYsafe(x, y)] = PixelA + PixelB;
        }

        PixelA = leds2[XYsafe(0, y)];
        PixelB = leds2[XYsafe(MWIDTH - 1, y)];

        PixelA %= 255 - fractions;
        PixelB %= fractions;

        leds[XYsafe(0, y)] = PixelA + PixelB;
    }
}

void MoveFractionalNoiseY(byte amt = 16) {
    // move delta pixelwise
    for (int x = 0; x < MWIDTH; x++) {
        uint16_t amount = noise[x][0] * amt;
        byte delta = 31 - (amount / 256);

        for (int y = 0; y < MWIDTH - delta; y++) {
            leds2[XYsafe(x, y)] = leds[XYsafe(x, y + delta)];
        }
        for (int y = MWIDTH - delta; y < MWIDTH; y++) {
            leds2[XYsafe(x, y)] = leds[XYsafe(x, y + delta - MWIDTH)];
        }
    }

    //move fractions
    CRGB PixelA;
    CRGB PixelB;

    for (uint16_t x = 0; x < MHEIGHT; x++) {
        uint16_t amount = noise[x][0] * amt;
        byte delta = 31 - (amount / 256);
        byte fractions = amount - (delta * 256);

        for (uint16_t y = 1; y < MWIDTH; y++) {
            PixelA = leds2[XYsafe(x, y)];
            PixelB = leds2[XYsafe(x, y - 1)];

            PixelA %= 255 - fractions;
            PixelB %= fractions;

            leds[XYsafe(x, y)] = PixelA + PixelB;
        }

        PixelA = leds2[XYsafe(x, 0)];
        PixelB = leds2[XYsafe(x, MWIDTH - 1)];

        PixelA %= 255 - fractions;
        PixelB %= fractions;

        leds[XYsafe(x, 0)] = PixelA + PixelB;
    }
}

void MoveX(byte delta) {
    for (int y = 0; y < MHEIGHT; y++) {
        for (int x = 0; x < MWIDTH - delta; x++) {
            leds2[XYsafe(x, y)] = leds[XYsafe(x + delta, y)];
        }
        for (int x = MWIDTH - delta; x < MWIDTH; x++) {
            leds2[XYsafe(x, y)] = leds[XYsafe(x + delta - MWIDTH, y)];
        }
    }

    // write back to leds
    for (uint16_t y = 0; y < MHEIGHT; y++) {
        for (uint16_t x = 0; x < MWIDTH; x++) {
            leds[XYsafe(x, y)] = leds2[XYsafe(x, y)];
        }
    }
}

void MoveY(byte delta) {
    for (int x = 0; x < MWIDTH; x++) {
        for (int y = 0; y < MHEIGHT - delta; y++) {
            leds2[XYsafe(x, y)] = leds[XYsafe(x, y + delta)];
        }
        for (int y = MHEIGHT - delta; y < MHEIGHT; y++) {
            leds2[XYsafe(x, y)] = leds[XYsafe(x, y + delta - MHEIGHT)];
        }
    }

    // write back to leds
    for (uint16_t y = 0; y < MHEIGHT; y++) {
        for (uint16_t x = 0; x < MWIDTH; x++) {
            leds[XYsafe(x, y)] = leds2[XYsafe(x, y)];
        }
    }
}

void standardNoiseSmearing() {
    noise_x += 1000;
    noise_y += 1000;
    noise_scale_x = 4000;
    noise_scale_y = 4000;
    FillNoise();

    MoveX(3);
    MoveFractionalNoiseY(4);

    MoveY(3);
    MoveFractionalNoiseX(4);
}

// rotates the first 16x16 quadrant 3 times onto a 32x32 (+90 degrees rotation for each one)
void Caleidoscope1() {
    for (int x = 0; x < MATRIX_CENTER_X; x++) {
        for (int y = 0; y < MATRIX_CENTER_Y; y++) {
            leds[XYsafe(MWIDTH - 1 - x, y)] = leds[XYsafe(x, y)];
            leds[XYsafe(MWIDTH - 1 - x, MHEIGHT - 1 - y)] = leds[XYsafe(x, y)];
            leds[XYsafe(x, MHEIGHT - 1 - y)] = leds[XYsafe(x, y)];
        }
    }
}

// copy one diagonal triangle into the other one within a 16x16
void Caleidoscope3() {
    for (int x = 0; x <= MATRIX_CENTRE_X; x++) {
        for (int y = 0; y <= x; y++) {
            leds[XYsafe(x, y)] = leds[XYsafe(y, x)];
        }
    }
}

// ---------------------------------------------------------------------

#define xorswap(a, b) \
    {                 \
        a = a ^ b;    \
        b = a ^ b;    \
        a = a ^ b;    \
    }

uint32_t passThruColor;
boolean passThruFlag = false;

// Expand 16-bit input color (Adafruit_GFX colorspace) to 24-bit (NeoPixel)
// (w/gamma adjustment)
static uint32_t expandColor(uint16_t color) {
    return ((uint32_t)pgm_read_byte(&gamma5[color >> 11]) << 16) |
           ((uint32_t)pgm_read_byte(&gamma6[(color >> 5) & 0x3F]) << 8) |
           pgm_read_byte(&gamma5[color & 0x1F]);
}

// Pass raw color value to set/enable passthrough
void setPassThruColor(uint32_t c) {
    passThruColor = c;
    passThruFlag = true;
}

// Call without a value to reset (disable passthrough)
void setPassThruColor(void) { passThruFlag = false; }

void drawPixel(int16_t x, int16_t y, CRGB color) {
    if ((x < 1) || (y < 0) || (x >= MWIDTH) || (y >= MHEIGHT)) return;

    if (passThruFlag)
        leds[XYsafe(x, y)] = color;
    else
        leds[XYsafe(x, y)] = expandColor(color);
}

void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, CRGB color) {
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        xorswap(x0, y0);
        xorswap(x1, y1);
    }

    if (x0 > x1) {
        xorswap(x0, x1);
        xorswap(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0 <= x1; x0++) {
        if (steep) {
            drawPixel(y0, x0, color);
        } else {
            drawPixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

void drawFastVLine(int16_t x, int16_t y, int16_t h, CRGB color) {
    drawLine(x, y, x, y + h - 1, color);
}

void drawFastHLine(int16_t x, int16_t y, int16_t w, CRGB color) {
    drawLine(x, y, x + w - 1, y, color);
}

void drawCircle(int16_t x0, int16_t y0, int16_t r, CRGB color) {
    // int realR = floor(r/4)
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    drawPixel(x0, y0 + r, color);
    drawPixel(x0, y0 - r, color);
    drawPixel(x0 + r * 5, y0, color);
    drawPixel(x0 - r * 5, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        drawPixel(x0 + x * 5, y0 + y, color);
        drawPixel(x0 - x * 5, y0 + y, color);
        drawPixel(x0 + x * 5, y0 - y, color);
        drawPixel(x0 - x * 5, y0 - y, color);
        drawPixel(x0 + y * 5, y0 + x, color);
        drawPixel(x0 - y * 5, y0 + x, color);
        drawPixel(x0 + y * 5, y0 - x, color);
        drawPixel(x0 - y * 5, y0 - x, color);
    }
}

void drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, CRGB color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        if (cornername & 0x4) {
            drawPixel(x0 + x, y0 + y, color);
            drawPixel(x0 + y, y0 + x, color);
        }
        if (cornername & 0x2) {
            drawPixel(x0 + x, y0 - y, color);
            drawPixel(x0 + y, y0 - x, color);
        }
        if (cornername & 0x8) {
            drawPixel(x0 - y, y0 + x, color);
            drawPixel(x0 - x, y0 + y, color);
        }
        if (cornername & 0x1) {
            drawPixel(x0 - y, y0 - x, color);
            drawPixel(x0 - x, y0 - y, color);
        }
    }
}

void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, CRGB color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        if (cornername & 0x1) {
            drawFastVLine(x0 + x, y0 - y, 2 * y + 1 + delta, color);
            drawFastVLine(x0 + y, y0 - x, 2 * x + 1 + delta, color);
        }
        if (cornername & 0x2) {
            drawFastVLine(x0 - x, y0 - y, 2 * y + 1 + delta, color);
            drawFastVLine(x0 - y, y0 - x, 2 * x + 1 + delta, color);
        }
    }
}

void fillCircle(int16_t x0, int16_t y0, int16_t r, CRGB color) {
    drawFastVLine(x0, y0 - r, 2 * r + 1, color);
    fillCircleHelper(x0, y0, r, 3, 0, color);
}

void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, CRGB color) {
    // w *= 4;
    drawFastHLine(x, y, w, color);
    drawFastHLine(x, y + h - 1, w, color);
    drawFastVLine(x, y, h, color);
    drawFastVLine(x + w - 1, y, h, color);
}

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, CRGB color) {
    for (int16_t i = x; i < x + w; i++) {
        drawFastVLine(i, y, h, color);
    }
}

void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, CRGB color) {
    // smarter version
    drawFastHLine(x + r, y, w - 2 * r, color);          // Top
    drawFastHLine(x + r, y + h - 1, w - 2 * r, color);  // Bottom
    drawFastVLine(x, y + r, h - 2 * r, color);          // Left
    drawFastVLine(x + w - 1, y + r, h - 2 * r, color);  // Right
    // draw four corners
    drawCircleHelper(x + r, y + r, r, 1, color);
    drawCircleHelper(x + w - r - 1, y + r, r, 2, color);
    drawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
    drawCircleHelper(x + r, y + h - r - 1, r, 8, color);
}

void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, CRGB color) {
    // smarter version
    fillRect(x + r, y, w - 2 * r, h, color);

    // draw four corners
    fillCircleHelper(x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color);
    fillCircleHelper(x + r, y + r, r, 2, h - 2 * r - 1, color);
}

void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, CRGB color) {
    drawLine(x0, y0, x1, y1, color);
    drawLine(x1, y1, x2, y2, color);
    drawLine(x2, y2, x0, y0, color);
}

void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, CRGB color) {
    int16_t a, b, y, last;
    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1) {
        xorswap(y0, y1);
        xorswap(x0, x1);
    }
    if (y1 > y2) {
        xorswap(y2, y1);
        xorswap(x2, x1);
    }
    if (y0 > y1) {
        xorswap(y0, y1);
        xorswap(x0, x1);
    }

    if (y0 == y2) {  // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if (x1 < a)
            a = x1;
        else if (x1 > b)
            b = x1;
        if (x2 < a)
            a = x2;
        else if (x2 > b)
            b = x2;
        drawFastHLine(a, y0, b - a + 1, color);
        return;
    }

    int16_t
        dx01 = x1 - x0,
        dy01 = y1 - y0,
        dx02 = x2 - x0,
        dy02 = y2 - y0,
        dx12 = x2 - x1,
        dy12 = y2 - y1;
    int32_t
        sa = 0,
        sb = 0;

    // For upper part of triangle, find scanline crossings for segments
    // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
    // is included here (and second loop will be skipped, avoiding a /0
    // error there), otherwise scanline y1 is skipped here and handled
    // in the second loop...which also avoids a /0 error here if y0=y1
    // (flat-topped triangle).
    if (y1 == y2)
        last = y1;  // Include y1 scanline
    else
        last = y1 - 1;  // Skip it

    for (y = y0; y <= last; y++) {
        a = x0 + sa / dy01;
        b = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        /* longhand:
      a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
      b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
        if (a > b) xorswap(a, b);
        drawFastHLine(a, y, b - a + 1, color);
    }

    // For lower part of triangle, find scanline crossings for segments
    // 0-2 and 1-2.  This loop is skipped if y1=y2.
    sa = dx12 * (y - y1);
    sb = dx02 * (y - y0);
    for (; y <= y2; y++) {
        a = x1 + sa / dy12;
        b = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        /* longhand:
      a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
      b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
        if (a > b) xorswap(a, b);
        drawFastHLine(a, y, b - a + 1, color);
    }
}

#endif