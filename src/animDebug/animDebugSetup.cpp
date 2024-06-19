#include "animDebug.h"
#include "../ps2/lump.h"
#include "../ps2/texture.h"

#include <string.h>
#include <cstdio>

int AnimDebug::npcSelLmpIdx = -1;

const int maxAnimDebugVifsEntries = 32;
AnimDebug::AnimDebugVif AnimDebug::animDebugVifs[maxAnimDebugVifsEntries];
const int maxAnimDebugAnmsEntries = 32;
AnimDebug::AnimDebugAnm AnimDebug::animDebugAnms[maxAnimDebugAnmsEntries];

TextureHeader AnimDebug::modelTex;

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
      strcpy(AnimData::animStateData.animName, lmpName);
      */
    int maxAnimDebugVifIdx = 0;
    LmpDirEntry* lmpDirEntry;
    int entryNum = 0;
    int prevVifLen = 0;
    // Element 0 is the largest VIF
    while ((lmpDirEntry = searchLmpForNthFileWithExt(npcLmp, "vif", entryNum)) != nullptr) {
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
        VifData* pVifData = (VifData*)lmpDirEntry->payload;
        if (pVifData != animDebugVifs[0].vifData) {
            animDebugVifs[maxAnimDebugVifIdx].unk = 0;
            animDebugVifs[maxAnimDebugVifIdx].lmpDirEntry = lmpDirEntry;
            animDebugVifs[maxAnimDebugVifIdx].vifData = pVifData;
            ++maxAnimDebugVifIdx;
        }
        ++entryNum;
    }
    
    // find the main texture for each vif based on the name being the same
    entryNum = 0;
    while ((lmpDirEntry = searchLmpForNthFileWithExt(npcLmp, "tex", entryNum)) != nullptr) {
        const char * dotPos = strchr(lmpDirEntry->name,'.');
        int textureBaseLen = dotPos - lmpDirEntry->name;
        for (int idx = 0; idx < maxAnimDebugVifIdx; ++idx) {
            if (0 == strncmp(lmpDirEntry->name, (animDebugVifs[idx].lmpDirEntry)->name, textureBaseLen)) {
                // Found a texture with the same name as the vif file
                animDebugVifs[idx].mainTex = (TextureHeader*)lmpDirEntry->payload;
                break;
            }
        }
        ++entryNum;
    }

    /*
    FUN_ram_00138ff0((npcObject1 *)&AnimData::animStateData);
    FUN_ram_00138ff0(&npcObject1_ram_004558e0);
    */
    entryNum = 0;
    while (entryNum < maxAnimDebugAnmsEntries && (lmpDirEntry = searchLmpForNthFileWithExt(npcLmp, "anm", entryNum)) != nullptr) {
        animDebugAnms[entryNum].lmpDirEntry = lmpDirEntry;
        animDebugAnms[entryNum].anmData = lmpDirEntry->payload;
        ++entryNum;
    }
    memccpy(&modelTex, animDebugVifs[0].mainTex, sizeof(TextureHeader), 1);
/*
    registerFrameFunction(animInput,1,"animInput");
    registerFrameFunction(animCamera,2,"animCamera");
    registerFrameFunction(animFrame,10,"animFrame");
    registerFrameFunction(animMenuDraw,100,"animMenuDraw");
    rotXAxis = 0x4000;
    zoom = 0.26;
    numPlayers = 0;
    camZpos = 0.0;
*/
}
