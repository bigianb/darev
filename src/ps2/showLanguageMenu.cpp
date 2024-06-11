#include <tamtypes.h>
#include "trace.h"

#include "elfData.h"
#include "texture.h"
#include "lump.h"
#include "draw.h"
#include "menu.h"
#include "pad.h"
#include "state.h"

MenuDef languageSelectMenuData[6] =
{
    {"English", 0, 0}, {"Fran\xe7" "ais", 0, 0},
    {"Deutsch", 0, 0},
    {"Espa\xF1" "ol", 0, 0},
    {"Italiano", 0, 0},
    {nullptr, 0, 0}
};

int showLanguageMenu()
{
    TextureHeader* langmenuTex = (TextureHeader*)findLmpEntry("langmenu.lmp", "langmenu.tex");
    drawOpaqueSprite(langmenuTex, 0, 0, 1, 0);
    TextureHeader* menuLongTex = (TextureHeader*)findLmpEntry("hud.lmp", "menulong.tex");

    drawMenu(0x140, 0x182, languageSelectMenuData, 0x80464646, 0x80408080, (Font*)menuFont, menuLongTex);

    updateSelectedMenuItem(languageSelectMenuData, -1);

    int rval = 0;
    if (((convertedControllerInput[0].updatedButtons | convertedControllerInput[1].updatedButtons) & DA_PAD_CROSS) == DA_PAD_CROSS) {
        int selIdx = getSelectedMenuItem(languageSelectMenuData);
        switch (selIdx) {
        case 0:
            selectedLanguageId = 0;
            break;
        case 1:
            selectedLanguageId = 3;
            break;
        case 2:
            selectedLanguageId = 4;
            break;
        case 3:
            selectedLanguageId = 1;
            break;
        case 4:
            selectedLanguageId = 2;
        }
        rval = 1;
    }
    return rval;
}
