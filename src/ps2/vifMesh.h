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
    u8  field_0x18;
    char pad3[3];
    u8 field_1c;
    char pad4[3];
    char field_0x20;
    u8 changeDefsOffset;

};

int getNumTrisOfSelectedVifs(VifData *vifData, u64 meshMask);
u64 getMeshMask(VifData *vifData, u32 activeChangeItems);
