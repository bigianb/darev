#pragma once

#include <cstdint>
#include "texture.h"

class GlyphInfo
{
public:
    int16_t charId;
    int16_t x0;
    int16_t x1;
    int16_t y0;
    int16_t y1;
    int16_t yOffset;
    int16_t width;
    int16_t kernId;
};

class GlyphKernPair
{
public:
    uint16_t glyph1;
    uint16_t glyph2;
    int16_t kern;
};

class Font
{
public:
    int16_t numGlyphs;
    int16_t pad1;
    int32_t pad2;
    GlyphInfo* glyphInfoArray;
    GlyphKernPair* kernArray;
    TextureHeader* texture;
};

void charsToGlyphs(Font* font, u16* text);
int measureTextW(Font *font, u16 *text, int nChars, bool interlaced);
