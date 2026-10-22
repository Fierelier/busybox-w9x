#include "libbb.h"
#include "lazyload.h"

#undef GetSystemTimeAsFileTime
#undef GetFileAttributesExA
#undef GetProcessTimes
#undef GetDiskFreeSpaceEx
#undef LockFileEx
#undef CreateRemoteThread

void compat_get_system_time_as_file_time(LPFILETIME ft)
{
	DECLARE_PROC_ADDR(void, GetSystemTimeAsFileTime, LPFILETIME);
	SYSTEMTIME st;

	if (INIT_PROC_ADDR(kernel32.dll, GetSystemTimeAsFileTime)) {
		GetSystemTimeAsFileTime(ft);
		return;
	}

	GetSystemTime(&st);
	SystemTimeToFileTime(&st, ft);
}

BOOL compat_get_file_attributes_ex_a(LPCSTR name, GET_FILEEX_INFO_LEVELS level,
		LPVOID info)
{
	DECLARE_PROC_ADDR(BOOL, GetFileAttributesExA, LPCSTR,
			GET_FILEEX_INFO_LEVELS, LPVOID);
	WIN32_FILE_ATTRIBUTE_DATA *data = (WIN32_FILE_ATTRIBUTE_DATA *)info;
	WIN32_FIND_DATAA fd;
	HANDLE h;
	DWORD attr;
	char buf[MAX_PATH];
	size_t len;

	if (INIT_PROC_ADDR(kernel32.dll, GetFileAttributesExA))
		return GetFileAttributesExA(name, level, info);

	if (level != GetFileExInfoStandard) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	len = strlen(name);
	if (len >= sizeof(buf)) {
		SetLastError(ERROR_FILENAME_EXCED_RANGE);
		return FALSE;
	}
	memcpy(buf, name, len + 1);
	while (len > 1 && (buf[len - 1] == '\\' || buf[len - 1] == '/') &&
			buf[len - 2] != ':') {
		buf[--len] = '\0';
	}

	memset(data, 0, sizeof(*data));
	h = FindFirstFileA(buf, &fd);
	if (h != INVALID_HANDLE_VALUE) {
		FindClose(h);
		data->dwFileAttributes = fd.dwFileAttributes;
		data->ftCreationTime = fd.ftCreationTime;
		data->ftLastAccessTime = fd.ftLastAccessTime;
		data->ftLastWriteTime = fd.ftLastWriteTime;
		data->nFileSizeHigh = fd.nFileSizeHigh;
		data->nFileSizeLow = fd.nFileSizeLow;
		return TRUE;
	}

	attr = GetFileAttributesA(buf);
	if (attr == INVALID_FILE_ATTRIBUTES)
		return FALSE;
	data->dwFileAttributes = attr;
	return TRUE;
}

BOOL compat_get_process_times(HANDLE process, LPFILETIME creation,
		LPFILETIME exit_time, LPFILETIME kernel, LPFILETIME user)
{
	DECLARE_PROC_ADDR(BOOL, GetProcessTimes, HANDLE, LPFILETIME, LPFILETIME,
			LPFILETIME, LPFILETIME);

	if (!INIT_PROC_ADDR(kernel32.dll, GetProcessTimes)) {
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return FALSE;
	}
	return GetProcessTimes(process, creation, exit_time, kernel, user);
}

BOOL compat_get_disk_free_space_ex_a(LPCSTR dir, PULARGE_INTEGER avail,
		PULARGE_INTEGER total, PULARGE_INTEGER total_free)
{
	DECLARE_PROC_ADDR(BOOL, GetDiskFreeSpaceExA, LPCSTR, PULARGE_INTEGER,
			PULARGE_INTEGER, PULARGE_INTEGER);
	DWORD sectors_per_cluster, bytes_per_sector, free_clusters, clusters;
	ULONGLONG cluster_bytes;

	if (INIT_PROC_ADDR(kernel32.dll, GetDiskFreeSpaceExA))
		return GetDiskFreeSpaceExA(dir, avail, total, total_free);

	if (!GetDiskFreeSpaceA(dir, &sectors_per_cluster, &bytes_per_sector,
			&free_clusters, &clusters))
		return FALSE;

	cluster_bytes = (ULONGLONG)sectors_per_cluster * bytes_per_sector;
	if (avail)
		avail->QuadPart = cluster_bytes * free_clusters;
	if (total)
		total->QuadPart = cluster_bytes * clusters;
	if (total_free)
		total_free->QuadPart = cluster_bytes * free_clusters;
	return TRUE;
}

BOOL compat_lock_file_ex(HANDLE h, DWORD flags, DWORD reserved, DWORD len_low,
		DWORD len_high, LPOVERLAPPED ovlp)
{
	DECLARE_PROC_ADDR(BOOL, LockFileEx, HANDLE, DWORD, DWORD, DWORD, DWORD,
			LPOVERLAPPED);

	if (INIT_PROC_ADDR(kernel32.dll, LockFileEx))
		return LockFileEx(h, flags, reserved, len_low, len_high, ovlp);

	for (;;) {
		if (LockFile(h, ovlp->Offset, ovlp->OffsetHigh, len_low, len_high))
			return TRUE;
		if ((flags & LOCKFILE_FAIL_IMMEDIATELY) ||
				GetLastError() != ERROR_LOCK_VIOLATION)
			return FALSE;
		Sleep(50);
	}
}

HANDLE compat_create_remote_thread(HANDLE process, LPSECURITY_ATTRIBUTES sa,
		SIZE_T stack, LPTHREAD_START_ROUTINE start, LPVOID arg, DWORD flags,
		LPDWORD id)
{
	DECLARE_PROC_ADDR(HANDLE, CreateRemoteThread, HANDLE,
			LPSECURITY_ATTRIBUTES, SIZE_T, LPTHREAD_START_ROUTINE, LPVOID,
			DWORD, LPDWORD);

	if (!INIT_PROC_ADDR(kernel32.dll, CreateRemoteThread)) {
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return NULL;
	}
	return CreateRemoteThread(process, sa, stack, start, arg, flags, id);
}
