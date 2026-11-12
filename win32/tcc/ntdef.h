#ifndef BB_TCC_NTDEF_H
#define BB_TCC_NTDEF_H
#include <windows.h>
#ifndef NT_SUCCESS
#define NT_SUCCESS(s) (((NTSTATUS)(s)) >= 0)
#endif
#endif
