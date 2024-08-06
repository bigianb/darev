#pragma once

#include <tamtypes.h>

#include "../vector.h"
#include "../matrix.h"

extern int vif_ITOP;

class AnimStateData;
struct DlistNode;
struct VifData;
struct TextureHeader;

DlistNode *
drawAnimatedModel(VifData *pVif, TextureHeader *pTex, int dmaSlot, Vec3 *pos, Matrix_3x4 *mtx_3x4,
                 AnimStateData *animData, u64 meshDrawMask, DlistNode* headNode);
