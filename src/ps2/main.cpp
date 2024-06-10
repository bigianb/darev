#include <stdio.h>
#include <sifrpc.h>
#include <loadfile.h>

#include "trace.h"
#include "dlist.h"
#include "gsAllocator.h"
#include "display.h"
#include "elfData.h"
#include "filesys.h"
#include "lump.h"
#include "pad.h"
#include "scene.h"
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
    
    readElfData();
    fixupFnt((Font *)menuFont);
    
    initGs();
    initPads();
    initCD();
    traceln("cd init done");

    initTextureAllocStuff();
    initDMA();
    traceln("DMA init done");

    getLmp("fx.lmp");
    //getLmp("mouth.lmp");
    setLmpGeneration(100);

    initDisplay();
    traceln("about to run scene");

    runScene(&showLanguageMenu);

    freeElfData();
    traceln("bye");

    while (true) {
    }
    return 0;
}
