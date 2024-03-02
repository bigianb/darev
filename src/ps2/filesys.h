#pragma once
#include <tamtypes.h>

void initCD();

void makeCDROMPath(char* pDest, const char* pSrc);
void readFile(const char* filename, u8* buf, u8* pStreamingDone, int size, int startOffset);
void dealWithCdStreaming();
void waitForFrame();

struct TextureHeader;
class Font;
void fixupTex(TextureHeader* texData);
void fixupFnt(Font* font);

extern u8 cdStreamingFlag;

struct fileISOLocationEntry
{
    char filename[16];
    int startOffset;
    int length;
};

extern fileISOLocationEntry fileISOLocTable[];

extern int fileISOLocTableSize;

// Set by loadLmp
extern u8 loadLmpStreamDone;
extern int streamLmpLen; 
extern u8* streamLmpBuf; 
extern int numLoadedLmps;
extern char streamLmpName[];

struct loadedLmpDirEntry
{
    char name[48];
    // File data is actually on a 256 byte boundary within this buffer.
    u8* fileDataUnaligned;
    int size;
    int vagSize;
    u8 generation;
    u8 pad[3];
};

extern loadedLmpDirEntry loadedLmpInfo[];
