#include "animDebug.h"
#include "../ps2/lump.h"

#include <string.h>
#include <cstdio>

int AnimDebug::npcSelLmpIdx = -1;

const int maxAnimDebugVifsEntries = 32;
extern AnimDebugVif animDebugVifs[maxAnimDebugVifsEntries];

const int numLumpNames = 3;

const char* allLmpNames[numLumpNames] = {
    "eldrith.lmp",
    "gargolye.lmp",
    "ratgiant.lmp"};

void AnimDebug::setup(int argc, char** argv)
{
    /*
      if (topDLight == NULL) {
        initLighting();
        piVar7 = (DLightData *)memAlloc(0x38);
        if (piVar7 != NULL) {
          piVar7->field18_0x34 = 0;
          piVar7->field1_0x1 = 0;
          puVar1 = (undefined *)((int)&(piVar7->field17_0x28).y + 3);
          uVar4 = (uint)puVar1 & 7;
          puVar5 = (ulong *)(puVar1 + -uVar4);
          *puVar5 = *puVar5 & -1L << (uVar4 + 1) * 8 | 0x42100000c2100000U >> (7 - uVar4) * 8;
          uVar4 = (uint)&piVar7->field17_0x28 & 7;
          puVar5 = (ulong *)((int)&piVar7->field17_0x28 - uVar4);
          *puVar5 = 0x42100000c2100000 << uVar4 * 8 | *puVar5 & 0xffffffffffffffffU >> (8 - uVar4) * 8;
          (piVar7->field17_0x28).z = 36.0;
          piVar7->field14_0x1c = 120.0;
          piVar7->field15_0x20 = 6.944445e-05;
          piVar7->field8_0xe = 0x40;
          piVar7->field12_0x14 = 120.0;
          piVar7->field13_0x18 = 6.944445e-05;
          piVar7->field16_0x24 = 0;
          piVar7->field2_0x2 = 0x40;
          piVar7->field3_0x4 = 0x40;
          piVar7->field4_0x6 = 0x40;
          piVar7->field5_0x8 = 0;
          piVar7->field6_0xa = 0x40;
          piVar7->field7_0xc = 0x40;
          piVar7->field9_0x10 = 0;
          piVar7->field0_0x0 = 0;
          registerLight(piVar7);
        }
        topDLight = piVar7;
        disableLight(piVar7);
        pGVar7 = (DLightData *)memAlloc(0x38);
        if (pGVar7 != NULL) {
          pGVar7->field18_0x34 = 0;
          pGVar7->field1_0x1 = 0;
          puVar1 = (undefined *)((int)&(pGVar7->field17_0x28).y + 3);
          uVar4 = (uint)puVar1 & 7;
          puVar5 = (ulong *)(puVar1 + -uVar4);
          *puVar5 = *puVar5 & -1L << (uVar4 + 1) * 8 | 0UL >> (7 - uVar4) * 8;
          uVar4 = (uint)&pGVar7->field17_0x28 & 7;
          puVar5 = (ulong *)((int)&pGVar7->field17_0x28 - uVar4);
          *puVar5 = 0L << uVar4 * 8 | *puVar5 & 0xffffffffffffffffU >> (8 - uVar4) * 8;
          (pGVar7->field17_0x28).z = -36.0;
          pGVar7->field14_0x1c = 120.0;
          pGVar7->field15_0x20 = 6.944445e-05;
          pGVar7->field8_0xe = 0x80;
          pGVar7->field12_0x14 = 120.0;
          pGVar7->field13_0x18 = 6.944445e-05;
          pGVar7->field16_0x24 = 0;
          pGVar7->field2_0x2 = 0x80;
          pGVar7->field3_0x4 = 0x80;
          pGVar7->field4_0x6 = 0x80;
          pGVar7->field5_0x8 = 0;
          pGVar7->field6_0xa = 0x80;
          pGVar7->field7_0xc = 0x80;
          pGVar7->field9_0x10 = 0;
          pGVar7->field0_0x0 = 0;
          registerLight(pGVar7);
        }
        bottomDLight = pGVar7;
        disableLight(pGVar7);
      }
    */
    /*
      maxMenuLmpEntry = 0;
      menuLevel = 0xffffffff;
      slectedLmpIdx = 0;
      selectedChangeMenuIdx = 0;
      activeChangeItems = 0;
    */
    char lmpName[16];
    if (argc == 2) {
        sprintf(lmpName, "./%s", argv[1]);
    } else {
        strcpy(lmpName, "ratgiant.lmp");
    }

    if (npcSelLmpIdx >= 0 && npcSelLmpIdx < numLumpNames) {
        strcpy(lmpName, allLmpNames[npcSelLmpIdx]);
    }

    u8* npcLmp = getLmp(lmpName);

    /*
      iVar16 = 0;
      pvVar18 = NULL;
      strcpy(AnimData::animStateData.animName, lmpName);
      iVar15 = 0;

      
      */
    int maxAnimDebugVifIdx = 0;
    LmpDirEntry* lmpDirEntry;
    int entryNum = 0;
    int prevVifLen = 0;
    // Element 0 is the largest VIF
    while (lmpDirEntry = searchLmpForNthFileWithExt(npcLmp, "vif", entryNum)) {
        if (lmpDirEntry->len > prevVifLen) {
            animDebugVifs[maxAnimDebugVifIdx].unk = 0;
            animDebugVifs[maxAnimDebugVifIdx].vifData = (VifData*)lmpDirEntry->payload;
            animDebugVifs[maxAnimDebugVifIdx].lmpDirEntry = lmpDirEntry;
            prevVifLen = lmpDirEntry->len;
        }
        ++entryNum;
    }
    maxAnimDebugVifIdx = 1;
    entryNum = 0;
    while (entryNum < maxAnimDebugVifsEntries && (lmpDirEntry = searchLmpForNthFileWithExt(npcLmp, "vif", entryNum)) != nullptr) {
        VifData* pVVar3 = (VifData*)lmpDirEntry->payload;
        if (pVVar3 != animDebugVifs[0].vifData) {
            animDebugVifs[maxAnimDebugVifIdx].unk = 0;
            animDebugVifs[maxAnimDebugVifIdx].lmpDirEntry = lmpDirEntry;
            animDebugVifs[maxAnimDebugVifIdx].vifData = pVVar3;
            ++maxAnimDebugVifIdx;
        }
        ++entryNum;
    }
    /*
    iVar16 = 0;
    do { // while true
        uVar19 = 0x320000;
                      // tex
      pLVar7 = searchLmpForNthFileWithExt(npcLmp,(char *)((int)uVar19 + 0x4700),iVar16);
      if (pLVar7 == NULL) {
        uVar13 = 0x450000;
        FUN_ram_00138ff0((npcObject1 *)&AnimData::animStateData);
        uVar14 = 0x450000;
        FUN_ram_00138ff0(&npcObject1_ram_004558e0);
        maxMenuLmpEntry = 0;

        while( true ) {
          uVar11 = 0x324708;
          pLVar7 = searchLmpForNthFileWithExt(npcLmp,"anm",maxMenuLmpEntry);
          if (pLVar7 == NULL) break;
          iVar16 = maxMenuLmpEntry * 8;
          maxMenuLmpEntry += 1;
          *(int *)(AnimData::animStateData.animName + iVar16 + 0x54) = pLVar7->payload;
          *(LmpDirEntry **)(AnimData::animStateData.animName + iVar16 + 0x50) = pLVar7;
        }

        pTVar10 = &AnimData::modelTex;
        iVar16 = 0;
        uVar12 = 0;
        do {
          uVar6 = animDebugVifs[0]._12_4_;
          iVar16 += -1;
          uVar4 = animDebugVifs[0]._12_4_ + 7 & 7;
          uVar2 = animDebugVifs[0]._12_4_ & 7;
          uVar11 = (*(long *)((animDebugVifs[0]._12_4_ + 7) - uVar4) <<
                    (7 - uVar4) * 8 | uVar11 & 0xffffffffffffffffU >> (uVar4 + 1) * 8) &
                   -1L << (8 - uVar2) * 8 |
                   *(ulong *)(animDebugVifs[0]._12_4_ - uVar2) >> uVar2 * 8;
          uVar4 = animDebugVifs[0]._12_4_ + 0xf & 7;
          uVar2 = animDebugVifs[0]._12_4_ + 8 & 7;
          uVar12 = (*(long *)((animDebugVifs[0]._12_4_ + 0xf) - uVar4) <<
                    (7 - uVar4) * 8 | uVar12 & 0xffffffffffffffffU >> (uVar4 + 1) * 8) &
                   -1L << (8 - uVar2) * 8 |
                   *(ulong *)((animDebugVifs[0]._12_4_ + 8) - uVar2) >> uVar2 * 8;
          uVar4 = animDebugVifs[0]._12_4_ + 0x17 & 7;
          uVar2 = animDebugVifs[0]._12_4_ + 0x10 & 7;
          uVar13 = (*(long *)((animDebugVifs[0]._12_4_ + 0x17) - uVar4) <<
                    (7 - uVar4) * 8 | uVar13 & 0xffffffffffffffffU >> (uVar4 + 1) * 8) &
                   -1L << (8 - uVar2) * 8 |
                   *(ulong *)((animDebugVifs[0]._12_4_ + 0x10) - uVar2) >> uVar2 * 8
          ;
          uVar4 = animDebugVifs[0]._12_4_ + 0x1f & 7;
          uVar2 = animDebugVifs[0]._12_4_ + 0x18 & 7;
          uVar14 = (*(long *)((animDebugVifs[0]._12_4_ + 0x1f) - uVar4) <<
                    (7 - uVar4) * 8 | uVar14 & 0xffffffffffffffffU >> (uVar4 + 1) * 8) &
                   -1L << (8 - uVar2) * 8 |
                   *(ulong *)((animDebugVifs[0]._12_4_ + 0x18) - uVar2) >> uVar2 * 8
          ;
          puVar1 = (undefined *)((int)&pTVar10->qwc + 1);
          uVar4 = (uint)puVar1 & 7;
          puVar5 = (ulong *)(puVar1 + -uVar4);
          *puVar5 = *puVar5 & -1L << (uVar4 + 1) * 8 | uVar11 >> (7 - uVar4) * 8;
          pTVar10->width = (short)uVar11;
          pTVar10->height = (short)(uVar11 >> 0x10);
          pTVar10->requiredGSMem = (short)(uVar11 >> 0x20);
          pTVar10->qwc = (short)(uVar11 >> 0x30);
          puVar1 = (undefined *)((int)&pTVar10->tbw + 3);
          uVar4 = (uint)puVar1 & 7;
          puVar5 = (ulong *)(puVar1 + -uVar4);
          *puVar5 = *puVar5 & -1L << (uVar4 + 1) * 8 | uVar12 >> (7 - uVar4) * 8;
          pTVar10->flags = (short)uVar12;
          pTVar10->unk2 = (short)(uVar12 >> 0x10);
          pTVar10->tbw = (int)(uVar12 >> 0x20);
          puVar1 = (undefined *)((int)&pTVar10->gif_tadr_val + 3);
          uVar4 = (uint)puVar1 & 7;
          puVar5 = (ulong *)(puVar1 + -uVar4);
          *puVar5 = *puVar5 & -1L << (uVar4 + 1) * 8 | uVar13 >> (7 - uVar4) * 8;
          pTVar10->gif_madr_val = (int)uVar13;
          pTVar10->gif_tadr_val = (void *)(int)(uVar13 >> 0x20);
          puVar1 = (undefined *)((int)&pTVar10->resetFrameNo + 3);
          uVar4 = (uint)puVar1 & 7;
          puVar5 = (ulong *)(puVar1 + -uVar4);
          *puVar5 = *puVar5 & -1L << (uVar4 + 1) * 8 | uVar14 >> (7 - uVar4) * 8;
          pTVar10->gsAllocInfo = (GSAllocInfo *)(int)uVar14;
          pTVar10->resetFrameNo = (int)(uVar14 >> 0x20);
          uVar4 = uVar6 + 0x27 & 7;
          uVar2 = uVar6 + 0x20 & 7;
          uVar11 = (*(long *)((uVar6 + 0x27) - uVar4) << (7 - uVar4) * 8 |
                   uVar11 & 0xffffffffffffffffU >> (uVar4 + 1) * 8) & -1L << (8 - uVar2) * 8 |
                   *(ulong *)((uVar6 + 0x20) - uVar2) >> uVar2 * 8;
          uVar4 = uVar6 + 0x2f & 7;
          uVar2 = uVar6 + 0x28 & 7;
          uVar12 = (*(long *)((uVar6 + 0x2f) - uVar4) << (7 - uVar4) * 8 |
                   uVar12 & 0xffffffffffffffffU >> (uVar4 + 1) * 8) & -1L << (8 - uVar2) * 8 |
                   *(ulong *)((uVar6 + 0x28) - uVar2) >> uVar2 * 8;
          uVar4 = uVar6 + 0x37 & 7;
          uVar2 = uVar6 + 0x30 & 7;
          uVar13 = (*(long *)((uVar6 + 0x37) - uVar4) << (7 - uVar4) * 8 |
                   uVar13 & 0xffffffffffffffffU >> (uVar4 + 1) * 8) & -1L << (8 - uVar2) * 8 |
                   *(ulong *)((uVar6 + 0x30) - uVar2) >> uVar2 * 8;
          uVar4 = uVar6 + 0x3f & 7;
          uVar2 = uVar6 + 0x38 & 7;
          uVar14 = (*(long *)((uVar6 + 0x3f) - uVar4) << (7 - uVar4) * 8 |
                   uVar14 & 0xffffffffffffffffU >> (uVar4 + 1) * 8) & -1L << (8 - uVar2) * 8 |
                   *(ulong *)((uVar6 + 0x38) - uVar2) >> uVar2 * 8;
          puVar1 = (undefined *)((int)pTVar10->slotNodes + 7);
          uVar4 = (uint)puVar1 & 7;
          puVar5 = (ulong *)(puVar1 + -uVar4);
          *puVar5 = *puVar5 & -1L << (uVar4 + 1) * 8 | uVar11 >> (7 - uVar4) * 8;
          *(ulong *)pTVar10->slotNodes = uVar11;
          puVar1 = (undefined *)((int)pTVar10->slotNodes + 0xf);
          uVar4 = (uint)puVar1 & 7;
          puVar5 = (ulong *)(puVar1 + -uVar4);
          *puVar5 = *puVar5 & -1L << (uVar4 + 1) * 8 | uVar12 >> (7 - uVar4) * 8;
          *(ulong *)(pTVar10->slotNodes + 2) = uVar12;
          puVar1 = (undefined *)((int)pTVar10->slotNodes + 0x17);
          uVar4 = (uint)puVar1 & 7;
          puVar5 = (ulong *)(puVar1 + -uVar4);
          *puVar5 = *puVar5 & -1L << (uVar4 + 1) * 8 | uVar13 >> (7 - uVar4) * 8;
          *(ulong *)(pTVar10->slotNodes + 4) = uVar13;
          puVar1 = (undefined *)((int)pTVar10->slotNodes + 0x1f);
          uVar4 = (uint)puVar1 & 7;
          puVar5 = (ulong *)(puVar1 + -uVar4);
          *puVar5 = *puVar5 & -1L << (uVar4 + 1) * 8 | uVar14 >> (7 - uVar4) * 8;
          *(ulong *)(pTVar10->slotNodes + 6) = uVar14;
          pTVar10 = pTVar10 + 1;
        } while (-1 < iVar16);
        registerFrameFunction(animInput,1,"animInput");
        registerFrameFunction(animCamera,2,"animCamera");
        registerFrameFunction(animFrame,10,"animFrame");
        registerFrameFunction(animMenuDraw,100,"animMenuDraw");
        rotXAxis = 0x4000;
        zoom = 0.26;
        numPlayers = 0;
        camZpos = 0.0;
        return;
      }

      iVar15 = pLVar7->payload;
      iVar8 = strchr(pLVar7->name,'.');
      iVar16 += 1;
      for (iVar17 = 0; iVar17 < maxAnimDebugVifIdx; iVar17 += 1) {
        iVar9 = strncmp(pLVar7->name,(AnimDebugVif_ARRAY_ram_00455f40[iVar17].lmpDirEntry)->name,
                        iVar8 - (int)pLVar7);
        if (iVar9 == 0) {
          *(int *)&AnimDebugVif_ARRAY_ram_00455f40[iVar17].field_0xc = iVar15;
          break;
        }
      }
    } while( true );
    */
}
