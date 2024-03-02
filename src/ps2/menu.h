#pragma once

struct MenuDef
{
    const char* menuText;
    int typeOrSelectedIdx;
    int value;
};

struct Font;
struct TextureHeader;

void drawMenu(int xpos, int ypos, MenuDef* menuDefs, u32 color, u32 colorSel, Font* font, TextureHeader* tex);

int updateSelectedMenuItem(MenuDef* menuDefs, int controllerPort);
int getSelectedMenuItem(MenuDef* menuDefs);
