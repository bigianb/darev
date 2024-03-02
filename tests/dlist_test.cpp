#include "runtests.h"

#include "../src/ps2/dlist.h"
#include "../src/ps2/state.h"
#include "../src/ps2/texture.h"


// Private things we used from dlist
extern int firstFreeDlistNodeIndex;
extern int activeDlistBank; // 0 or 1
extern DlistNode* dlistHeads[2][8];
extern int dmaInProgress;
extern int activeDmaBank;
extern int nullNodeCounter;
extern int GSFinishCounter;
extern int curDmaSlot;
extern int zeroOrTwo;
extern u32 currentActiveChannels;
extern DlistNode* curDmaNode;
extern TextureHeader* textureBeingTransferred;

bool test_QueueDma()
{
    BEGIN_TEST
    initDMA();

    expectEqual(activeDlistBank, 1);
    expectEqual(firstFreeDlistNodeIndex, 8);

    u64 dmaBuffer[2];

    // expect a node to be allocated and be appended to slot 5 list
    DlistNode* node = queueDMA(dmaBuffer, 5, nullptr, nullptr, false);
    auto* slot5Head = dlistHeads[activeDlistBank][5];
    
    expectEqualPtr(slot5Head->next, node);
    expectEqual(firstFreeDlistNodeIndex, 9);
    
    // Append another. Expect this to be appended after the first one.
    // *note* actually gets prepended because of no texture data.
    // Append means to append to the list of vifs which share the same texture.
    DlistNode* node2 = queueDMA(dmaBuffer, 5, nullptr, nullptr, false);
    slot5Head = dlistHeads[activeDlistBank][5];
    
    expectEqualPtr(slot5Head->next, node2);
    expectEqualPtr(node2->next, node);
    expectEqual(firstFreeDlistNodeIndex, 10);
    
    END_TEST
}

struct TexData
{
    TextureHeader header;
    u8 data[0x600];
};

TexData* generateTexture(int w, int h, int gsMem, int qwc, int flags)
{
    TexData* tex = new TexData();
    memset(tex, 0, sizeof(TexData));
    tex->header.width = w;
    tex->header.height = h;
    tex->header.requiredGSMem = gsMem;
    tex->header.qwc = qwc;
    tex->header.flags = flags;

    tex->header.gif_madr_val = tex->data;
    return tex;
}

bool test_QueueTexture()
{
	BEGIN_TEST
	initDMA();

    frameCount = 1;

	expectEqual(activeDlistBank, 1);
	expectEqual(firstFreeDlistNodeIndex, 8);

	u64 dmaBuffer[2];
    TexData* tex1 = generateTexture(0x180, 0x80, 0xAF, 0x08CD, 0x14);

    TexData* tex2 = generateTexture(0x180, 0x80, 0xAF, 0x08CD, 0x14);

	// expect a node to be allocated and be appended to slot 5 list
	DlistNode* node = queueDMA(dmaBuffer, 5, &tex1->header, nullptr, false);
	auto* slot5Head = dlistHeads[activeDlistBank][5];
	
    // slot5 -> node
	expectEqualPtr(slot5Head->next, node);
	expectEqual(firstFreeDlistNodeIndex, 9);
	
	// Append another. Expect this to be appended after the first one because it shares a texture.
	DlistNode* node2 = queueDMA(dmaBuffer, 5, &tex1->header, nullptr, false);
	slot5Head = dlistHeads[activeDlistBank][5];
	
    // slot5 -> node(tex1) -> node2(tex1)
	expectEqualPtr(slot5Head->next, node);
	expectEqualPtr(node->next, node2);
	expectEqual(firstFreeDlistNodeIndex, 10);
	
    // Prepend a different texture.
	DlistNode* node3 = queueDMA(dmaBuffer, 5, &tex2->header, nullptr, true);
	slot5Head = dlistHeads[activeDlistBank][5];
	
    // expect slot5 -> node3(tex2) -> node(tex1) -> node2(tex1)
	expectEqualPtr(slot5Head->next, node3);
	expectEqualPtr(node3->next, node);
    expectEqualPtr(node->next, node2);
	expectEqual(firstFreeDlistNodeIndex, 11);

    // Append another node with tex2 and expect it to be inserted at the correct place.
    DlistNode* node4 = queueDMA(dmaBuffer, 5, &tex2->header, nullptr, false);
	slot5Head = dlistHeads[activeDlistBank][5];
	
    // expect slot5 -> node3(tex2) -> node4(tex2) -> node(tex1) -> node2(tex1)
	expectEqualPtr(slot5Head->next, node3);
	expectEqualPtr(node3->next, node4);
    expectEqualPtr(node4->next, node);
    expectEqualPtr(node->next, node2);
	expectEqual(firstFreeDlistNodeIndex, 12);

    delete tex1;
    delete tex2;

	END_TEST
}

#define VIF1_ACTIVE 2
#define GIF_ACTIVE 4

#define VIF1_CHANNEL 1
#define GIF_CHANNEL 2
#define GS_FINISH_CHANNEL -2

extern int dmaHandler(int channel);

bool test_DmaHandler()
{
	BEGIN_TEST

    initTextureAllocStuff();
    initDMA();

    frameCount = 1;

	u64 dmaBuffer[2];
    TexData* tex1 = generateTexture(0x180, 0x80, 0xAF, 0x08CD, 0x14);

    DlistNode* node = queueDMA(dmaBuffer, 5, &tex1->header, nullptr, false);
    
    expectEqual(activeDmaBank, 0);
    expectEqual(dmaInProgress, 0);
    
    // kickoff will step forward to the first slot in use which is 5 in this case,
    // allocate a GS location for tex1 and start the GIF transfer.
    kickoffDMA();
    expectEqual(activeDmaBank, 1);
    expectEqual(dmaInProgress, 1);
    expectEqual(curDmaSlot, 5);
    // texture transfer has started but need to wait until it has finished before VIF
    expectEqualPtr(curDmaNode, node);
    expectEqualPtr(textureBeingTransferred, tex1);
    expectEqual(nullNodeCounter, 0);
    expectEqual(GSFinishCounter, 0);
    
    expectEqual(currentActiveChannels, GIF_ACTIVE);
	expectEqual(zeroOrTwo, 0);
	
	// now, signal the GIF finished
	dmaHandler(GIF_CHANNEL);
	expectEqual(currentActiveChannels, VIF1_ACTIVE);
	expectEqual(nullNodeCounter, 0);
	expectEqual(zeroOrTwo, 0);
	expectEqualPtr(curDmaNode, nullptr);
	
	// VIF is done and is the last one in this slot so GS finish
	dmaHandler(VIF1_CHANNEL);
	expectEqual(zeroOrTwo, VIF1_ACTIVE);
	expectEqual(currentActiveChannels, VIF1_ACTIVE);
	expectEqual(dmaInProgress, 1);
	expectEqual(curDmaSlot, 5);
	expectEqual(nullNodeCounter, 1);
	
	// not sure which is first GS or VIF
	dmaHandler(VIF1_CHANNEL);
	expectEqual(nullNodeCounter, 1);
	expectEqual(dmaInProgress, 1);
	
	dmaHandler(GS_FINISH_CHANNEL);
	expectEqual(nullNodeCounter, 0);
	expectEqual(zeroOrTwo, 2);
	expectEqual(currentActiveChannels, 0);
	expectEqual(dmaInProgress, 0);
	expectEqual(curDmaSlot, 8);
	
    delete tex1;

    END_TEST
}

bool runDlistTests()
{
    bool okay = true;
    RUNTEST(QueueDma)
    RUNTEST(QueueTexture)
    RUNTEST(DmaHandler)

    return okay;
}
