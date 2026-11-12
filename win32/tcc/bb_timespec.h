#ifndef BB_TCC_TIMESPEC
#define BB_TCC_TIMESPEC
struct timespec {
#if ENABLE_MINGW_TIME32
	__time64_t tv_sec;
#else
	time_t tv_sec;
#endif
	long tv_nsec;
};
#endif
