#include "patterns.h"

void setup() {
    // Serial.begin(57600);
    // delay(3000);
    FastLED.addLeds<2, WS2812B, 1, GRB>(leds, 1617).setCorrection(TypicalLEDStrip);

    FastLED.clear(true);
    FastLED.show();

    spiral.start();
    wave.start();
    mandala.start();
    simplex.start();

    PaletteCtrl::palIndex = 0;
    PaletteCtrl::cyclePalette();

    NoiseVariablesSetup();
}

byte pattern_it = 1;
byte pattern_sz = 9;

void loop() {
    EVERY_N_SECONDS(15) {
        pattern_it++;

        if (pattern_it > pattern_sz) pattern_it = 1;

        if (pattern_it == 1) {
            FastLED.clear();
            FastLED.show();
        } else if (pattern_it == 8)
            rainbowFlow.y = 0;
    }

    switch (pattern_it) {
        case 1:
            snake.draw();
            break;
        case 2:
            text.draw();
            break;
        case 3:
            fireworks.draw();
            break;
        case 4:
            spiral.draw();
            break;
        case 5:
            munch.draw();
            break;
        case 6:
            plasma.draw();
            break;
        case 7:
            pulse.draw();
            break;
        case 8:
            rainbowFlow.draw();
            break;
        case 9:
            simplex.draw();
            break;
    }

    FastLED.setBrightness(brightness);
    FastLED.show();
    delay(1);
}
