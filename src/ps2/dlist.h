#pragma once

#include "tamtypes.h"

struct TextureHeader;

// display list handling

struct DlistNode
{
    void* pCleanDmaData;
    int flags;
    TextureHeader* texData;
    DlistNode* next;
    u8 unkb;
    u8 unkb2;
    short pad;
};

DlistNode*
queueDMA(u64* dma_buffer, int slot, TextureHeader* pTexData, DlistNode* headNode, bool prepend);

void initDMA();
void kickoffDMA();
void waitDMASema();

int sceGsSyncPath(int mode, unsigned short timeout);

