#ifndef BB_TCC_SYS_TIMEB_H
#define BB_TCC_SYS_TIMEB_H
#define timespec bb_tcc_crt_timespec
#define itimerspec bb_tcc_crt_itimerspec
#include_next <sys/timeb.h>
#undef timespec
#undef itimerspec
#include "bb_timespec.h"
#endif
