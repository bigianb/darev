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
  u64* tail;      // where we're writing
  u64* dmaHead;   // The most recent DMA tag
  u64* gifHead;   // The most recent DIRECT tag
};

void setVifProjMatrix(DmaPacketPtrs *pDmaPacketPtrs, Matrix_4x4 *matrix,Vec3 *vec,VifData *vifData, char alphaFix)
{  

    // u64 DMATag
    // u32 VIFcode0

    pDmaPacketPtrs->dmaHead = pDmaPacketPtrs->tail;
    pDmaPacketPtrs->tail += 1;

    u32 *pDma = (u32 *)pDmaPacketPtrs->tail;
    int idx=0;
    pDma[idx++] = 0x11000000;     // FLUSH

    bool hasAlpha2 = (vifData->flags & 0x10) == 0x10;

    // Direct 5 or 6 qwords
    pDmaPacketPtrs->gifHead = (u64*)&pDma[idx];
    pDma[idx++] = hasAlpha2 ?  0x50000006 : 0x50000005;
    
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

    pDma[idx++] = vif_ITOP | 0x04000000;  // ITOP    
    pDma[idx++] = 0x14000002;   // MSCAL 02 - set proj matrix


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
    while ((u32)pTail & 0x0f){
        *pTail++ = 0;
    }

    pDmaPacketPtrs->tail = (u64*)pTail;
    // write the dma tag length (cnt)
    int numqwc = pDmaPacketPtrs->tail - pDmaPacketPtrs->dmaHead - 1;
    *(u32*)pDmaPacketPtrs->dmaHead = numqwc | 0x10000000;
}

DlistNode *
drawAnimatedModel(VifData *pVif, TextureHeader *pTex, int dmaSlot, Vec3 *pos, Matrix_3x4 *mtx_3x4,
                 AnimStateData *animData, u64 meshDrawMask, DlistNode* headNode)
{  
    DmaPacketPtrs dmaPacketPtrs;

/*
    puVar1 = (undefined *)((int)&pos->y + 3);
    uVar2 = (uint)puVar1 & 7;
    uVar3 = (uint)pos & 7;
    localPos._0_8_ =
        (*(long *)(puVar1 + -uVar2) << (7 - uVar2) * 8 |
        in_v1 & 0xffffffffffffffffU >> (uVar2 + 1) * 8) & -1L << (8 - uVar3) * 8 |
        *(ulong *)((int)pos - uVar3) >> uVar3 * 8;
    fVar4 = pos->z;
    puVar1 = (undefined *)((int)&localPos.y + 3);
    uVar2 = (uint)puVar1 & 7;
    puVar15 = (ulong *)(puVar1 + -uVar2);

    *puVar15 = *puVar15 & -1L << (uVar2 + 1) * 8 | (ulong)localPos._0_8_ >> (7 - uVar2) * 8;
    localPos.z = fVar4;
*/
    dmaPacketPtrs.tail = curDMABufTail;
    
    Matrix_4x4 projMtx;
    matrixMul3x4_4x4(&projMtx, mtx_3x4, &camXformMat);
    setVifProjMatrix(&dmaPacketPtrs, &projMtx, pos, pVif, modelAphaFix);
  
/*
  puVar15 = dmaTailHead[0];
  if (animModelDrawFlags != 0) {
    dmaTailHead[1] = dmaTailHead[0];
    for (puVar12 = dmaTailHead[0] + 1; ((uint)puVar12 & 0xf) != 0xc;
        puVar12 = (ulong *)((int)puVar12 + 1)) {
      *(undefined *)puVar12 = 0;
    }
    *(uint *)puVar12 = 0x50000000;
    dmaTailHead[2] = (ulong *)((int)puVar12 + 4);
    *(undefined8 *)((int)puVar12 + 4) = 0x1000000000008000;
    uVar2 = animModelDrawFlags;
    dmaTailHead[0] = (ulong *)((int)puVar12 + 0x14);
    *(undefined8 *)((int)puVar12 + 0xc) = 0xe;
    if ((uVar2 & 1) != 0) {
      dmaTailHead[0] = (ulong *)((int)puVar12 + 0x34);
      *(undefined8 *)((int)puVar12 + 0x14) = zbuf_val;
      *(undefined8 *)((int)puVar12 + 0x1c) = 0x4e;
      uVar6 = zbuf_val;
      *(undefined8 *)((int)puVar12 + 0x2c) = 0x4f;
      *(undefined8 *)((int)puVar12 + 0x24) = uVar6;
    }
    if ((uVar2 & 8) != 0) {
                    // TEST_1
      dmaTailHead[0][1] = 0x47;
      dmaTailHead[0][2] = 0x52011;
      dmaTailHead[0][3] = 0x48;
      *dmaTailHead[0] = 0x52011;
      dmaTailHead[0] = dmaTailHead[0] + 4;
    }
    if ((uVar2 & 2) != 0) {
      *dmaTailHead[0] = 0x1212303012123030;
      dmaTailHead[0][1] = 0x44;
      dmaTailHead[0] = dmaTailHead[0] + 2;
    }
    if ((uVar2 & 4) != 0) {
      uVar10 = (long)scissor_x0 | (long)scissor_y1 << 0x30 |
               (long)scissor_y0 << 0x20 | (long)scissor_x1 << 0x10;
      dmaTailHead[0][1] = 0x40;
      dmaTailHead[0][2] = uVar10;
      dmaTailHead[0][3] = 0x41;
      *dmaTailHead[0] = uVar10;
      dmaTailHead[0] = dmaTailHead[0] + 4;
    }
    *dmaTailHead[2] =
         *dmaTailHead[2] | (long)(((int)dmaTailHead[0] - (int)dmaTailHead[2] >> 3) + -2) >> 1;
    *(uint *)puVar12 = *(uint *)puVar12 | (int)dmaTailHead[0] + (-4 - (int)puVar12) >> 4;
    *puVar15 = (long)(((int)dmaTailHead[0] - (int)puVar15 >> 3) + -2 >> 1) | 0x10000000;
  }
  */
 u64 mscal_val;
  if (animData == NULL) {
    mscal_val = 0x1400001e;
  }
  else {
    if (headNode == nullptr) {
      //writeAnimVifTags((byte **)dmaTailHead, animData);
    }
    mscal_val = 0x1400000c;
  }

    dmaPacketPtrs.tail[0] = 0x10000000;
    dmaPacketPtrs.tail[1] = mscal_val;
    dmaPacketPtrs.dmaHead = dmaPacketPtrs.tail;
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
    uVar2 = animModelDrawFlags;
    lVar14 = (long)pVif->numChanges;
    lVar11 = (long)*(int *)&pVif->field_0x28;
    lVar13 = 0;
    uVar10 = meshDrawMask & ~(1L << lVar14);
    puVar15 = dmaTailHead[0];
    puVar12 = dmaTailHead[0];
    if (-1 < lVar14) {
        do {
            puVar12 = puVar15;
            if ((((uint)uVar10 ^ 1) & 1) != 0) {
                iVar5 = *(int *)(pVif->pad + (int)lVar13 * 4 + 0x15);
                if (lVar11 != iVar5) {
                    puVar15[1] = 0;
                    puVar12 = puVar15 + 2;
                    *puVar15 = lVar11 << 0x20 | (long)(iVar5 - (int)lVar11 >> 4) | 0x30000000U;
                    dmaTailHead[0] = puVar12;
                    dmaTailHead[1] = puVar15;
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
  if (uVar2 != 0) {
    *(undefined **)(puVar12 + 1) = &DAT_ram_11000000;
    for (; ((uint)puVar9 & 0xf) != 0xc; puVar9 = (uint *)((int)puVar9 + 1)) {
      *(undefined *)puVar9 = 0;
    }
    *puVar9 = 0x50000000;
    dmaTailHead[2] = (ulong *)(puVar9 + 1);
    *(undefined8 *)(puVar9 + 1) = 0x1000000000008000;
    uVar2 = animModelDrawFlags;
    dmaTailHead[0] = (ulong *)(puVar9 + 5);
    *(undefined8 *)(puVar9 + 3) = 0xe;
    if ((uVar2 & 1) != 0) {
      dmaTailHead[0] = (ulong *)(puVar9 + 0xd);
      *(undefined8 *)(puVar9 + 5) = zbuf_val;
      *(undefined8 *)(puVar9 + 7) = 0x4e;
      uVar6 = zbuf_val;
      *(undefined8 *)(puVar9 + 0xb) = 0x4f;
      *(undefined8 *)(puVar9 + 9) = uVar6;
    }
    if ((uVar2 & 8) != 0) {
      *dmaTailHead[0] = 0x50013;
      dmaTailHead[0][1] = 0x48;
      dmaTailHead[0] = dmaTailHead[0] + 2;
    }
    if ((uVar2 & 2) != 0) {
      *dmaTailHead[0] = 0x3131757531317575;
      dmaTailHead[0][1] = 0x44;
      dmaTailHead[0] = dmaTailHead[0] + 2;
    }
    if ((uVar2 & 4) != 0) {
      uVar10 = (long)scissorY1 << 0x30 | (long)scissorY0 << 0x20 | 0x4ff0000U;
      dmaTailHead[0][1] = 0x40;
      dmaTailHead[0][2] = uVar10;
      dmaTailHead[0][3] = 0x41;
      *dmaTailHead[0] = uVar10;
      dmaTailHead[0] = dmaTailHead[0] + 4;
    }
    *dmaTailHead[2] =
         *dmaTailHead[2] | (long)(((int)dmaTailHead[0] - (int)dmaTailHead[2] >> 3) + -2) >> 1;
    *puVar9 = *puVar9 | (int)dmaTailHead[0] + (-4 - (int)puVar9) >> 4;
    *puVar12 = (long)(((int)dmaTailHead[0] - (int)puVar12 >> 3) + -2 >> 1) | 0x10000000;
    puVar15 = dmaTailHead[0];
    dmaTailHead[1] = puVar12;
  }
*/


  u64* puVar12 = curDMABufTail;
/*
  uVar10 = (long)*dmaPacketPtrs.dmaHead >> 0x1c & 7;
  if (uVar10 == 3) {
    uVar10 = 0;
  }
  else if (uVar10 == 1) {
    uVar10 = 7;
  }

  *dmaPacketPtrs.dmaHead = *dmaPacketPtrs.dmaHead & 0xffffffff8fffffff | uVar10 << 0x1c;
  curDMABufTail = puVar15;
  */

  return queueDMA(puVar12, dmaSlot, pTex, headNode, false);

}







