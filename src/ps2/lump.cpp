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

void hunkFree(void *p)
{
    if (p != nullptr){
        free(p);
    }
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
            traceln("Found lmp %s", lmpName);
            pData = loadedLmpInfo[i].fileDataUnaligned;
        }
    }
    if (pData == nullptr) {
        traceln("start with %d loaded lumps", numLoadedLmps);
        loadLmp(lmpName);
        bool loggedWait = false;
        while (cdStreamingFlag != 0) {
            if (!loggedWait) {
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
    if (pData == nullptr) {
        traceln("Failed to read %s", lmpName);
    }
    return pData;
}

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

int lmpCleanupCount = 1;

void lmpCleanup(int minGen)
{
    if (numLoadedLmps <= 0) {
        return;
    }

    //DAT_ram_00448480 = 0;
    //cleanupVags(minLmpGen);

    int gen = loadedLmpInfo[numLoadedLmps - 1].generation;
    while (numLoadedLmps > 0 && gen <= minGen) {
        freeLmpData(loadedLmpInfo[numLoadedLmps - 1].fileDataUnaligned);
        gen = loadedLmpInfo[numLoadedLmps - 1].generation;
    }
    ++lmpCleanupCount;
}

void freeLmpData(u8* unalignedData)
{
    LmpDir* lmpDir = (LmpDir*)((u32)(unalignedData + 0xff) & 0xffffff00);
    int lmpIdx = 0;

    while ((lmpIdx < numLoadedLmps && (loadedLmpInfo[lmpIdx].fileDataUnaligned != unalignedData))) {
        lmpIdx += 1;
    }
    if (lmpIdx == numLoadedLmps) {
        // not found
        return;
    }

    if (lmpDir != nullptr) {
        int entryNum = 0;
        while (entryNum < lmpDir->numEntries) {
            const char* dotPos = strrchr(lmpDir->entries[entryNum].name, '.');
            if (dotPos != nullptr) {
                const char* ext = dotPos + 1;
                if (strcmp(ext, "tex") == 0) {
                    //        resetTexData((TextureHeader*)lmpDir->entries[entryNum].payload);
                } else if (strcmp(ext, "world") == 0) {
                    //        FUN_ram_001466b8((WorldData*)lmpDir->entries[entryNum].payload);
                } else if (strcmp(ext, "uni") == 0) {
                    //        FUN_ram_0013b4e0((short*)lmpDir->entries[entryNum].payload);
                }
            }
            ++entryNum;
        }
    }

    // number of element beyond lmpIdx
    int finalLmpIdx = numLoadedLmps-1;
    int numToCopy = finalLmpIdx - lmpIdx;

    if (numToCopy > 0){
        // copy lmpIdx + 1 to lmpIdx to close the gap.
        void* pDest = &loadedLmpInfo[lmpIdx];
        void* pSrc = &loadedLmpInfo[lmpIdx+1];
        memmove(pDest, pSrc, numToCopy * sizeof(lmpDir->entries[0]));
    }
    --numLoadedLmps;
    hunkFree(unalignedData);
}


u8* findLmpEntry(const char* lmpName, const char* entryName)
{
    u8* unalignedData = getLmp(lmpName);
    if (unalignedData == nullptr) {
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

LmpDirEntry* searchLmpForNthFileWithExt(u8* lmpDataUnaligned, const char* ext, int n)
{
    // LmpDir is aligned to a 256 byte boundary
    LmpDir* lmpDir = (LmpDir*)((u32)(lmpDataUnaligned + 0xff) & 0xffffff00);

    int numFound = -1;
    int i = 0;
    while (i < lmpDir->numEntries && numFound != n) {
        const char* pDot = strrchr(lmpDir->entries[i].name, '.');
        if (pDot != nullptr) {
            if (0 == strcmp(pDot + 1, ext)) {
                ++numFound;
            }
        }
        ++i;
    }
    LmpDirEntry* entry = nullptr;
    if (numFound == n) {
        entry = &lmpDir->entries[i - 1];
    }
    return entry;
}
