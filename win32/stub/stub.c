#include <windows.h>

static char *skip_arg0(char *p)
{
	if (*p == '"') {
		p++;
		while (*p && *p != '"')
			p++;
		if (*p)
			p++;
	} else {
		while (*p && *p != ' ' && *p != '\t')
			p++;
	}
	while (*p == ' ' || *p == '\t')
		p++;
	return p;
}

static int len_of(const char *s)
{
	int n = 0;
	while (s[n])
		n++;
	return n;
}

void stub_main(void)
{
	char self[MAX_PATH];
	char *cmd;
	char *rest;
	char *name;
	char *app;
	char *dot;
	char *w;
	DWORD n;
	DWORD code = 255;
	int i;
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	WIN32_FIND_DATAA fd;
	HANDLE h;
	n = GetModuleFileNameA(NULL, self, sizeof(self));
	if (!n || n >= sizeof(self))
		ExitProcess(255);
	self[n] = 0;
	name = self;
	for (i = 0; self[i]; i++)
		if (self[i] == '\\' || self[i] == '/')
			name = self + i + 1;
	app = name;
	h = FindFirstFileA(self, &fd);
	if (h != INVALID_HANDLE_VALUE) {
		FindClose(h);
		app = fd.cFileName;
	}
	dot = 0;
	for (i = 0; app[i]; i++) {
		if (app[i] >= 'A' && app[i] <= 'Z')
			app[i] += 'a' - 'A';
		if (app[i] == '.')
			dot = app + i;
	}
	if (dot)
		*dot = 0;
	rest = skip_arg0(GetCommandLineA());
	cmd = (char *)HeapAlloc(GetProcessHeap(), 0, (SIZE_T)(len_of(self) + len_of(app) + len_of(rest) + 32));
	if (!cmd)
		ExitProcess(255);
	w = cmd;
	*w++ = '"';
	for (i = 0; self + i < name; i++)
		*w++ = self[i];
	for (i = 0; "busybox.exe"[i]; i++)
		*w++ = "busybox.exe"[i];
	*w++ = '"';
	*w++ = ' ';
	for (i = 0; app[i]; i++)
		*w++ = app[i];
	if (*rest) {
		*w++ = ' ';
		for (i = 0; rest[i]; i++)
			*w++ = rest[i];
	}
	*w = 0;
	for (i = 0; i < (int)sizeof(si); i++)
		((char *)&si)[i] = 0;
	si.cb = sizeof(si);
	SetConsoleCtrlHandler(NULL, TRUE);
	if (!CreateProcessA(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
		ExitProcess(255);
	WaitForSingleObject(pi.hProcess, INFINITE);
	GetExitCodeProcess(pi.hProcess, &code);
	ExitProcess(code);
}
