#include "animDebug.h"
#include "../ps2/elfData.h"
#include "../ps2/font.h"
#include "../ps2/lump.h"
#include "../ps2/text.h"

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
        iVar2 = 0;
        beginText((Font*)menuFont, 3, 1, 0x80808080);
        if (animDebugAnmsEntries > 0) {
            int iVar5 = 0x18;
            auto* pAVar7 = animDebugAnms;
            do {
                if ((-0x29 < yMenuStartPos + iVar5) && (yMenuStartPos + iVar5 < 0x1f5)) {
                    if (iVar2 == slectedLmpIdx) {
                        setTextColor(0x80408080);
                    }
                    displayText(0x14, yMenuStartPos + iVar5, pAVar7->lmpDirEntry->name);
                    if (iVar2 == slectedLmpIdx) {
                        setTextColor(0x80808080);
                    }
                }
                iVar2 += 1;
                pAVar7 += 1;
                iVar5 += 0x14;
            } while (iVar2 < animDebugAnmsEntries);
        }
        flushText();
    } break;
    case 1:
    {
        /*
        iVar1 = 1;
        beginText(pMenuFont, 3, 1, 0x80808080);
        iVar2 = maxAnimDebugVifIdx;
        if (1 < maxAnimDebugVifIdx) {
            do {
                uVar6 = 0x2d;
                if (animDebugVifs[iVar1].field2_0x8 != 0) {
                    uVar6 = 0x2b;
                }
                iVar5 = yMenuStartPos + iVar1 * 0x14 + 0x18;
                if ((-0x29 < iVar5) && (iVar5 < 0x1f5)) {
                    if (iVar1 == selectedVifMenuIdx) {
                        setTextColor(0x80408080);
                    }
                    displayFormattedText(0x14, yMenuStartPos + (iVar1 + -1) * 0x14 + 0x18, "%c%s\n", uVar6,
                                         (long)(int)animDebugVifs[iVar1].lmpDirEntry);
                    iVar2 = maxAnimDebugVifIdx;
                    if (iVar1 == selectedVifMenuIdx) {
                        setTextColor(0x80808080);
                        iVar2 = maxAnimDebugVifIdx;
                    }
                }
                iVar1 += 1;
            } while (iVar1 < iVar2);
        }
        flushText();
        */
    } break;
    case 2:
    { /*
         lVar4 = 0;
         beginText(pMenuFont, 3, 1, 0x80808080);
         iVar2 = 0x18;
         do {
             uVar3 = (uint)lVar4;
             if ((-0x29 < yMenuStartPos + iVar2) && (yMenuStartPos + iVar2 < 0x1f5)) {
                 uVar6 = 0x2d;
                 if ((activeChangeItems >> (uVar3 & 0x1f) & 1U) != 0) {
                     uVar6 = 0x2b;
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
    default:
    {
        beginText((Font*)menuFont, 3, 1, 0x80808080);
        displayText(0x14, 100, "menu level -1");
        flushText();
    } break;
    }
}
