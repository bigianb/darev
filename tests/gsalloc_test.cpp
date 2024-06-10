
#include "runtests.h"
#include "../src/ps2/gsAllocator.h"
#include "../src/ps2/state.h"
#include "../src/ps2/texture.h"

bool test_SimpleAlloc()
{
    BEGIN_TEST
    initTextureAllocStuff();

    frameCount = 1;

    TextureHeader* tex1 = new TextureHeader();
    tex1->gsAllocInfo = nullptr;
    tex1->requiredGSMem = 100;
    gsAllocateTex(tex1);

    expectEqual(tex1->gsAllocInfo->size ,100);
    expectEqual(tex1->gsAllocInfo->dbp, 0x3310);
    expectEqual(tex1->gsAllocInfo->frameCountPlusOne, 2);
    expectEqual(tex1->gsAllocInfo->commitCount, 0);

    commitTex(3, tex1);
    expectEqual(tex1->gsAllocInfo->commitCount, 1);

    // Next frame, the commit count is reduced but it allocation is kept.
	// Something else can use this space if needed or we can just commit it again if we need it.
    ++frameCount;
    resetTextureAlloc();
    expectEqual(tex1->gsAllocInfo->commitCount, 0);
    
	commitTex(3, tex1);
	expectEqual(tex1->gsAllocInfo->commitCount, 1);

    END_TEST
}

bool runGSAllocTests()
{
    bool okay = true;
    RUNTEST(SimpleAlloc);
    return okay;
}

