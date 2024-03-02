#pragma once

#include <cstdint>

typedef uint8_t u8;
typedef uint16_t u16;



typedef int32_t s32;
typedef uint32_t u32;
typedef uint64_t u64;

typedef volatile u8 vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;

static_assert(sizeof(u64) == 8, "expect u64 to be 64 bits");
