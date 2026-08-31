#include "libbb.h"
#include "lazyload.h"

int FAST_FUNC inet_aton(const char *cp, struct in_addr *inp)
{
	unsigned long val = inet_addr(cp);

	if (val == INADDR_NONE)
		return 0;
	inp->S_un.S_addr = val;
	return 1;
}

void init_winsock(void)
{
	WSADATA wsa;
	static int initialized = 0;

	if (initialized)
		return;

	if (WSAStartup(MAKEWORD(2,2), &wsa))
		bb_error_msg_and_die("WSAStartup failed, error %d", WSAGetLastError());

	atexit((void(*)(void)) WSACleanup);
	initialized = 1;
}

#undef gethostname
int FAST_FUNC mingw_gethostname(char *name, int namelen)
{
	init_winsock();
	return gethostname(name, namelen);
}

#undef gethostbyaddr
struct hostent * FAST_FUNC
mingw_gethostbyaddr(const void *addr, socklen_t len, int type)
{
	init_winsock();
	return gethostbyaddr(addr, len, type);
}

#undef getaddrinfo
static int getaddrinfo_via_gethostbyname(const char *node, const char *service,
			const struct addrinfo *hints, struct addrinfo **res)
{
	struct hostent *he;
	struct addrinfo *head = NULL, *tail = NULL;
	int family = hints ? hints->ai_family : AF_UNSPEC;
	int i;

	/* classic Winsock has no IPv6 stack, nothing to emulate it with.
	 * AF_UNSPEC falls through to the IPv4 lookup. */
	if (family == AF_INET6)
		return EAI_FAMILY;
	/* none of busybox's 3 getaddrinfo call sites ever pass a service -
	 * they set the port separately afterward. Keep the emulation to what
	 * is actually exercised rather than a full port/service parser. */
	if (service)
		return EAI_SERVICE;

	he = gethostbyname(node);
	if (!he || he->h_addrtype != AF_INET || !he->h_addr_list[0])
		return EAI_NONAME;

	for (i = 0; he->h_addr_list[i]; i++) {
		struct addrinfo *ai = xzalloc(sizeof(*ai));
		struct sockaddr_in *sin = xzalloc(sizeof(*sin));

		sin->sin_family = AF_INET;
		memcpy(&sin->sin_addr, he->h_addr_list[i], sizeof(sin->sin_addr));
		ai->ai_family = AF_INET;
		ai->ai_socktype = hints ? hints->ai_socktype : SOCK_STREAM;
		ai->ai_addrlen = sizeof(*sin);
		ai->ai_addr = (struct sockaddr *)sin;
		/* ai_canonname deliberately left NULL: no caller reads it
		 * (the one spot that would is commented out upstream) */
		if (tail)
			tail->ai_next = ai;
		else
			head = ai;
		tail = ai;
	}
	if (!head)
		return EAI_NONAME;
	*res = head;
	return 0;
}

int FAST_FUNC mingw_getaddrinfo(const char *node, const char *service,
				const struct addrinfo *hints, struct addrinfo **res)
{
	DECLARE_PROC_ADDR(int, getaddrinfo, const char *, const char *,
			const struct addrinfo *, struct addrinfo **);

	init_winsock();
	if (INIT_PROC_ADDR(ws2_32.dll, getaddrinfo))
		return getaddrinfo(node, service, hints, res);
	return getaddrinfo_via_gethostbyname(node, service, hints, res);
}

#undef freeaddrinfo
void FAST_FUNC mingw_freeaddrinfo(struct addrinfo *res)
{
	DECLARE_PROC_ADDR(void, freeaddrinfo, struct addrinfo *);

	if (INIT_PROC_ADDR(ws2_32.dll, freeaddrinfo)) {
		freeaddrinfo(res);
		return;
	}
	while (res) {
		struct addrinfo *next = res->ai_next;
		free(res->ai_addr);
		free(res);
		res = next;
	}
}

#undef getnameinfo
int FAST_FUNC mingw_getnameinfo(const struct sockaddr *sa, socklen_t salen,
			char *host, socklen_t hostlen,
			char *serv, socklen_t servlen, int flags)
{
	DECLARE_PROC_ADDR(int, getnameinfo, const struct sockaddr *, socklen_t,
			char *, socklen_t, char *, socklen_t, int);
	const struct sockaddr_in *sin;
	struct hostent *he;

	if (INIT_PROC_ADDR(ws2_32.dll, getnameinfo))
		return getnameinfo(sa, salen, host, hostlen, serv, servlen, flags);

	/* same ceiling as getaddrinfo above - IPv4 only, no service-name
	 * lookup (callers already pass NI_NUMERICSERV). */
	if (sa->sa_family != AF_INET)
		return EAI_FAMILY;
	sin = (const struct sockaddr_in *)sa;

	if (host && hostlen) {
		he = (flags & NI_NUMERICHOST) ? NULL : gethostbyaddr(
				(const char *)&sin->sin_addr, sizeof(sin->sin_addr), AF_INET);
		if (he && he->h_name)
			safe_strncpy(host, he->h_name, hostlen);
		else if (flags & NI_NAMEREQD)
			return EAI_NONAME;
		else
			safe_strncpy(host, inet_ntoa(sin->sin_addr), hostlen);
	}
	if (serv && servlen)
		snprintf(serv, servlen, "%u", ntohs(sin->sin_port));
	return 0;
}

int FAST_FUNC mingw_socket(int domain, int type, int protocol)
{
	int sockfd;
	SOCKET s;

	init_winsock();
	s = WSASocket(domain, type, protocol, NULL, 0, 0);
	if (s == INVALID_SOCKET) {
		/*
		 * WSAGetLastError() values are regular BSD error codes
		 * biased by WSABASEERR.
		 * However, strerror() does not know about networking
		 * specific errors, which are values beginning at 38 or so.
		 * Therefore, we choose to leave the biased error code
		 * in errno so that _if_ someone looks up the code somewhere,
		 * then it is at least the number that are usually listed.
		 */
		errno = WSAGetLastError();
		return -1;
	}
	/* convert into a file descriptor */
	if ((sockfd = _open_osfhandle((intptr_t)s, O_RDWR|O_BINARY)) < 0) {
		closesocket(s);
		bb_error_msg("unable to make a socket file descriptor: %s",
			     strerror(errno));
		return -1;
	}
	return sockfd;
}

#undef connect
int FAST_FUNC mingw_connect(int sockfd, const struct sockaddr *sa, size_t sz)
{
	SOCKET s = (SOCKET)_get_osfhandle(sockfd);
	return connect(s, sa, sz);
}

#undef bind
int FAST_FUNC mingw_bind(int sockfd, struct sockaddr *sa, size_t sz)
{
	SOCKET s = (SOCKET)_get_osfhandle(sockfd);
	return bind(s, sa, sz);
}

#undef setsockopt
int FAST_FUNC
mingw_setsockopt(int sockfd, int lvl, int optname, void *optval, int optlen)
{
	SOCKET s = (SOCKET)_get_osfhandle(sockfd);
	return setsockopt(s, lvl, optname, (const char*)optval, optlen);
}

#undef shutdown
int FAST_FUNC mingw_shutdown(int sockfd, int how)
{
	SOCKET s = (SOCKET)_get_osfhandle(sockfd);
	return shutdown(s, how);
}

#undef listen
int FAST_FUNC mingw_listen(int sockfd, int backlog)
{
	SOCKET s = (SOCKET)_get_osfhandle(sockfd);
	return listen(s, backlog);
}

#undef accept
int FAST_FUNC mingw_accept(int sockfd1, struct sockaddr *sa, socklen_t *sz)
{
	int sockfd2;

	SOCKET s1 = (SOCKET)_get_osfhandle(sockfd1);
	SOCKET s2 = accept(s1, sa, sz);

	/* convert into a file descriptor */
	if ((sockfd2 = _open_osfhandle((intptr_t)s2, O_RDWR|O_BINARY)) < 0) {
		int err = errno;
		closesocket(s2);
		bb_error_msg("unable to make a socket file descriptor: %s",
			strerror(err));
		return -1;
	}
	return sockfd2;
}

#undef getpeername
int FAST_FUNC mingw_getpeername(int fd, struct sockaddr *sa, socklen_t *sz)
{
	SOCKET sock;

	init_winsock();
	sock = (SOCKET)_get_osfhandle(fd);
	if (sock == INVALID_SOCKET) {
		errno = EBADF;
		return -1;
	}
	return getpeername(sock, sa, sz);
}
