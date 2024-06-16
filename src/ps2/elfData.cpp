#include <libcdvd.h>
#include <malloc.h>
#include <cstring>
#include "elfData.h"
#include "trace.h"

typedef struct
{
    u8 e_ident[16];
    u16 e_type;    // 0=NONE, 1=REL, 2=EXEC, 3=SHARED, 4=CORE
    u16 e_machine; // 8=MIPS R3000
    u32 e_version;
    u32 e_entry;     // Entry point address
    u32 e_phoff;     // Start of program headers (offset from file start)
    u32 e_shoff;     // Start of section headers (offset from file start)
    u32 e_flags;     // Processor specific flags = 0x20924001 noreorder, mips
    u16 e_ehsize;    // ELF header size (0x34 = 52 bytes)
    u16 e_phentsize; // Program headers entry size
    u16 e_phnum;     // Number of program headers
    u16 e_shentsize; // Section headers entry size
    u16 e_shnum;     // Number of section headers
    u16 e_shstrndx;  // Section header stringtable index
} ELF_HEADER;

typedef struct
{
    u32 p_type;   // 1=Load the segment into memory, no. of bytes specified by p_filesz and p_memsz
    u32 p_offset; // Offset from file start to program segment.
    u32 p_vaddr;  // Virtual address of the segment
    u32 p_paddr;  // Physical address of the segment
    u32 p_filesz; // Number of bytes in the file image of the segment
    u32 p_memsz;  // Number of bytes in the memory image of the segment
    u32 p_flags;  // Flags for segment
    u32 p_align;  // Alignment. The address of 0x08 and 0x0C must fit this alignment. 0=no alignment
} ELF_PROGRAM_HDR;

// 0x0023a970 -> 0x00245840

struct ElfFile
{
    u32 startAddr;
    u32 endAddr; // Exclusive
    void** bufferPtr;
};

u8* menuFont = nullptr;

ElfFile elfFiles[] = {
    {0x0023a970, 0x00245840, (void**)&menuFont}
    };

void findFile(ElfFile& ef, ELF_HEADER* elfHeader, ELF_PROGRAM_HDR* ph)
{
    for (int i = 0; i < elfHeader->e_phnum; i++) {
        if (ph[i].p_type == 1) {
            u32 segStart = ph[i].p_paddr;
            u32 segEnd = segStart + ph[i].p_filesz;
            if (ef.startAddr >= segStart && ef.endAddr < segEnd) {
                u32 offsetIntoSeg = ef.startAddr - segStart;
                memcpy((u8*)*ef.bufferPtr, (u8*)elfHeader + ph[i].p_offset + offsetIntoSeg, ef.endAddr - ef.startAddr);
            }
        }
    }
}

void readElfData()
{
    sceCdRMode ourCdRMode;
    ourCdRMode.trycount = 200;
    ourCdRMode.spindlctrl = 1;
    ourCdRMode.datapattern = 0;

    // Allocate memory first so not to fragment the heap when we deallocate the ELF file data.
    for (ElfFile& ef : elfFiles) {
        int lenBytes = ef.endAddr - ef.startAddr;
        *ef.bufferPtr = malloc(lenBytes);
    }

    sceCdlFILE file;
    int found = sceCdSearchFile(&file, "\\SLES_506.72;1");
    if (found) {
        u32 sizeBytes = file.size;
        u32 sizeSectors = (sizeBytes + 0x7ffU) >> 0xb;
        u8* readBuf = (u8*)malloc(sizeSectors * 2048 + 64);

        u8* alignedReadBuf = (u8*)(((u32)readBuf + 0x3f) & ~0x3f);
        sceCdRead(file.lsn, sizeSectors, alignedReadBuf, &ourCdRMode);
        sceCdSync(0);

        ELF_HEADER* elfHeader = (ELF_HEADER*)alignedReadBuf;
        ELF_PROGRAM_HDR* ph = (ELF_PROGRAM_HDR*)(alignedReadBuf + elfHeader->e_phoff);

        for (ElfFile& ef : elfFiles) {
            findFile(ef, elfHeader, ph);
        }

        free(readBuf);
    } else {
        traceln("Failed to find file SLES_506.72");
    }
}

void freeElfData()
{
    for (ElfFile& ef : elfFiles) {
        if (*ef.bufferPtr != nullptr) {
            free(*ef.bufferPtr);
            *ef.bufferPtr = nullptr;
        }
    }
}
