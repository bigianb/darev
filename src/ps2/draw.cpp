#include "dlist.h"
#include "draw.h"
#include "texture.h"
#include "display.h"

u64 dmaBuffers[2][20000] __attribute__((aligned(16)));
u64* curDMABufHead = nullptr;
u64* curDMABufTail = nullptr;

inline u64* paddAlign16(u64* in, int n)
{
    u64* out = in + n;

    u8* oAddr = (u8*)out;

    while (((u32)oAddr & 0x0f) != 0) *oAddr++ = 0;
    return (u64*)oAddr;
}

void drawOpaqueSprite(TextureHeader* pTex, int xpos, int ypos, int slot, bool append)
{
    drawSprite(pTex, xpos, ypos, slot, append, 0x80808080);
}

void drawSprite(TextureHeader* pTexData, int xpos, int ypos, int slot, bool append, u32 vertexColor)
{
    const GsGParam_t* dp = GsGetGParam();

    // FLUSH followed by DIRECT, num qwords filled in later
    curDMABufTail[1] = 0x5000000011000000ULL;  

    // GIF tag, EOP, PACKED mode, 1 reg A+D, nloop is written later
    curDMABufTail[2] = 0x1000000000008000ULL;   
    curDMABufTail[3] = 0xe;

    curDMABufTail[4] = 0;
    curDMABufTail[5] = GSReg::TEXFLUSH; 
    
    curDMABufTail[6] = 0;           // has to be at byte offset 0x30 (see dlist handler)
    curDMABufTail[7] = 6;           // TEX0_1 = 0

    curDMABufTail[8] = 1;           // has to be at byte offset 0x40 (see dlist handler)
    curDMABufTail[9] = 0x14;        // TEX1_1 = 1 (Fixed LOD)

    curDMABufTail[0x0a] = vertexColor;  // Q=0
    curDMABufTail[0x0b] = GSReg::RGBAQ; 

    curDMABufTail[0x0c] = 0x44;
    curDMABufTail[0x0d] = 0x42;     // ALPHA 1 = D=Cd, C=As, B=Cd, A = Cs. Cv = (Cs - Cd)*As>>7 + Cd

    curDMABufTail[0x0e] = 0x156;
    curDMABufTail[0x0f] = GSReg::PRIM;
    
    curDMABufTail[0x10] = 0x3001d;
    curDMABufTail[0x11] = 0x47;     // TEST_1
    
    int idx = 0x12;

    if (isInterlaced == 0) {
        int primX = xpos * 0x10 + 0x7000;
        int primY = ypos * 0x10 + 0x6c00;
        curDMABufTail[idx++] = 0x80008;
        curDMABufTail[idx++] = 3;        // UV

        curDMABufTail[idx++] = 0xa00000000 | (primY << 0x10) | primX;
        curDMABufTail[idx++] = 5;        // XYZ2
        
        int primH = pTexData->height * 0x10;
        int primW = pTexData->width * 0x10;

        curDMABufTail[idx++] = ((primH + 8) << 0x10) | (primW + 8);
        curDMABufTail[idx++] = 3;        // UV

        curDMABufTail[idx++] = 0xa00000000 | ((primY+primH) << 0x10) | (primX + primW);
        curDMABufTail[idx++] = 5;        // XYZ2
    } else if (dp->omode == GS_MODE_DTV_480P) {

        int primX = xpos * 0x10 + 0x5800;
        int primY = ypos * 0x10 + 0x7000;
        curDMABufTail[idx++] = 0x80008;
        curDMABufTail[idx++] = 3;        // UV

        curDMABufTail[idx++] = 0xa00000000 | (primY << 0x10) | primX;
        curDMABufTail[idx++] = 5;        // XYZ2
        
        int primH = pTexData->height * 0x10;
        int primW = pTexData->width * 0x10;

        curDMABufTail[idx++] = ((primH + 8) << 0x10) | (primW + 8);
        curDMABufTail[idx++] = 3;        // UV

        curDMABufTail[idx++] = 0xa00000000 | ((primY+primH) << 0x10) | (primX + primW);
        curDMABufTail[idx++] = 5;        // XYZ2
        
    } else {
        int primX = xpos * 0x20 + 0x5800;
        int primY = ypos * 0x10 + 0x7000;

        int v0 = 0;
        if ((primY & 0x10) == 0) {
            //Even scanline
            if (isOddField) {
                v0 = (pTexData->height / 2) * 0x10 - 4;
            }
        } else {
            if (isOddField) {
                primY = ypos * 0x10 + 0x6ff0;
                v0 = -4;
            } else {
                primY = ypos * 0x10 + 0x7010;
                v0 = (pTexData->height / 2) * 0x10;
            }
        }
        curDMABufTail[idx++] = ((v0 + 4) << 0x10) | 4;
        curDMABufTail[idx++] = 3;    // UV

        curDMABufTail[idx++] = 0xa00000000 | (primY << 16) | primX;
        curDMABufTail[idx++] = 5;

        curDMABufTail[idx++] = ((pTexData->height * 8 + v0 + 4) << 0x10) | (pTexData->width * 0x10 + 4);
        curDMABufTail[idx++] = 3;
        
        int primX1 = primX + pTexData->width * 0x20;
        int primY1 = primY + pTexData->height * 0x10;
        curDMABufTail[idx++] = 0xa00000000 | (primY1 << 16) | primX1;
        curDMABufTail[idx++] = 5;
    }

    const int entriesUsed = idx;

    // Fill in the DMA tag

    // -2 to exclude the DMA Tag itself
    const u64 numQwords = (entriesUsed - 2) / 2;
    curDMABufTail[0] = numQwords | 0x70000000ULL;     // end, no interrupt
    
    // write the GIF tag nloop (-4 excludes DMA tag and GIF tag)
    u64 nloop = (entriesUsed - 4) / 2;
    curDMABufTail[2] |= nloop;

    // fill in the DIRECT command
    curDMABufTail[1] |= ((nloop+1) << 0x20);

    u64 *dma_buffer = curDMABufTail;
    curDMABufTail = paddAlign16(curDMABufTail, entriesUsed);
    queueDMA(dma_buffer, slot, pTexData, nullptr, !append);
    return;
}
