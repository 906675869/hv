#include <ia32.hpp>
#include "utils.h"
#include "tools.h"


ULONG_PTR GetModuleBaseByHashW(ULONG pid, UINT32 moduleHash) {
    PEPROCESS process;
    if (!NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)pid, &process))) {
        DbgPrint("PsLookupProcessByProcessId Fail %d", pid);
        return 0;
    }
    KAPC_STATE apcState;
    KeStackAttachProcess(process, &apcState);

    PVOID PsGetProcessPebAddr = GetKernelFunction(L"PsGetProcessPeb");
    if (!PsGetProcessPebAddr) {
        DbgPrint("PsGetProcessPeb Get Fail %d", pid);
        KeUnstackDetachProcess(&apcState);
        if (process) ObDereferenceObject(process);
        return 0;
    }
    typedef PVOID(*_PsGetProcessPebType)(_In_ PEPROCESS Process);
    _PsGetProcessPebType _PsGetProcessPeb = (_PsGetProcessPebType)PsGetProcessPebAddr;

    __try {
        if (_PsGetProcessPeb(process)) {
            PPEB64 peb = (PPEB64)_PsGetProcessPeb(process);
            PLIST_ENTRY head = &peb->Ldr->InLoadOrderModuleList;

            for (PLIST_ENTRY entry = head->Flink; entry != head; entry = entry->Flink) {
                PLDR_DATA_TABLE_ENTRY module = CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
                if (GetTextHashW(module->BaseDllName.Buffer) == moduleHash) {
                    DbgPrint("Find Module Base %p %wZ", module->DllBase, &module->BaseDllName);
                    KeUnstackDetachProcess(&apcState);
                    if (process) ObDereferenceObject(process);
                    return (ULONG_PTR)module->DllBase;
                }
                DbgPrint("ModuleList Name %wZ", &module->BaseDllName);
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        DbgPrint("Module Not Find  [UNKNOWN] %ul", moduleHash);
    }
    KeUnstackDetachProcess(&apcState);
    if (process) ObDereferenceObject(process);
    DbgPrint("Module Not Find  [UNKNOWN] %ul", moduleHash);
    return 0;
}



NTSTATUS RtlForceDeleteFile(PUNICODE_STRING pFilePath) {
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE hFile = NULL;
    LPBYTE pFileObject = NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;

    InitializeObjectAttributes(&ObjectAttributes, pFilePath, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, 0, 0);
    Status = IoCreateFileEx(&hFile, SYNCHRONIZE | DELETE, &ObjectAttributes, &IoStatusBlock, NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_DELETE, FILE_OPEN, FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0, CreateFileTypeNone, NULL, IO_NO_PARAMETER_CHECKING, NULL);
    if (!NT_SUCCESS(Status)) {
        return STATUS_UNSUCCESSFUL;
    }
    Status = ObReferenceObjectByHandleWithTag(hFile, SYNCHRONIZE | DELETE, *IoFileObjectType, KernelMode, 'ELIF', (LPVOID*)&pFileObject, NULL);
    if (NT_SUCCESS(Status)) {
        ((PFILE_OBJECT)pFileObject)->SectionObjectPointer->ImageSectionObject = NULL;
        if (MmFlushImageSection(((PFILE_OBJECT)pFileObject)->SectionObjectPointer, MmFlushForDelete)) {
            Status = ZwDeleteFile(&ObjectAttributes);
        }
        ObfDereferenceObject(pFileObject);
    }
    ObCloseHandle(hFile, KernelMode);
    return Status;
}