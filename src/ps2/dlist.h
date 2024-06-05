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

void initTextureAllocStuff();
void resetTextureAlloc();
void initDMA();
void kickoffDMA();
void waitDMASema();

int sceGsSyncPath(int mode, unsigned short timeout);

struct GSAllocInfo
{
    int frameCountPlusOne;
    short dbp;
    short size;
    unsigned char commitCount;
    char pad[3];

    // Points to the element in the texture where this info is used.
    // Allows us to deallocate this.
    GSAllocInfo** allocTex;
    GSAllocInfo* next;
    GSAllocInfo* prev;
};

