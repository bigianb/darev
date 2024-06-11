#pragma once

#include <stdint.h>

class DataUtil
{
public:
	DataUtil(const unsigned char* p_in, int offset_in) : p(p_in), offset(offset_in)
	{
	}

	float getLEFloat()
	{
		const float f = *(float*)(p + offset);
		offset += 4;
		return f;
	}

	int getLEInt()
	{
		const int i = *(int32_t*)(p + offset);
		offset += 4;
		return i;
	}

	unsigned short getLEUShort()
	{
		const unsigned short s = *(uint16_t*)(p + offset);
		offset += 2;
		return s;
	}

	static int getLEInt(const unsigned char* data, int offset)
	{
		return *(int32_t *)(data + offset);
	}

    short getLEShort()
    {
        return (short)getLEUShort();
    }
    
	static short getLEShort(const unsigned char* data, int offset)
	{
		return *(int16_t *)(data + offset);
	}

	static unsigned short getLEUShort(const unsigned char* data, int offset)
	{
		return *(uint16_t *)(data + offset);
	}

	static float getLEFloat(const unsigned char* data, int offset)
	{
		return *(float*)(data + offset);
	}

private:
	const unsigned char* p;
	int offset;
};

