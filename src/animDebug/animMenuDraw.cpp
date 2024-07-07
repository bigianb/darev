#include "animDebug.h"
#include "../ps2/elfData.h"
#include "../ps2/font.h"
#include "../ps2/lump.h"
#include "../ps2/pad.h"
#include "../ps2/text.h"
#include "../ps2/vifMesh.h"

#include "../vector.h"

int AnimDebug::animDebugFrameSkip = 0;
int AnimDebug::slectedAnmIdx = 0;

int selectedLmp = 0;
int numTris = 0;

void AnimDebug::drawLmpList()
{
    beginText((Font*)menuFont, 7, 1, 0x80808080);

    int yAdj = 0;
    int startYpos = selectedLmp * 0x14 + 0x14;
    while (startYpos > 400) {
        startYpos -= 0x14;
        yAdj -= 0x14;
    }

    int curLmpIdx = 0;
    while (allLmpNames[curLmpIdx] != nullptr) {
        if (curLmpIdx == selectedLmp) {
            setTextColor(0x80408080);
        }
        int ypos = curLmpIdx * 0x14 + yAdj + 0x14;
        if (ypos < 0x1f5) {
            displayText(0x1e, ypos, allLmpNames[curLmpIdx]);
            if (curLmpIdx == selectedLmp) {
                setTextColor(0x80808080);
            }
        }
        ++curLmpIdx;
    }

    if (selectedLmp >= curLmpIdx) {
        selectedLmp = curLmpIdx - 1;
    }
    if (((convertedControllerInput[0].updatedButtons2 & DA_PAD_UP) != 0) && (selectedLmp > 0)) {
        --selectedLmp;
    }
    if (((convertedControllerInput[0].updatedButtons2 & DA_PAD_DOWN) != 0) &&
        (allLmpNames[selectedLmp + 1] != nullptr)) {
        ++selectedLmp;
    }

    flushText();
}

#define MENU_LEVEL_LMP -1
#define MENU_LEVEL_ANM 0
#define MENU_LEVEL_VIF 1
#define MENU_LEVEL_CHG 2
#define MENU_LEVEL_NUL 3

void AnimDebug::animInput()
{
    if (animDebugFrameSkip == 1) {
        lmpCleanup(0);
        setup(0, NULL);
        animDebugFrameSkip = 0;
    } else {
        if (animDebugFrameSkip != 0) {
            --animDebugFrameSkip;
        }
        if (((convertedControllerInput[0].updatedButtons & DA_PAD_LEFT) != 0) && (MENU_LEVEL_LMP < menuLevel)) {
            --menuLevel;
        }
        if (((convertedControllerInput[0].updatedButtons & DA_PAD_RIGHT) != 0) && (menuLevel < MENU_LEVEL_NUL)) {
            ++menuLevel;
        }

        if (menuLevel == MENU_LEVEL_ANM) {
            // anm list

            if (((convertedControllerInput[0].updatedButtons2 & DA_PAD_UP) != 0) && (slectedAnmIdx > 0)) {
                --slectedAnmIdx;
            }
            if (((convertedControllerInput[0].updatedButtons2 & DA_PAD_DOWN) != 0) && (slectedAnmIdx < animDebugAnmsEntries - 1)) {
                ++slectedAnmIdx;
            }
            if (animDebugAnmsEntries > 0) {
                if ((convertedControllerInput[0].updatedButtons & DA_PAD_TRIANGLE) != 0) {
                    //FUN_ram_00138d28(0.2, 0.2, &AnimData::animStateData, animDebugAnms[slectedAnmIdx].anmData, 0);
                    //DAT_ram_0032468c = 10;
                }
                if ((convertedControllerInput[0].updatedButtons & DA_PAD_CROSS) != 0) {
                    //FUN_ram_00138c50(0.3, &AnimData::animStateData, animDebugAnms[slectedAnmIdx].anmData, 3);
                    //DAT_ram_0032468c = 10;
                }
            }
        } else if (menuLevel == MENU_LEVEL_LMP) {
            drawLmpList();
            if ((convertedControllerInput[0].updatedButtons & DA_PAD_CROSS) != 0) {
                animDebugFrameSkip = 3;
                npcSelLmpIdx = selectedLmp;
            }
        }

        else if (menuLevel == MENU_LEVEL_VIF) {

            if (((convertedControllerInput[0].updatedButtons2 & DA_PAD_UP) != 0) && (selectedVifMenuIdx > 0)) {
                selectedVifMenuIdx -= 1;
            }
            if (((convertedControllerInput[0].updatedButtons2 & DA_PAD_DOWN) != 0) &&
                (selectedVifMenuIdx < maxAnimDebugVifIdx - 1)) {
                selectedVifMenuIdx += 1;
            }
            /*
            if ((convertedControllerInput[0].updatedButtons & DA_PAD_CIRCLE) != 0) {
                animDebugVifs[selectedVifMenuIdx].field2_0x8 ^= 1;
            }
            */
        } else if (menuLevel == MENU_LEVEL_CHG) {

            if (((convertedControllerInput[0].updatedButtons2 & DA_PAD_UP) != 0) && (0 < selectedChangeMenuIdx)) {
                --selectedChangeMenuIdx;
            }
            if (((convertedControllerInput[0].updatedButtons2 & DA_PAD_DOWN) != 0) && (selectedChangeMenuIdx < 0x1f)) {
                ++selectedChangeMenuIdx;
            }
            if ((convertedControllerInput[0].updatedButtons & DA_PAD_CROSS) != 0) {
                activeChangeItems ^= 1 << (selectedChangeMenuIdx & 0x1f);
            }
        }
        /*
        if ((convertedControllerInput[0].RLUD_buttons & 0x400U) == 0) {
            iVar1 = (uint)rotXAxis + (int)(convertedControllerInput[0].r_stick_y * 900.0);
            rotZAxis += (short)(int)(convertedControllerInput[0].r_stick_x * 900.0);
            rotXAxis = (ushort)iVar1;
            if (iVar1 * 0x10000 >> 0x10 < -10000) {
                rotXAxis = 0xd8f0;
            }
            if (10000 < (short)rotXAxis) {
                rotXAxis = 10000;
            }
            if ((convertedControllerInput[0].RLUD_buttons & 4U) == 0) {
                zoom = zoom + convertedControllerInput[0].l_stick_y * 0.02;
                if (zoom < 0.1) {
                    zoom = 0.1;
                }
                if (2.0 < zoom) {
                    zoom = 2.0;
                }
            } else {
                fVar2 = 1.0 / SQRT(particle_3x3Matrix.m00 * particle_3x3Matrix.m00 +
                                   particle_3x3Matrix.m01 * particle_3x3Matrix.m01);
                fVar3 = 1.0 / SQRT(particle_3x3Matrix.m20 * particle_3x3Matrix.m20 +
                                   particle_3x3Matrix.m21 * particle_3x3Matrix.m21);
                camYPos = camYPos + particle_3x3Matrix.m21 * fVar3 * convertedControllerInput[0].l_stick_y * -3.0 +
                          particle_3x3Matrix.m01 * fVar2 *
                              convertedControllerInput[0].l_stick_x * 3.0;
                camXPos = camXPos + particle_3x3Matrix.m20 * fVar3 * convertedControllerInput[0].l_stick_y * -3.0 +
                          particle_3x3Matrix.m00 * fVar2 *
                              convertedControllerInput[0].l_stick_x * 3.0;
            }
        } else {
            camZpos = camZpos - convertedControllerInput[0].r_stick_y;
        }
        */
    }
}

void AnimDebug::animMenuDraw()
{
    /*
    if ((convertedControllerInput[0].updatedButtons & 8U) != 0) {
        // r1
        if (topLightEnabled == 0) {
            enableLight(topDLight);
        } else {
            disableLight(topDLight);
        }
        topLightEnabled ^= 1;
    }
    if ((convertedControllerInput[0].updatedButtons & 2U) != 0) {
        // r2
        if (bottomLightEnabled == 0) {
            enableLight(bottomDLight);
        } else {
            disableLight(bottomDLight);
        }
        bottomLightEnabled ^= 1;
    }
    */

    int yMenuStartPos = 0;

    int iVar2 = 0;
    int iVar1 = slectedLmpIdx;
    if ((menuLevel == 0) || (iVar1 = selectedVifMenuIdx, menuLevel == 1)) {
        iVar2 = iVar1;
    }
    for (iVar2 = iVar2 * 0x14 + yMenuStartPos + -200; iVar2 < -200; iVar2 += 8) {
        yMenuStartPos += 8;
    }
    for (; 0xb4 < iVar2; iVar2 += -8) {
        yMenuStartPos += -8;
    }

    if (menuLevel < MENU_LEVEL_NUL) {
        beginText((Font*)menuFont, 3, 1, 0x80808080);
        //if ((zoom < FLOAT_ram_0032246c) && (0.25 < zoom)) {
        displayTextCentered(0x140, 400, "Actual Size");
        //}
        displayFormattedText(500, 0x14, "%d tri", numTris);
        flushText();
    }

    switch (menuLevel) {
    case MENU_LEVEL_ANM:
    {
        int aeIdx = 0;
        beginText((Font*)menuFont, 3, 1, 0x80808080);

        int ypos = yMenuStartPos + 0x18;
        while (aeIdx < animDebugAnmsEntries) {
            if ((-0x29 < ypos) && (ypos < 0x1f5)) {
                if (aeIdx == slectedAnmIdx) {
                    setTextColor(0x80408080);
                }
                displayText(0x14, ypos, animDebugAnms[aeIdx].lmpDirEntry->name);
                if (aeIdx == slectedAnmIdx) {
                    setTextColor(0x80808080);
                }
            }
            ++aeIdx;
            ypos += 0x14;
        }

        flushText();
    } break;
    case MENU_LEVEL_VIF:
    {
        int vifIdx = 0;
        beginText((Font*)menuFont, 3, 1, 0x80808080);

        while (vifIdx < maxAnimDebugVifIdx) {
            char uVar6 = '-';
            //if (animDebugVifs[vifIdx].field2_0x8 != 0) {
            //    uVar6 = '+';
            // }
            int iVar5 = yMenuStartPos + vifIdx * 0x14 + 0x18;
            if ((-0x29 < iVar5) && (iVar5 < 0x1f5)) {
                if (vifIdx == selectedVifMenuIdx) {
                    setTextColor(0x80408080);
                }
                displayFormattedText(0x14, yMenuStartPos + (vifIdx + -1) * 0x14 + 0x18, "%c%s\n", uVar6, animDebugVifs[vifIdx].lmpDirEntry->name);
                if (vifIdx == selectedVifMenuIdx) {
                    setTextColor(0x80808080);
                }
            }
            ++vifIdx;
        }

        flushText();

    } break;
    case MENU_LEVEL_CHG:
    {
        int changeId = 0;
        beginText((Font*)menuFont, 3, 1, 0x80808080);
        int ypos = 0x18;
        do {
            if ((-0x29 < yMenuStartPos + ypos) && (yMenuStartPos + ypos < 0x1f5)) {
                char uVar6 = '-';
                if ((activeChangeItems >> (changeId & 0x1f) & 1U) != 0) {
                    uVar6 = '+';
                }
                if (changeId == selectedChangeMenuIdx) {
                    setTextColor(0x80408080);
                } else {
                    iVar1 = 0; //vifHasChange(animDebugVifs[0].vifData, changeId);
                    if (iVar1 == 0) {
                        setTextColor(0x80202020);
                    } else {
                        setTextColor(0x80808080);
                    }
                }
                displayFormattedText(0x14, yMenuStartPos + ypos + -0x14, "%cchange%d\n", uVar6, changeId);
            }
            ++changeId;
            ypos += 0x14;
        } while (changeId < 0x20);
        flushText();

    } break;
    }
}

void AnimDebug::animFrame()
{
    if (animDebugFrameSkip == 0) {
        Vec3 local_510(0.0f, 0.0f, 0.0f);

        //FUN_ram_001368d0(1.0, &AnimData::animStateData, &local_510);

        VifData* vifData = animDebugVifs[0].vifData;
        u64 meshMask = getMeshMask(animDebugVifs[0].vifData, activeChangeItems);
        /*
        getVifBBox(vifData,-1,&iStack_c0,&iStack_bc,&local_b8,&iStack_b4,&iStack_b0,local_ac);
        camZPos = (float)((local_ac[0] + local_b8) / 2);
        if (camZPos <= 0.0) {
          camZPos = 0.0;
        }
        */

        /*
         iVar9 = 1;
         iVar7 = 0;
         if (1 < maxAnimDebugVifIdx) {
           do {
             if (animDebugVifs[iVar9].field2_0x8 != 0) {
               pVVar2 = animDebugVifs[iVar9].vifData;
               uVar6._0_1_ = pVVar2->sig[0];
               uVar6._1_1_ = pVVar2->sig[1];
               uVar6._2_1_ = pVVar2->sig[2];
               uVar6._3_1_ = pVVar2->sig[3];
               uVar6._4_4_ = *(undefined4 *)&pVVar2->field_0x4;
               local_500[iVar7 * 2 + 1] = (VifData *)animDebugVifs[iVar9].mainTex;
               local_500[iVar7 * 2] = pVVar2;
               iVar7 += 1;
               meshMask &= ~uVar6;
             }
             iVar9 += 1;
           } while (iVar9 < maxAnimDebugVifIdx);
         }
     */

        int iVar7 = getNumTrisOfSelectedVifs(vifData, meshMask);

        int iVar9 = 1;
        if (1 < maxAnimDebugVifIdx) {
            VifData** ppVVar8 = &animDebugVifs[1].vifData;

            do {
                if (animDebugVifs[iVar9].selected) {
                    int iVar4 = getNumTrisOfSelectedVifs(animDebugVifs[iVar9].vifData, 0xffffffffffffffff);
                    iVar7 += iVar4;
                }
                iVar9 += 1;

            } while (iVar9 < maxAnimDebugVifIdx);
        }

        numTris = iVar7;
        /*
            local_510.x = camXPos;
            local_510.y = camYPos;
            local_510.z = camZpos;

            makeIdentityMtx_3x4(&mStack_100);
            translate(camXPos,camYPos,camZpos,&mStack_100);
            DAT_ram_00324604 = 1;
            _auStack_d0 = CONCAT44(local_510.y,local_510.x);
            puVar1 = auStack_d0 + 7;
            uVar3 = (uint)puVar1 & 7;
            *(ulong *)(puVar1 + -uVar3) =
                 *(ulong *)(puVar1 + -uVar3) & -1L << (uVar3 + 1) * 8 | _auStack_d0 >> (7 - uVar3) * 8;
            uStack_c9._1_4_ = local_510.z;
                            // anim state at 0x4559e8
                            //
            drawAnimatedModel(vifData,&modelTex,1,(Vec3 *)auStack_d0,&mStack_100,
                              (animStateData *)(iVar4 + 0x59e8),meshMask,0);
            FUN_ram_0013fa70();
            if (('\0' < *(char *)(iVar4 + 0x59ec)) &&
               ((float)((animStateData *)(iVar4 + 0x59e8))->stateArray[*(char *)(iVar4 + 0x59ec) + -1].
                       desiredFrame < 2.0)) {
              drawColorSprite(0x21c,0x15c,0x24e,0x18e,6,0x80ffffff);
              if (*(char *)(iVar4 + 0x59ec) == '\x01') {
                drawColorSprite(0x221,0x161,0x249,0x189,7,0x80000000);
              }
              DAT_ram_0032468c += -1;
            }
            */
        if (menuLevel < 3) {
            beginText((Font*)menuFont, 3, 1, 0x80808080);
            // displayTextCentered(0x140, 0x1a4, AnimData::animStateData.animName);
            flushText();
        }
    }
}
