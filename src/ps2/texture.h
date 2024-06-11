#pragma once
#include <tamtypes.h>

struct DlistNode;
struct GSAllocInfo;

struct TextureHeader
{
    short width;
    short height;
    short requiredGSMem;
    unsigned short qwc;
    unsigned short flags;
    short unk2;
    int tbw;
    void * gif_madr_val;
    void * gif_tadr_val;
    GSAllocInfo * gsAllocInfo;
    int resetFrameNo;
    DlistNode * slotNodes[8];
};

void deinterlace(TextureHeader* texData);

class Texture
{
public:
    Texture(int logw, int logh, int w, int h, unsigned char* dataIn, int dataLengthIn)
    {
		logicalWidth = logw;
		logicalHeight = logh;
        widthPixels = w;
        heightPixels = h;
        dataLength = dataLengthIn;
        data = dataIn;
    }

    ~Texture()
    {
        delete data;
    }

	int logicalWidth;
	int logicalHeight;

    int widthPixels;
    int heightPixels;

    int dataLength;
    unsigned char* data;
};
