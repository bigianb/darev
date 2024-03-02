#include "font.h"

const int GLYPH_MASK = 0xf800;
const u16 NEWLINE_GLYPH_ID = 0x07ff;

void charsToGlyphs(Font* font, u16* text)
{
    if ((*text & GLYPH_MASK) == GLYPH_MASK) {
        // already glyphs
        return;
    }

    u16* p = text;
    while (*p) {
        u16 glyphId = NEWLINE_GLYPH_ID;
        const u16 charId = *p;
        if (charId != 10) {
            glyphId = 0;
            int idx = 0;
            while (idx < font->numGlyphs) {
                if (font->glyphInfoArray[idx].charId == charId) {
                    glyphId = idx;
                    break;
                }
                ++idx;
            }
        }
        *p++ = glyphId | GLYPH_MASK;
    }
}

int measureTextW(Font* font, u16* text, int nChars, bool interlaced)
{
    if (nChars == 0){
        return 0;
    }

    charsToGlyphs(font, text);

    int width = 0;

    int remainingChars = nChars;
    const u16* p = text;
    u16 prevGlyph = 0xFFFF;
    while (remainingChars > 0 && *p != 0)
    {
        const u16 cleanGlyphId = (*p & ~GLYPH_MASK);
        if (cleanGlyphId != NEWLINE_GLYPH_ID){
            const int glyphWidth = font->glyphInfoArray[cleanGlyphId].width;
            if (cleanGlyphId != 0){
                int kernId = font->glyphInfoArray[cleanGlyphId].kernId;
                
                while (font->kernArray[kernId].glyph2 == cleanGlyphId){

                    // we're the second part of the kern pair
                    if (font->kernArray[kernId].glyph1 == prevGlyph){
                        // ... and the previous glyph is the first pair so apply the kern adjustment
                        const int kern = font->kernArray[kernId].kern;
                        if (interlaced) {
                            width += kern * 16;
                        } else {
                            width += kern * 8;
                        }
                        break;
                    }
                    ++kernId;
                }
            }
            width += glyphWidth * 16;
        }
        prevGlyph = cleanGlyphId;
        --remainingChars;
        ++p;
    }

    return width / 16;
}
