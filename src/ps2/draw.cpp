#include "dlist.h"
#include "draw.h"
#include "texture.h"
#include "display.h"

u64 dmaBuffers[2][20000] __attribute__((aligned(16)));
u64* curDMABufHead = nullptr;
u64* curDMABufTail = nullptr;

inline u64* addAlign16(u64* in, int n)
{
    u64* out = in + n;
    if (((u32)out & 0x0f) != 0) ++out;
    return out;
}

void drawOpaqueSprite(TextureHeader* pTex, int xpos, int ypos, int slot, bool append)
{
    drawSprite(pTex, xpos, ypos, slot, append, 0x80808080);
    return;
}

// @pal: 00140eb0
void drawSprite(TextureHeader* pTexData, int xpos, int ypos, int slot, bool append, u32 vertexColor)
{
    // FLUSH followed by DIRECT, 12 qwords
    curDMABufTail[1] = 0x5000000c11000000ULL;  

    // GIF tag, EOP, PACKED mode, 1 reg A+D, nloop is written later
    curDMABufTail[2] = 0x1000000000008000ULL;   
    curDMABufTail[3] = 0xe;

    curDMABufTail[4] = 0;
    curDMABufTail[5] = 0x3f;        // TEXFLUSH
    
    curDMABufTail[6] = 0;
    curDMABufTail[7] = 6;           // TEX0_1 = 0

    curDMABufTail[8] = 1;
    curDMABufTail[9] = 0x14;        // TEX1_1 = 1 (Fixed LOD)

    curDMABufTail[0x0a] = vertexColor;  // Q=0
    curDMABufTail[0x0b] = 1;         // RGBAQ

    curDMABufTail[0x0c] = 0x44;
    curDMABufTail[0x0d] = 0x42;     // ALPHA 1 = D=Cd, C=As, B=Cd, A = Cs. Cv = (Cs - Cd)*As>>7 + Cd

    curDMABufTail[0x0e] = 0x156;
    curDMABufTail[0x0f] = 0;        // PRIM
    
    curDMABufTail[0x10] = 0x3001d;
    curDMABufTail[0x11] = 0x47;     // TEST_1
    
    if (isInterlaced == 0) {
        int primX = ypos * 0x10 + 0x7000;
        int primY = xpos * 0x10 + 0x6c00;
        curDMABufTail[0x12] = 0x80008;
        curDMABufTail[0x13] = 3;        // UV

        curDMABufTail[0x14] = 0xa00000000 | (primY << 0x10) | primX;
        curDMABufTail[0x15] = 5;        // XYZ2
        
        int primH = pTexData->height * 0x10;
        int primW = pTexData->width * 0x10;

        curDMABufTail[0x16] = ((primH + 8) << 0x10) | (primW + 8);
        curDMABufTail[0x17] = 3;        // UV

        curDMABufTail[0x18] = 0xa00000000 | ((primY+primH) << 0x10) | (primX + primW);
        curDMABufTail[0x19] = 5;        // XYZ2
    } else {
        int primX = xpos * 0x20 + 0x5800;
        int primY = ypos * 0x10 + 0x7000;

        int v0 = 0;
        if ((primY & 0x10) == 0) {
            //Even scanline
            if (isEvenField) {
                v0 = (pTexData->height / 2) * 0x10 - 4;
            }
        } else {
            if (isEvenField) {
                primY = ypos * 0x10 + 0x6ff0;
                v0 = -4;
            } else {
                primY = ypos * 0x10 + 0x7010;
                v0 = (pTexData->height / 2) * 0x10;
            }
        }
        curDMABufTail[0x12] = ((v0 + 4) << 0x10) | 4;
        curDMABufTail[0x13] = 3;    // UV

        curDMABufTail[0x14] = 0xa00000000 | (primY << 16) | primX;
        curDMABufTail[0x15] = 5;

        curDMABufTail[0x16] = ((pTexData->height * 8 + v0 + 4) << 0x10) | (pTexData->width * 0x10 + 4);
        curDMABufTail[0x17] = 3;
        
        int primX1 = primX + pTexData->width * 0x20;
        int primY1 = primY + pTexData->height * 0x10;
        curDMABufTail[0x18] = 0xa00000000 | (primY1 << 16) | primX1;
        curDMABufTail[0x19] = 5;
    }

    const int entriesUsed = 0x1A;

    // Fill in the DMA tag

    // -2 to exclude the DMA Tag itself
    const u64 numQwords = (entriesUsed - 2) / 2;
    curDMABufTail[0] = numQwords | 0x70000000ULL;     // end, no interrupt
    
    // write the GIF tag nloop (-4 excludes DMA tag and GIF tag)
    u64 nloop = (entriesUsed - 4) / 2;
    curDMABufTail[2] |= nloop;

    u64 *dma_buffer = curDMABufTail;
    curDMABufTail = addAlign16(curDMABufTail, entriesUsed);
    queueDMA(dma_buffer, slot, pTexData, nullptr, !append);
    return;
}
