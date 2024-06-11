#pragma once
#include <tamtypes.h>

struct DlistNode;
struct GSAllocInfo;

struct TextureHeader
{
    short width;
    short height;
    short requiredGSMem;
    unsigned short qwc;
    unsigned short flags;
    short unk2;
    int tbw;
    void * gif_madr_val;
    void * gif_tadr_val;
    GSAllocInfo * gsAllocInfo;
    int resetFrameNo;
    DlistNode * slotNodes[8];
};

void deinterlace(TextureHeader* texData);
