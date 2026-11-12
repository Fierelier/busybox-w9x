#ifndef BB_TCC_SHELLAPI_H
#define BB_TCC_SHELLAPI_H
#include_next <shellapi.h>
#ifndef SEE_MASK_DEFAULT
#define SEE_MASK_DEFAULT 0x0
#define SEE_MASK_NOCLOSEPROCESS 0x40
#define SEE_MASK_NO_CONSOLE 0x8000
typedef struct _SHELLEXECUTEINFOA {
	DWORD cbSize;
	ULONG fMask;
	HWND hwnd;
	LPCSTR lpVerb;
	LPCSTR lpFile;
	LPCSTR lpParameters;
	LPCSTR lpDirectory;
	int nShow;
	HINSTANCE hInstApp;
	void *lpIDList;
	LPCSTR lpClass;
	HKEY hkeyClass;
	DWORD dwHotKey;
	union {
		HANDLE hIcon;
		HANDLE hMonitor;
	};
	HANDLE hProcess;
} SHELLEXECUTEINFOA, *LPSHELLEXECUTEINFOA;
typedef SHELLEXECUTEINFOA SHELLEXECUTEINFO;
typedef LPSHELLEXECUTEINFOA LPSHELLEXECUTEINFO;
BOOL WINAPI ShellExecuteExA(SHELLEXECUTEINFOA *);
#define ShellExecuteEx ShellExecuteExA
#endif
#endif
