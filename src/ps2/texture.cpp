#include "texture.h"
#include "trace.h"

void deinterlace(TextureHeader* tex)
{
        // we make some assumptions here:
        // 1. The data is 8 bit palettised
        // 2. The first IMAGE GIF tag is the palete
        // 3. The subsequent IMAGE blocks are vertical slices where there is more than one.

        // Just do menuLong for now
        if (tex->width == 238 && tex->height == 38){
                int tbw = tex->tbw & 0x3f;
                int psm = (tex->tbw >> 6)  & 0x3f;
                int tw = (tex->tbw >> 12)  & 0xf;
                int th = (tex->tbw >> 16)  & 0xf;
                int tfx = (tex->tbw >> 20)  & 0x3;
                traceln("tbw = 0x%x, psm = 0x%x, tw=0x%x, th=0x%x, tfx=0x%x, flags = 0x%x", tbw, psm, tw, th, tfx, tex->flags);
        }
}
