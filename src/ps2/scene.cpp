#include <kernel.h>
#include <ee_regs.h>
#include "trace.h"

#include "state.h"

#include "pad.h"
#include "scene.h"
#include "display.h"
#include "filesys.h"
#include "dlist.h"
#include "draw.h"

void runScene(sceneHandler sceneFunc)
{
    int done = 0;
    int displayEnableCountdown = 2;
    isInterlaced = 1;
    displayEnvironment.dispfb &= 0xfffffe00;

    WR_EE_GS_PMODE(0);
    setDisplayRegs(&displayEnvironment);
    WR_EE_GS_PMODE(0);
    do {
        startFrame();
        FlushCache(0);
        
        kickoffDMA();
        curDMABufHead = (u64*)UCAB_SEG(dmaBuffers[sceneFrameNum & 1]); 
        curDMABufTail = curDMABufHead;
        readInput();
        if (sceneFunc) {
            done = (*sceneFunc)();
        }
        dealWithCdStreaming();
        /*
        if (DAT_ram_0032445 > 0){
            if ((((id = &DAT_ram_00238890, (DAT_ram_00238894 & 0xff0fffff) == 0)) && 
            ((DAT_ram_002388e8 & 0xff0fffff) == 0)) {
                DAT_ram_00324454 -= 1;
            }
        }
        */
        waitDMASema();
        sceGsSyncPath(0,0);
        sceneFrameNum += 1;
        frameCount += 1;
        //FUN_ram_00140a78();
        //traceln("Wait on vbl sema id = %d", vblankSema);
        while (vblankSetsMeToFF != 0xff) {
            WaitSema(vblankSema);
        }
        // Interlaced FRAME mode (read every line)
        WR_EE_GS_SMODE2(3);
        if (displayEnableCountdown < 1) {    
            // Enable read circuit 2, disable read circuit 1
            WR_EE_GS_PMODE(2);
        } else {
            // Both read circuits off
            WR_EE_GS_PMODE(0);
            --displayEnableCountdown;
        }
        //REG_RCNT0_COUNT = 0;
        endFrame();

        DI();
        vblankSetsMeToFF = 0; //BYTE_ram_00325c8b == 0;
        // INT_ram_0032445c = (char)BYTE_ram_00325c8c + 1;

        EI();
        //  FUN_ram_001f0db0();
    } while (done == 0);

    //FUN_ram_00182310(1);
    WR_EE_GS_PMODE(0);
    /*
    FUN_ram_001f02c0(0);
    FUN_ram_001f02c0(1);
    FUN_ram_001f02c0(2);
    FUN_ram_0013b000(1);
    */
    return;
}
