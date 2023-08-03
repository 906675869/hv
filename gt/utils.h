#pragma once
#include "NtStruct.h"

UINT32 GetTextHashA(char* Str);

UINT GetTextHashW(wchar_t* Str);

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
