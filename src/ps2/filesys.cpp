#include <libcdvd.h>
#include <iopheap.h>
#include <stdint.h>
#include <tamtypes.h>
#include <ctype.h>
#include <cstring>

#include "trace.h"
#include "state.h"

#include "display.h"
#include "filesys.h"
#include "lump.h"
#include "texture.h"
#include "font.h"

sceCdRMode ourCdRMode;
u8* ptrStreamingDoneFlag = nullptr;

fileISOLocationEntry fileISOLocTable[256];

int fileISOLocTableSize;

void enumerateDirectory(int lsn, int numSectors)
{
    u8 readBuf[2048 + 64]; // TODO: align to 64 byte

    fileISOLocTableSize = 0;
    for (int secNo = 0; secNo < numSectors; ++secNo) {
        sceCdRead(lsn + secNo, 1, readBuf, &ourCdRMode);
        sceCdSync(0);
        u8* pRec = readBuf;
        while (*pRec) {
            u8 nameLen = pRec[0x20];
            u8* pSrc = pRec + 0x21;
            u8* pEnd = pRec + 0x20 + nameLen;

            u8* pDest = (u8*)fileISOLocTable[fileISOLocTableSize].filename;
            while (pSrc <= pEnd) {
                *pDest++ = *pSrc++;
            }
            *pDest = 0;

            traceln("ISO loc table entry %d is %s", fileISOLocTableSize, fileISOLocTable[fileISOLocTableSize].filename);

            fileISOLocTable[fileISOLocTableSize].startOffset = (pRec[5] << 24) | (pRec[4] << 16) | (pRec[3] << 8) | pRec[2];
            fileISOLocTable[fileISOLocTableSize].length = (pRec[0x0d] << 24) | (pRec[0x0c] << 16) | (pRec[0x0b] << 8) | pRec[0x0a];

            ++fileISOLocTableSize;

            pRec += *pRec;
        }
    }
    return;
}

void initCD(void)
{
    sceCdlFILE file;

    ourCdRMode.trycount = 200;
    ourCdRMode.spindlctrl = 1;
    ourCdRMode.datapattern = 0;

    int iVar1 = sceCdSearchFile(&file, "\\BG\\DATA");
    if (iVar1 == 0) {
        iVar1 = sceCdSearchFile(&file, "\\.");
        if (iVar1 != 0) {
            enumerateDirectory(file.lsn, (file.size + 0x7ffU) >> 0xb);
        }
    } else {
        enumerateDirectory(file.lsn, (file.size + 0x7ffU) >> 0xb);
    }
    // Alloc 1 sector more than necessary for alignment.
    char* iopBuf = (char*)SifAllocIopHeap(0x51 * 0x800);
    char* alignedIopBuf = (char*)(((unsigned int)iopBuf + 0x7ff) & 0xfffff800);
    // 0x50 sectors and 5 ring buffers
    sceCdStInit(0x50, 5, alignedIopBuf);
}

u8 cdStreamingFlag = 0;
u8 cdStreamingActive = 0;
u8 cdStreamingPaused = 0;
u8* cdStreamBuffer = nullptr;
int cdStreamSectorStart;
int cdReadType; // 1 is EE, 2 is IOP
int cdStreamNumSectors;

int cdBytesRemaingToStream = 0;
u32* cdStreamReadBuf = nullptr;

int streamLmpLen; 
u8* streamLmpBuf; 

int numLoadedLmps;  
char streamLmpName[80]; 

u8 loadLmpStreamDone = 0; 

u8* etexData = nullptr; 

// Guess.
#define MAX_LOADED_LUMPS 64

loadedLmpDirEntry loadedLmpInfo[MAX_LOADED_LUMPS]; 

void* offsetToPtr(void* offset, void* base)
{
    return (void*)((u32)offset + (u32)base);
}

void fixupTex(TextureHeader* texData, const char* lmpName, const char* entryName)
{
    if (texData->resetFrameNo == 0) {
        texData->resetFrameNo = -1;
        texData->gif_madr_val = offsetToPtr(texData->gif_madr_val, texData);
        if ((texData->flags & 0x100) != 0) {

            traceln("*** unsupported tex file ... maybe level tex");
            /*
            iVar8 = 0x48;
            uVar7 = 0x48;
            void * piVar5 = texData->gif_madr_val;
            *piVar5 = offsetToPtr(*piVar5, texData);
            piVar5 = piVar5 + 1;

            cVar4 = *(char*)piVar5;
            cVar2 = *(char*)piVar5;
            while (cVar2 != -1) {
                cVar2 = *(char*)((int)piVar5 + 2);
                pcVar1 = (char*)((int)piVar5 + 1);
                lVar6 = (long)*pcVar1;
                cVar3 = *(char*)((int)piVar5 + 3);
                piVar5 = piVar5 + 1;
                iVar8 = iVar8 + 6 +
                        (((int)cVar3 - (int)*pcVar1) + 1) * (((int)cVar2 - (int)cVar4) + 1) * 0x10;
                uVar7 = (ushort)iVar8;
                while (lVar6 <= cVar3) {
                    lVar6 = (long)((int)lVar6 + 1);
                    for (iVar9 = (int)cVar4; (long)iVar9 <= (long)cVar2; iVar9 += 1) {
                        *piVar5 = (int)texData->slotNodes + *piVar5 + -0x20;
                        piVar5 = piVar5 + 1;
                    }
                }
                cVar4 = *(char*)piVar5;
                cVar2 = *(char*)piVar5;
            }
            texData->qwc = uVar7;
            */
        } else if (isProgressiveModeDisplay()){
            traceln("lmpName = %s", lmpName);
            if (0 == strcmp(lmpName, "elf") || 0 == strcmp(lmpName, "langmenu.lmp") || 0 == strcmp(lmpName, "hud.lmp")){
                deinterlace(texData);
            }
        }
    }
    return;
}

void fixupFnt(Font* font, const char* lmpName, const char* entryName)
{
    font->glyphInfoArray = (GlyphInfo *)offsetToPtr(font->glyphInfoArray, font);
    font->kernArray = (GlyphKernPair*)offsetToPtr(font->kernArray, font);
    font->texture = (TextureHeader*)offsetToPtr(font->texture, font);

    fixupTex(font->texture, lmpName, entryName);
    return;
}

u32 fixupLmpData(u8* lmpFileData, char* lmpName)
{
    // lmp data is aligned to a 256 byte boundary
    LmpDir* lmp = (LmpDir*)(((u32)lmpFileData + 0xff) & 0xffffff00);

    int bytesRemoved = 0;
    u8* prevEntryPayload = nullptr;

    for (int entryIdx = 0; entryIdx < lmp->numEntries; ++entryIdx) {
        LmpDirEntry* pEntry = &lmp->entries[entryIdx];
        // convert offset to ptr
        pEntry->payload += (u32)lmp;

        if (prevEntryPayload == nullptr || prevEntryPayload < pEntry->payload) {
            if (bytesRemoved > 0) {
                // subfiles in a lump are in the same order as the lump directory entries.
                // If we have removed

                for (int nextEntryIdx = entryIdx + 1; nextEntryIdx < lmp->numEntries; ++nextEntryIdx) {
                    LmpDirEntry* pNextEntry = &lmp->entries[nextEntryIdx];
                    // Next entry offset hasn't yet been converted to a pointer
                    u8* ptrPayload = pNextEntry->payload + (u32)lmp;
                    if (ptrPayload == pEntry->payload) {
                        pNextEntry->payload -= bytesRemoved;
                    }
                }
                u8* originalPayload = pEntry->payload;
                pEntry->payload -= bytesRemoved;
                memcpy(pEntry->payload, originalPayload, pEntry->len);
            }

            // Remove NTSC files for PAL region.
            // Change the name to remove the ntsc_ or pal_ prefix
            int ntscMatch = strncmp(pEntry->name, "ntsc_", 5);
            int palMatch = strncmp(pEntry->name, "pal_", 4);

            bool removeFile = false;
            int nameLen = strlen(pEntry->name);
            if (ntscMatch == 0) {
                removeFile = true;
                memmove(pEntry->name, pEntry->name + 5, nameLen - 4); // no need to do this.
            }
            if (palMatch == 0) {
                memmove(pEntry->name, pEntry->name + 4, nameLen - 3);
            }

            // Remove any files which are not the correct language
            char* pSub = strstr(pEntry->name, ".uni");
            if (pSub != nullptr) {
                int langId = 0;
                if (nullptr != strstr(pEntry->name, "_sp.")) {
                    langId = 1;
                }
                if (langId != 0 && nullptr != strstr(pEntry->name, "_it.")) {
                    langId = 2;
                }
                if (langId != 0 && nullptr != strstr(pEntry->name, "_fr.")) {
                    langId = 3;
                }
                if (langId != 0 && nullptr != strstr(pEntry->name, "_ge.")) {
                    langId = 4;
                }
                removeFile = langId != selectedLanguageId;
            }

            prevEntryPayload = pEntry->payload;
            if (removeFile) {
                // remove an entry from the entries array. The actual payload that the entry points to is
                // removed on the next loop. There is nothing to do if we remove the last entry.
                bytesRemoved += pEntry->len & 0xffffff80;
                // Consider we have 3 entries and we remove the first one.
                // Then numEntries is 3, entryIdx is 0 and we want to copy 2 entries.
                int bytesToCopy = (lmp->numEntries - entryIdx - 1) * 0x40;
                memmove(pEntry, pEntry + 1, bytesToCopy);
                entryIdx -= 1;
                lmp->numEntries -= lmp->numEntries;
            } else {
                char* dotpos = strrchr(pEntry->name, '.');
                if (dotpos != nullptr) {
                    char* ext = dotpos + 1;

                    if (0 == strcmp(ext, "anm")) {
                        // TODO: fixupAnmPointers(pEntry->payload);
                    } else if (0 == strcmp(ext, "vif")) {
                        // TODO: fixupVif(pEntry->payload);
                    } else if (0 == strcmp(ext, "tex")) {
                        fixupTex((TextureHeader*)pEntry->payload, lmpName, pEntry->name);
                    } else if (0 == strcmp(ext, "fnt")) {
                        fixupFnt((Font*)pEntry->payload, lmpName, pEntry->name);
                    } else if (0 == strcmp(ext, "world")) {
                        // TODO: fixupWorld(pEntry->payload);
                    } else if (0 == strcmp(ext, "vag")) {
                        // TODO: fixupVag(pEntry->name, pEntry->payload);
                        // for vag files, remove the payload data but keep the entry
                        bytesRemoved += pEntry->len - 0x40;
                        bytesRemoved &= 0xffffff80;
                    } else if (0 == strcmp(ext, "uni")) {
                        // TODO: fixupUni(pEntry->payload);
                    } else if (0 == strcmp(ext, "etex")) {
                        // TODO: this will not survive a file removal
                        etexData = pEntry->payload;
                    }
                }
            }
        }
    }
    for (int entryIdx = 0; entryIdx < lmp->numEntries; ++entryIdx) {
        auto& entry = lmp->entries[entryIdx];
        char* dotpos = strrchr(entry.name, '.');
        if (dotpos != nullptr && (0 == strcmp(dotpos + 1, "anm"))) {
            // TODO: FUN_ram_00136318(entry.payload);
        }
    }
    return bytesRemoved;
}

int vagSpaceAvailable()
{
    // something to do with amount of free sound data
    return 0x20000;
}

void trimAllocatedSpace(u8* lmpData, int newLen)
{
}

void dealWithCdStreaming()
{
    if (cdStreamingFlag == 1) {
        bool done = false;
        if (cdStreamingActive == 0) {
            int isCompleted = sceCdSync(1); // non-blocking
            if (isCompleted == 0) {
                int iVar1 = sceCdGetError();
                if (iVar1 != SCECdErNO) {
                    if (cdReadType == 1) {
                        while (0 == sceCdRead(cdStreamSectorStart, cdStreamNumSectors, cdStreamBuffer, &ourCdRMode)) {}
                    } else if (cdReadType == 2) {
                        while (0 == sceCdReadIOPMem(cdStreamSectorStart, cdStreamNumSectors, cdStreamBuffer, &ourCdRMode)) {}
                    }
                } else {
                    // There was an error
                    done = true;
                }
            }
        } else {
            // streaming is active
            if (cdBytesRemaingToStream > 0) {
                u32 errVal = 0;
                int sectorsRead = sceCdStRead(cdBytesRemaingToStream >> 0xb, cdStreamReadBuf, 0, &errVal);
                cdBytesRemaingToStream -= sectorsRead * 0x800;
                cdStreamReadBuf += sectorsRead * 0x200;
                if (cdBytesRemaingToStream <= 0) {
                    done = true;
                }
            }
        }
        if (done) {
            *ptrStreamingDoneFlag = 1;
            cdStreamingFlag = 0;
        }
    }

    // If we were reading a Lmp file, fix up what we read and add to the cache.
    if ((streamLmpBuf != nullptr) && (loadLmpStreamDone != 0)) {
        int vagSpaceBefore = vagSpaceAvailable();

        int bytesRemoved = fixupLmpData(streamLmpBuf, streamLmpName);
        int bytesToKeep = streamLmpLen - (bytesRemoved - 0x100);
        trimAllocatedSpace(streamLmpBuf, bytesToKeep);
        loadedLmpInfo[numLoadedLmps].size = bytesToKeep;

        int vagSpaceAfter = vagSpaceAvailable();

        // vag size maybe
        loadedLmpInfo[numLoadedLmps].vagSize = vagSpaceBefore - vagSpaceAfter;
        loadedLmpInfo[numLoadedLmps].generation = 0;
        strcpy(loadedLmpInfo[numLoadedLmps].name, streamLmpName);
        loadedLmpInfo[numLoadedLmps].fileDataUnaligned = streamLmpBuf;

        streamLmpBuf = nullptr;
        loadLmpStreamDone = 0;
        numLoadedLmps = numLoadedLmps + 1;
    }

    return;
}

void waitForFrame()
{
}

void waitForCurStreamCompletion(void)
{
    if (cdStreamingFlag != 0) {
        do {
            dealWithCdStreaming();
            waitForFrame();
        } while (cdStreamingFlag != 0);
    }
    if (cdStreamingActive != 0) {
        sceCdStPause();
        cdStreamingActive = 0;
        cdStreamingPaused = 1;
    }
    return;
}

void makeCDROMPath(char* pDest, const char* pSrc)
{
    char c = *pSrc;
    while (c) {
        *pDest++ = toupper(c);
        ++pSrc;
        c = *pSrc;
    }
    *pDest++ = ';';
    *pDest++ = '1';
    *pDest++ = 0;
}

void readFile(const char* filename, u8* buf, u8* pStreamingDone, int size, int startOffset)
{
    waitForCurStreamCompletion();

    char cdFilename[80];
    makeCDROMPath(cdFilename, filename);

    int fileLocIdx;
    for (fileLocIdx = 0; fileLocIdx < fileISOLocTableSize; ++fileLocIdx) {
        //traceln("Considering '%s' when looking for '%s'", fileISOLocTable[fileLocIdx].filename, cdFilename);
        if (strcmp(cdFilename, fileISOLocTable[fileLocIdx].filename) == 0) {
            //traceln("found");
            break;
        }
    }

    if (fileLocIdx != fileISOLocTableSize) {
        int fileStartOffset = fileISOLocTable[fileLocIdx].startOffset;
        int fileLength = fileISOLocTable[fileLocIdx].length;

        if (size == 0) {
            size = fileLength;
        }
        ptrStreamingDoneFlag = pStreamingDone;
        *pStreamingDone = 0;
        cdStreamSectorStart = fileStartOffset + (startOffset >> 0xb);
        cdReadType = 1;
        cdStreamNumSectors = (size + 0x7ff) >> 0xb;
        cdStreamBuffer = buf;
        int rval;
        do {
            rval = sceCdRead(cdStreamSectorStart, cdStreamNumSectors, cdStreamBuffer, &ourCdRMode);
        } while (rval == 0);
        cdStreamingFlag = 1;
    } else {
        traceln("Can't find file '%s' in ISO table", cdFilename);
    }
    return;
}
