#include <ps2_reg_defs.h>
#include <stdio.h>

#include <sifrpc.h>
#include <loadfile.h>

#include "animDebug/animDebug.h"
#include "trace.h"
#include "dlist.h"
#include "gsAllocator.h"
#include "display.h"
#include "elfData.h"
#include "filesys.h"
#include "levelLoop.h"
#include "lump.h"
#include "pad.h"
#include "scene.h"
#include "state.h"
#include "showLanguageMenu.h"

static void
loadModules(void)
{
    int ret = SifLoadModule("rom0:SIO2MAN", 0, NULL);
    if (ret < 0) {
        traceln("sifLoadModule sio failed: %d\n", ret);
    }

    ret = SifLoadModule("rom0:PADMAN", 0, NULL);
    if (ret < 0) {
        traceln("sifLoadModule pad failed: %d\n", ret);
    }
}



int main(int argc, char* argv[])
{
    loadModules();
    
    initGs();

    readElfData();
    fixupFnt((Font *)menuFont, "elf", "menuFont");
    

    initPads();
    initCD();
    traceln("cd init done");

    initTextureAllocStuff();
    initDMA();
    traceln("DMA init done");

    bootstrapVIFs();

    WR_EE_T0_MODE(0x83);      // H-blank count, not interrupt
    WR_EE_T0_COUNT(0);
    WR_EE_T1_MODE(0x9f);      // H-blank clock, gate on Vblank, reset on rising edge


    getLmp("fx.lmp");
    //getLmp("mouth.lmp");
    setLmpGeneration(100);

    initDisplay();
    traceln("about to run scene");

    runScene(&showLanguageMenu);

    bool showAnimDebug = true;
    if (showAnimDebug){
        disableBackgroundColourEffects = 1;
        curLevelId[0] = 0;
        AnimDebug::setup(argc, argv);
        runLevelLoop();
    }
    freeElfData();
    traceln("bye");

    while (true) {
    }
    return 0;
}
