#pragma once

#define CACHED_SEG(x) \
    ((void *)(((u32)(x)) & 0x0fffffff))
