#pragma once

inline void setVif1_CHCR(int val)
{
    //EE_SYNC();
    //*(vu32*)0x10009000 = val;
}

inline void setVif1_tadr(void* val)
{
    //*(vu32*)0x10009030 = (u32)val;
}

inline void setGif_CHCR(int val)
{
    //EE_SYNC();
    //*(vu32*)0x1000A000 = val;
}

inline void setGif_madr(void* val)
{
    //*(vu32*)0x1000A010 = (u32)val;
}

inline void setGif_qwc(u32 val)
{
    //*(vu32*)0x1000A020 = val;
}

inline void setGif_tadr(void* val)
{
    //*(vu32*)0x1000A030 = (u32)val;
}
