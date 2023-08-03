#include "utils.h"
#include "hv.h"

#include "kmclass.h"
#include <ia32.hpp>
#include "cm.h"


// simple hypercall wrappers
static uint64_t ping() {
    hv::hypercall_input input;
    input.code = hv::hypercall_ping;
    input.key = hv::hypercall_key;
    return hv::vmx_vmcall(input);
}

void DriverUnload(PDRIVER_OBJECT) {
    RegisterNotifyInit(false);
    hv::stop();
    DbgPrint("Devirtualized the system.\n");
    DbgPrint("Driver unloaded.\n");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT const driver, PUNICODE_STRING) {
    DbgPrint("[hv] Driver loaded.\n");
    NTSTATUS status;
    if (driver)
        driver->DriverUnload = DriverUnload;

    if (!hv::start()) {
        DbgPrint(" Failed to virtualize system.\n");
        return STATUS_HV_OPERATION_FAILED;
    }

    if (ping() == hv::hypervisor_signature)
        DbgPrint("[client] Hypervisor signature matches.\n");
    else
        DbgPrint("[client] Failed to ping hypervisor!\n");
    // ËÑË÷¼üÅÌ
    status = SearchKdbServiceCallBack(driver);
    if (!NT_SUCCESS(status))
    {
        DbgPrint("KEYBOARD_DEVICE ERROR, error = 0x%08lx\n", status);
        return status;
    }
    //// ËÑË÷Êó±ê
    status = SearchMouServiceCallBack(driver);
    if (!NT_SUCCESS(status))
    {
        DbgPrint("MOUSE_DEVICE ERROR, error = 0x%08lx\n", status);
        return status;
    }
    driverData.kernelBase = GetKernelBase(driver);
    if (!driverData.kernelBase) {
        /*PVOID jmprcx = SearchSignForImage(driverData.kernelBase, "\xFF\xE1", "xx", 2);
        DbgPrint("kernel jmp rcx is 0x%p", jmprcx);

        PVOID jmprcx3 = SearchSignForImage(driverData.kernelBase, "\xFF\x21", "xx", 2);
        DbgPrint("kernel jmp qword ptr [rcx] is 0x%p", jmprcx3);*/
        return status;
    }
    RegisterNotifyInit(true);

    if (driver)
    RtlForceDeleteFile(&((PKLDR_DATA_TABLE_ENTRY)driver->DriverSection)->FullDllName);
	return STATUS_SUCCESS;
}