#include <tamtypes.h>
#include <cstring>
#include "menu.h"
#include "font.h"
#include "texture.h"
#include "draw.h"
#include "pad.h"
#include "text.h"
#include "trace.h"

void drawMenu(int xpos, int ypos, MenuDef* menuDefs, u32 color, u32 colorSel, Font* font, TextureHeader* tex)
{
    int numMenuEntries = 0;
    while (menuDefs[numMenuEntries].menuText != nullptr) {
        numMenuEntries += 1;
    }

    int totalHeight = numMenuEntries * 0x25 + 6;
    int selectedIdx = menuDefs[numMenuEntries].typeOrSelectedIdx;
    int local_b0 = totalHeight >> 0x1f;
    int iUnk = (selectedIdx * 0x25 - totalHeight / 2) + 0xed;

    int ySomething = 0;
    int local_a8 = -3;
    if (iUnk < 0x14) {
        do {
            iUnk += 0x25;
            local_a8 += 0x25;
            ySomething += 0x25;
        } while (iUnk < 0x14);
    }

    int iVar5 = (selectedIdx * 0x25 + 0xf0) - totalHeight / 2;
    if (400 < iVar5 + local_a8) {
        local_a8 = ySomething + -3;
        iVar5 = local_a8 + iVar5;
        do {
            iVar5 += -0x25;
            local_a8 += -0x25;
            ySomething += -0x25;
        } while (400 < iVar5);
    }

    beginText(font, 7, 0, scaleColor(textBrightness[0], color));

    u16 wTextBuf[128];

    int idx = 0;
    while (menuDefs[idx].menuText != nullptr) {
        MenuDef& menuDef = menuDefs[idx];

        int textXpos = xpos;
        int textYpos = (ypos - ((totalHeight - local_b0) >> 1)) + idx * 0x25 + local_a8;
        const char* pcVar2 = strstr(menuDef.menuText, "menu_");
        if (pcVar2 == nullptr) {
            atowcs(wTextBuf, menuDef.menuText);
        } else {
            // TODO: remove when translateText is implemented.
            atowcs(wTextBuf, menuDef.menuText);
            //wcscpy(wTextBuf, translateText(menuDef.menuText));
        }
        /*
                    if (menuDef.typeOrSelectedIdx == 3) {
                        FUN_ram_001361e0(wTextBuf, ":");
                        if (*(int*)menuDef.value == 0) {
                            pwVar3 = translateText("off");
                        } else {
                            pwVar3 = translateText("on");
                        }
                        wcscat(wTextBuf, pwVar3);
                    } else if (menuDef.typeOrSelectedIdx == 4) {
                        textXpos = measureTextW(local_font, wTextBuf, 1000000, isInterlaced != 0);
                        textXpos = (xpos + -0x17) - textXpos / 2;
                    }
                    */
        bool colorResetNeeded = false;
        if (idx == selectedIdx) {
            setTextColor(scaleColor(textBrightness[0], colorSel));
            colorResetNeeded = true;
        }
        /*
        int iVar11 = menuDef.typeOrSelectedIdx;
        if (iVar11 == 5) {
            setTextColor(scaleColor(textBrightness[0] * 0.5, color));
            colorResetNeeded = true;
        }
        if (iVar11 == 6) {
            setTextColor(scaleColor(textBrightness[0], 0x80202040));
            colorResetNeeded = true;
        }
        if (iVar11 == 7) {
            setTextColor(scaleColor(textBrightness[0], 0x80402020));
            colorResetNeeded = true;
        }
        */
        displayTextCenteredW(textXpos, textYpos, wTextBuf, 1000000);
        if (colorResetNeeded) {
            setTextColor(scaleColor(textBrightness[0], color));
        }

        idx += 1;
    }

    flushText();

    int local_ac = ySomething + -7;

    int menuNo = 0;
    while (menuDefs[menuNo].menuText != nullptr) {
        int local_b8 = menuNo * 0x25;
        MenuDef* iVar12 = &menuDefs[menuNo];

        int local_bc = (totalHeight - local_b0) >> 1;
        int local_b4 = (ypos - local_bc) + local_b8;
        int menuType = iVar12->typeOrSelectedIdx;
        int iVar5 = local_b4 + local_a8;
        if (menuType == 4) {
            int x0 = xpos - 0x12;
            int iVar11 = xpos - 0x14;
            int x1 = xpos + 100;
            //drawColorSprite(x0, iVar5 + 9, (int)((float)x0 + **(float**)(iVar12 + 8) * 116.0), iVar5 + 0x12, 7, 0x800c2b40);
            //drawColorSprite(x0, iVar5 + 9, xpos + 0x62, iVar5 + 0x12, 7, 0x80000000);
            //drawColorSprite(iVar11, iVar5 + 7, x1, iVar5 + 0x14, 7, 0x80808080);
        }
        int brightness = 0x80;
        if (menuNo == selectedIdx) {
            brightness = 0xd1;
        }
        if (menuType == 5) {
            brightness = 0x40;
        }

        int width = tex->width << 0x10;

        brightness = (int)((float)brightness * textBrightness[0]);
        menuNo += 1;
        if (0xff < brightness) {
            brightness = 0xff;
        }
        u32 rgba = brightness | 0x80000000 | brightness << 0x10 | brightness << 8;
        drawSprite(tex, 0x140 - (((width >> 0x10) - (width >> 0x1f)) >> 1), local_b4 + local_ac, 7, 0, rgba);

        // shadow
        drawSprite(tex, 0x144 - (((width >> 0x10) - (width >> 0x1f)) >> 1), (ypos - (local_bc + -4)) + local_b8 + local_ac, 7, 0, 0x3f000000);
    }
}

int getSelectedMenuItem(MenuDef* menuDefs)
{
    int numDefs = 0;
    while (menuDefs[numDefs].menuText != nullptr) {
        numDefs += 1;
    }

    return menuDefs[numDefs].typeOrSelectedIdx;
}

int updateSelectedMenuItem(MenuDef* menuDefs, int controllerPort)
{
    int numDefs = 0;
    while (menuDefs[numDefs].menuText != nullptr) {
        numDefs += 1;
    }

    int selectedIdx = menuDefs[numDefs].typeOrSelectedIdx;
    int updatedButtons;
    int buttons;
    if (controllerPort == -1) {
        updatedButtons = convertedControllerInput[0].updatedButtons2 | convertedControllerInput[1].updatedButtons2;
        buttons = convertedControllerInput[0].RLUD_buttons | convertedControllerInput[1].RLUD_buttons;
    } else {
        updatedButtons = convertedControllerInput[controllerPort].updatedButtons2;
        buttons = convertedControllerInput[controllerPort].RLUD_buttons;
    }
    int idxChange = 0;
    int newSelIdx = selectedIdx;
    if ((updatedButtons & (DA_RSTICK_UP | DA_PAD_UP)) != 0) {
        newSelIdx -= 1;
        idxChange = -1;
    }
    if ((updatedButtons & (DA_RSTICK_DOWN | DA_PAD_DOWN)) != 0) {
        newSelIdx += 1;
        idxChange = 1;
    }
    int maxSel = numDefs - 1;
    if (newSelIdx < 0) {
        newSelIdx = 0;
    }
    if (maxSel < newSelIdx) {
        newSelIdx = maxSel;
    }

    int nextSelIdx;
    do {
        nextSelIdx = newSelIdx;
        if ((idxChange == 0) || (menuDefs[newSelIdx].typeOrSelectedIdx != 5)) {
            break;
        }
        newSelIdx += idxChange;
        if (newSelIdx < 0) {
            nextSelIdx = 0;
            break;
        }
        nextSelIdx = maxSel;
    } while (newSelIdx <= maxSel);

    if ((menuDefs[nextSelIdx].typeOrSelectedIdx == 5)) {
        nextSelIdx = selectedIdx;
        if (menuDefs[selectedIdx].typeOrSelectedIdx == 5) {
            nextSelIdx = 0;
        }
    }

    bool showGameButnBrowse = false;
    if (nextSelIdx != selectedIdx) {
        traceln("Buttons = 0x%x", buttons);
        traceln("updatedButtons = 0x%x", updatedButtons);
        /*
                if (DAT_ram_00324460 != 0x65) {
                    showGameButnBrowse = true;
                }
                */
    }

    if ((updatedButtons & (DA_RSTICK_LEFT | DA_PAD_LEFT)) != 0) {
        int menuEntryType = menuDefs[nextSelIdx].typeOrSelectedIdx;
        if (menuEntryType == 1) {
            *(int*)menuDefs[nextSelIdx].value -= 1;
            showGameButnBrowse = true;
        } else if (menuEntryType == 2) {
            *(float*)menuDefs[nextSelIdx].value -= 0.1f;
            showGameButnBrowse = true;
        } else if (menuEntryType == 3) {
            *(u32*)menuDefs[nextSelIdx].value = (u32)(*(u32*)menuDefs[nextSelIdx].value == 0);
            showGameButnBrowse = true;
        }
    }

    if (((buttons & (DA_RSTICK_LEFT | DA_PAD_LEFT)) != 0) && (menuDefs[nextSelIdx].typeOrSelectedIdx == 4)) {
        float* pfVar2 = (float*)menuDefs[nextSelIdx].value;
        float fVar10 = *pfVar2;
        *pfVar2 = fVar10 - 0.01;
        if (fVar10 - 0.01 < 0.0) {
            *pfVar2 = 0.0;
        }
    }
    if (((buttons & (DA_RSTICK_RIGHT | DA_PAD_RIGHT)) != 0) && (menuDefs[nextSelIdx].typeOrSelectedIdx == 4)) {
        float* pfVar2 = (float*)menuDefs[nextSelIdx].value;
        float fVar10 = *pfVar2;
        *pfVar2 = fVar10 + 0.01;
        if (1.0 < fVar10 + 0.01) {
            *pfVar2 = 1.0;
        }
    }

    if (((updatedButtons & acceptPadButton) != 0) && (menuDefs[nextSelIdx].typeOrSelectedIdx == 3)) {
        *(u32*)menuDefs[nextSelIdx].value = (u32)(*(u32*)menuDefs[nextSelIdx].value == 0);
        showGameButnBrowse = true;
    }

    if ((updatedButtons & (DA_RSTICK_RIGHT | DA_PAD_RIGHT)) != 0) {
        int menuEntryType = menuDefs[nextSelIdx].typeOrSelectedIdx;
        if (menuEntryType == 1) {
            *(int*)menuDefs[nextSelIdx].value += 1;
            showGameButnBrowse = true;
        } else if (menuEntryType == 2) {
            *(float*)menuDefs[nextSelIdx].value += 0.1f;
            showGameButnBrowse = true;
        } else if (menuEntryType == 3) {
            *(u32*)menuDefs[nextSelIdx].value = (u32)(*(u32*)menuDefs[nextSelIdx].value == 0);
            showGameButnBrowse = true;
        }
    }

    if (showGameButnBrowse) {
        //pvVar3 = findLmpEntry("fx.lmp", "game_butn_browse.anm");
        //FUN_ram_001f2740(1.0, pvVar3, NULL);
    }

    menuDefs[numDefs].typeOrSelectedIdx = nextSelIdx;
    return nextSelIdx;
}
