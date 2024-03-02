#pragma once
#include <tamtypes.h>

struct GsDispEnv
{
    u64 pmode;
    u64 smode2;
    u64 dispfb;
    u64 display;
    u64 bgcolor;
};

enum GSReg
{
    PRIM = 0,
    RGBAQ = 0x01,
    XYZ2 = 5,
    TEX0_1 = 6,
    TEX1_1 = 0x14,
    XYOFFSET_1 = 0x18,
    XYOFFSET_2 = 0x19,
    PRMODECONT = 0x1A,
    FOGCOL = 0x3d,
    TEXFLUSH = 0x3f,
    SCISSOR_1 = 0x40,
    ALPHA_1 = 0x42,
    DTHE = 0x45,
    COLCLAMP = 0x46,
    TEST_1 = 0x47,
    PABE = 0x49,
    FRAME_1 = 0x4c,
    ZBUF_1 = 0x4e,
    TRXDIR = 0x53
};

extern volatile int frameCount;
extern volatile int vblankSetsMeToFF;
extern volatile bool isOddField;
extern volatile bool isEvenField;
extern GsDispEnv displayEnvironment;
extern s32 vblankSema;
extern int isInterlaced;

void initGs();
void setDisplayRegs(GsDispEnv* regs);

void initDisplay();
void startFrame();
void endFrame();
