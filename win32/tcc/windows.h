#ifndef BB_TCC_WINDOWS_H
#define BB_TCC_WINDOWS_H
#include_next <windows.h>
#ifndef __MSABI_LONG
#define __MSABI_LONG(x) x##l
#endif
#include <shellapi.h>
#ifndef _NTDEF_
typedef LONG NTSTATUS, *PNTSTATUS;
#endif
typedef struct _TOKEN_ELEVATION {
	DWORD TokenIsElevated;
} TOKEN_ELEVATION, *PTOKEN_ELEVATION;
#define TokenElevation ((TOKEN_INFORMATION_CLASS)20)
#ifndef IO_REPARSE_TAG_SYMLINK
#define IO_REPARSE_TAG_SYMLINK 0xA000000CL
#endif
#ifndef IO_REPARSE_TAG_APPEXECLINK
#define IO_REPARSE_TAG_APPEXECLINK 0x8000001BL
#endif
#ifndef FSCTL_GET_REPARSE_POINT
#define FSCTL_SET_REPARSE_POINT 0x000900a4
#define FSCTL_GET_REPARSE_POINT 0x000900a8
#define FSCTL_SET_SPARSE 0x000900c4
#define FSCTL_SET_ZERO_DATA 0x000980c8
typedef struct _FILE_ZERO_DATA_INFORMATION {
	LARGE_INTEGER FileOffset;
	LARGE_INTEGER BeyondFinalZero;
} FILE_ZERO_DATA_INFORMATION, *PFILE_ZERO_DATA_INFORMATION;
#endif
typedef struct _REPARSE_DATA_BUFFER {
	ULONG ReparseTag;
	USHORT ReparseDataLength;
	USHORT Reserved;
	union {
		struct {
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			ULONG Flags;
			WCHAR PathBuffer[1];
		} SymbolicLinkReparseBuffer;
		struct {
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			WCHAR PathBuffer[1];
		} MountPointReparseBuffer;
		struct {
			UCHAR DataBuffer[1];
		} GenericReparseBuffer;
	};
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;
#define REPARSE_DATA_BUFFER_HEADER_SIZE FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer)
#ifndef FILE_NAME_NORMALIZED
#define FILE_NAME_NORMALIZED 0x0
#endif
#ifndef VOLUME_NAME_DOS
#define VOLUME_NAME_DOS 0x0
#endif
#define FindExInfoBasic ((FINDEX_INFO_LEVELS)1)
#ifndef PROCESSOR_ARCHITECTURE_ARM64
#define PROCESSOR_ARCHITECTURE_ARM64 12
#endif
typedef struct _CONSOLE_SCREEN_BUFFER_INFOEX {
	ULONG cbSize;
	COORD dwSize;
	COORD dwCursorPosition;
	WORD wAttributes;
	SMALL_RECT srWindow;
	COORD dwMaximumWindowSize;
	WORD wPopupAttributes;
	BOOL bFullscreenSupported;
	COLORREF ColorTable[16];
} CONSOLE_SCREEN_BUFFER_INFOEX, *PCONSOLE_SCREEN_BUFFER_INFOEX;
#endif
