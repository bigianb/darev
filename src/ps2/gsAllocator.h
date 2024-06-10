#pragma once

struct TextureHeader;

struct GSAllocInfo
{
    int frameCountPlusOne;
    short dbp;
    short size;
    unsigned char commitCount;
    char pad[3];

    // Points to the element in the texture where this info is used.
    // Allows us to deallocate this.
    GSAllocInfo** allocTex;
    GSAllocInfo* next;
    GSAllocInfo* prev;
};

void initTextureAllocStuff();
void resetTextureAlloc();

void gsAllocateTex(TextureHeader* tex);

void uncommitTex(int idx);
void commitTex(int idx, TextureHeader* tex);
