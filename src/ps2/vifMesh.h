#pragma once

#include "tamtypes.h"

struct VifData
{
    int field0;
    int field1;
    u64 meshMask;
    u16 flags;
    char numChanges;
    char pad[3];
    u16 pad2;
    u32 alphaRegLo;
    u32 alphaRegHi;
    char alpha2FixVal;
    u8 changeDefsOffset;

};

int getNumTrisOfSelectedVifs(VifData *vifData, u64 meshMask);
u64 getMeshMask(VifData *vifData, u32 activeChangeItems);
