#ifndef BB_TCC_TIME_H
#define BB_TCC_TIME_H
#define localtime_r bb_tcc_crt_localtime_r
#include_next <time.h>
#undef localtime_r
#endif
