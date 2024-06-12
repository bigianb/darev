#include "texture.h"
#include "TexDecoder.h"
#include "trace.h"

void deinterlace(TextureHeader* texHeader)
{
    if(texHeader->qwc == 0x2a5 ||       // HUD:menulong.tex
    texHeader->qwc == 0x5052            // LANGMENU:langmenu.tex
    ){    // menu long

        TexDecoder decoder;
        Texture* tex = decoder.decode(texHeader);
        TexDecoder::deinterlace(tex);

        encode(texHeader, tex);
        delete tex;
    }
}

// encodes the data in tex to the texHeader, replacing what currently exists there.
// Used for de-interlacing. The encoded data must not be any larger than what currently exists.
void encode(TextureHeader* texHeader, Texture* tex)
{
    if(tex->palette && tex->palette->numEntries == 256){
        // 0x460 is BITBLTBUF GIFTAG
        /*
        Looking at menushort.tex as an example

        w = 0x53
        h = 0x26
        qwc = 0x13d
        flags = 0x0844 (langmenu is 0x0044)
        
        0x460 (0x4E0):
             GIFTAG: nloop: 1, eop: true, pre: false, prim: 0x00000000, flg: PACKED, nreg: 1, regs: 14
              A+D, addr: BITBLTBUF:
                                SBP = 0, SBW = 0, SPSM = 0
                                DBP = 0, DBW = 1, DPSM = 0
        0x480:
            GIFTAG: nloop: 3, eop: true, pre: false, prim: 0x00000000, flg: PACKED, nreg: 1, regs: 14
              A+D, addr: TRXREG:
                                RRH = 20, RRW = 48
              A+D, addr: TRXPOS:
                                SSAX = 0, SSAY = 0
                                DSAX = 0, DSAY = 0
              A+D, addr: TRXDIR: 0

        0x4C0:
            GIFTAG: nloop: 240, eop: true, pre: false, prim: 0x00000000, flg: IMAGE, 

        */
       
       // we need to update the SPSM, DPSM and DBW because we're not going to pre-swizzle at the moment
       // we need to update the RRH and RRW as they are for the notional 32 bit source
       // we need to update the nloop in the IMAGE
       // we need to update the qwc value in the texture header
    
        unsigned char * madr = (unsigned char *)texHeader->gif_madr_val;

        unsigned char* pBitBltBufVal = madr + 0x470;

        int dbw = (texHeader->width +63) / 64;

        pBitBltBufVal[7] = 0x13;        // DPSM - PSMT8
        pBitBltBufVal[6] = dbw;         // DBW, pixels
        pBitBltBufVal[3] = 0x13;        // SPSM - PSMT8

        unsigned char* pTrxRegVal = pBitBltBufVal + 0x20;

        unsigned short rrw = tex->widthPixels;
        unsigned short rrh = tex->heightPixels;

        *(unsigned short*)(pTrxRegVal) = rrw;
        *(unsigned short*)(pTrxRegVal+4) = rrh;

        const int pixLen = rrw * rrh;

        const int imageNloop = (pixLen + 15) / 16;
        const int padding = imageNloop * 16 - pixLen;

        unsigned char* const pImageTag = pTrxRegVal + 0x30;
        *(unsigned short*)(pImageTag) = (unsigned short)((imageNloop & 0x7FFF) | 0x8000);

        u8* pDest = pImageTag + 0x10;
        u32* pixels32 = (u32*)tex->data;
        for (int i=0; i<pixLen; ++i){
            pDest[i] = tex->palette->lookup(pixels32[i]);
        }
        for (int i=pixLen; i<pixLen + padding; ++i){
            pDest[i] = 0;
        }

        // update the qwc in the header 0x4D0 + image data bytes
        texHeader->qwc = 0x4D + imageNloop;
    }
}
