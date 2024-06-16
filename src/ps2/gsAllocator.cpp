#include "gsAllocator.h"
#include "texture.h"
#include "state.h"

#include "trace.h"

#include <cstring>

const int GS_ALLOC_HEAP_SIZE = 178;
GSAllocInfo gsAllocHeap[GS_ALLOC_HEAP_SIZE];
GSAllocInfo* firstFreeGSAlloc;
GSAllocInfo* gsAllocListHead;
GSAllocInfo* gsAllocListTail;

GSAllocInfo* comittedTexturesGSInfo[8];

void uncommitTex(int idx)
{
    if(idx > 0 && idx < 8){
        if (comittedTexturesGSInfo[idx]) {
            comittedTexturesGSInfo[idx]->commitCount -= 1;
            comittedTexturesGSInfo[idx] = nullptr;
        }
    }
}

void commitTex(int idx, TextureHeader* tex)
{
    if(idx > 0 && idx < 8){
        if (tex->gsAllocInfo) {
            if (comittedTexturesGSInfo[idx]) {
                comittedTexturesGSInfo[idx]->commitCount -= 1;
            }
            comittedTexturesGSInfo[idx] = tex->gsAllocInfo;

            tex->gsAllocInfo->commitCount += 1;
        }
    }
}

const int gsInitAlloc_initSize = 0xCF0;
const int gsInitAlloc_initDbp = 0x3310; // 13072. Why not 0x3200?

void initTextureAllocStuff()
{
    for (int i = 0; i < 8; ++i) {
        comittedTexturesGSInfo[i] = nullptr;
    }

    memset(gsAllocHeap, 0, GS_ALLOC_HEAP_SIZE * sizeof(gsAllocHeap[0]));

    // link up the entries in the heap
    for (int i = 0; i < GS_ALLOC_HEAP_SIZE - 1; ++i) {
        gsAllocHeap[i].next = &gsAllocHeap[i + 1];
    }

    gsAllocListHead = &gsAllocHeap[0];
    gsAllocListTail = &gsAllocHeap[1];
    GSAllocInfo* midAlloc = &gsAllocHeap[2];
    firstFreeGSAlloc = &gsAllocHeap[3];

    gsAllocHeap[GS_ALLOC_HEAP_SIZE - 1].next = nullptr;

    // link up gsAllocListHead -> midAlloc -> gsAllocListTail
    gsAllocListHead->prev = nullptr;
    gsAllocListHead->next = midAlloc;

    midAlloc->prev = gsAllocListHead;
    midAlloc->next = gsAllocListTail;

    gsAllocListTail->prev = midAlloc;
    gsAllocListTail->next = nullptr;

    gsAllocListHead->size = 0;
    gsAllocListHead->dbp = 0;
    gsAllocListHead->allocTex = nullptr;
    gsAllocListHead->frameCountPlusOne = -1;

    gsAllocListTail->size = 0;
    gsAllocListTail->dbp = 0;
    gsAllocListTail->allocTex = nullptr;
    gsAllocListTail->frameCountPlusOne = -1;

    midAlloc->frameCountPlusOne = -1;
    midAlloc->size = gsInitAlloc_initSize;
    midAlloc->dbp = gsInitAlloc_initDbp;
    midAlloc->allocTex = nullptr;

    /*

    DAT_ram_00325f1c = DAT_ram_00324630 + 0x70;

    piVar7 = INT_ARRAY_ram_00453f34 + 0x4ff;
    iVar3 = 0x4ff;
    do {
        *piVar7 = 0;
        iVar3 += -1;
        piVar7 = piVar7 + -2;
    } while (-1 < iVar3);
    DAT_ram_0032463c = 0;
    pbVar2 = texCellLookupArray + 0x1fff;
    iVar3 = 0x1fff;
    do {
        *pbVar2 = 0xff;
        iVar3 += -1;
        pbVar2 = pbVar2 + -1;
    } while (-1 < iVar3);
    iVar4 = 0;
    iVar3 = 0;
    do {
        *(undefined2*)((int)SHORT_ARRAY_ram_00455678 + iVar3) = 0xffff;
        puVar5 = &TexThing_ARRAY_ram_00455338[iVar4].field_0x3f;
        iVar4 += 1;
        iVar3 = 0x3f;
        do {
            *puVar5 = 0xff;
            iVar3 += -1;
            puVar5 = puVar5 + -1;
        } while (-1 < iVar3);
        iVar3 = iVar4 * 2;
    } while (iVar4 < 9);
    iVar3 = 0xff;
    pbVar2 = BYTE_ARRAY_ram_00455578 + 0xff;
    do {
        *pbVar2 = 0;
        iVar3 += -1;
        pbVar2 = pbVar2 + -1;
    } while (-1 < iVar3);
    memset((byte*)levelTextureChunks, 0, 0x24);
    buildDecodeTable();
    DAT_ram_00324629 = 1;

    */
    return;
}

int smallestTexGSAllocated = 10000000;

void resetTextureAlloc()
{
    /*
    if ((BYTE_ram_00324628 != 0) && ((char)BYTE_ram_00324628 < '\n')) {
      BYTE_ram_00324628 = BYTE_ram_00324628 + 1;
    }
    */

    smallestTexGSAllocated = 10000000;

    for (int i = 0; i < 8; ++i) {
        if (comittedTexturesGSInfo[i] != nullptr) {
            comittedTexturesGSInfo[i]->commitCount -= 1;
            comittedTexturesGSInfo[i] = nullptr;
        }
    }

    GSAllocInfo* curInfo = gsAllocListHead->next;
    GSAllocInfo* nextInfo = curInfo->next;
    while (nextInfo != nullptr && frameCount <= curInfo->frameCountPlusOne) {
        curInfo = nextInfo;
        nextInfo = curInfo->next;
    }

    if (curInfo->allocTex != nullptr) {
        *curInfo->allocTex = nullptr;
        curInfo->allocTex = nullptr;
    }

    while (nextInfo != nullptr && nextInfo->next) {
        while (nextInfo->frameCountPlusOne < frameCount) {
            if (nextInfo->commitCount != 0) {
                for (int i = 0; i < 8; ++i) {
                    // How can this be, we cleared them all at the top.
                    if (comittedTexturesGSInfo[i] == nextInfo) {
                        comittedTexturesGSInfo[i]->commitCount -= 1;
                        comittedTexturesGSInfo[i] = nullptr;
                    }
                }
            }

            GSAllocInfo* infoToFree = nextInfo;

            if (infoToFree->allocTex) {
                *infoToFree->allocTex = nullptr;
            }

            // Remove infoToFree from the list
            nextInfo = infoToFree->next;
            nextInfo->prev = curInfo;
            curInfo->next = nextInfo;

            curInfo->size += infoToFree->size;

            // prepend the removed node to the free list
            infoToFree->next = firstFreeGSAlloc;
            firstFreeGSAlloc = infoToFree;
            
            if (nextInfo->next == nullptr) {
                return;
            }
        }

        curInfo = nextInfo;
        nextInfo = nextInfo->next;

        while (nextInfo != nullptr && frameCount <= curInfo->frameCountPlusOne) {
            curInfo = nextInfo;
            nextInfo = nextInfo->next;
        }
        if (nextInfo) {       
            if (curInfo->allocTex) {
                *curInfo->allocTex = nullptr;
                curInfo->allocTex = nullptr;
            }
        }
    }

    return;
}

void gsAllocateTex(TextureHeader* tex)
{
    int requiredGSMem = tex->requiredGSMem;
    if (tex->requiredGSMem == 0) {
        requiredGSMem = 1;
    }
    traceln("requiredGSMem = 0x%x, smallestTexGSAllocated = 0x%x", requiredGSMem, smallestTexGSAllocated);
    if (requiredGSMem <= smallestTexGSAllocated) {
        GSAllocInfo* pGSInfo = gsAllocListHead->next;
        //traceln("pGSInfo = %p", pGSInfo);
        //traceln("pGSInfo->next = %p", pGSInfo->next);
        if (pGSInfo->next) {
            int infoFc = pGSInfo->frameCountPlusOne;
            GSAllocInfo* nextGSInfo = pGSInfo->next;
            //traceln("frameCount = %d, pGSInfo->frameCountPlusOne=%d", frameCount, pGSInfo->frameCountPlusOne);
            //traceln("pGSInfo->size = %p", pGSInfo->size);
            //traceln("pGSInfo->commitCount = %p", pGSInfo->commitCount);
            while (((frameCount <= infoFc || (pGSInfo->size < requiredGSMem)) || (pGSInfo->commitCount != 0))) {
                if (nextGSInfo->next == nullptr) {
                    nextGSInfo = nullptr;           // got to the end, skip setting up a new GSInfo
                    break;
                }
                infoFc = nextGSInfo->frameCountPlusOne;
                pGSInfo = nextGSInfo;
                nextGSInfo = nextGSInfo->next;
                //traceln("Step to next GSInfo");
            }
            //traceln("nextGSInfo = %p", nextGSInfo);
            if (nextGSInfo != nullptr) {
                pGSInfo->frameCountPlusOne = frameCount + 1;
                GSAllocInfo* newGSInfo = firstFreeGSAlloc;
                if ((requiredGSMem + 4 < (int)pGSInfo->size) && (newGSInfo != nullptr)) {
                    //traceln("setting up new GSInfo");
                    firstFreeGSAlloc = firstFreeGSAlloc->next;

                    newGSInfo->size = pGSInfo->size - requiredGSMem;
                    pGSInfo->size = requiredGSMem;

                    newGSInfo->prev = pGSInfo;
                    newGSInfo->next = pGSInfo->next;

                    newGSInfo->dbp = pGSInfo->dbp + requiredGSMem;

                    if (pGSInfo->next) {
                        pGSInfo->next->prev = newGSInfo;
                    }
                    pGSInfo->next = newGSInfo;

                    newGSInfo->allocTex = nullptr;
                }
                tex->gsAllocInfo = pGSInfo;
                if (pGSInfo->allocTex) {
                    *pGSInfo->allocTex = nullptr;
                }
                pGSInfo->allocTex = &tex->gsAllocInfo;

                traceln("allocted dbp = 0x%x, size=0x%x", pGSInfo->dbp, pGSInfo->size);
                traceln("frameCount = %d, pGSInfo->frameCountPlusOne=%d", frameCount, pGSInfo->frameCountPlusOne);
                traceln("pGSInfo->commitCount = %p", pGSInfo->commitCount);

                return;
            }
        }

        if (requiredGSMem < smallestTexGSAllocated) {
            smallestTexGSAllocated = requiredGSMem;
        }
    }

    GSAllocInfo* pGSInfo = gsAllocListTail->prev->prev;
    traceln("Bigger than previous allocs pGSInfo = %p", pGSInfo);
    if (pGSInfo != nullptr) {
        GSAllocInfo* nextGSInfo = gsAllocListTail->prev;
        GSAllocInfo** ppGVar3;
        do {
            int lVar4 = 0;
            bool bVar2 = requiredGSMem != 0;
            GSAllocInfo* pGVar7 = nextGSInfo;
            if (!bVar2) {
            LAB_ram_001409a8:
                if (nextGSInfo->allocTex != nullptr) {
                    *nextGSInfo->allocTex = nullptr;
                }
                int iVar6 = frameCount;
                tex->gsAllocInfo = nextGSInfo;
                nextGSInfo->allocTex = &tex->gsAllocInfo;
                nextGSInfo->frameCountPlusOne = iVar6 + 1;
                pGSInfo = nextGSInfo->next;
                if (pGSInfo == pGVar7) {
                    pGVar7->prev = nextGSInfo;
                } else {
                    GSAllocInfo** ppGVar3 = pGSInfo->allocTex;
                    while (true) {
                        GSAllocInfo* pGVar5 = pGSInfo;
                        if (ppGVar3 != nullptr) {
                            *ppGVar3 = nullptr;
                        }
                        if (pGVar5->commitCount != 0) {
                            iVar6 = 9;
                            ppGVar3 = comittedTexturesGSInfo;
                            do {
                                iVar6 += -1;
                                if (*ppGVar3 == pGVar5) {
                                    *ppGVar3 = nullptr;
                                    pGVar5->commitCount = pGVar5->commitCount - 1;
                                }
                                ppGVar3 = ppGVar3 + 1;
                            } while (-1 < iVar6);
                        }
                        pGSInfo = pGVar5->next;
                        pGVar5->next = firstFreeGSAlloc;
                        firstFreeGSAlloc = pGVar5;
                        if (pGSInfo == pGVar7) {
                            break;
                        }
                        ppGVar3 = pGSInfo->allocTex;
                    }
                    pGVar7->prev = nextGSInfo;
                }
                nextGSInfo->next = pGVar7;
                pGSInfo = firstFreeGSAlloc;
                if ((requiredGSMem + 4 < lVar4) && (firstFreeGSAlloc != nullptr)) {
                    pGVar7 = firstFreeGSAlloc->next;
                    nextGSInfo->size = requiredGSMem;
                    firstFreeGSAlloc->size = lVar4 - requiredGSMem;
                    firstFreeGSAlloc = pGVar7;

                    pGVar7 = nextGSInfo->next;
                    pGSInfo->prev = nextGSInfo;
                    pGSInfo->next = pGVar7;
                    int prevDbp = nextGSInfo->dbp;
                    pGVar7 = nextGSInfo->next;
                    pGSInfo->allocTex = nullptr;
                    pGVar7->prev = pGSInfo;
                    nextGSInfo->next = pGSInfo;
                    pGSInfo->dbp = prevDbp + requiredGSMem;
                    return;
                }
                nextGSInfo->size = (short)lVar4;
                return;
            }
            if ((nextGSInfo->next != nullptr) && (nextGSInfo->commitCount == 0)) {
                int prevDbp = nextGSInfo->size;
                pGVar7 = nextGSInfo->next;
                while (true) {
                    lVar4 = lVar4 + prevDbp;
                    bVar2 = lVar4 < requiredGSMem;
                    if (!bVar2) {
                        goto LAB_ram_001409a8;
                    }
                    if ((pGVar7->next == nullptr) || (pGVar7->commitCount != 0)) {
                        break;
                    }
                    prevDbp = pGVar7->size;
                    pGVar7 = pGVar7->next;
                }
            }
            if (!bVar2) {
                goto LAB_ram_001409a8;
            }
            ppGVar3 = &pGSInfo->prev;
            nextGSInfo = pGSInfo;
            pGSInfo = *ppGVar3;
        } while (*ppGVar3 != nullptr);
    }
    return;
}
