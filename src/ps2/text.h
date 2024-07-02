#pragma once

#include <tamtypes.h>

extern float textBrightness[2];

class Font;

void beginText(Font* font, int slot, int append, u32 color);
void flushText();
void setTextColor(u32 color);

u32 scaleColor(float scale, u32 color);

void atowcs(u16 *dest, const char *src);
void displayTextCentered(int xCenter, int ypos, const char* text);
void displayTextCenteredW(int xCenter, int ypos, u16* text, int maxChars);

void displayText(int xpos, int ypos, const char* text);
void displayTextW(int x, int y, u16* text, int maxChars);

void displayFormattedText(int xpos, int ypos, const char* format, ...);
