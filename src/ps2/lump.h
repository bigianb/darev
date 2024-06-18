#pragma once

#include <tamtypes.h>

// Specifies a sub-file in a lump.
struct LmpDirEntry
{
    char name[56];
    // Serialised as an offset but converted to a pointer on load.
    u8 *payload;     // 0x38
    int len;        // 0x3C
};

// A lump file is a simple directory. Each entry points to a sub-file.
struct LmpDir
{
    int numEntries;
    LmpDirEntry entries[];
};

u8* getLmp(const char* lmpName);
void setLmpGeneration(int gen);
u8* findLmpEntry(const char *lmpName, const char *entryName);
LmpDirEntry * searchLmpForNthFileWithExt(u8 *lmpDataUnaligned, const char *ext, int n);
