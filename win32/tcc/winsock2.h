#ifndef BB_TCC_WINSOCK2_H
#define BB_TCC_WINSOCK2_H
#pragma push_macro("POLLRDNORM")
#undef POLLRDNORM
#pragma push_macro("POLLRDBAND")
#undef POLLRDBAND
#pragma push_macro("POLLIN")
#undef POLLIN
#pragma push_macro("POLLPRI")
#undef POLLPRI
#pragma push_macro("POLLWRNORM")
#undef POLLWRNORM
#pragma push_macro("POLLOUT")
#undef POLLOUT
#pragma push_macro("POLLWRBAND")
#undef POLLWRBAND
#pragma push_macro("POLLERR")
#undef POLLERR
#pragma push_macro("POLLHUP")
#undef POLLHUP
#pragma push_macro("POLLNVAL")
#undef POLLNVAL
#define pollfd bb_tcc_wsapollfd
#include_next <winsock2.h>
#undef pollfd
#undef POLLRDNORM
#pragma pop_macro("POLLRDNORM")
#undef POLLRDBAND
#pragma pop_macro("POLLRDBAND")
#undef POLLIN
#pragma pop_macro("POLLIN")
#undef POLLPRI
#pragma pop_macro("POLLPRI")
#undef POLLWRNORM
#pragma pop_macro("POLLWRNORM")
#undef POLLOUT
#pragma pop_macro("POLLOUT")
#undef POLLWRBAND
#pragma pop_macro("POLLWRBAND")
#undef POLLERR
#pragma pop_macro("POLLERR")
#undef POLLHUP
#pragma pop_macro("POLLHUP")
#undef POLLNVAL
#pragma pop_macro("POLLNVAL")
typedef struct _WSAMSG {
	LPSOCKADDR name;
	INT namelen;
	LPWSABUF lpBuffers;
	DWORD dwBufferCount;
	WSABUF Control;
	DWORD dwFlags;
} WSAMSG, *PWSAMSG, *LPWSAMSG;
#ifndef IPV6_V6ONLY
#define IPV6_V6ONLY 27
#endif
#endif
