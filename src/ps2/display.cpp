
#include <kernel.h>
#include <ee_regs.h>
#include <libgs.h>

#include "trace.h"
#include "display.h"
#include "state.h"
#include "dlist.h"

#define GS_MODE_DTV_576P  0x53

enum DmaChannel
{
    VIF0 = 0,
    VIF1,
    GIF,
    fromIPU,
    toIPU,
    fromSIF0,
    toSIF1,
    SIF2,
    fromSPR,
    toSPR
};

// Channel Control
static u32 dma_chcr[10] = {0x10008000, 0x10009000, 0x1000A000, 0x1000B000, 0x1000B400, 0x1000C000, 0x1000C400, 0x1000C800, 0x1000D000, 0x1000D400};
// Quadword Count
static u32 dma_qwc[10] = {0x10008020, 0x10009020, 0x1000A020, 0x1000B020, 0x1000B420, 0x1000C020, 0x1000C420, 0x1000C820, 0x1000D020, 0x1000D420};
// Memory Address
static u32 dma_madr[10] = {0x10008010, 0x10009010, 0x1000A010, 0x1000B010, 0x1000B410, 0x1000C010, 0x1000C410, 0x1000C810, 0x1000D010, 0x1000D410};
// Tag Address
static u32 dma_tadr[10] = {0x10008030, 0x10009030, 0x1000A030, 0x1000B030, 0x1000B430, 0x1000C030, 0x1000C430, 0x1000C830, 0x1000D030, 0x1000D430};

void dmaSend(DmaChannel chan, u32* pData)
{

    if ((*(vu32*)dma_chcr[chan] & 0x100) != 0) {
        int timeout = 0xffffff;
        do {
            if (timeout < 0) {
                traceln("libdma: sync timeout");
                if ((*(vu32*)dma_chcr[chan] >> 8 & 1) != 0) {
                    *(vu32*)dma_chcr[chan] = *(vu32*)dma_chcr[chan] & 0xfffffeff;
                }
            }
            --timeout;
        } while ((*(vu32*)dma_chcr[chan] & 0x100) != 0);
    }
    if (*(vu32*)dma_tadr[chan] != 0xffffffff) {
        *(vu32*)dma_tadr[chan] = (u32)pData;
    }
    *(vu32*)dma_qwc[chan] = 0;
    *(vu32*)dma_chcr[chan] = (*(vu32*)dma_chcr[chan] & 0xfffffff3) | 0x105;
    return;
}

int vblankHandlerId;

volatile int vblankSetsMeToFF;
volatile bool isOddField;
volatile bool isEvenField;

int isInterlaced;
s32 vblankSema;

#define GS_CSR_NFIELD 0x00001000
#define GS_CSR_FIELD  0x00002000

int vblank_handler(int cause)
{
    vblankSetsMeToFF = 0xff;
    u32 csr = *R_EE_GS_CSR;
    isOddField = (csr & GS_CSR_FIELD) == GS_CSR_FIELD;
    isEvenField = !isOddField;
    
    ++frameCount;
    iSignalSema(vblankSema);

    ExitHandler();
    return 0;
}

void waitFramecountChange()
{
    int fc = frameCount;
    do {
        asm volatile("nop\nnop\nnop\nnop\nnop");
    } while (fc == frameCount);
}

GsGParam_t gp_15 = {GS_INTERLACED, GS_MODE_NTSC, GS_FFMD_FRAME, 3};

// 002056b0
void GsResetGraph(short mode, short interlace, short omode, short ffmode)
{
    switch (mode) {
    case GS_INIT_RESET:
    {
        GsGParam_t* dp = GsGetGParam();
        GS_SET_CSR_reset(1);
        dp->ffmode = ffmode;
        dp->interlace = interlace;
        dp->omode = omode;
        dp->version = GS_GET_CSR_gs_rev_number >> 16;
        GsPutIMR(0xFF00);

        SetGsCrt(interlace & 1, omode & 0xFF, ffmode & 1);
    } break;
    case GS_INIT_DRAW_RESET:
        GS_SET_CSR_flush(1);
        break;
    }
    return;
}

#define GS_SET_DISPLAY(display_x, display_y,magnify_h,magnify_v,display_w,display_h) \
		(u64)((display_x) & 0x00000FFF) <<  0 |	\
		(u64)((display_y) & 0x000007FF) << 12 |	\
		(u64)((magnify_h) & 0x0000000F) << 23 |	\
		(u64)((magnify_v) & 0x00000003) << 27 |	\
		(u64)((display_w) & 0x00000FFF) << 32 |	\
		(u64)((display_h) & 0x000007FF) << 44

#define GS_SET_TEX0(TBA, TBW, PSM, TW, TH, TCC, TFNCT, CBA, CPSM, CSM, CSA, CLD) \
    (u64)((TBA)&0x00003FFF) << 0 | (u64)((TBW)&0x0000003F) << 14 |               \
        (u64)((PSM)&0x0000003F) << 20 | (u64)((TW)&0x0000000F) << 26 |           \
        (u64)((TH)&0x0000000F) << 30 | (u64)((TCC)&0x00000001) << 34 |           \
        (u64)((TFNCT)&0x00000003) << 35 | (u64)((CBA)&0x00003FFF) << 37 |        \
        (u64)((CPSM)&0x0000000F) << 51 | (u64)((CSM)&0x00000001) << 55 |         \
        (u64)((CSA)&0x0000001F) << 56 | (u64)((CLD)&0x00000007) << 61

#define GS_SET_PMODE(EN1,EN2,MMOD,AMOD,SLBG,ALP) \
        ((u64)(EN1)     << 0)   | \
        ((u64)(EN2)     << 1)   | \
        ((u64)(001)     << 2)   | \
        ((u64)(MMOD)    << 5)   | \
        ((u64)(AMOD)	<< 6)	| \
        ((u64)(SLBG)	<< 7)	| \
        ((u64)(ALP)     << 8)

#define WRITE_PMODE(EN1,EN2,MMOD,AMOD,SLBG,ALP) \
        *(vu64*)gs_p_pmode = ((u64)(EN1) << 0)   | \
        ((u64)(EN2)     << 1)   | \
        ((u64)(001)     << 2)   | \
        ((u64)(MMOD)    << 5)   | \
        ((u64)(AMOD)	<< 6)	| \
        ((u64)(SLBG)	<< 7)	| \
        ((u64)(ALP)     << 8)

GsDispEnv displayEnvironment;

void SetDefDispEnv(void)
{
    GsGParam_t* dp = GsGetGParam();

    traceln("dp omode = 0x%x", dp->omode);

    //displayEnvironment.pmode = 0x66;    // Enable Read circuit 2, Disble circuit 1, alpha reg, alpha Read Circuit 2, fixed alpha of 0
    displayEnvironment.pmode = GS_SET_PMODE(0, 1, 1, 1, 0, 0);
    displayEnvironment.dispfb = 0x1400; // FBP: 0, FBW: 10 (640 pixels)
    displayEnvironment.bgcolor = 0;

    if (dp->interlace) {
        if (dp->ffmode == GS_FFMD_FRAME) {
            displayEnvironment.smode2 = 3;
        } else {
            displayEnvironment.smode2 = 1;
        }
    } else {
        displayEnvironment.smode2 = 2;
    }

    if (dp->omode == GS_MODE_NTSC) {
        u32 dx = 0x29c;
        if (dp->interlace != 1) {
            u32 dy = 0x19;
            // dw = 2559, dh = 255, magh = 4, magv = 1
            displayEnvironment.display = dx | dy << 0xc | 0xff9ff01800000;
        } else {
            u32 dy = 0x32;
            u64 dh = dp->ffmode ? 511 : 255;
            // dw = 2559, magh = 4, magv = 1
            displayEnvironment.display = dx | dy << 0xc | dh << 0x2c | 0x9ff01800000;
        }
    } else if (dp->omode == GS_MODE_PAL) {
        // 3, GS_MODE_PAL
        u32 dx = 0x2b0;
        if (dp->interlace != GS_INTERLACED) {
            u32 dy = 0x24;
            displayEnvironment.display = dx | dy << 0xc | 0xff9ff01800000;
        } else {
            u32 dy = 0x48;
            u64 dh = dp->ffmode ? 511 : 255;
            displayEnvironment.display = dx | dy << 0xc | dh << 0x2c | 0x9ff01800000;
        }
    } else if (dp->omode == GS_MODE_DTV_480P){
        int StartX = 232;
        int StartY = 35;
        int DW = 1440;
        int DH = 480;

        int width = 640;
        int height = 480;               // Display buffer width and height
        int StartXOffset = 0;
        int StartYOffset = 0;

        int MagH = (DW / width) - 1;  // multiple of the screen width
        int MagV = (DH / height) - 1; // multiple of the screen height

        // Keep the framebuffer in the center of the screen
        StartX += (DW - ((MagH + 1) * width)) / 2;
        StartY += (DH - ((MagV + 1) * height)) / 2;

        // Calculate the actual display width and height
        DW = (MagH + 1) * width;
        DH = (MagV + 1) * height;

        traceln("MagH = 0x%x, DW = 0x%x", MagH, DW);
        traceln("MagV = 0x%x, DH = 0x%x", MagV, DH);
        traceln("StartX = 0x%x, StartY = 0x%x", StartX, StartY);

        displayEnvironment.display = GS_SET_DISPLAY(
                    StartX + StartXOffset, // X position in the display area (in VCK units)
                    StartY + StartYOffset, // Y position in the display area (in Raster units)
                    MagH,                  // Horizontal Magnification
                    MagV,                  // Vertical Magnification
                    DW - 1,                // Display area width
                    DH - 1);               // Display area height
    } else if (dp->omode == GS_MODE_DTV_576P){
        int StartX = 320;
        int StartY = 64;
        int DW = 1312;
        int DH = 576;

        int width = 640;
        int height = 512;
        
        int StartXOffset = 0;
        int StartYOffset = 0;

        int MagH = (DW / width) - 1;  // multiple of the screen width
        int MagV = (DH / height) - 1; // multiple of the screen height

        // Calculate the actual display width and height
        DW = (MagH + 1) * width;
        DH = (MagV + 1) * height;

        // Keep the framebuffer in the center of the screen
        StartX += (DW - ((MagH + 1) * width)) / 2;
        StartY += (DH - ((MagV + 1) * height)) / 2;

        displayEnvironment.display = GS_SET_DISPLAY(
                    StartX + StartXOffset, // X position in the display area (in VCK units)
                    StartY + StartYOffset, // Y position in the display area (in Raster units)
                    MagH,                  // Horizontal Magnification
                    MagV,                  // Vertical Magnification
                    DW - 1,                // Display area width
                    DH - 1);               // Display area height
    }

    return;
}

void setDisplayRegs(GsDispEnv* regs)
{
    *(vu64*)gs_p_pmode = regs->pmode;
    *(vu64*)gs_p_smode2 = regs->smode2;
    *(vu64*)gs_p_dispfb1 = regs->dispfb;
    *(vu64*)gs_p_dispfb2 = regs->dispfb;
    *(vu64*)gs_p_display1 = regs->display;
    *(vu64*)gs_p_display2 = regs->display;
    *(vu64*)gs_p_bgcolor = regs->bgcolor;
}

void disableDisplay()
{
    *(vu64*)gs_p_pmode = displayEnvironment.pmode & 0xFFFFFFC;
}

void enableDisplay()
{
    *(vu64*)gs_p_pmode = displayEnvironment.pmode;
}

#define RQ_EE_VIF1_FIFO ((vu128*)A_EE_VIF1_FIFO)
#define WRQ_EE_VIF1_FIFO(x) (*RQ_EE_VIF1_FIFO = (x))

#define UINT128(hi, lo) (((__uint128_t)(hi)) << 64 | (lo))

void GsResetPath(void)
{
    // Set Cycle reg WL=4, CL=4
    // Set Mask reg to 0
    // Set Mode reg to 0
    u128 fifo1 = UINT128(0x0500000000000000, 0x2000000001000404);
    // set MSKPATH3 - enable transfer
    // set BASE to 0
    // set OFFSET to 0
    // set ITOP to 0
    u128 fifo2 = UINT128(0x0400000002000000, 0x0300000006000000);

    WR_EE_VIF1_FBRST(1);
    WR_EE_VIF1_ERR(2);
    __asm__ __volatile__("sync.l");

    u32 asmtmp = 0;
    __asm__ __volatile__(
        "cfc2 %0, $vi28\n"
        "ori  %0, %0, 0x0200\n"
        "ctc2 %0, $vi28\n"
        "sync.p\n" ::"r"(asmtmp));

    WRQ_EE_VIF1_FIFO(fifo1);
    WRQ_EE_VIF1_FIFO(fifo2);
    WR_EE_GIF_CTRL(1);
}

// around 203f38 in main
void initGs()
{
    ee_sema_t semaParam;
    semaParam.init_count = 0;
    semaParam.max_count = 0x7ffffc17;
    vblankSema = CreateSema(&semaParam);

//    GsResetGraph(GS_INIT_RESET, GS_INTERLACED, GS_MODE_PAL, GS_FFMD_FRAME);
    GsResetGraph(GS_INIT_RESET, GS_NONINTERLACED, GS_MODE_DTV_480P, GS_FFMD_FRAME);

    vblankHandlerId = AddIntcHandler(2, vblank_handler, 0);
    EnableIntc(2);

    waitFramecountChange();

    disableDisplay();
    waitFramecountChange();

    SetDefDispEnv();
    setDisplayRegs(&displayEnvironment);
}

u64 zbuf_val;

u32 frameDMAProg[4000] __attribute__((aligned(16)));
u32* pDmaProg_RGBAQval;
u32* pDmaProg_ALPHA1_FIX_val;

inline u32 makeUV(u32 u, u32 v)
{
    return u | (v << 16);
}

inline u32 makeXY(u32 x, u32 y)
{
    return x | (y << 16);
}

void build480PEndFrameDMA()
{
    frameDMAProg[2] = 0;
    frameDMAProg[3] = 0;
    frameDMAProg[4] = 0x8000;
    frameDMAProg[5] = 0x10000000;
    frameDMAProg[6] = 0xe; // A+D Reg
    frameDMAProg[7] = 0;

    frameDMAProg[8] = 0; // no dither
    frameDMAProg[9] = 0;
    frameDMAProg[10] = 0x45; // DTHE
    frameDMAProg[11] = 0;

    frameDMAProg[12] = 0x000a0000; // FBP = 0, FBW = 10 (640), PSM = PSMCT32
    frameDMAProg[13] = 0;
    frameDMAProg[14] = 0x4c; // FRAME_1
    frameDMAProg[15] = 0;

    frameDMAProg[16] = 0x27f0000; // (0,0) -> (639, 511)
    frameDMAProg[17] = 0x1ff0000;
    frameDMAProg[18] = 0x40; // SCISSOR_1
    frameDMAProg[19] = 0;

    // TODO
    frameDMAProg[20] = 0x6c00; // (0x6c00, 0x7800)
    frameDMAProg[21] = 0x7800;
    frameDMAProg[22] = 0x18; // XYOFFSET_1
    frameDMAProg[23] = 0;

    frameDMAProg[24] = 0x61; // no mipmap
    frameDMAProg[25] = 0;
    frameDMAProg[26] = 0x14; // TEX1_1
    frameDMAProg[27] = 0;

    // ATE on, ATST never (all fail), AREF=0, AFAIL = FB_ONLY, DATE OFF, ZTEST ALWAYS
    frameDMAProg[28] = 0x31001; // only update FB, z always
    frameDMAProg[29] = 0;
    frameDMAProg[30] = 0x47; // TEST_1
    frameDMAProg[31] = 0;

    pDmaProg_RGBAQval = &frameDMAProg[32];
    frameDMAProg[32] = 0x00808080; // HIGHLIGHT mode
    frameDMAProg[33] = 0;
    frameDMAProg[34] = 1; // RGBAQ
    frameDMAProg[35] = 0;

    frameDMAProg[36] = 0x156; // flat shaded sprite, textured, alpha blended, no fog, UV mapped, context 1
    frameDMAProg[37] = 0;
    frameDMAProg[38] = 0; // PRIM
    frameDMAProg[39] = 0;

    pDmaProg_ALPHA1_FIX_val = &frameDMAProg[41];

    frameDMAProg[40] = 0x64; // 01 10 01 00, d=Cd, c=FIX, b=Cd, a=Cs. Cv=(Cs - Cd)*FIX>>7 + Cd
    frameDMAProg[41] = 0x80; // FIX
    frameDMAProg[42] = 0x42; // ALPHA_1
    frameDMAProg[43] = 0;

    frameDMAProg[44] = 0;
    frameDMAProg[45] = 0;
    frameDMAProg[46] = 0x3f; // TEXFLUSH
    frameDMAProg[47] = 0;

    u64 tex0 = GS_SET_TEX0(0x1400, 0x0A, 0, // PSMCT32
    10,     // TW
    10,     // TH
    0,      //RGB
    2,      // HIGHLIGHT
    0,      // CBP
    0,      // CPSM
    0,      // CSM
    0,      // CSA
    0       // CLD
     );
    *(u64*)&frameDMAProg[48] = tex0;
    frameDMAProg[50] = 6; // TEX0_1
    frameDMAProg[51] = 0;

    int progIdx = 52;

    unsigned int u0 = 0;
    unsigned int v0 = 0;
    unsigned int u1 = 640 * 16;
    unsigned int v1 = 512 * 16;

    unsigned int x0 = 0x6c00;
    unsigned int y0 = 0x7800;
    unsigned int x1 = x0 + 640 * 16;
    unsigned int y1 = y0 + 512 * 16;

    frameDMAProg[progIdx++] = makeUV(u0, v0);
    frameDMAProg[progIdx++] = 0;
    frameDMAProg[progIdx++] = 3; // UV
    frameDMAProg[progIdx++] = 0;

    frameDMAProg[progIdx++] = makeXY(x0, y0);
    frameDMAProg[progIdx++] = 0x0A;
    frameDMAProg[progIdx++] = 5; // XYZ2
    frameDMAProg[progIdx++] = 0;

    frameDMAProg[progIdx++] = makeUV(u1, v1);
    frameDMAProg[progIdx++] = 0;
    frameDMAProg[progIdx++] = 3; // UV
    frameDMAProg[progIdx++] = 0;

    frameDMAProg[progIdx++] = makeXY(x1, y1);
    frameDMAProg[progIdx++] = 0x0A;
    frameDMAProg[progIdx++] = 5; // XYZ2
    frameDMAProg[progIdx++] = 0;

    frameDMAProg[progIdx++] = 1;
    frameDMAProg[progIdx++] = 0;
    frameDMAProg[progIdx++] = 0x45; // DTHE enable
    frameDMAProg[progIdx++] = 0;

    frameDMAProg[progIdx++] = 0x31317575;
    frameDMAProg[progIdx++] = 0x31317575;
    frameDMAProg[progIdx++] = 0x44; // DTHE enable
    frameDMAProg[progIdx++] = 0;

    while (progIdx & 0x03) {
        frameDMAProg[progIdx++] = 0;
    }

    //traceln("progIdx = %d", progIdx);

    int qwc = progIdx / 4;

    // Write DMA Tag
    frameDMAProg[0] = 0x70000000 | (qwc - 1);
    frameDMAProg[1] = 0;

    // write GIF Tag nloop
    frameDMAProg[4] |= (qwc - 2);
}

void buildInterlacedFrameDMA()
{
    frameDMAProg[2] = 0;
    frameDMAProg[3] = 0;
    frameDMAProg[4] = 0x8000;
    frameDMAProg[5] = 0x10000000;
    frameDMAProg[6] = 0xe; // A+D Reg
    frameDMAProg[7] = 0;

    frameDMAProg[8] = 0; // no dither
    frameDMAProg[9] = 0;
    frameDMAProg[10] = 0x45; // DTHE
    frameDMAProg[11] = 0;

    frameDMAProg[12] = 0x000a0000; // FBP = 0, FBW = 10 (640), PSM = PSMCT32
    frameDMAProg[13] = 0;
    frameDMAProg[14] = 0x4c; // FRAME_1
    frameDMAProg[15] = 0;

    frameDMAProg[16] = 0x27f0000; // (0,0) -> (639, 255)
    frameDMAProg[17] = 0xff0000;
    frameDMAProg[18] = 0x40; // SCISSOR_1
    frameDMAProg[19] = 0;

    frameDMAProg[20] = 0x6c00; // (0x6c00, 0x7800)
    frameDMAProg[21] = 0x7800;
    frameDMAProg[22] = 0x18; // XYOFFSET_1
    frameDMAProg[23] = 0;

    frameDMAProg[24] = 0x61; // no mipmap
    frameDMAProg[25] = 0;
    frameDMAProg[26] = 0x14; // TEX1_1
    frameDMAProg[27] = 0;

    // ATE on, ATST never (all fail), AREF=0, AFAIL = FB_ONLY, DATE OFF, ZTEST ALWAYS
    frameDMAProg[28] = 0x31001; // only update FB, z always
    frameDMAProg[29] = 0;
    frameDMAProg[30] = 0x47; // TEST_1
    frameDMAProg[31] = 0;

    pDmaProg_RGBAQval = &frameDMAProg[32];
    frameDMAProg[32] = 0x00808080; // HIGHLIGHT mode
    frameDMAProg[33] = 0;
    frameDMAProg[34] = 1; // RGBAQ
    frameDMAProg[35] = 0;

    frameDMAProg[36] = 0x156; // flat shaded sprite, textured, alpha blended, no fog, UV mapped, context 1
    frameDMAProg[37] = 0;
    frameDMAProg[38] = 0; // PRIM
    frameDMAProg[39] = 0;

    pDmaProg_ALPHA1_FIX_val = &frameDMAProg[41];

    frameDMAProg[40] = 0x64; // 01 10 01 00, d=Cd, c=FIX, b=Cd, a=Cs. Cv=(Cs - Cd)*FIX>>7 + Cd
    frameDMAProg[41] = 0x80; // FIX
    frameDMAProg[42] = 0x42; // ALPHA_1
    frameDMAProg[43] = 0;

    frameDMAProg[44] = 0;
    frameDMAProg[45] = 0;
    frameDMAProg[46] = 0x3f; // TEXFLUSH
    frameDMAProg[47] = 0;

    // 0010 0000 0000 0000 0000 0000 0001 0010 1010 1000 0010 0101 0000 1010 0000 0000
    // 001 00000 0 0000 00000000000000 10 0 1010 1010 000010 01_0100 00101000000000
    //

    // TBP0 = 0x0A00 (28000)
    // TBW  = 0x14 (1280)
    // PSM = PSMCT16
    // TW = 2^10 = 1024
    // TH = 2^10 = 1024
    // TCC = RGB
    // TFX = HIGHLIGHT
    // CBP = 0
    // CPSM = PSMCT32
    // CSM = CSM1
    // CSA = 0
    // CLD = 0

    frameDMAProg[48] = 0xa8250a00;
    frameDMAProg[49] = 0x20000012;
    frameDMAProg[50] = 6; // TEX0_1
    frameDMAProg[51] = 0;

    int progIdx = 52;

    unsigned int y0 = 0;
    unsigned int y1 = 64;
    do {
        int xpos = 0;
        unsigned int u0 = 0x10;
        unsigned int x0 = 0x6c00;
        do {
            unsigned int x1 = x0 + 0x200;
            unsigned int u1 = u0 + 0x400;

            frameDMAProg[progIdx++] = makeUV(u0, y0 * 0x10 + 0x10);
            frameDMAProg[progIdx++] = 0;
            frameDMAProg[progIdx++] = 3; // UV
            frameDMAProg[progIdx++] = 0;

            frameDMAProg[progIdx++] = makeXY(x0, y0 * 8 + 0x7800);
            frameDMAProg[progIdx++] = 0x0A;
            frameDMAProg[progIdx++] = 5; // XYZ2
            frameDMAProg[progIdx++] = 0;

            frameDMAProg[progIdx++] = makeUV(u1, y1 * 0x10 + 0x10);
            frameDMAProg[progIdx++] = 0;
            frameDMAProg[progIdx++] = 3; // UV
            frameDMAProg[progIdx++] = 0;

            frameDMAProg[progIdx++] = makeXY(x1, y1 * 8 + 0x7800);
            frameDMAProg[progIdx++] = 0x0A;
            frameDMAProg[progIdx++] = 5; // XYZ2
            frameDMAProg[progIdx++] = 0;

            u0 = u1;
            x0 = x1;

            xpos += 32;
        } while (xpos < 320);
        y0 = y1;
        y1 += 64;
    } while (y0 < 512);

    frameDMAProg[progIdx++] = 0xa8250b40; // TBP0 = 0x0B40 (2D000)
    frameDMAProg[progIdx++] = 0x20000012;
    frameDMAProg[progIdx++] = 6; // TEX0_1
    frameDMAProg[progIdx++] = 0;

    y1 = 0x40;
    y0 = 0;

    do {
        int xpos = 0;
        int u0 = 0x10;
        int x0 = 0x8000;

        do {
            int x1 = x0 + 0x200;
            int u1 = u0 + 0x400;

            frameDMAProg[progIdx++] = makeUV(u0, y0 * 0x10 + 0x10);
            frameDMAProg[progIdx++] = 0;
            frameDMAProg[progIdx++] = 3; // UV
            frameDMAProg[progIdx++] = 0;

            frameDMAProg[progIdx++] = makeXY(x0, y0 * 8 + 0x7800);
            frameDMAProg[progIdx++] = 0x0A;
            frameDMAProg[progIdx++] = 5; // XYZ2
            frameDMAProg[progIdx++] = 0;

            frameDMAProg[progIdx++] = makeUV(u1, y1 * 0x10 + 0x10);
            frameDMAProg[progIdx++] = 0;
            frameDMAProg[progIdx++] = 3; // UV
            frameDMAProg[progIdx++] = 0;

            frameDMAProg[progIdx++] = makeXY(x1, y1 * 8 + 0x7800);
            frameDMAProg[progIdx++] = 0x0A;
            frameDMAProg[progIdx++] = 5; // XYZ2
            frameDMAProg[progIdx++] = 0;

            u0 = u1;
            x0 = x1;

            xpos += 32;
        } while (xpos < 320);
        y0 = y1;
        y1 = y1 + 64;
    } while (y0 < 512);

    frameDMAProg[progIdx++] = 1;
    frameDMAProg[progIdx++] = 0;
    frameDMAProg[progIdx++] = 0x45; // DTHE enable
    frameDMAProg[progIdx++] = 0;

    frameDMAProg[progIdx++] = 0x31317575;
    frameDMAProg[progIdx++] = 0x31317575;
    frameDMAProg[progIdx++] = 0x44; // DTHE enable
    frameDMAProg[progIdx++] = 0;

    while (progIdx & 0x03) {
        frameDMAProg[progIdx++] = 0;
    }

    //traceln("progIdx = %d", progIdx);

    int qwc = progIdx / 4;

    // Write DMA Tag
    frameDMAProg[0] = 0x70000000 | (qwc - 1);
    frameDMAProg[1] = 0;

    // write GIF Tag nloop
    frameDMAProg[4] |= (qwc - 2);
    return;
}

void buildFrameDMAProg()
{
    GsGParam_t* dp = GsGetGParam();
    if (dp->omode == GS_MODE_DTV_480P){
        build480PEndFrameDMA();
    } else {
        buildInterlacedFrameDMA();
    }
}

// 0x002006f0
void initDisplay(void)
{
    int data[48];

    displayEnvironment.dispfb &= 0xfffffe00;
    zbuf_val = 0x1020000f0;

    data[0] = 0x7000000a; // DMA end, 10 qwords follow
    data[1] = 0;
    data[2] = 0;
    data[3] = 0;

    data[4] = 0x8009; // NLOOP = 9, EOP, NREG=1
    data[5] = 0x10000000;

    data[6] = 0xe; // A+D REG
    data[7] = 0;

    data[8] = 0x2140050; // FBW = 0x14 (w=1280), PSM = PSMCT16, FBP = 0x50 (0x28000)
    data[9] = 0;
    data[10] = 0x4d; // FRAME_2
    data[11] = 0;

    data[12] = 0x20000f0; // PSM = PSMZ16, ZBP = 0xF0 (0x78000)
    data[13] = 0;
    data[14] = 0x4f; // ZBUF_2
    data[15] = 0;

    data[16] = 0x4ff0000; // (0, 0) -> (1279, 511)
    data[17] = 0x1ff0000;
    data[18] = 0x41; // SCISSOR_2
    data[19] = 0;

    data[20] = 0x5800; // (0x5800, 0x7000)
    data[21] = 0x7000;
    data[22] = 0x19; // XYOFFSET_2
    data[23] = 0;

    data[24] = 0x2a; // 00 10 10 10: D=Cs, C=0, B=0, A=0
    data[25] = 0;
    data[26] = 0x43; // ALPHA_2
    data[27] = 0;

    data[28] = 0x50013; // 0101 0000 0000 0001 0011
    data[29] = 0;       // ZTST=GEQUAL, ZTE=Depth test on, Dest Alpha test off, AREF=1, ATST=ALWAYS, ATE=On
    data[30] = 0x48;    // TEST_2
    data[31] = 0;

    //    2    0    0    0    0    0    0    1    d    c    0    0    b    2    0    0
    // 0010 0000 0000 0000 0000 0000 0000 0001 1101 1100 0000 0000 1011 0010 0000 0000
    // CLD  CSA                  CBP
    // 001 00000 0 0000 00000000000000 00 0 0111 0111 000000 000010 11_0010_0000_0000

    // TBP0 = 0x3200 (0xC8000)
    // TBW = 2 (128)
    // PSM = PSMCT32
    // TW = 128
    // TH = 128
    // TCC = RGB
    // TFX = MODULATE
    // CBP = 0
    // CPSM = PSMCT32
    // CSM = CSM1
    // CSA = 0
    // CLD = 1 Load is performed to CSA position of buffer

    data[32] = 0xdc00b200;
    data[33] = 0x20000001;
    data[34] = 7; // TEX0_2
    data[35] = 0;

    data[36] = 0x61; // 01 1 000 01, MMIN=LINEAR, MMAG=LINEAR, MXL=0, LCM=FIXED
    data[37] = 0;
    data[38] = 0x15; // TEX1_2
    data[39] = 0;

    data[40] = 0; // Repeat
    data[41] = 0;
    data[42] = 9; // CLAMP_2
    data[43] = 0;

    data[44] = 0;
    data[45] = 0;
    data[46] = 0;
    data[47] = 0;

    zbuf_val = 0x020000f0; // PSMZ16, ZBP = 0xF0 (0x78000)

    /*
        FlushCache(WRITEBACK_DCACHE);
        sceDmaSend((int*)GIF_DCHR, (int*)pData);
        sceGSSyncPath(0, 0);
        addr = environmentTexture.gif_madr_val;
        // set SBP to 0x3200 (C8000)
        *(undefined2*)((environmentTexture.gif_madr_val | 0x20000000U) + 0x14) = 0x3200;
        SYNC(0);
        sceDmaSendN(GIF_DCHR, (void*)addr, (uint)environmentTexture.qwc);

        */
    buildFrameDMAProg();
}


u32 startFrameDmaBuffer[300] __attribute__((aligned(16)));

// This offset is from the previous frame.
u32 y_offset = 0x7000;

// sometimes set to 0x18
u32 DAT_ram_00325c50 = 0;

int scissorY0;
int scissorY1;

void startFrameInterlaced()
{
    //traceln("startFrame");

    u32* ucabBuf = (u32*)UCAB_SEG(startFrameDmaBuffer); 

    ucabBuf[2] = 0;
    ucabBuf[3] = 0;
    ucabBuf[4] = 0x8000; // GIF Tag A+D
    ucabBuf[5] = 0x10000000;
    ucabBuf[6] = 0xe;
    ucabBuf[7] = 0;


    // FBP = 0x50 = 0x28000 word address = 0xA0000 byte address
    // FBW = 0x14 = 0x500 = 1280 pixels
    // Height of 512 would take it up to the z buffer
    // PSM = PSMCT16
    ucabBuf[8] = 0x2140050;
    ucabBuf[9] = 0;
    ucabBuf[10] = GSReg::FRAME_1;
    ucabBuf[11] = 0;


    // ZBP = 0xF0 = 0x78000 word addres, 0x1E0000 byte address
    // PSM = PSMZ16
    // Z buffer is updated
    ucabBuf[12] = 0x20000f0;
    ucabBuf[13] = 0;
    ucabBuf[14] = GSReg::ZBUF_1;
    ucabBuf[15] = 0;

    //if ((DAT_ram_003248b8 == '\0') || (isLoading != 0)) {
    scissorY1 = 0x1ff;
    scissorY0 = 0;
    /*}
    else {
      scissorY1 = 0x1c8 - 2 * DAT_ram_00325c50;
      scissorY0 = 0x37 - 2 * DAT_ram_00325c50;
    }
*/

    // scissor 0 -> 1279, 0 -> 511
    ucabBuf[16] = 0x4ff0000;
    ucabBuf[17] = 0x1ff0000;
    ucabBuf[18] = GSReg::SCISSOR_1;
    ucabBuf[19] = 0;

    // Primitive coordinate system is 0 -> 4095.9375
    // Window coordinates have an origin at the top left of the framebuffer.

    // Centre the FB in the primitive coord space.
    // 1408 = (4096 - 1280) / 2
    // 1792 = (4096 - 512)  / 2

    // Offset (1408, 1792)

    ucabBuf[20] = 0x5800;
    ucabBuf[21] = y_offset;
    ucabBuf[22] = GSReg::XYOFFSET_1;
    ucabBuf[23] = 0;

    ucabBuf[24] = 0x5800;
    ucabBuf[25] = y_offset;
    ucabBuf[26] = GSReg::XYOFFSET_2;
    ucabBuf[27] = 0;

    y_offset = 0x7000;
    if (isOddField) {
        y_offset = 0x7008;
    }
    /*
    if (isLoading == 0) {
        // sometimes set to 0x18
        y_offset += DAT_ram_00325c50 * 0x20;
    }
    */

    // enable dither
    ucabBuf[28] = 1;
    ucabBuf[29] = 0;
    ucabBuf[30] = GSReg::DTHE;
    ucabBuf[31] = 0;

    // Alpha blend control disabled
    ucabBuf[32] = 0;
    ucabBuf[33] = 0;
    ucabBuf[34] = GSReg::PABE;
    ucabBuf[35] = 0;

    // Color clamping performed
    ucabBuf[36] = 1;
    ucabBuf[37] = 0;
    ucabBuf[38] = GSReg::COLCLAMP;
    ucabBuf[39] = 0;

    int dmaIdx = 40;

    /*
        // Sets the colour based on world flags.

        if (DAT_ram_00325c2c == 0) {
            fVar9 = (Vec3_ram_00233d68.z + 600.0) * 0.001666667;
            if (fVar9 < 0.0) {
                fVar9 = 0.0;
            }
            if (1.0 < fVar9) {
                fVar9 = 1.0;
            }
            local_30 = (ulong*)(ucabBuf + 0x2c);

            uVar8 = (uint)((float)DAT_ram_00325c30 + (float)(DAT_ram_00325c34 - DAT_ram_00325c30) * fVar9);
            ucabBuf[40] =
                uVar8 | 0x80000000 | (uint)((long)(int)uVar8 << 0x10) | (uint)((long)(int)uVar8 << 8);
            ucabBuf[41] = 0;
            ucabBuf[42] = GSReg::RGBAQ;
            ucabBuf[43] = 0;

            if ((DAT_ram_00325074 & 0x80) != 0) {
                ucabBuf[44] = 0x80644646;
                ucabBuf[45] = 0;
                ucabBuf[46] = GSReg::RGBAQ;
                ucabBuf[47] = 0;

                local_30 = (ulong*)(uint32_t_ARRAY_ram_30643ce0 + 0x30);
            }
            if ((DAT_ram_00325074 & 0x2000000) != 0) {
                *(undefined4*)local_30 = 0x803f3b34;
                *(undefined4*)((int)local_30 + 4) = 0;
                local_30[1] = 1;
                local_30 = local_30 + 2;
            }
            if ((DAT_ram_00325074 & 0x400000) != 0) {
                *(undefined4*)local_30 = 0x80455d75;
                *(undefined4*)((int)local_30 + 4) = 0;
                local_30[1] = 1;
                local_30 = local_30 + 2;
            }
            if ((DAT_ram_00325074 & 0x200) != 0) {
                *(undefined4*)local_30 = 0x80272727;
                *(undefined4*)((int)local_30 + 4) = 0;
                local_30[1] = 1;
                local_30 = local_30 + 2;
            }
            if ((DAT_ram_00325074 & 0x4000000) != 0) {
                *(undefined4*)local_30 = 0x80142121;
                *(undefined4*)((int)local_30 + 4) = 0;
                local_30[1] = 1;
                local_30 = local_30 + 2;
            }
            if (((DAT_ram_00325c3c != 0) || (DAT_ram_00325c44 != 0)) || (isLoading != 0)) {
                *(undefined4*)local_30 = 0x80000000;
                local_30[1] = 1;
                *(undefined4*)((int)local_30 + 4) = 0;
                local_30 = local_30 + 2;
            }
            if ((DAT_ram_00325074 & 1) == 0) {
                set_324014(1.0);
            } else {
                fVar9 = (Vec3_ram_00233d68.z + 360.0) * 0.002777778;
                if (fVar9 < 0.0) {
                    fVar9 = 0.0;
                }
                if (1.0 < fVar9) {
                    fVar9 = 1.0;
                }
                set_324014(fVar9 * 0.7 + 1.0);
            }
        } else {
            local_30 = (ulong*)(ucabBuf + 0x2c);
    */
    ucabBuf[dmaIdx++] = 0x80303030;
    ucabBuf[dmaIdx++] = 0;
    ucabBuf[dmaIdx++] = GSReg::RGBAQ;
    ucabBuf[dmaIdx++] = 0;
    /*
        }
    */

    // Alpha test off
    // Destination alpha test off
    // ztest greater
    ucabBuf[dmaIdx++] = 0x387f4;
    ucabBuf[dmaIdx++] = 0;
    ucabBuf[dmaIdx++] = GSReg::TEST_1;
    ucabBuf[dmaIdx++] = 0;

    ucabBuf[dmaIdx++] = 1; // PRIM REG
    ucabBuf[dmaIdx++] = 0;
    ucabBuf[dmaIdx++] = GSReg::PRMODECONT;
    ucabBuf[dmaIdx++] = 0;

    ucabBuf[dmaIdx++] = 6; // untextured sprite
    ucabBuf[dmaIdx++] = 0;
    ucabBuf[dmaIdx++] = GSReg::PRIM;
    ucabBuf[dmaIdx++] = 0;

    u32 iVar1 = DAT_ram_00325c50;

    u32 physXpos = 0x5800;      // 0 in window pos
    do {
        // Clears the frame by vertical strips
        ucabBuf[dmaIdx++] = physXpos | 0x70000000;
        
        ucabBuf[dmaIdx++] = 0;
        ucabBuf[dmaIdx++] = GSReg::XYZ2;
        ucabBuf[dmaIdx++] = 0;

        physXpos += 0x400;     // 64.0

        ucabBuf[dmaIdx++] = physXpos | ((iVar1 * 2 + 0x900) * 0x10) << 0x10;
        ucabBuf[dmaIdx++] = 0;
        ucabBuf[dmaIdx++] = GSReg::XYZ2;
        ucabBuf[dmaIdx++] = 0;
    } while (physXpos < 0x5800 + 1280*16);

    ucabBuf[dmaIdx++] = 0x425045;
    ucabBuf[dmaIdx++] = 0;
    ucabBuf[dmaIdx++] = GSReg::FOGCOL;
    ucabBuf[dmaIdx++] = 0;

    ucabBuf[dmaIdx++] = 0x4ff0000; // (0, 1279) -> (scissorY0, scissorY1)
    ucabBuf[dmaIdx++] = scissorY1 << 0x10 | scissorY0;
    ucabBuf[dmaIdx++] = GSReg::SCISSOR_1;
    ucabBuf[dmaIdx++] = 0;

    //traceln("dmaIdx=%d", dmaIdx);

    int qwc = dmaIdx / 4;

    // Write DMA Tag
    ucabBuf[0] = 0x70000000 | (qwc - 1);
    ucabBuf[1] = 0;

    // write GIF Tag nloop
    ucabBuf[4] |= (qwc - 2);

    dmaSend(DmaChannel::GIF, startFrameDmaBuffer);
    sceGsSyncPath(0, 0);
}

void startFrame480P()
{
    u32* ucabBuf = (u32*)UCAB_SEG(startFrameDmaBuffer); 

    ucabBuf[2] = 0;
    ucabBuf[3] = 0;
    ucabBuf[4] = 0x8000; // GIF Tag A+D
    ucabBuf[5] = 0x10000000;
    ucabBuf[6] = 0xe;
    ucabBuf[7] = 0;


    // FBP = 0xA0 = 0x140000 byte address
    // FBW = 0xA = 640 pixels
    // Height of 512 would take it up to the z buffer
    // PSM = PSMCT32
    ucabBuf[8] = 0x00A00A0;
    ucabBuf[9] = 0;
    ucabBuf[10] = GSReg::FRAME_1;
    ucabBuf[11] = 0;


    // ZBP = 140 = 0x280000 byte address
    // PSM = PSMZ16
    // Z buffer is updated
    ucabBuf[12] = 0x2000140;
    ucabBuf[13] = 0;
    ucabBuf[14] = GSReg::ZBUF_1;
    ucabBuf[15] = 0;

    scissorY1 = 0x1ff;
    scissorY0 = 0;

    // scissor 0 -> 639, 0 -> 511
    ucabBuf[16] = 0x27f0000;
    ucabBuf[17] = 0x1ff0000;
    ucabBuf[18] = GSReg::SCISSOR_1;
    ucabBuf[19] = 0;

    // Primitive coordinate system is 0 -> 4095.9375
    // Window coordinates have an origin at the top left of the framebuffer.

    // Centre the FB in the primitive coord space.
    // 1408 = (4096 - 1280) / 2
    // 1792 = (4096 - 512)  / 2

    // Offset (1408, 1792)

    ucabBuf[20] = 0x5800;
    ucabBuf[21] = 0x7000;
    ucabBuf[22] = GSReg::XYOFFSET_1;
    ucabBuf[23] = 0;

    ucabBuf[24] = 0x5800;
    ucabBuf[25] = 0x7000;
    ucabBuf[26] = GSReg::XYOFFSET_2;
    ucabBuf[27] = 0;

    /*
    if (isLoading == 0) {
        // sometimes set to 0x18
        y_offset += DAT_ram_00325c50 * 0x20;
    }
    */

    // enable dither
    ucabBuf[28] = 1;
    ucabBuf[29] = 0;
    ucabBuf[30] = GSReg::DTHE;
    ucabBuf[31] = 0;

    // Alpha blend control disabled
    ucabBuf[32] = 0;
    ucabBuf[33] = 0;
    ucabBuf[34] = GSReg::PABE;
    ucabBuf[35] = 0;

    // Color clamping performed
    ucabBuf[36] = 1;
    ucabBuf[37] = 0;
    ucabBuf[38] = GSReg::COLCLAMP;
    ucabBuf[39] = 0;

    int dmaIdx = 40;

 
    ucabBuf[dmaIdx++] = 0x80303030;
    ucabBuf[dmaIdx++] = 0;
    ucabBuf[dmaIdx++] = GSReg::RGBAQ;
    ucabBuf[dmaIdx++] = 0;


    // Alpha test off
    // Destination alpha test off
    // ztest greater
    ucabBuf[dmaIdx++] = 0x387f4;
    ucabBuf[dmaIdx++] = 0;
    ucabBuf[dmaIdx++] = GSReg::TEST_1;
    ucabBuf[dmaIdx++] = 0;

    ucabBuf[dmaIdx++] = 1; // PRIM REG
    ucabBuf[dmaIdx++] = 0;
    ucabBuf[dmaIdx++] = GSReg::PRMODECONT;
    ucabBuf[dmaIdx++] = 0;

    ucabBuf[dmaIdx++] = 6; // untextured sprite
    ucabBuf[dmaIdx++] = 0;
    ucabBuf[dmaIdx++] = GSReg::PRIM;
    ucabBuf[dmaIdx++] = 0;

    u32 iVar1 = DAT_ram_00325c50;

    u32 physXpos = 0x5800;      // 0 in window pos
    do {
        // Clears the frame by vertical strips
        ucabBuf[dmaIdx++] = physXpos | 0x70000000;
        
        ucabBuf[dmaIdx++] = 0;
        ucabBuf[dmaIdx++] = GSReg::XYZ2;
        ucabBuf[dmaIdx++] = 0;

        physXpos += 0x400;     // 64.0

        ucabBuf[dmaIdx++] = physXpos | ((iVar1 * 2 + 0x900) * 0x10) << 0x10;
        ucabBuf[dmaIdx++] = 0;
        ucabBuf[dmaIdx++] = GSReg::XYZ2;
        ucabBuf[dmaIdx++] = 0;
    } while (physXpos < 0x5800 + 640*16);

    ucabBuf[dmaIdx++] = 0x425045;
    ucabBuf[dmaIdx++] = 0;
    ucabBuf[dmaIdx++] = GSReg::FOGCOL;
    ucabBuf[dmaIdx++] = 0;

    ucabBuf[dmaIdx++] = 0x27f0000; // (0, 639) -> (scissorY0, scissorY1)
    ucabBuf[dmaIdx++] = scissorY1 << 0x10 | scissorY0;
    ucabBuf[dmaIdx++] = GSReg::SCISSOR_1;
    ucabBuf[dmaIdx++] = 0;

    //traceln("dmaIdx=%d", dmaIdx);

    int qwc = dmaIdx / 4;

    // Write DMA Tag
    ucabBuf[0] = 0x70000000 | (qwc - 1);
    ucabBuf[1] = 0;

    // write GIF Tag nloop
    ucabBuf[4] |= (qwc - 2);

    dmaSend(DmaChannel::GIF, startFrameDmaBuffer);
    sceGsSyncPath(0, 0);
}

void startFrame()
{
    GsGParam_t* dp = GsGetGParam();
    if (dp->omode == GS_MODE_DTV_480P){
        startFrame480P();
    } else {
        startFrameInterlaced();
    }
}

void endFrame()
{
    //traceln("EndFrame");
    /*
        int clippedAlpha = frameDisplayAlpha;
        if (0x80 < frameDisplayAlpha) {
            clippedAlpha = 0x80;
        }

        if (clippedAlpha < 0) {
            clippedAlpha = 0;
        }
        *pDmaProg_ALPHA1_FIX_val = clippedAlpha;
        frameDisplayAlpha += 3;
    */

    /*
        float fVar5 = 0.2;
        if (0.2 <= FLOAT_00325c5c) {
            fVar5 = FLOAT_00325c5c;
        }

        // Using highlight, so RGB modulate and then add alpha as a white offest

        int iVar2 = INT_ram_00325c80;
        if (0xff < INT_ram_00325c80) {
            iVar2 = 0xff;
        }
        if (iVar2 < 0) {
            iVar2 = 0;
        }

        iVar8 = (int)((float)DAT_ram_00325c78 * fVar5 + (float)DAT_ram_00325c78 * fVar5);
        iVar6 = (int)((float)INT_ram_00325c7c * fVar5 + (float)INT_ram_00325c7c * fVar5);
        uVar7 = (uint)((float)DAT_ram_00325c74 * fVar5 + (float)DAT_ram_00325c74 * fVar5);
        iVar4 = 0xff;
        if (iVar8 < 0x100) {
            iVar4 = iVar8;
        }
        iVar8 = 0xff;
        if (iVar6 < 0x100) {
            iVar8 = iVar6;
        }
        uVar3 = 0xff;
        if ((int)uVar7 < 0x100) {
            uVar3 = uVar7;
        }
        if (iVar4 < 0) {
            iVar4 = 0;
        }
        if (iVar8 < 0) {
            iVar8 = 0;
        }
        if ((int)uVar3 < 0) {
            uVar3 = 0;
        }
        *(uint*)pDmaProg_RGBAQval = uVar3 | iVar2 << 0x18 | iVar8 << 0x10 | iVar4 << 8;
    */

    /*
        // Modulate colours

        bVar1 = 0x7b < DAT_ram_00325c74;
        if (DAT_ram_00325c74 < 0x85) {
            DAT_ram_00325c74 += 4;
            if (bVar1) {
                DAT_ram_00325c74 = 0x80;
            }
        } else {
            DAT_ram_00325c74 += -4;
        }
        bVar1 = 0x7b < DAT_ram_00325c78;
        if (DAT_ram_00325c78 < 0x85) {
            DAT_ram_00325c78 += 4;
            if (bVar1) {
                DAT_ram_00325c78 = 0x80;
            }
        } else {
            DAT_ram_00325c78 += -4;
        }
        bVar1 = 0x7b < INT_ram_00325c7c;
        if (INT_ram_00325c7c < 0x85) {
            INT_ram_00325c7c += 4;
            if (bVar1) {
                INT_ram_00325c7c = 0x80;
            }
        } else {
            INT_ram_00325c7c += -4;
        }
        bVar1 = -9 < INT_ram_00325c80;
        if (INT_ram_00325c80 < 9) {
            INT_ram_00325c80 += 8;
            if (bVar1) {
                INT_ram_00325c80 = 0;
            }
        } else {
            INT_ram_00325c80 += -8;
        }
    */

    FlushCache(WRITEBACK_DCACHE);
    dmaSend(DmaChannel::GIF, frameDMAProg);
    sceGsSyncPath(0, 0);
}
