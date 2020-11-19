#ifndef PALETTE_CONTROL_H
#define PALETTE_CONTROL_H

#include <FastLED.h>

const CRGBPalette16 GrayscaleColors_p = CRGBPalette16(CRGB::Black, CRGB::White);
const CRGBPalette16 IceColors_p = CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);
const byte paletteSize = 9;
const CRGBPalette16 palettes[paletteSize] = {
    RainbowColors_p,
    OceanColors_p,
    CloudColors_p,
    ForestColors_p,
    PartyColors_p,
    GrayscaleColors_p,
    HeatColors_p,
    LavaColors_p,
    IceColors_p,
};

class PaletteCtrl {
   public:
    static CRGBPalette16 curPalette;
    static uint8_t palIndex;

    static CRGBPalette16 getPalette(uint8_t idx) {
        return palettes[constrain(idx, 0, 8)];
    }

    static void cyclePalette() {
        if (palIndex >= paletteSize) palIndex = 0;
        curPalette = getPalette(palIndex++);
    }
};

CRGBPalette16 PaletteCtrl::curPalette;
uint8_t PaletteCtrl::palIndex;

#endif