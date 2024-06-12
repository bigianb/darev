
#include <kernel.h>
#include <ee_regs.h>
#include <tamtypes.h>
#include <libgs.h>

#include "trace.h"

// for memset
#include <cstdio>


// in hw dir but don't specify that here so the tests can override it
#include "dma_setters.h"
#include "ptr_manip.h"

#include "dlist.h"
#include "gsAllocator.h"

// for frameCount
#include "state.h"
#include "texture.h"

#if 1
#define DLIST_TRACELN( format, ... )
#else
#define DLIST_TRACELN traceln
#endif

int dmaSemaId;
int vif0HandlerId;
int vif1HandlerId;
int gifHandlerId;

int dmaInProgress = 0;
int zeroOrTwo = 0;
int waitingOnGSFinish = 0;
int GSFinishCounter = 0;

u32 currentActiveChannels = 0;
int curDmaSlot = 0;

int dlistBankBeingUploaded = 0;
// dlist bank being written to
int activeDlistBank = 1; // 0 or 1

DlistNode* curDmaNode = nullptr;

int firstFreeDlistNodeIndex;

const int DLIST_HEAP_SIZE = 1024;
DlistNode dlistHeap[2][DLIST_HEAP_SIZE];

// Head of the linked list for each bank and slot
DlistNode* dlistHeads[2][8];
DlistNode* dlistHeads2[2][8];

int nullNodeCounter = 0;
int dmaSemaSignalled = 0;
// In scene.cpp maybe should be here.
extern int sceneFrameNum;

TextureHeader* textureBeingTransferred = nullptr;

// dma handler called with this to start a new transfer
#define START_CHANNEL -1
// dma handler called with this when GS triggers a FINISH
#define GS_FINISH_CHANNEL -2
#define VIF0_CHANNEL 0
#define VIF1_CHANNEL 1
#define GIF_CHANNEL 2

#define VIF1_ACTIVE 2
#define GIF_ACTIVE 4

// VIF DMA data is 128 bit aligned.
u32 flushAndFinishDma[12] __attribute__((aligned(16))) = {
    0x70000002, 0,

    0x11000000, // FLUSH
    0x50000002, // DIRECT, 2x16 bytes

    0x8001, 0x10000000,    // GIF EOP NLOOP=1, NREG=1, REGS=A+D
    0x0e, 0,

    0, 2,
    0x61, 0 // GS FINISH = 2
};

void flushAndFinish()
{
    DLIST_TRACELN("flushAndFinish, taddr=0x%x", &flushAndFinishDma[0]);
    setVif1_tadr(&flushAndFinishDma[0]);

    currentActiveChannels |= VIF1_ACTIVE;
    // from memory, chain mode, no ASP, DMA tag transferred, disable IRQ, start DMA.
    setVif1_CHCR(0x145);
}

// 0 or 1, used to define texture registration
int texId_0_or_1 = 0;

int dmaHandler(int channel)
{
    DLIST_TRACELN("dmaHandler(%d), currentActiveChannels = 0x%x", channel, currentActiveChannels);

    if ((dmaInProgress == 0) && (channel != START_CHANNEL)) {
        DLIST_TRACELN("dmaHandler - ignore as idle");
        // idle and this is not an instruction to start
        return 1;
    }

    if (channel == START_CHANNEL) {
        DLIST_TRACELN("dmaHandler - start");
        // kickoff new transfer
        curDmaSlot = 0;

        dlistBankBeingUploaded ^= 1;
        dmaInProgress = 1;

        currentActiveChannels = 0;
        curDmaNode = dlistHeads[dlistBankBeingUploaded][0]->next;
        zeroOrTwo = (curDmaNode == nullptr) ? VIF1_ACTIVE : 0;

        dmaSemaSignalled = 0;

        GSFinishCounter = 0;
        nullNodeCounter = 0;
        textureBeingTransferred = nullptr;
    }

    if (channel == GS_FINISH_CHANNEL) {
        DLIST_TRACELN("dmaHandler - GS Finish");
        if ((zeroOrTwo & 2) == 0) {
            uncommitTex(0);
            uncommitTex(1);
            waitingOnGSFinish = 0;
        } else {
            ++GSFinishCounter;
        }
        GS_SET_CSR_finish_evnt(1);
    }

    //DLIST_TRACELN("a. zeroOrTwo = %d", zeroOrTwo);
    // doing VIF and we've finished the ones for this texture
    if ((zeroOrTwo == VIF1_ACTIVE) && (GSFinishCounter == nullNodeCounter)) {
        while (zeroOrTwo == VIF1_ACTIVE && curDmaSlot < 8) {
            curDmaSlot += 1;

            if (curDmaSlot > 7) {
                curDmaNode = nullptr;
            } else {
                curDmaNode = dlistHeads[dlistBankBeingUploaded][curDmaSlot]->next;
            }
            zeroOrTwo = (curDmaNode == nullptr) ? VIF1_ACTIVE : 0;

            nullNodeCounter = 0;
            GSFinishCounter = 0;
        };
    }

    //DLIST_TRACELN("b. zeroOrTwo = %d", zeroOrTwo);

    // mark the channel that triggered this interrupt inactive.
    currentActiveChannels &= ~(1 << (channel & 0x1f));

    if (channel == GIF_CHANNEL) {
        textureBeingTransferred = nullptr;
    }

    if (waitingOnGSFinish != 0) {
        //DLIST_TRACELN("exit 1 as waitingOnGSFinish");
        ExitHandler();
        return 1;
    }

    // If GIF is inactive, think about starting a GIF transfer.
    if ((currentActiveChannels & GIF_ACTIVE) == 0) {
        //DLIST_TRACELN("consider GIF transfer");
        for (int i = 2; i < 5; ++i) {
            // de-register textures 2, 3 and 4
            uncommitTex(i);
        }

        int texId = 2;

        DlistNode* pNode = curDmaNode;
        while (pNode != nullptr && texId >= 0) {
            TextureHeader* pTexData = pNode->texData;
            //DLIST_TRACELN("pTexData = 0x%x", pTexData);
            //DLIST_TRACELN("curDmaSlot = 0x%x", curDmaSlot);
            if (pTexData != nullptr) {
                DLIST_TRACELN("pTexData->gsAllocInfo = 0x%x", pTexData->gsAllocInfo);
                if (pTexData->gsAllocInfo == nullptr) {
                    gsAllocateTex(pTexData);
                    if (pTexData->gsAllocInfo == nullptr) {
                        DLIST_TRACELN("GS Allocation failed");
                        if ((texId == 2) && ((currentActiveChannels & VIF1_ACTIVE) == 0)) {
                            waitingOnGSFinish = 1;
                            flushAndFinish();
                        }
                    } else {
                        DLIST_TRACELN("allocted dbp = 0x%x, size=0x%x", pTexData->gsAllocInfo->dbp, pTexData->gsAllocInfo->size);
                        DLIST_TRACELN("frameCount = %d, pGSInfo->frameCountPlusOne=%d", frameCount, pTexData->gsAllocInfo->frameCountPlusOne);
                        DLIST_TRACELN("pGSInfo->commitCount = %p", pTexData->gsAllocInfo->commitCount);

                        textureBeingTransferred = pTexData;
                        commitTex(texId + 2, pTexData);
                        if (pTexData->gif_tadr_val == nullptr) {
                            // GS memory location for texture
                            u16 dbp = pTexData->gsAllocInfo->dbp;

                            u8* pGifUcab = (u8*)UNCACHED_SEG(pTexData->gif_madr_val);

                            //DLIST_TRACELN("Transfer dbp=0x%x, madr ucab = %p", dbp, pGifUcab);

                            // write DBP in first GIFTAG (palette)
                            *(u16*)(pGifUcab + 0x14) = dbp;
                            if ((pTexData->flags & 4) == 0) {
                                if ((pTexData->flags & 0x2000) != 0) {
                                    *(u16*)(pGifUcab + 0xb4) = dbp + 4;
                                }
                            } else {
                                // 8 bit palette, write image pointer
                                *(u16*)(pGifUcab + 0x474) = dbp + 4;
                            }

                            setGif_madr(pTexData->gif_madr_val);
                            setGif_qwc(pTexData->qwc);

                            /* from memory, normal, no address, transfer DMA tag, disable IRQ bit, start DMA */
                            setGif_CHCR(0x141);
                        } else {
                            // GS memory location for texture
                            u16 dbp = pTexData->gsAllocInfo->dbp;
                            setGif_tadr(pTexData->gif_tadr_val);
                            u8* pGifUcab = (u8*)UNCACHED_SEG(pTexData->gif_tadr_val);
                            *(short*)(pGifUcab + 0x24) = dbp;
                            *(short*)(pGifUcab + 0x484) = dbp + 4;

                            // from memory, chain, no address, no tag transfer, disable IRQ bit, start DMA.
                            setGif_CHCR(0x105);
                        }
                        currentActiveChannels |= GIF_ACTIVE;
                    }
                    texId = -1; // done
                } else {
                    // register texture
                    DLIST_TRACELN("already allocated, just flag as used");
                    commitTex(texId + 2, pTexData);
                }
            }
            --texId;
            pNode = pNode->next;
        }
    }

    // if VIF1 is inactive, think about starting a VIF1 transfer
    if (((zeroOrTwo | currentActiveChannels) & VIF1_ACTIVE) == 0) {
        DLIST_TRACELN("consider VIF transfer");
        DLIST_TRACELN("  zeroOrTwo = 0x%x", zeroOrTwo);
        DLIST_TRACELN("  currentActiveChannels = 0x%x", currentActiveChannels);
        DLIST_TRACELN("  curDmaNode = %p", curDmaNode);

        // VIF1 inactive
        if (curDmaNode == nullptr) {
            nullNodeCounter += 1;

            zeroOrTwo |= VIF1_ACTIVE;

            flushAndFinish();
        } else {
            TextureHeader* pTexData = curDmaNode->texData;
            bool doVif = true;

            if (pTexData != nullptr) {

                if (pTexData->gsAllocInfo == nullptr || pTexData == textureBeingTransferred) {
                    // Can't start the VIF transfer if we're waiting for the texture to be transferred or if there
                    // is no space yet to hold the texture
                    DLIST_TRACELN("Transferring Tex, hold off on VIF");
                    doVif = false;
                } else {
                    DLIST_TRACELN("Do VIF transfer");
                    // 0 or 1
                    commitTex(texId_0_or_1, pTexData);

                    u8* vifAddr = (u8*)UNCACHED_SEG(curDmaNode->pCleanDmaData); // VIF dma data
                    u64 tex0_1_val;
                    const u16 dbp = pTexData->gsAllocInfo->dbp;
                    u64 tbw = pTexData->tbw;

                    if ((curDmaNode->flags & 0x14) == 0) {
                        // 8 bit palettised
                        u64 uVar7 = (u64)dbp << 0x25 | tbw << 0xe;
                        // set tbp0
                        tex0_1_val = (u64)(dbp + 4) | 0x4000000000000000;
                        tex0_1_val |= uVar7;
                    } else {
                        if ((curDmaNode->flags & 4) == 0) {
                            int iVar9 = (dbp + 0x1f) >> 5;
                            // TEX1_1
                            *(short*)(vifAddr + 0x40) = (short)iVar9;
                            u64 uVar7 = iVar9 << 5;
                            tex0_1_val = tbw << 0xe;
                            tex0_1_val |= uVar7;
                        } else {
                            // TEX1_1
                            *(u64*)(vifAddr + 0x40) |= (u64)dbp << 0x20;
                            tex0_1_val = dbp | tbw << 0xe;
                        }
                    }

                    // TEX0_1
                    *(u64*)(vifAddr + 0x30) = tex0_1_val;

                    texId_0_or_1 ^= 1;
                }
            }
            if (doVif) {
                DLIST_TRACELN("setting vif tadr to 0x%x", curDmaNode->pCleanDmaData);
                setVif1_tadr(curDmaNode->pCleanDmaData);

                curDmaNode = curDmaNode->next;
                /* from memory, chain mode, no ASP, DMA tag transferred, disable IRQ, start DMA */
                setVif1_CHCR(0x145);

                currentActiveChannels |= VIF1_ACTIVE;
            }
        }
    }

    if ((curDmaSlot > 7) && (currentActiveChannels == 0) && (dmaSemaSignalled == 0)) {
        /* done all slots, no channels active and DMA not yet signalled. */
        dmaSemaSignalled = 1;
        dmaInProgress = 0;
        //DLIST_TRACELN("signal DMA sema id=%d", dmaSemaId);
        iSignalSema(dmaSemaId);
    }

    //DLIST_TRACELN("handler done");
    ExitHandler();
    return 1;
}

void waitDMASema()
{
    //DLIST_TRACELN("wait DMA sema id=%d", dmaSemaId);
    WaitSema(dmaSemaId);
}

int GsIntcHandler(int cause)
{
    //DLIST_TRACELN("GsIntcHandler(%x)", cause);
    dmaHandler(GS_FINISH_CHANNEL);
    ExitHandler();
    return 1;
}

void initDListHeads()
{
    firstFreeDlistNodeIndex = 0;

    for (int idx = 0; idx < 8; ++idx) {
        DlistNode* pNode = &dlistHeap[activeDlistBank][firstFreeDlistNodeIndex];
        pNode->next = nullptr;

        dlistHeads[activeDlistBank][idx] = pNode;
        dlistHeads2[activeDlistBank][idx] = pNode;

        ++firstFreeDlistNodeIndex;
    }
}

void kickoffDMA()
{
    // set-up the next bank. Effectively swap buffers.
    activeDlistBank ^= 1;
    initDListHeads();

    DI();
    dmaHandler(START_CHANNEL);
    EI();
    //DLIST_TRACELN("kickoff done");
}

void initDMA()
{
    // VIFs transfer tag, gif does not.
#if defined(__mips__)
    *(vu32*)0x10008000 = *(vu32*)0x10008000 | 0x40;       // VIF0_DCHR
    *(vu32*)0x10009000 = *(vu32*)0x10009000 | 0x40;       // VIF1_DCHR
    *(vu32*)0x1000A000 = *(vu32*)0x1000A000 & 0xffffffbf; // GIF_DCHR
    FlushCache(0);
#endif

    activeDlistBank = 1;
    dlistBankBeingUploaded = 0;

    initDListHeads();

    ee_sema_t dmaSema;

    dmaSema.init_count = 0;
    dmaSema.max_count = 100000;
    dmaSemaId = CreateSema(&dmaSema);

    vif0HandlerId = AddDmacHandler(0, dmaHandler, 0);
    EnableDmac(0);
    vif1HandlerId = AddDmacHandler(1, dmaHandler, 0);
    EnableDmac(1);
    gifHandlerId = AddDmacHandler(2, dmaHandler, 0);
    EnableDmac(2);
    AddIntcHandler(0, GsIntcHandler, 0);
    EnableIntc(0);

    //GsPutIMR(0x7D00);  // mask all but FINISH
    WR_EE_GS_IMR(0x7D00); // mask all but FINISH
    WR_EE_GIF_MODE(4);    // Intermittent transfer mode

    return;
}

DlistNode* queueDMA(u64* dma_buffer, int slot, TextureHeader* pTexData, DlistNode* headNode, bool prepend)
{
    DlistNode* pNode = &dlistHeap[activeDlistBank][firstFreeDlistNodeIndex++];

    if (((pTexData == nullptr) || ((pTexData->flags & 0x100) == 0)) || (pTexData->gif_tadr_val != nullptr)) {
        if (firstFreeDlistNodeIndex < DLIST_HEAP_SIZE) {
            pNode->flags = prepend ? 1 : 0;
            pNode->texData = pTexData;
            pNode->unkb = 0;
            pNode->pCleanDmaData = (u32*)CACHED_SEG(dma_buffer);
            pNode->unkb2 = 0;
            DlistNode* prevObjectsWithThisTexAndLayer = nullptr;
            if (pTexData) {
                DI();
                if (pTexData->gsAllocInfo != nullptr) {
                    pTexData->gsAllocInfo->frameCountPlusOne = sceneFrameNum + 1;
                }
                int frameCountCopy = frameCount;
                EI();
                if (pTexData->resetFrameNo == frameCount) {
                    /* not the first object in this frame, so look if the texture is already used in this layer */
                    prevObjectsWithThisTexAndLayer = pTexData->slotNodes[slot];
                } else {
                    /* First object this frame, so clear the layer lists and add this one. */
                    for (int i = 0; i < 8; ++i) {
                        pTexData->slotNodes[i] = nullptr;
                    }
                    pTexData->resetFrameNo = frameCountCopy;
                }
                pTexData->slotNodes[slot] = pNode;
            }
            if (headNode == nullptr) {
                if ((prevObjectsWithThisTexAndLayer == nullptr) || prepend) {
                    prevObjectsWithThisTexAndLayer = dlistHeads[activeDlistBank][slot];
                    pNode->next = prevObjectsWithThisTexAndLayer->next;
                } else {
                    while ((prevObjectsWithThisTexAndLayer->flags & 2) != 0) {
                        prevObjectsWithThisTexAndLayer = prevObjectsWithThisTexAndLayer->next;
                    }
                    pNode->next = prevObjectsWithThisTexAndLayer->next;
                }
                prevObjectsWithThisTexAndLayer->next = pNode;
            } else {
                if ((headNode->flags & 2) == 0) {
                    headNode->flags |= 2;
                } else {
                    pNode->flags |= 2;
                }
                pNode->next = headNode->next;
                headNode->next = pNode;
            }
        } else {
            pNode = nullptr;
        }
    } else {
        //DAT_ram_00325c48 = 0;
        //queueLevelTexture(pTexData);
        pNode = nullptr;
    }
    return pNode;
}

void logPathNonTerminating(const char* msg)
{
    DLIST_TRACELN(msg);
    trace("<D1_CHCR=%x:", RD_EE_D1_CHCR());
    trace("D1_TADR=%x:", RD_EE_D1_TADR());
    trace("D1_MADR=%x:", RD_EE_D1_MADR());
    trace("D1_QWC=%x>\n", RD_EE_D1_QWC());

    trace("<D2_CHCR=%x:", RD_EE_D2_CHCR());
    trace("D2_TADR=%x:", RD_EE_D2_TADR());
    trace("D2_MADR=%x:", RD_EE_D2_MADR());
    trace("D2_QWC=%x>\n", RD_EE_D2_QWC());

    trace("<VIF1_STAT=%x:", RD_EE_VIF1_STAT());
    trace("GIF_STAT=%x>\n", RD_EE_GIF_STAT());
}

int sceGsSyncPath(int mode, unsigned short timeout)
{
#if defined(__mips__)
    unsigned int reg;
    unsigned int vcnt = 0;

    const int MAXWAIT = 0x1000000;

    if (mode == 0) {
        // blocking mode
        while (RD_EE_D1_CHCR() & 0x0100) {
            if (vcnt++ > MAXWAIT) {
                logPathNonTerminating("sceGsSyncPath: DMA Ch.1 does not terminate");
                return -1;
            }
        }
        while (RD_EE_D2_CHCR() & 0x0100) {
            if (vcnt++ > MAXWAIT) {
                logPathNonTerminating("sceGsSyncPath: DMA Ch.2 does not terminate");
                return -1;
            }
        }
        while (RD_EE_VIF1_STAT() & 0x1f000003) {
            if (vcnt++ > MAXWAIT) {
                logPathNonTerminating("sceGsSyncPath: VIF1 does not terminate");
                return -1;
            }
        }
        __asm__ __volatile("cfc2 %0,$vi29" : "=r"(reg) :);
        while (reg & 0x0100) {
            if (vcnt++ > MAXWAIT) {
                logPathNonTerminating("sceGsSyncPath: VU1 does not terminate");
                return -1;
            }
            __asm__ __volatile("cfc2 %0,$vi29" : "=r"(reg) :);
        }
        while (RD_EE_GIF_STAT() & 0x0c00) {
            if (vcnt++ > MAXWAIT) {
                logPathNonTerminating("sceGsSyncPath: GIF does not terminate");
                return -1;
            }
        }
        //DLIST_TRACELN("gsSyncPath done");
        return 0;
    } else {
        // non-blocking mode
        int ret = 0;
        if (RD_EE_D1_CHCR() & 0x0100) {
            ret = ret | 0x1;
        }
        if (RD_EE_D2_CHCR() & 0x0100) {
            ret = ret | 0x2;
        }
        if (RD_EE_VIF1_STAT() & 0x1f000003) {
            ret = ret | 0x4;
        }
        __asm__ __volatile("cfc2 %0, $vi29" : "=r"(reg) :);
        if (reg & 0x0100) {
            ret = ret | 0x8;
        }
        if (RD_EE_GIF_STAT() & 0x0c00) {
            ret = ret | 0x10;
        }
        return ret;
    }
#else
    // not PS2
    return 0;
#endif
}
