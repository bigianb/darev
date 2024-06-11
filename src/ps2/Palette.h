#pragma once

#include <string.h>
#include <stdint.h>

class Palette
{
public:
    Palette()
    {
        rgbaData = nullptr;
        numEntries = 0;
    }

    ~Palette()
    {
        delete[] rgbaData;
    }

    // in ps2 0x80 is fully transparent and 0 is opaque.
    unsigned char* rgbaData;

    int numEntries;

    int32_t getValue(unsigned int idx)
    {
        return ((int32_t*)rgbaData)[idx];
    }

    void read(const unsigned char* data, int palw, int palh)
    {
        numEntries = palw * palh;
        rgbaData = new unsigned char[numEntries*4];
        memcpy(rgbaData, data, numEntries*4);
    }

    void unswizzle()
    {
        if (numEntries == 256) {
            unsigned char* unswizzled = new unsigned char[numEntries*4];

            int j = 0;
            for (int i = 0; i < 256; i += 32, j += 32) {
                copy(unswizzled, i, rgbaData, j, 8);
                copy(unswizzled, i + 16, rgbaData, j + 8, 8);
                copy(unswizzled, i + 8, rgbaData, j + 16, 8);
                copy(unswizzled, i + 24, rgbaData, j + 24, 8);
            }
            delete[] rgbaData;
            rgbaData = unswizzled;
        }
    }

    void copy(unsigned char* unswizzled, int i, unsigned char* swizzled, int j, int num)
    {
        int32_t* unswizzled32 = (int32_t *)unswizzled;
        int32_t* swizzled32 = (int32_t *)swizzled;
        for (int x = 0; x < num; ++x) {
            unswizzled32[i + x] = swizzled32[j + x];
        }
    }
};
