#include <ctype.h>
#include <cstring>
#include <cstdlib>
#include "filesys.h"
#include "lump.h"
#include "trace.h"

void* allocMem(int size, const char* name)
{
    return malloc(size);
}

void doLoadLmp(const char* lmpName)
{
    char cdPath[80];
    makeCDROMPath(cdPath, lmpName);

    int fileLocIdx = 0;
    while (fileLocIdx < fileISOLocTableSize) {
        int cmp = strcmp(cdPath, fileISOLocTable[fileLocIdx].filename);
        if (cmp == 0) {
            break;
        }
        ++fileLocIdx;
    }

    if (fileLocIdx != fileISOLocTableSize) {

        int fileLen = fileISOLocTable[fileLocIdx].length;

        u8* buf = (u8*)allocMem(((fileLen + 0x7ffU) & 0xfffff800) | 0x100, lmpName);
        readFile(lmpName, (u8*)((u32)(buf + 0xff) & 0xffffff00), &loadLmpStreamDone, 0, 0);

        streamLmpLen = fileLen;
        streamLmpBuf = buf;
        strcpy(streamLmpName, lmpName);
        traceln("Start read %s", streamLmpName);
    }
}

// @pal: 13a1d8
void loadLmp(const char* lmpName)
{
    bool needLoad = true;
    for (int i = 0; i < numLoadedLmps; ++i) {
        if (strcmp(loadedLmpInfo[i].name, lmpName) == 0) {
            // found it
            if (loadedLmpInfo[i].fileDataUnaligned != nullptr) {
                needLoad = false;
            }
        }
    }
    if (needLoad) {
        doLoadLmp(lmpName);
    }
}

u8* getLmp(const char* lmpName)
{
    u8* pData = nullptr;
    for (int i = 0; i < numLoadedLmps && pData == nullptr; ++i) {
        if (0 == strcmp(loadedLmpInfo[i].name, lmpName)) {
            pData = loadedLmpInfo[i].fileDataUnaligned;
        }
    }
    if (pData == nullptr) {
        traceln("start with %d loaded lumps", numLoadedLmps);
        loadLmp(lmpName);
        bool loggedWait = false;
        while (cdStreamingFlag != 0) {
            if (!loggedWait){
                loggedWait = true;
                traceln("waiting");
            }
            dealWithCdStreaming();
            waitForFrame();
        }
        traceln("got %d loaded lumps", numLoadedLmps);
        for (int i = 0; i < numLoadedLmps && pData == nullptr; ++i) {
            if (0 == strcmp(loadedLmpInfo[i].name, lmpName)) {
                pData = loadedLmpInfo[i].fileDataUnaligned;
            }
        }
    }
    if (pData == nullptr){
        traceln("Failed to read %s", lmpName);
    }
    return pData;
}

// @pal: 139018
void setLmpGeneration(int gen)
{
    for (int i = 0; i < numLoadedLmps; ++i) {
        if (loadedLmpInfo[i].generation < gen) {
            loadedLmpInfo[i].generation = gen;
        }
    }
    /*
        for (int i=0; i < numVagLumps; ++i){
            if (vagLumps[i].generation < gen){
                vagLumps[i].generation = gen;
            }
        }
    */
    return;
}

// @pal: 13a848
u8* findLmpEntry(const char* lmpName, const char* entryName)
{
    u8* unalignedData = getLmp(lmpName);
    if (unalignedData == nullptr){
        return nullptr;
    }
    // LmpDir is aligned to a 256 byte boundary
    LmpDir* lmpDir = (LmpDir*)((u32)(unalignedData + 0xff) & 0xffffff00);

    u8* payload = nullptr;
    // names are in alphabetical order so could do a binary search for speed.
    for (int i = 0; i < lmpDir->numEntries && payload == nullptr; ++i) {
        if (0 == strcmp(lmpDir->entries[i].name, entryName)) {
            payload = lmpDir->entries[i].payload;
        }
    }
    return payload;
}
