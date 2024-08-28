#include <string.h>

#include "model.h"
#include "../vector.h"
#include "../matrix.h"
#include "vifMesh.h"
#include "display.h"
#include "dlist.h"
#include "draw.h"

int vif_ITOP = 0;
u8 modelAphaFix = 0;

Matrix_4x4 camXformMat;

class AnimStateData;

struct DmaPacketPtrs
{
    u64* tail;    // where we're writing
    u64* dmaHead; // The most recent DMA tag
    u64* gifHead; // The most recent DIRECT tag
};

void setVifProjMatrix(DmaPacketPtrs* pDmaPacketPtrs, Matrix_4x4* matrix, Vec3* vec, VifData* vifData, char alphaFix)
{

    // u64 DMATag
    // u32 VIFcode0

    pDmaPacketPtrs->dmaHead = pDmaPacketPtrs->tail;
    pDmaPacketPtrs->tail += 1;

    u32* pDma = (u32*)pDmaPacketPtrs->tail;
    int idx = 0;
    pDma[idx++] = 0x11000000; // FLUSH

    bool hasAlpha2 = (vifData->flags & 0x10) == 0x10;

    // Direct 5 or 6 qwords
    pDmaPacketPtrs->gifHead = (u64*)&pDma[idx];
    pDma[idx++] = hasAlpha2 ? 0x50000006 : 0x50000005;

    pDma[idx++] = 0x8000;
    pDma[idx++] = 0x10000000;
    pDma[idx++] = 0xe; // A+D Reg
    pDma[idx++] = 0;

    pDma[idx++] = 0;
    pDma[idx++] = 0;
    pDma[idx++] = GSReg::TEXFLUSH;
    pDma[idx++] = 0;

    pDma[idx++] = 0;
    pDma[idx++] = 0;
    pDma[idx++] = GSReg::TEX0_1;
    pDma[idx++] = 0;

    if (alphaFix == 0) {
        pDma[idx++] = vifData->alphaRegLo;
        pDma[idx++] = vifData->alphaRegHi;
    } else {
        pDma[idx++] = 0x64;
        pDma[idx++] = 0x80 - alphaFix;
    }
    pDma[idx++] = GSReg::ALPHA_1;
    pDma[idx++] = 0;

    if ((vifData->flags & VifFlags::HAS_ALPHA2) != 0) {
        pDma[idx++] = 0x64;
        pDma[idx++] = vifData->alpha2FixVal;
        pDma[idx++] = GSReg::ALPHA_2;
        pDma[idx++] = 0;
    }

    pDmaPacketPtrs->tail = (u64*)&pDma[idx];

    pDma[idx++] = 0x507f5;
    pDma[idx++] = 0;
    pDma[idx++] = GSReg::TEST_1;
    pDma[idx++] = 0;

    // End of Direct data

    // UNPACK addr = 0, unsigned, add VIF1_TOPS, num = 4
    // m = 0, vn = 3, vl =0 .. V4-32
    pDma[idx++] = 0x6c04c000;

    float* pMatrixDest = (float*)&pDma[idx];
    memcpy(pMatrixDest, matrix->cell, sizeof(float) * 16);
    idx += 16;

    pDma[idx++] = vif_ITOP | 0x04000000; // ITOP
    pDma[idx++] = 0x14000002;            // MSCAL 02 - set proj matrix

    /*
        *pDmaTailHead = (ulong *)((int)puVar6 + 0x5c);
      puVar1 = (undefined *)((int)&vec->y + 3);
      uVar2 = (uint)puVar1 & 7;
      uVar3 = (uint)vec & 7;
      auStack_30._0_8_ =
           (*(long *)(puVar1 + -uVar2) << (7 - uVar2) * 8 |
           (uVar5 | 0x4000000) & 0xffffffffffffffffU >> (uVar2 + 1) * 8) & -1L << (8 - uVar3) * 8 |
           *(ulong *)((int)vec - uVar3) >> uVar3 * 8;
      fVar7 = vec->z;
      puVar1 = (undefined *)((int)&auStack_30.y + 3);

      uVar2 = (uint)puVar1 & 7;
      puVar6 = (ulong *)(puVar1 + -uVar2);
      *puVar6 = *puVar6 & -1L << (uVar2 + 1) * 8 | (ulong)auStack_30._0_8_ >> (7 - uVar2) * 8;
      auStack_30.z = fVar7;
      FUN_ram_00106ee8((uint32_t **)pDmaTailHead, &auStack_30);

      puVar6 = *pDmaTailHead;
    */

    // write 0 padding until qword boundary.
    u32* pTail = &pDma[idx];
    while ((u32)pTail & 0x0f) {
        *pTail++ = 0;
    }

    pDmaPacketPtrs->tail = (u64*)pTail;
    // write the dma tag length (cnt)
    int numqwc = pDmaPacketPtrs->tail - pDmaPacketPtrs->dmaHead - 1;
    *(u32*)pDmaPacketPtrs->dmaHead = numqwc | 0x10000000;
}

DlistNode*
drawAnimatedModel(VifData* pVif, TextureHeader* pTex, int dmaSlot, Vec3* pos, Matrix_3x4* mtx_3x4,
                  AnimStateData* animData, u64 meshDrawMask, DlistNode* headNode)
{
    DmaPacketPtrs dmaPacketPtrs;

    dmaPacketPtrs.tail = curDMABufTail;

    Matrix_4x4 projMtx;
    matrixMul3x4_4x4(&projMtx, mtx_3x4, &camXformMat);
    setVifProjMatrix(&dmaPacketPtrs, &projMtx, pos, pVif, modelAphaFix);

    /*
      puVar15 = dmaPacketPtrs.tail;
      if (animModelDrawFlags != 0) {
        dmaPacketPtrs.dmaHead = dmaPacketPtrs.tail;
        for (puVar12 = dmaPacketPtrs.tail + 1; ((uint)puVar12 & 0xf) != 0xc;
            puVar12 = (ulong *)((int)puVar12 + 1)) {
          *(undefined *)puVar12 = 0;
        }
        *(uint *)puVar12 = 0x50000000;
        dmaPacketPtrs.gifHead = (ulong *)((int)puVar12 + 4);
        *(undefined8 *)((int)puVar12 + 4) = 0x1000000000008000;
        dmaPacketPtrs.tail = (ulong *)((int)puVar12 + 0x14);
        *(undefined8 *)((int)puVar12 + 0xc) = 0xe;
        if ((animModelDrawFlags & 1) != 0) {
          dmaPacketPtrs.tail = (ulong *)((int)puVar12 + 0x34);
          *(undefined8 *)((int)puVar12 + 0x14) = zbuf_val;
          *(undefined8 *)((int)puVar12 + 0x1c) = 0x4e;
          uVar6 = zbuf_val;
          *(undefined8 *)((int)puVar12 + 0x2c) = 0x4f;
          *(undefined8 *)((int)puVar12 + 0x24) = uVar6;
        }
        if ((animModelDrawFlags & 8) != 0) {
                        // TEST_1
          dmaPacketPtrs.tail[1] = 0x47;
          dmaPacketPtrs.tail[2] = 0x52011;
          dmaPacketPtrs.tail[3] = 0x48;
          *dmaPacketPtrs.tail = 0x52011;
          dmaPacketPtrs.tail = dmaPacketPtrs.tail + 4;
        }
        if ((animModelDrawFlags & 2) != 0) {
          *dmaPacketPtrs.tail = 0x1212303012123030;
          dmaPacketPtrs.tail[1] = 0x44;
          dmaPacketPtrs.tail = dmaPacketPtrs.tail + 2;
        }
        if ((animModelDrawFlags & 4) != 0) {
          uVar10 = (long)scissor_x0 | (long)scissor_y1 << 0x30 |
                   (long)scissor_y0 << 0x20 | (long)scissor_x1 << 0x10;
          dmaPacketPtrs.tail[1] = 0x40;
          dmaPacketPtrs.tail[2] = uVar10;
          dmaPacketPtrs.tail[3] = 0x41;
          *dmaPacketPtrs.tail = uVar10;
          dmaPacketPtrs.tail = dmaPacketPtrs.tail + 4;
        }
        *dmaPacketPtrs.gifHead =
             *dmaPacketPtrs.gifHead | (long)(((int)dmaPacketPtrs.tail - (int)dmaPacketPtrs.gifHead >> 3) + -2) >> 1;
        *(uint *)puVar12 = *(uint *)puVar12 | (int)dmaPacketPtrs.tail + (-4 - (int)puVar12) >> 4;
        *puVar15 = (long)(((int)dmaPacketPtrs.tail - (int)puVar15 >> 3) + -2 >> 1) | 0x10000000;
      }
      */

    u64 mscal_val;
    if (animData == NULL) {
        mscal_val = 0x1400001e;
    } else {
        if (headNode == nullptr) {
            //writeAnimVifTags((byte **)dmaTailHead, animData);
        }
        mscal_val = 0x1400000c;
    }

    dmaPacketPtrs.dmaHead = dmaPacketPtrs.tail;
    dmaPacketPtrs.tail[0] = 0x10000000;
    dmaPacketPtrs.tail[1] = mscal_val;
    dmaPacketPtrs.tail += 2;

    /*
    if (((pVif->flags & VifFlags::FLAG_20) != 0) && (0 < DAT_ram_00324610)) {
        FUN_ram_0013c4b8(dmaTailHead, pVif, '\x01', (int)PTR_DAT_ram_00238938_ram_0032460c, DAT_ram_00324610);
    }
    */

    /*
    if ((pVif->flags & VifFlags::FLAG_40) != 0) {
        FUN_ram_0013d000(pVif, animData, mtx_3x4);
    }
    */

    /*
        lVar14 = (long)pVif->numChanges;
        lVar11 = (long)*(int *)&pVif->field_0x28;
        lVar13 = 0;
        uVar10 = meshDrawMask & ~(1L << lVar14);
        puVar15 = dmaPacketPtrs.tail;
        puVar12 = dmaPacketPtrs.tail;
        if (-1 < lVar14) {
            do {
                puVar12 = puVar15;
                if ((((uint)uVar10 ^ 1) & 1) != 0) {
                    iVar5 = *(int *)(pVif->pad + (int)lVar13 * 4 + 0x15);
                    if (lVar11 != iVar5) {
                        puVar15[1] = 0;
                        puVar12 = puVar15 + 2;
                        *puVar15 = lVar11 << 0x20 | (long)(iVar5 - (int)lVar11 >> 4) | 0x30000000U;
                        dmaPacketPtrs.tail = puVar12;
                        dmaPacketPtrs.dmaHead = puVar15;
                    }
                    lVar11 = (long)*(int *)(pVif->pad + (int)lVar13 * 4 + 0x15 + 4);
                }
                lVar13 = (long)((int)lVar13 + 1);
                uVar10 = (long)uVar10 >> 1;
                puVar15 = puVar12;
            } while (lVar13 <= lVar14);
        }
    */

    /*
      puVar9 = (uint *)((int)puVar12 + 0xc);
      puVar15 = puVar12;
      if (animModelDrawFlags != 0) {
        *(undefined **)(puVar12 + 1) = &DAT_ram_11000000;
        for (; ((uint)puVar9 & 0xf) != 0xc; puVar9 = (uint *)((int)puVar9 + 1)) {
          *(undefined *)puVar9 = 0;
        }
        *puVar9 = 0x50000000;     // DIRECT
        dmaPacketPtrs.gifHead = (ulong *)(puVar9 + 1);
        *(undefined8 *)(puVar9 + 1) = 0x1000000000008000;

        dmaPacketPtrs.tail = (ulong *)(puVar9 + 5);
        *(undefined8 *)(puVar9 + 3) = 0xe;
        if ((animModelDrawFlags & 1) != 0) {
          dmaPacketPtrs.tail = (ulong *)(puVar9 + 0xd);
          *(undefined8 *)(puVar9 + 5) = zbuf_val;
          *(undefined8 *)(puVar9 + 7) = 0x4e;
          uVar6 = zbuf_val;
          *(undefined8 *)(puVar9 + 0xb) = 0x4f;
          *(undefined8 *)(puVar9 + 9) = uVar6;
        }
        if ((animModelDrawFlags & 8) != 0) {
          *dmaPacketPtrs.tail = 0x50013;
          dmaPacketPtrs.tail[1] = 0x48;
          dmaPacketPtrs.tail = dmaPacketPtrs.tail + 2;
        }
        if ((animModelDrawFlags & 2) != 0) {
          *dmaPacketPtrs.tail = 0x3131757531317575;
          dmaPacketPtrs.tail[1] = 0x44;
          dmaPacketPtrs.tail = dmaPacketPtrs.tail + 2;
        }
        if ((animModelDrawFlags & 4) != 0) {
          uVar10 = (long)scissorY1 << 0x30 | (long)scissorY0 << 0x20 | 0x4ff0000U;
          dmaPacketPtrs.tail[1] = 0x40;
          dmaPacketPtrs.tail[2] = uVar10;
          dmaPacketPtrs.tail[3] = 0x41;
          *dmaPacketPtrs.tail = uVar10;
          dmaPacketPtrs.tail = dmaPacketPtrs.tail + 4;
        }
        *dmaPacketPtrs.gifHead =
             *dmaPacketPtrs.gifHead | (long)(((int)dmaPacketPtrs.tail - (int)dmaPacketPtrs.gifHead >> 3) + -2) >> 1;
        *puVar9 = *puVar9 | (int)dmaPacketPtrs.tail + (-4 - (int)puVar9) >> 4;
        *puVar12 = (long)(((int)dmaPacketPtrs.tail - (int)puVar12 >> 3) + -2 >> 1) | 0x10000000;
        puVar15 = dmaPacketPtrs.tail;
        dmaPacketPtrs.dmaHead = puVar12;
      }
    */

    u64* originalTail = curDMABufTail;

    // write the dma tag length (cnt)
    int numqwc = dmaPacketPtrs.tail - dmaPacketPtrs.dmaHead - 1;
    *(u32*)dmaPacketPtrs.dmaHead = numqwc | 0x10000000;

    curDMABufTail = dmaPacketPtrs.tail;

    return queueDMA(originalTail, dmaSlot, pTex, headNode, false);
}
