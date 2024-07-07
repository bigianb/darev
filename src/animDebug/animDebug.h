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
        int selected;
        TextureHeader* mainTex;
    };

    extern short rotXAxis;
    extern float zoom;

    extern float camXpos;
    extern float camYpos;
    extern float camZpos;

    extern TextureHeader modelTex;
    extern int npcSelLmpIdx;
    extern int menuLevel;
    extern int slectedLmpIdx;
    extern int selectedVifMenuIdx;
    extern int slectedAnmIdx;
    extern int selectedChangeMenuIdx;
    extern int maxAnimDebugVifIdx;
    extern unsigned int activeChangeItems;

    extern int animDebugFrameSkip;

    // first num entries in animDebugAnms
    extern int animDebugAnmsEntries;

    extern const char* allLmpNames[];

    extern AnimDebugVif animDebugVifs[];
    extern AnimDebugAnm animDebugAnms[];
    void setup(int argc,char **argv);

    void animMenuDraw();
    void animInput();
    void animFrame();

    void drawLmpList();
};


