#pragma once
#include <ia32.hpp>

#define ERROR_SUCCESS 0xE0000420
#define ERROR_FAIL    0xE0000421


typedef struct _DriverData {

	PVOID kernelBase;

}DriverData, *PDriverData;

extern DriverData driverData;

typedef struct _RegisterNotifyBuffer {
	BOOLEAN Enable;
	PVOID   HookPoint;
	PVOID   Buffer;
	LARGE_INTEGER Cookie;

}RegisterNotifyBuffer, *PRegisterNotifyBuffer;

extern RegisterNotifyBuffer registerNotifyBuffer;

NTSTATUS RegisterNotifyInit(BOOLEAN Enable);