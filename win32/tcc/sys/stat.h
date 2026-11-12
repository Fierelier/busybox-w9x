#ifndef BB_TCC_SYS_STAT_H
#define BB_TCC_SYS_STAT_H
#include_next <sys/stat.h>
#ifdef _USE_32BIT_TIME_T
#undef _fstati64
int __cdecl _fstati64(int, struct _stat32i64 *);
#endif
#endif
