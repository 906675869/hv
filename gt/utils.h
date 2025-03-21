#pragma once
#include "NtStruct.h"

<<<<<<< HEAD
typedef struct _AddressRegion {
    PVOID start;
    PVOID end;
}AddressRegion, * AR;

=======
>>>>>>> 3997a5215897bb8ad32895dd437e341d953aac6c
UINT32 GetTextHashA(char* Str);

UINT GetTextHashW(wchar_t* Str);

<<<<<<< HEAD
ULONG_PTR GetModuleBaseByHashW(ULONG pid, UINT32 moduleHash);

ULONG_PTR GetModuleBase(ULONG pid, PCWSTR moduleName);

PVOID SearchSignForImage(PVOID ImageBase, CHAR* Pattern, CHAR* Mask, unsigned long MaskLen);

PVOID GetKernelBase(PDRIVER_OBJECT driver_object);

NTSTATUS GetDriverTextRegion(PDRIVER_OBJECT DriverObject, AR region);

=======
>>>>>>> 3997a5215897bb8ad32895dd437e341d953aac6c
PVOID GetKernelFunction(PCWSTR fName);

NTSTATUS RtlForceDeleteFile(PUNICODE_STRING pFilePath);

inline UINT32 GetTextHashA(char* Str) {

    UINT32 Hash = NULL;

    while (Str != NULL && *Str) {

        Hash = (UINT32)(65599 * (Hash + (*Str++) + (*Str > 64 && *Str < 91 ? 32 : 0)));
    }

    return Hash;
}

inline UINT32 GetTextHashW(wchar_t* Str) {

    UINT32 Hash = NULL;

    while (Str != NULL && *Str) {

        Hash = (UINT32)(65599 * (Hash + (*Str++) + (*Str > 64 && *Str < 91 ? 32 : 0)));
    }

    return Hash;
}

inline PVOID GetKernelFunction(PCWSTR fName) {
    UNICODE_STRING fNameUStr;
    RtlInitUnicodeString(&fNameUStr, fName);
    PVOID result = MmGetSystemRoutineAddress(&fNameUStr);
    if (!result || !MmIsAddressValid(result)) {
        DbgPrint("MmGetSystemRoutineAddress Get Address Fail %wZ", fNameUStr);
    }
    return result;
}
