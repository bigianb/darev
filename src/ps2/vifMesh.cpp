#include "vifMesh.h"

int getNumTrisOfSelectedVifs(VifData* vifData, u64 meshMask)
{
    int totalNumTris = 0;
    int iVar1 = vifData->numChanges - 1;
    short* puVar2 = (short*)((u8*)vifData + 0x2c + vifData->numChanges * 4);
    if ((-1 < iVar1) && (meshMask != 0)) {
        do {
            iVar1 -= 1;
            if (((meshMask ^ 1) & 1) == 0) {
                totalNumTris += puVar2[4];
            }
            meshMask = meshMask >> 1;
            puVar2 += 5;
        } while ((-1 < iVar1) && (meshMask != 0));
    }
    return totalNumTris;
}

u64 getMeshMask(VifData* vifData, u32 activeChangeItems)
{
    u64 meshMask = ~vifData->meshMask;
    if (((vifData->flags & 2) != 0) && (activeChangeItems != 0)) {
        u64* p64 = (u64*)((u8*)vifData + vifData->changeDefsOffset * 0x10);
        do {
            if ((activeChangeItems & 1) != 0) {
                meshMask |= (p64[0] & ~p64[1]);
            }
            activeChangeItems >>= 1;
            p64 += 2;
        } while (activeChangeItems != 0);
    }
    return meshMask;
}
