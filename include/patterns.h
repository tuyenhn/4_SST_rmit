#ifndef PATTERNS_H
#define PATTERNS_H

#include "char_map.h"
#include "fastled_main.h"

const CRGBPalette16 GrayscaleColors_p = CRGBPalette16(CRGB::Black, CRGB::White);
const CRGBPalette16 IceColors_p = CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);

class PaletteCtrl {
   public:
    static CRGBPalette16 curPalette;
    static uint8_t palIndex;

    static void loadPalette(uint8_t idx) {
        switch (idx) {
            case 0:
                curPalette = RainbowColors_p;
                break;
            case 1:
                curPalette = OceanColors_p;
                break;
            case 2:
                curPalette = CloudColors_p;
                break;
            case 3:
                curPalette = ForestColors_p;
                break;
            case 4:
                curPalette = PartyColors_p;
                break;
            case 5:
                curPalette = GrayscaleColors_p;
                break;
            case 6:
                curPalette = HeatColors_p;
                break;
            case 7:
                curPalette = LavaColors_p;
                break;
            case 8:
                curPalette = IceColors_p;
                break;
        }
    }

    static void cyclePalette() {
        if (palIndex > 8) palIndex = 0;
        loadPalette(palIndex++);
    }
};

// HEAVILY DERIVED FROM https://github.com/marcmerlin/FastLED_NeoMatrix_SmartMatrix_LEDMatrix_GFX_Demos/tree/master/GFX/Aurora

class Drift {
   public:
    void draw() {
        uint8_t dim = beatsin8(2, 170, 250);
        fadeAll(dim);

        for (uint8_t i = 0; i < MWIDTH; i++) {
            CRGB color;

            uint8_t x = 0;
            uint8_t y = 0;

            if (i < MATRIX_CENTER_X) {
                x = beatcos8((i + 1) * 2, i, MWIDTH - i);
                y = beatsin8((i + 1) * 2, i, MHEIGHT - i);
                color = ColorFromPalette(PaletteCtrl::curPalette, i * 14);
            } else {
                x = beatsin8((MWIDTH - i) * 2, MWIDTH - i, i + 1);
                y = beatcos8((MHEIGHT - i) * 2, MHEIGHT - i, i + 1);
                color = ColorFromPalette(PaletteCtrl::curPalette, (31 - i) * 14);
            }

            drawPixel(x, y, color);
        }
    }
};

Drift drift;

class Pendulum {
   public:
    void draw() {
        fadeAll(170);

        for (int x = 0; x < MWIDTH; x++) {
            //uint8_t y = beatsin8(x + MWIDTH, 0, MHEIGHT);
            uint8_t y = beatsin16(x + 1, 0, MHEIGHT);

            CRGB color = ColorFromPalette(PaletteCtrl::curPalette, x * 7);
            drawPixel(x, y, color);
        }
    }
};

Pendulum pendulum;

class Spiral {
   private:
    // Timer stuff (Oszillators)
    struct timer {
        unsigned long takt;
        unsigned long lastMillis;
        unsigned long count;
        int delta;
        byte up;
        byte down;
    };
    timer multiTimer[5];

    int timers = sizeof(multiTimer) / sizeof(multiTimer[0]);

    // counts all variables with different speeds linear up and down
    void UpdateTimers() {
        unsigned long now = millis();
        for (int i = 0; i < timers; i++) {
            while (now - multiTimer[i].lastMillis >= multiTimer[i].takt) {
                multiTimer[i].lastMillis += multiTimer[i].takt;
                multiTimer[i].count = multiTimer[i].count + multiTimer[i].delta;
                if ((multiTimer[i].count == multiTimer[i].up) || (multiTimer[i].count == multiTimer[i].down)) {
                    multiTimer[i].delta = -multiTimer[i].delta;
                }
            }
        }
    }

   public:
    void start() {
        // set all counting directions positive for the beginning
        for (int i = 0; i < timers; i++) multiTimer[i].delta = 1;

        // set range (up/down), speed (takt=ms between steps) and starting point of all oszillators

        unsigned long now = millis();

        multiTimer[0].lastMillis = now;
        multiTimer[0].takt = 42;  //x1
        multiTimer[0].up = MWIDTH - 1;
        multiTimer[0].down = 0;
        multiTimer[0].count = 0;

        multiTimer[1].lastMillis = now;
        multiTimer[1].takt = 55;  //y1
        multiTimer[1].up = MHEIGHT - 1;
        multiTimer[1].down = 0;
        multiTimer[1].count = 0;

        multiTimer[2].lastMillis = now;
        multiTimer[2].takt = 3;  //color
        multiTimer[2].up = 255;
        multiTimer[2].down = 0;
        multiTimer[2].count = 0;

        multiTimer[3].lastMillis = now;
        multiTimer[3].takt = 71;  //x2
        multiTimer[3].up = MWIDTH - 1;
        multiTimer[3].down = 0;
        multiTimer[3].count = 0;

        multiTimer[4].lastMillis = now;
        multiTimer[4].takt = 89;  //y2
        multiTimer[4].up = MHEIGHT - 1;
        multiTimer[4].down = 0;
        multiTimer[4].count = 0;
    }

    void draw() {
        // manage the Oszillators
        UpdateTimers();

        // draw just a line defined by 5 oszillators
        // this works -- merlin
        BresenhamLine(
            multiTimer[3].count,  // x1
            multiTimer[4].count,  // y1
            multiTimer[0].count,  // x2
            multiTimer[1].count,  // y2
            multiTimer[2].count   // color
        );

        // this doesn't work -- merlin
        // manipulate the screen buffer
        // with fixed parameters (could be oszillators too)
        // center x, y, radius, scale color down
        // --> affects always a square with an odd length
        SpiralStream(15, 15, 16, 128);

        // why not several times?!
        SpiralStream(16, 6, 6, 128);
        SpiralStream(10, 24, 10, 128);

        // increase the contrast
        fadeAll(224);
    }
};

Spiral spiral;

class Wave {
   private:
    byte thetaUpdate = 0;
    byte thetaUpdateFrequency = 0;
    byte theta = 0;

    byte hueUpdate = 0;
    byte hueUpdateFrequency = 0;
    byte hue = 0;

    byte rotation = 0;

    uint8_t scale = max(256 / MWIDTH, 1);

    uint16_t maxX = MWIDTH - 1;
    uint16_t maxY = MHEIGHT - 1;

    uint8_t waveCount = 1;

   public:
    void start() {
        rotation = random(0, 4);
        if (MHEIGHT > MWIDTH) rotation = 1;
        if (MHEIGHT < MWIDTH) rotation = 0;
        waveCount = random(1, 3);
        //thetaUpdateFrequency = random(1, 2);
        //hueUpdateFrequency = random(1, 6);
    }

    void draw() {
        int n = 0;

        switch (rotation) {
            case 0:
                for (int x = 0; x < MWIDTH; x++) {
                    n = quadwave8(x * 2 + theta) / scale;
                    //backgroundLayer.drawPixel(x, n, ColorFromPalette(PaletteCtrl::curPalette, x + hue));
                    CRGB color = ColorFromPalette(PaletteCtrl::curPalette, x + hue);
                    drawPixel(x, n, color);
                    if (waveCount == 2)
                        //backgroundLayer.drawPixel(x, maxY - n, ColorFromPalette(PaletteCtrl::curPalette, x + hue));
                        drawPixel(x, maxY - n, color);
                }
                break;

            case 1:
                for (int y = 0; y < MHEIGHT; y++) {
                    n = quadwave8(y * 2 + theta) / scale;
                    // backgroundLayer.drawPixel(n, y, ColorFromPalette(PaletteCtrl::curPalette, y + hue));
                    CRGB color = ColorFromPalette(PaletteCtrl::curPalette, y + hue);
                    drawPixel(n, y, color);
                    if (waveCount == 2)
                        //backgroundLayer.drawPixel(maxX - n, y, ColorFromPalette(PaletteCtrl::curPalette, y + hue));
                        drawPixel(maxX - n, y, color);
                }
                break;

            case 2:
                for (int x = 0; x < MWIDTH; x++) {
                    n = quadwave8(x * 2 - theta) / scale;
                    // backgroundLayer.drawPixel(x, n, ColorFromPalette(PaletteCtrl::curPalette, x + hue));
                    CRGB color = ColorFromPalette(PaletteCtrl::curPalette, x + hue);
                    drawPixel(x, n, color);
                    if (waveCount == 2)
                        //backgroundLayer.drawPixel(x, maxY - n, ColorFromPalette(PaletteCtrl::curPalette, x + hue));
                        drawPixel(x, maxY - n, color);
                }
                break;

            case 3:
                for (int y = 0; y < MHEIGHT; y++) {
                    n = quadwave8(y * 2 - theta) / scale;
                    // backgroundLayer.drawPixel(n, y, ColorFromPalette(PaletteCtrl::curPalette, y + hue));
                    CRGB color = ColorFromPalette(PaletteCtrl::curPalette, y + hue);
                    drawPixel(n, y, color);
                    if (waveCount == 2)
                        //backgroundLayer.drawPixel(maxX - n, y, ColorFromPalette(PaletteCtrl::curPalette, y + hue));
                        drawPixel(maxX - n, y, color);
                }
                break;
        }

        fadeAll(254);

        if (thetaUpdate >= thetaUpdateFrequency) {
            thetaUpdate = 0;
            theta++;
        } else {
            thetaUpdate++;
        }

        if (hueUpdate >= hueUpdateFrequency) {
            hueUpdate = 0;
            hue++;
        } else {
            hueUpdate++;
        }
    }
};

Wave wave;

class Mandala {
   private:
    // used for the random based animations
    int16_t dx;
    int16_t dy;
    int16_t dz;
    int16_t dsx;
    int16_t dsy;

   public:
    void start() {
        // set to reasonable values to avoid a black out
        noisesmoothing = 200;

        // just any free input pin
        //random16_add_entropy(analogRead(18));

        // fill coordinates with random values
        // set zoom levels
        noise_x = random16();
        noise_y = random16();
        noise_z = random16();
        noise_scale_x = 6000;
        noise_scale_y = 6000;

        // for the random movement
        dx = random8();
        dy = random8();
        dz = random8();
        dsx = random8();
        dsy = random8();
    }

    void draw() {
        // a new parameter set every 15 seconds
        EVERY_N_SECONDS(5) {
            //SetupRandomPalette3();
            dy = random16(500) - 250;  // random16(2000) - 1000 is pretty fast but works fine, too
            dx = random16(500) - 250;
            dz = random16(500) - 250;
            noise_scale_x = random16(10000) + 2000;
            noise_scale_y = random16(10000) + 2000;
        }

        noise_y += dy;
        noise_x += dx;
        noise_z += dz;

        FillNoise();
        ShowNoiseLayer(0, 1, 0);

        Caleidoscope3();
        Caleidoscope1();
    }

    // show just one layer
    void ShowNoiseLayer(byte layer, byte colorrepeat, byte colorshift) {
        for (uint8_t i = 0; i < MWIDTH; i++) {
            for (uint8_t j = 0; j < MHEIGHT; j++) {
                uint8_t color = noise[i][j];

                uint8_t bri = color;

                // assign a color depending on the actual palette
                CRGB pixel = ColorFromPalette(PaletteCtrl::curPalette, colorrepeat * (color + colorshift), bri);
                leds[XYsafe(i, j)] = pixel;
            }
        }
    }
};

Mandala mandala;

class Plasma {
   private:
    int time = 0;
    int cycles = 0;
    CRGBPalette16 pal = CRGBPalette16(CRGB::Black, CRGB::Red, CRGB::Pink, CRGB::White);

   public:
    void draw() {
        for (int x = 0; x < MWIDTH; x++) {
            for (int y = 0; y < MHEIGHT; y++) {
                int16_t v = 0;
                uint8_t wibble = sin8(time);
                v += sin16(x * wibble * 2 + time);
                v += cos16(y * (128 - wibble) * 2 + time);
                v += sin16(y * x * cos8(-time) / 2);

                Pixel(x, y, (v >> 8) + 127);
            }
        }

        time += 1;
        cycles++;

        if (cycles >= 2048) {
            time = 0;
            cycles = 0;
        }
    }
};

Plasma plasma;

class Pulse {
   private:
    int hue;
    int centerX = 0;
    int centerY = 0;
    int step = -1;
    int maxSteps = 16;
    float fadeRate = 0.8;
    int diff;

   public:
    void draw() {
        fadeAll(235);

        if (step == -1) {
            centerX = random(MWIDTH);
            centerY = random(MHEIGHT);
            hue = random(256);  // 170;
            step = 0;
        }

        if (step == 0) {
            //backgroundLayer.drawCircle(centerX, centerY, step, ColorFromPalette(PaletteCtrl::curPalette, hue));
            CRGB color = ColorFromPalette(PaletteCtrl::curPalette, hue);
            setPassThruColor(color.r * 65536 + color.g * 256 + color.b);
            drawCircle(centerX, centerY, step, ColorFromPalette(PaletteCtrl::curPalette, hue));
            setPassThruColor();
            step++;
        } else {
            if (step < maxSteps) {
                // initial pulse
                //backgroundLayer.drawCircle(centerX, centerY, step, ColorFromPalette(PaletteCtrl::curPalette, hue, pow(fadeRate, step - 2) * 255));
                CRGB color = ColorFromPalette(PaletteCtrl::curPalette, hue, pow(fadeRate, step - 2) * 255);
                setPassThruColor(color.r * 65536 + color.g * 256 + color.b);
                drawCircle(centerX, centerY, step, ColorFromPalette(PaletteCtrl::curPalette, hue, pow(fadeRate, step - 2) * 255));

                // secondary pulse
                if (step > 3) {
                    //backgroundLayer.drawCircle(centerX, centerY, step - 3, ColorFromPalette(PaletteCtrl::curPalette, hue, pow(fadeRate, step - 2) * 255));
                    CRGB color = ColorFromPalette(PaletteCtrl::curPalette, hue, pow(fadeRate, step - 2) * 255);
                    setPassThruColor(color.r * 65536 + color.g * 256 + color.b);
                    drawCircle(centerX, centerY, step - 3, ColorFromPalette(PaletteCtrl::curPalette, hue, pow(fadeRate, step - 2) * 255));
                }
                step++;
            } else {
                step = -1;
            }
        }

        standardNoiseSmearing();
    }
};

Pulse pulse;

class Munch {
   private:
    byte count = 0;
    byte dir = 1;
    byte flip = 0;
    byte generation = 0;
    CRGBPalette16 pal = CloudColors_p;

   public:
    void draw() {
        for (byte x = 0; x < MWIDTH; x++) {
            for (byte y = 0; y < MHEIGHT; y++) {
                leds[XYsafe(x, y)] = (x ^ y ^ flip) < count ? ColorFromPalette(pal, ((x ^ y) << 3) + generation) : CRGB::Black;
            }
        }

        count += dir;

        if (count <= 0 || count >= MWIDTH) {
            dir = -dir;
        }

        if (count <= 0) {
            if (flip == 0)
                flip = 31;
            else
                flip = 0;
        }

        generation++;
    }
};

Munch munch;

class PatternSnake {
   private:
    CRGBPalette16 pal = ForestColors_p;
    static const byte SNAKE_LENGTH = MWIDTH / 2;

    CRGB colors[SNAKE_LENGTH];
    uint8_t initialHue;

    enum Direction {
        UP,
        DOWN,
        LEFT,
        RIGHT
    };

    struct Pixel {
        uint8_t x;
        uint8_t y;
    };

    struct Snake {
        Pixel pixels[SNAKE_LENGTH];

        Direction direction;

        void newDirection() {
            switch (direction) {
                case UP:
                case DOWN:
                    direction = random(0, 2) == 1 ? RIGHT : LEFT;
                    break;

                case LEFT:
                case RIGHT:
                    direction = random(0, 2) == 1 ? DOWN : UP;

                default:
                    break;
            }
        }

        void shuffleDown() {
            for (byte i = SNAKE_LENGTH - 1; i > 0; i--) {
                pixels[i] = pixels[i - 1];
            }
        }

        void reset() {
            direction = UP;
            for (int i = 0; i < SNAKE_LENGTH; i++) {
                pixels[i].x = 0;
                pixels[i].y = 0;
            }
        }

        void move() {
            switch (direction) {
                case UP:
                    pixels[0].y = (pixels[0].y + 1) % MHEIGHT;
                    break;
                case LEFT:
                    pixels[0].x = (pixels[0].x + 1) % MWIDTH;
                    break;
                case DOWN:
                    pixels[0].y = pixels[0].y == 0 ? MHEIGHT - 1 : pixels[0].y - 1;
                    break;
                case RIGHT:
                    pixels[0].x = pixels[0].x == 0 ? MWIDTH - 1 : pixels[0].x - 1;
                    break;
            }
        }

        void draw(CRGB colors[SNAKE_LENGTH]) {
            for (byte i = 0; i < SNAKE_LENGTH; i++) {
                leds[XYsafe(pixels[i].x, pixels[i].y)] = colors[i] %= (255 - i * (255 / SNAKE_LENGTH));
            }
        }
    };

    static const int snakeCount = MWIDTH / 4;
    Snake snakes[snakeCount];

   public:
    void draw() {
        //backgroundLayer.fillScreen(CRGB(CRGB::Black));
        FastLED.clear();

        fill_palette(colors, SNAKE_LENGTH, initialHue++, 5, pal, 255, LINEARBLEND);

        for (int i = 0; i < snakeCount; i++) {
            Snake *snake = &snakes[i];

            snake->shuffleDown();

            if (random(10) > 7) {
                snake->newDirection();
            }

            snake->move();
            snake->draw(colors);
        }
    }
};

PatternSnake snake;

class SimplexNoise {
   public:
    CRGBPalette16 pal = LavaColors_p;
    void start() {
        // Initialize our coordinates to some random values
        noise_x = random16();
        noise_y = random16();
        noise_z = random16();
    }

    void draw() {
        // a new parameter set every 2 seconds
        EVERY_N_SECONDS(2) {
            noise_x = random16();
            noise_y = random16();
            noise_z = random16();
        }

        uint32_t speed = 100;

        FillNoise();
        ShowNoiseLayer(0, 1, 0);

        // noise_x += speed;
        noise_y += speed;
        noise_z += speed;
    }

    // show just one layer
    void ShowNoiseLayer(byte layer, byte colorrepeat, byte colorshift) {
        for (uint8_t i = 0; i < MWIDTH; i++) {
            for (uint8_t j = 0; j < MHEIGHT; j++) {
                uint8_t pixel = noise[i][j];

                // assign a color depending on the actual palette
                leds[XYsafe(i, j)] = ColorFromPalette(pal, colorrepeat * (pixel + colorshift), pixel);
            }
        }
    }
};

SimplexNoise simplex;

// HEAVILY DERIVED FROM https://github.com/atuline/FastLED-Demos

class Serendipitous {
   private:
    CRGBPalette16 targetPalette;
    uint8_t maxChanges = 24;  // Value for blending between palettes.

    uint16_t Xorig = 0x012;
    uint16_t Yorig = 0x015;
    uint16_t X;
    uint16_t Y;
    uint16_t Xn;
    uint16_t Yn;
    uint8_t index;

   public:
    void serendipitous() {
        EVERY_N_SECONDS(5) {
            uint8_t baseC = random8();
            targetPalette = CRGBPalette16(
                CHSV(baseC - 3, 255, random8(192, 255)),
                CHSV(baseC + 2, 255, random8(192, 255)),
                CHSV(baseC + 5, 192, random8(192, 255)),
                CHSV(random8(), 255, random8(192, 255)));

            X = Xorig;
            Y = Yorig;
        }

        //  Xn = X-(Y/2); Yn = Y+(Xn/2);
        //  Xn = X-Y/2;   Yn = Y+Xn/2;
        //  Xn = X-(Y/2); Yn = Y+(X/2.1);
        Xn = X - (Y / 3);
        Yn = Y + (X / 1.5);
        //  Xn = X-(2*Y); Yn = Y+(X/1.1);

        X = Xn;
        Y = Yn;

        index = (sin8(X) + cos8(Y)) / 2;  // Guarantees maximum value of 255

        CRGB newcolor = ColorFromPalette(PaletteCtrl::curPalette, index, 255, LINEARBLEND);

        //  nblend(leds[X%NUM_LEDS-1], newcolor, 224);          // Try and smooth it out a bit. Higher # means less smoothing.
        nblend(leds[map(X, 0, 65535, 0, NUM_LEDS)], newcolor, 224);       // Try and smooth it out a bit. Higher # means less smoothing.
        nblend(leds[map(X + 5, 0, 65535, 0, NUM_LEDS)], newcolor, 224);   // Try and smooth it out a bit. Higher # means less smoothing.
        nblend(leds[map(X + 15, 0, 65535, 0, NUM_LEDS)], newcolor, 224);  // Try and smooth it out a bit. Higher # means less smoothing.
        nblend(leds[map(X + 20, 0, 65535, 0, NUM_LEDS)], newcolor, 224);  // Try and smooth it out a bit. Higher # means less smoothing.

        fadeToBlackBy(leds, NUM_LEDS, 50);  // 8 bit, 1 = slow, 255 = fast
    }
    // serendipitous()

    void draw() {
        EVERY_N_MILLISECONDS(60) {
            nblendPaletteTowardPalette(PaletteCtrl::curPalette, targetPalette, maxChanges);  // Blend towards the target palette
        }
        EVERY_N_MILLISECONDS(50) {
            serendipitous();
        }
    }
};

Serendipitous seren;

class FadeIn {
   private:
    CRGBPalette16 currentPalette = PartyColors_p;
    CRGBPalette16 targetPalette;
    TBlendType currentBlending = LINEARBLEND;

   public:
    void draw() {
        EVERY_N_MILLISECONDS(100) {  // FastLED based non-blocking FIXED delay.
            uint8_t maxChanges = 24;
            nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);  // AWESOME palette blending capability.
        }

        EVERY_N_SECONDS(5) {               // Change the target palette to a random one every 5 seconds.
            uint8_t baseC = random8(255);  // Use the built-in random number generator as we are re-initializing the FastLED one.
            targetPalette = CRGBPalette16(CHSV(baseC + random8(0, 32), 255, random8(128, 255)), CHSV(baseC + random8(0, 32), 255, random8(128, 255)), CHSV(baseC + random8(0, 32), 192, random8(128, 255)), CHSV(baseC + random8(0, 32), 255, random8(128, 255)));
        }

        fadein();
    }

    void fadein() {
        random16_set_seed(535);  // The randomizer needs to be re-set each time through the loop in order for the 'random' numbers to be the same each time through.

        for (int i = 0; i < NUM_LEDS; i++) {
            uint8_t fader = sin8(millis() / random8(10, 20));                            // The random number for each 'i' will be the same every time.
            leds[i] = ColorFromPalette(currentPalette, i * 20, fader, currentBlending);  // Now, let's run it through the palette lookup.
        }

        random16_set_seed(millis());  // Re-randomizing the random number seed for other routines.

    }  // fadein()
};

FadeIn fadein;

class RainbowMarch {
   public:
    void draw() {
        rainbow_march(200, 10);
    }

    void rainbow_march(uint8_t thisdelay, uint8_t deltahue) {  // The fill_rainbow call doesn't support brightness levels.
        uint8_t thishue = millis() * (255 - thisdelay) / 255;  // To change the rate, add a beat or something to the result. 'thisdelay' must be a fixed value.

        // thishue = beat8(50);                                       // This uses a FastLED sawtooth generator. Again, the '50' should not change on the fly.
        // thishue = beatsin8(50,0,255);                              // This can change speeds on the fly. You can also add these to each other.

        fill_rainbow(leds, NUM_LEDS, thishue, deltahue);  // Use FastLED's fill_rainbow routine.
    }
};

RainbowMarch rainbowMarch;

// HEAVILY DERIVED FROM https://github.com/darrenpmeyer/Arduino-FireBoard/blob/master/FireBoard.ino

class FirePlace {
   private:
    const unsigned int HOT = 400;
    const unsigned int MAXHOT = (HOT * MHEIGHT);
    const unsigned int COOLING = 80;

   public:
    void draw() {
        static unsigned int spark[MWIDTH] = {0};  // base heat
        CRGB stack[MWIDTH][MHEIGHT] = {0};        // stacks that are cooler

        // 1. Generate sparks to re-heat
        for (int i = 0; i < MWIDTH; i++) {
            if (spark[i] < HOT) {
                int base = HOT * 2;
                spark[i] = random16(base, MAXHOT);
            }
        }

        // 2. Cool all the sparks
        for (int i = 0; i < MWIDTH; i++) {
            spark[i] = qsub8(spark[i], random8(0, COOLING));
        }

        // 3. Build the stack
        /*    This works on the idea that pixels are "cooler"
        as they get further from the spark at the bottom */
        for (int i = 0; i < MWIDTH; i++) {
            unsigned int heat = constrain(spark[i], HOT / 2, MAXHOT);
            for (int j = MHEIGHT - 1; j >= 0; j--) {
                /* Calculate the color on the palette from how hot this pixel is */
                byte index = constrain(heat, 0, HOT);
                stack[i][j] = ColorFromPalette(HeatColors_p, index);

                /* The next higher pixel will be "cooler", so calculate the drop */
                unsigned int drop = random8(0, HOT);
                if (drop > heat)
                    heat = 0;  // avoid wrap-arounds from going "negative"
                else
                    heat -= drop;

                heat = constrain(heat, (unsigned int)0, MAXHOT);
            }
        }

        // 4. map stacks to led array
        for (int i = 0; i < MWIDTH; i++) {
            for (int j = 0; j < MHEIGHT; j++) {
                leds[XYsafe(i, j)] = stack[i][j];
            }
        }
    }
};

FirePlace firePlace;

// SELF WRITTEN ANIMATIONS

class Fireworks {
   public:
    void draw() {
        uint8_t orig_x = random8(151);
        uint8_t orig_y = random8(26);
        uint8_t rad = random8(10, 30);
        CHSV col = CHSV(random8(), 255, 255);

        for (int y = MHEIGHT; y > orig_y; y--) {
            leds[XYsafe(orig_x, y)] = col;
            FastLED.show();
            fadeAll(190);
            delay(1);
        }
        for (int r = 0; r < rad; r++) {
            drawCircle(orig_x, orig_y, r, col);
            FastLED.show();
            fadeAll(190);
            delay(1);
        }
    }
};

Fireworks fireworks;

class RainbowFlow {
   private:
    uint8_t hue = 0;

   public:
    uint8_t y = 0;
    void draw() {
        drawFastHLine(0, y++, MWIDTH, CHSV(hue++, 255, 255));
        if (hue > 255) hue = 0;
        if (y > MHEIGHT) y = 0;
        fadeAll(220);
    }
};

RainbowFlow rainbowFlow;

class ScrollingText {
   private:
    int str1_x = 1;
    int vel = -1;
    int str1_len = TextPrinter::getStrLen("rmit");
    int str2_x = str1_x + str1_len;
    CRGB col1 = CRGB(255, 0, 0);
    CRGB col2 = CRGB::WhiteSmoke;
    CRGB bg_color = CRGB::DarkBlue;

   public:
    void draw() {
        fillBG();
        TextPrinter::print("rmit", str1_x += vel, 22, col1);
        TextPrinter::print("sst", str2_x += vel, 22, col2);

        if (str1_x == -str1_len || str1_x == MWIDTH + 1 - str1_len) vel = -vel;
    }

    void fillBG() {
        for (int y = 25; y > 0; y--) {
            drawFastHLine(0, 25 - y, MWIDTH, bg_color);
            drawFastHLine(0, 25 + y, MWIDTH, bg_color);
            fadeAll(230);
        }
        drawFastHLine(0, 25, MWIDTH, bg_color);
    }

    void setBgColor(CRGB col) {
        this->bg_color = col;
    }
};

ScrollingText text;

#endif