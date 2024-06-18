#pragma once

struct VifData;
struct LmpDirEntry;

struct AnimDebugVif
{
    LmpDirEntry* lmpDirEntry;
    VifData* vifData;
    int unk;
    int pad;
};

namespace AnimDebug
{
    extern int npcSelLmpIdx;
    extern AnimDebugVif animDebugVifs[];
    void setup(int argc,char **argv);
};


