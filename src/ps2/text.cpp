#include "text.h"
#include "dlist.h"
#include "draw.h"
#include "font.h"
#include "trace.h"

float textBrightness[2] = {1.0f, 1.0f};

u64* pTextDma;
u16* pCharNloop;

int numQwordsInTextDma;
int numCharsInDmaQueue;

void atowcs(u16* dest, const char* src)
{
    while (*src) {
        unsigned char uc = *src++;
        *dest++ = uc;
    }
    *dest = 0;
}

void setTextColor(u32 color)
{
    // GIF tag, PACKED mode, 1 reg A+D, nloop 1
    pTextDma[0] = 0x1000000000000001ULL;
    pTextDma[1] = 0x0e;

    pTextDma[2] = color;
    pTextDma[3] = GSReg::RGBAQ;

    // GIF tag, eop, 2 regs, reglist mode, nloop written later
    pTextDma[4] = 0x2400000000008000ULL;
    pTextDma[5] = 0x53; // UV, XYZ2

    //DAT_ram_004558d8 = pTextDma;

    int numQwordsToAdd = numCharsInDmaQueue * 2;
    numQwordsInTextDma += numQwordsToAdd;
    *pCharNloop += numQwordsToAdd;

    numCharsInDmaQueue = 0;

    numQwordsInTextDma += 3;

    pCharNloop = (u16*)&pTextDma[4];
    pTextDma += 6;
}

u64* pTextDmaStart;
u16* pTextDIRECTStart;
bool textIsScissored;
Font* pTextFont;
int textDmaSlot;
int textAppendFlag;

void beginText(Font* font, int slot, int append, u32 color)
{
    pTextDma = curDMABufTail;
    pTextDmaStart = pTextDma;
    numCharsInDmaQueue = 0;

    // used to write num of DIRECT Qwords
    pTextDIRECTStart = (u16*)pTextDmaStart + 6;

    pTextFont = font;
    textDmaSlot = slot;
    textAppendFlag = append;

    // FLUSH followed by DIRECT, num qwords written later
    pTextDma[1] = 0x5000000011000000ULL;

    // GIF tag, PACKED mode, 1 reg A+D, nloop = 7
    pTextDma[2] = 0x1000000000000007ULL;
    pTextDma[3] = 0xe;

    pTextDma[4] = 0;
    pTextDma[5] = GSReg::TEXFLUSH;

    pTextDma[6] = 0;
    pTextDma[7] = GSReg::TEX0_1;

    pTextDma[8] = 1;
    pTextDma[9] = GSReg::TEX1_1;

    pTextDma[10] = color;
    pTextDma[11] = GSReg::RGBAQ;

    pTextDma[12] = 0x44;
    pTextDma[13] = GSReg::ALPHA_1;

    pTextDma[14] = 0x156;
    pTextDma[15] = GSReg::PRIM;

    pTextDma[16] = 0x3001d;
    pTextDma[17] = GSReg::TEST_1;

    // GIF tag, eop, 2 regs, reglist mode, nloop written later
    pTextDma[18] = 0x2400000000008000ULL;
    pTextDma[19] = 0x53; // UV, XYZ2

    pCharNloop = (u16*)&pTextDma[18];
    pTextDma += 20;

    numQwordsInTextDma = 9; // Excludes the 128 bit DMATag.

    textIsScissored = false;
}

void flushText()
{
    if (textIsScissored != 0) {
        // TODO: reset the scissor area
        //scissorText(0, 0, 639, 511);
    }

    u16 numCharQWords = (u16)(numCharsInDmaQueue * 2);
    *pCharNloop += numCharQWords;

    *pTextDIRECTStart = (u16)numQwordsInTextDma + numCharQWords;

    int totalQWordsInDma = (pTextDma - pTextDmaStart) / 2;

    *pTextDmaStart = (totalQWordsInDma - 1) | 0x70000000ULL;  // end, no interrupt

    curDMABufTail = pTextDma;
    queueDMA(pTextDmaStart, textDmaSlot, pTextFont->texture, nullptr, textAppendFlag == 0);
}

#define oddFieldAdj 4

void displayTextW(int x, int y, u16* text, int maxChars)
{
    const GsGParam_t* dp = GsGetGParam();

    charsToGlyphs(pTextFont, text);

    int xPhys;
    int yPhys;
    int wShift;     // Number of times to shift the width value left.
    int yShift;
    int yAdjust;        
    if ((y + 100 < 0x385) && (maxChars > 0)) {
        if (isInterlaced == 0) {
            xPhys = x * 0x10 + 0x6c00;
            yPhys = y * 0x10 + 0x7000;
            yAdjust = 0;
            wShift = 4;
            yShift = 4;
        } else if (dp->omode == GS_MODE_DTV_480P || dp->omode == GS_MODE_DTV_576P) {
            xPhys = x * 0x10 + 0x5800;
            yPhys = y * 0x10 + 0x7000;
            yAdjust = 0;
            wShift = 4;
            yShift = 4;
        } else {
            int yDev = y * 0x10;
            yPhys = yDev + 0x7000;
            xPhys = x * 0x20 + 0x5800;
            if ((yPhys & 0x10) == 0) {
                // Even row
                yAdjust = 0;
                if (isOddField) {
                    yAdjust = (pTextFont->texture->height /2) * 0x10 + -oddFieldAdj;
                }
            } else {
                yPhys = yDev + 0x6ff0;
                if (isOddField) {
                    yAdjust = -oddFieldAdj;
                } else {
                    yAdjust = (pTextFont->texture->height /2) * 0x10;
                    yPhys = yDev + 0x7010;
                }
            }
            wShift = 5;
            yShift = 3;
        }

        
        u16 prevGlyphId = 0xffff;
        while (maxChars > 0 && *text != 0) {
            u16 glyphId = *text;
            u16 cleanGlyphId = glyphId & 0x7ff;
            if (cleanGlyphId != 0x7ff) {
                GlyphInfo& gi = pTextFont->glyphInfoArray[cleanGlyphId];
                if (cleanGlyphId != 0) {
                    int physX0 = gi.x0 * 0x10;
                    
                    int physX1 = gi.x1 * 0x10;
                    int kernId = gi.kernId;

                    int iVar17 = gi.y0 << yShift;
                    int iVar16 = gi.y1 << yShift;

                    int physGlyphYOffset = gi.yOffset << yShift;
                    int physGlyphHeight = iVar16 - iVar17;

                    // TODO: should be a font function
                    
                    if ((kernId >= 0) && (pTextFont->kernArray[kernId].glyph2 == cleanGlyphId)) {  
                        do {
                            GlyphKernPair& pair = pTextFont->kernArray[kernId];
                            if (pair.glyph1 == prevGlyphId) {
                                const int kern = pair.kern;
                                if (isInterlaced == 0) {
                                    xPhys += kern * 8;
                                } else {
                                    xPhys += kern * 0x10;
                                }
                                break;
                            }
                            ++kernId;
                        } while (pTextFont->kernArray[kernId].glyph2 == cleanGlyphId);
                    }

                    if (isInterlaced == 0 || dp->omode == GS_MODE_DTV_480P || dp->omode == GS_MODE_DTV_576P) {
                        pTextDma[0] = (iVar17 + 8) * 0x10000 | (physX0 + 8);
                        pTextDma[1] = xPhys | ((yPhys + physGlyphYOffset) * 0x10000);
                        pTextDma[2] = (iVar16 + 8) * 0x10000 | (physX1 + 8);
                        pTextDma[3] = (xPhys + physX1 - physX0) | (yPhys + physGlyphYOffset + physGlyphHeight) * 0x10000;
                    } else {
                        pTextDma[0] = (yAdjust + iVar17 + 4) * 0x10000 | (physX0 + 4);
                        pTextDma[1] = xPhys | ((yPhys + physGlyphYOffset * 2) * 0x10000);
                        pTextDma[2] = (yAdjust + iVar16 + 4) * 0x10000 | (physX1 + 4);
                        pTextDma[3] = (xPhys + (physX1 - physX0) * 2) |
                                        (yPhys + (physGlyphYOffset + physGlyphHeight) * 2) * 0x10000;
                    }
                    pTextDma += 4;
                    numCharsInDmaQueue += 1;
                }
                xPhys += (gi.width << wShift);
            }
            --maxChars;
            ++text;
            prevGlyphId = cleanGlyphId;
        }
    }
    return;
}

int logCount=5;
void displayTextCenteredW(int xCenter, int ypos, u16* text, int maxChars)
{
    int w = measureTextW(pTextFont, text, maxChars, isInterlaced != 0);
    displayTextW(xCenter - w/2, ypos, text, maxChars);
}

u32 scaleColor(float scale, u32 color)
{
    int scaledR = (int)((float)(color >> 8 & 0xff) * scale);
    int scaledG = (int)((float)(color >> 16 & 0xff) * scale);
    int scaledB = (int)((float)(color & 0xff) * scale);

    // Clamp between 0 and 0xFF
    scaledR = scaledR < 0 ? 0 : (scaledR & 0xFF);
    scaledG = scaledG < 0 ? 0 : (scaledG & 0xFF);
    scaledB = scaledB < 0 ? 0 : (scaledB & 0xFF);

    return scaledB | (color & 0xff000000) | scaledG << 16 | scaledR << 8;
}
