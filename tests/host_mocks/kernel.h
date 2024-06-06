#pragma once
#include "tamtypes.h"

#define NULL nullptr

#define UNCACHED_SEG(x) (void *)(x)
#define UCAB_SEG(x) (void *)(x)

inline void EE_SYNC(){}
inline void ExitHandler(){}

inline void EI(){}
inline void DI(){}

inline void iSignalSema(int id){}

inline void EnableDmac(int id){}
inline void EnableIntc(int id){}

inline void WaitSema(int id){}

inline void FlushCache(int id){}

struct ee_sema_t
{
    int count;
    int max_count;
    int init_count;
    int wait_threads;
    u32 attr;
    u32 option;
} ;

inline int CreateSema(ee_sema_t*) {return 1;}

inline s32 AddIntcHandler(s32 cause, s32 (*handler_func)(s32 cause), s32 next) {return 1;}
inline s32 AddDmacHandler(s32 channel, s32 (*handler)(s32 channel), s32 next) {return 1;}

inline void GsPutIMR(int ){}
inline void WR_EE_GIF_MODE(int){}
inline void WR_EE_GS_IMR(int){}