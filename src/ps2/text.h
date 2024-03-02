#pragma once

#include <tamtypes.h>

extern float textBrightness;

class Font;

void beginText(Font* font, int slot, int append, u32 color);
void flushText();
void setTextColor(u32 color);

u32 scaleColor(float scale, u32 color);

void atowcs(u16 *dest, const char *src);
void displayTextCenteredW(int xCenter, int ypos, u16* text, int maxChars);
