#include "GIFTag.h"
#include <tamtypes.h>

void GIFTag::parse(const unsigned char* data)
{
    const u32* p32 = (u32*)data;

    int low32 = p32[0];
    nloop = low32 & 0x7FFF;
    eop = (low32 & 0x8000) == 0x8000;

    int next32 = p32[1];

    // bit 32 is bit 0 of next 32
    pre = ((next32 >> (46 - 32)) & 1) == 1;
    // prim 11 bits 47 - 57
    prim = ((next32 >> (47 - 32)) & 0x3FF);
    flg = ((next32 >> (58 - 32)) & 0x3);
    nreg = ((next32 >> (60 - 32)) & 0xf);

    if (0 == nreg){
        nreg = 16;
    }
    int regs64 = p32[2];
    int regs96 = p32[3];

    for (int reg=0; reg < nreg; ++reg){
        int rgs = reg > 7 ? regs96 : regs64;
        regs[reg] = (rgs >> ((reg & 7) * 4)) & 0x0f;
    }
}
