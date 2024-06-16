#include "Texture.h"
#include "TexDecoder.h"
#include "DataUtil.h"
#include "GIFTag.h"
#include "Palette.h"

#include <iostream>


TexDecoder::TexDecoder()
{
    pixels = nullptr;
    pixelsLength = 0;
}

TexDecoder::~TexDecoder()
{
    // shouldn't ever really happen
    if (pixels){
        delete[] pixels;
    }
}

Texture* TexDecoder::decode(TextureHeader* header)
{
	int sourcew = header->width;
	int sourceh = header->height;
	pixels = nullptr;
	pixelsLength = 0;
    Palette* palette = nullptr;

    const unsigned char* pGifData = (unsigned char*)header->gif_madr_val;

    GIFTag gifTag;
    gifTag.parse(pGifData);

    // This is basically heuristics. Writing a full GIF parser is complex and as the texture files are written by a tool,
    // we can safely make some assumptions about their structure.
    if (gifTag.nloop == 4) {

        int palw = DataUtil::getLEShort(pGifData, 0x30);
        int palh = DataUtil::getLEShort(pGifData, 0x34);

        pGifData += 0x50;
        GIFTag gifTag2;
        gifTag2.parse(pGifData);

        // 8 bit palletised
        palette = new Palette();
        palette->read(pGifData + 0x10, palw, palh);
        palette->unswizzle();

        int palLen = palw * palh * 4;
        pGifData += (palLen + 0x10);

        //GIFTag* gifTag50 = new GIFTag();
        //gifTag50->parse(data, curIdx);
        pGifData += 0x20;

        int dbw = (sourcew / 2 + 0x07) & ~0x07;
        int dbh = (sourceh / 2 + 0x07) & ~0x07;

        
		int totalRrw = 0;
		bool eop = false;
		// Need to find a better way than this.
        while (!eop || totalRrw < dbw) {
            GIFTag* gifTag3 = new GIFTag();
            gifTag3->parse(pGifData);

            int dimOffset = 0x10;

            int thisRrw = DataUtil::getLEShort(pGifData, dimOffset);
            int thisRrh = DataUtil::getLEShort(pGifData, dimOffset + 4);

            int startx = DataUtil::getLEShort(pGifData, dimOffset + 20);
            int starty = DataUtil::getLEShort(pGifData, dimOffset + 22);

            pGifData += gifTag.nloop * 0x10 + 0x10;
            readPixels32(pGifData, palette, 0, startx, starty, thisRrw, thisRrh, dbw, dbh);
            pGifData += thisRrw * thisRrh * 4;

			totalRrw += thisRrw;
			eop = gifTag3->eop;
            delete gifTag3;
        }
        if (palLen != 64) {
            unswizzle8bpp(dbw * 2, dbh * 2);
            sourcew = dbw * 2;
            sourceh = dbh * 2;
        } else {
            sourcew = dbw;
            sourceh = dbh;
        }    

    } else if (gifTag.nloop == 3) {
        pGifData += 0xC0;
        GIFTag* gifTag2 = new GIFTag();
        gifTag2->parse(pGifData);

        if (gifTag2->flg == 2) {
            // image mode
            readPixels32(pGifData+0x10, header->width, header->height);
        } else {
            std::cerr << "unrecognised texture format";
        }
        delete gifTag2;
    } else {
        std::cerr << "unrecognised texture format";
    }
    Texture* texture = new Texture(header->width, header->height, sourcew, sourceh, pixels, pixelsLength, palette);
    pixels = nullptr;   // Texture owns it now.
    return texture;
}

void TexDecoder::unswizzle8bpp(int w, int h)
{
    unsigned char* unswizzled = new unsigned char[pixelsLength];

    int32_t* unswizzled32 = (int32_t*)unswizzled;
    int32_t* pixels32 = (int32_t*)pixels;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {

            int block_location = (y & (~0xf)) * w + (x & (~0xf)) * 2;
            int swap_selector = (((y + 2) >> 2) & 0x1) * 4;
            int posY = (((y & (~3)) >> 1) + (y & 1)) & 0x7;
            int column_location = posY * w * 2 + ((x + swap_selector) & 0x7) * 4;

            int byte_num = ((y >> 1) & 1) + ((x >> 2) & 2);     // 0,1,2,3

            int idx = block_location + column_location + byte_num;
            if (idx < pixelsLength) {
                unswizzled32[(y * w) + x] = pixels32[idx];
            }
        }
    }
    delete[] pixels;
    pixels = unswizzled;
}

void TexDecoder::readPixels32(const unsigned char* data, Palette* palette, int startOffset, int startx, int starty, int rrw, int rrh, int dbw, int dbh)
{
    if (palette->numEntries == 256) {
        int numDestPixels = dbh * dbw * 4;
        int widthPixels = dbw * 4;
        if (pixels == nullptr) {
			pixelsLength = numDestPixels*4;
            pixels = new unsigned char[pixelsLength];
        }
        int32_t* pixels32 = (int32_t*)pixels;
        int idx = startOffset;
        for (int y = 0; y < rrh && (y+starty) < dbh; ++y) {
            for (int x = 0; x < rrw; ++x) {
                int destIdx = (y+starty) * widthPixels + (x + startx) * 4;

                if (4*(destIdx + 4) >= pixelsLength){
                    //std::cerr << "bang!";
                } else {

                pixels32[destIdx++] = palette->getValue(data[idx++]);
                pixels32[destIdx++] = palette->getValue(data[idx++]);
                pixels32[destIdx++] = palette->getValue(data[idx++]);
                pixels32[destIdx] = palette->getValue(data[idx++]);
                }
            }
        }

    } else {
        int numDestPixels = rrh * dbw;
        if (pixels == nullptr) {
            pixelsLength = numDestPixels*4;
            pixels = new unsigned char[pixelsLength];
        }
        int idx = startOffset;
        bool lowbit = false;
        for (int y = 0; y < rrh; ++y) {
            for (int x = 0; x < rrw; ++x) {
                int destIdx = (y + starty) * dbw + x + startx;
                if (!lowbit) {
                    pixels[destIdx] = palette->getValue(data[idx] >> 4 & 0x0F);
                } else {
                    pixels[destIdx] = palette->getValue(data[idx++] & 0x0F);
                }
                lowbit = !lowbit;
            }
        }
    }
}

    
void TexDecoder::readPixels32(const unsigned char* data, int w, int h)
{
    int numDestPixels = w * h;
    if (pixels == nullptr) {
        pixelsLength = numDestPixels*4;
        pixels = new unsigned char[pixelsLength];
    }
    memcpy(pixels, data, numDestPixels*4);
}


void TexDecoder::deinterlace(Texture* texture)
{
    const int scanlen = texture->widthPixels * 4;
    unsigned char* deinterlaced = new unsigned char[scanlen * texture->heightPixels];

    int centre = texture->logicalHeight / 2;
    unsigned char* dest = deinterlaced;
    for (int row = 0; row < texture->logicalHeight; ++row)
    {
        int srcRow = row / 2;
        if (row & 1) {
            srcRow += centre;
        }
        const unsigned char* src = texture->data + srcRow * scanlen;
        memcpy(dest, src, scanlen);
        dest += scanlen;
    }

    delete[] texture->data;
    texture->data = deinterlaced;
}
