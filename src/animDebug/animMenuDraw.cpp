#include "animDebug.h"
#include "../ps2/elfData.h"
#include "../ps2/font.h"
#include "../ps2/lump.h"
#include "../ps2/pad.h"
#include "../ps2/text.h"

int AnimDebug::animDebugFrameSkip = 0;
int AnimDebug::slectedAnmIdx = 0;

int selectedLmp = 0;

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

    if (curLmpIdx <= selectedLmp) {
        selectedLmp = curLmpIdx - 1;
    }
    if (((convertedControllerInput[0].updatedButtons2 & 0x1000U) != 0) && (0 < selectedLmp)) {
        --selectedLmp;
    }
    if (((convertedControllerInput[0].updatedButtons2 & 0x4000U) != 0) &&
        (allLmpNames[selectedLmp + 1] != nullptr)) {
        ++selectedLmp;
    }

    flushText();
}

void AnimDebug::animInput()
{
    if ((animDebugFrameSkip == 0) || (animDebugFrameSkip += -1, animDebugFrameSkip != 0)) {
        if (((convertedControllerInput[0].updatedButtons & DA_PAD_LEFT) != 0) && (-1 < menuLevel)) {
            --menuLevel;
        }
        if (((convertedControllerInput[0].updatedButtons & DA_PAD_RIGHT) != 0) && (menuLevel < 4)) {
            ++menuLevel;
        }

        if (menuLevel == 0) {
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
        } else if (menuLevel == -1) {
            drawLmpList();
            if ((convertedControllerInput[0].updatedButtons & DA_PAD_CROSS) != 0) {
                animDebugFrameSkip = 3;
                npcSelLmpIdx = selectedLmp;
            }
        }

        else if (menuLevel == 1) {
            
            if (((convertedControllerInput[0].updatedButtons2 & DA_PAD_UP) != 0) && (1 < selectedVifMenuIdx)) {
                selectedVifMenuIdx += -1;
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
        } else if (menuLevel == 2) {
            
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
    } else {
        //lmpCleanup(0);
        setup(0, NULL);
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
    /*
    if (DAT_ram_003246c4 != 0) {
        TextureHeader*  tex = nullptr;
        if (menuLevel == 1) {
            tex = animDebugVifs[selectedVifMenuIdx].mainTex;
        }
        if (menuLevel == 0) {
            tex = &modelTex;
        }
        if (tex != nullptr) {
            drawTexSprite(tex, 0x276 - tex->width, 0x1b6 - tex->height, 3, 0, 0x808080);
        }
    }
    */
    /*
     if (menuLevel < 3) {
         beginText(pMenuFont, 3, 1, 0x80808080);
         if ((zoom < FLOAT_ram_0032246c) && (0.25 < zoom)) {
             displayTextCentered(0x140, 400, "Actual Size");
         }
         displayFormattedText(500, 0x14, "%d tri", (long)numTris);
         flushText();
     }
     */
    switch (menuLevel) {
    case 0:
    {
        int aeIdx = 0;
        beginText((Font*)menuFont, 3, 1, 0x80808080);

        int ypos = yMenuStartPos + 0x18;
        while (aeIdx < animDebugAnmsEntries) {
            if ((-0x29 < ypos) && (ypos < 0x1f5)) {
                if (iVar2 == slectedLmpIdx) {
                    setTextColor(0x80408080);
                }
                displayText(0x14, ypos, animDebugAnms[aeIdx].lmpDirEntry->name);
                if (iVar2 == slectedLmpIdx) {
                    setTextColor(0x80808080);
                }
            }
            ++aeIdx;
            ypos += 0x14;
        }
        
        flushText();
    } break;
    case 1:
    {
        int iVar1 = 1;
        beginText((Font*)menuFont, 3, 1, 0x80808080);
        if (1 < maxAnimDebugVifIdx) {
            do {
                char uVar6 = '-';
                //if (animDebugVifs[iVar1].field2_0x8 != 0) {
                //    uVar6 = '+';
               // }
                int iVar5 = yMenuStartPos + iVar1 * 0x14 + 0x18;
                if ((-0x29 < iVar5) && (iVar5 < 0x1f5)) {
                    if (iVar1 == selectedVifMenuIdx) {
                        setTextColor(0x80408080);
                    }
                    displayText(0x14, yMenuStartPos + (iVar1 + -1) * 0x14 + 0x18, animDebugVifs[iVar1].lmpDirEntry->name);
                    // displayFormattedText(0x14, yMenuStartPos + (iVar1 + -1) * 0x14 + 0x18, "%c%s\n", uVar6, animDebugVifs[iVar1].lmpDirEntry->name);
                    if (iVar1 == selectedVifMenuIdx) {
                        setTextColor(0x80808080);
                    }
                }
                iVar1 += 1;
            } while (iVar1 < maxAnimDebugVifIdx);
        }
        flushText();
        
    } break;
    case 2:
    { /*
         lVar4 = 0;
         beginText(pMenuFont, 3, 1, 0x80808080);
         iVar2 = 0x18;
         do {
             uVar3 = (uint)lVar4;
             if ((-0x29 < yMenuStartPos + iVar2) && (yMenuStartPos + iVar2 < 0x1f5)) {
                 uVar6 = '-';
                 if ((activeChangeItems >> (uVar3 & 0x1f) & 1U) != 0) {
                     uVar6 = '+';
                 }
                 if (lVar4 == selectedChangeMenuIdx) {
                     setTextColor(0x80408080);
                 } else {
                     iVar1 = vifHasChange(animDebugVifs[0].vifData, uVar3);
                     if (iVar1 == 0) {
                         setTextColor(0x80202020);
                     } else {
                         setTextColor(0x80808080);
                     }
                 }
                 displayFormattedText(0x14, yMenuStartPos + iVar2 + -0x14, "%cchange%d\n", uVar6, lVar4);
             }
             lVar4 = (long)(int)(uVar3 + 1);
             iVar2 += 0x14;
         } while (lVar4 < 0x20);
         flushText();
         */
    } break;
    }
}
