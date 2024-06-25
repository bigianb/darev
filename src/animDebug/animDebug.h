#pragma once

struct VifData;
struct LmpDirEntry;
struct TextureHeader;

namespace AnimDebug
{
    struct AnimDebugAnm
    {
        LmpDirEntry* lmpDirEntry;
        unsigned char* anmData;
    };

    struct AnimDebugVif
    {
        LmpDirEntry* lmpDirEntry;
        VifData* vifData;
        int unk;
        TextureHeader* mainTex;
    };
    extern TextureHeader modelTex;
    extern int npcSelLmpIdx;
    extern int menuLevel;
    extern int slectedLmpIdx;
    extern int selectedVifMenuIdx;

    // first num entries in animDebugAnms
    extern int animDebugAnmsEntries;

    extern AnimDebugVif animDebugVifs[];
    extern AnimDebugAnm animDebugAnms[];
    void setup(int argc,char **argv);

    void animMenuDraw();
};


