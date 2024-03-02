#pragma once

#include <tamtypes.h>
#include "display.h"

struct TextureHeader;

extern u64 dmaBuffers[2][20000];
extern u64* curDMABufHead;
extern u64* curDMABufTail;

void drawOpaqueSprite(TextureHeader* pTex, int xpos, int ypos, int slot, bool append);
void drawSprite(TextureHeader* pTexData, int xpos, int ypos, int slot, bool append, u32 vertexColor);

