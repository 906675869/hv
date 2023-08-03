#include "NtStruct.h"
#include "utils.h"
#include "cm.h"


DriverData driverData;
RegisterNotifyBuffer registerNotifyBuffer;

NTSTATUS RegisterNotify(LPVOID, REG_NOTIFY_CLASS OperationType, PREG_SET_VALUE_KEY_INFORMATION PreSetValueInfo) {
	NTSTATUS Status = STATUS_SUCCESS;

	if (OperationType == RegNtPreSetValueKey && PreSetValueInfo->Type >= '0000') {
		if (PreSetValueInfo->Type == '0000'/*GS_Í¨Ñ¶²âÊÔ*/) {

			if (PreSetValueInfo->Data == NULL) {
				DbgPrint("GT Connect");
				Status = ERROR_SUCCESS;
			}
		}
	}
	return Status;
}



NTSTATUS RegisterNotifyInit(BOOLEAN Enable) {

	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	PRegisterNotifyBuffer pRegisterNotifyHookBuffer = &registerNotifyBuffer;

	if (pRegisterNotifyHookBuffer->Enable != Enable) {

		if (pRegisterNotifyHookBuffer->HookPoint == NULL) {
			// "\xFF\xE1" jmp rcx
			// \xFF\x21 jmp [rcx]
			// pRegisterNotifyHookBuffer->HookPoint = SearchSignForImage(driverData.kernelBase, "\xFF\xE1", "xx", 2);
			pRegisterNotifyHookBuffer->HookPoint = SearchSignForImage(driverData.kernelBase, "\xFF\x21", "xx", 2);
		}

		if (pRegisterNotifyHookBuffer->HookPoint != NULL) {

			if (Enable == TRUE) {
				PVOID buffer = ExAllocatePoolWithTag(NonPagedPoolNx, 128, 'NetF');
				if (!buffer) {

					return STATUS_UNSUCCESSFUL;
				}
				*(ULONG64*)buffer = (ULONG64)RegisterNotify;

				pRegisterNotifyHookBuffer->Buffer = buffer;
				Status = CmRegisterCallback((PEX_CALLBACK_FUNCTION)(pRegisterNotifyHookBuffer->HookPoint), buffer, &pRegisterNotifyHookBuffer->Cookie);

				if (NT_SUCCESS(Status)) {

					pRegisterNotifyHookBuffer->Enable = TRUE;
				}
			}

			if (Enable != TRUE) {

				if (pRegisterNotifyHookBuffer->HookPoint != NULL) {

					Status = CmUnRegisterCallback(pRegisterNotifyHookBuffer->Cookie);

					if (pRegisterNotifyHookBuffer->Buffer) {
						ExFreePoolWithTag(pRegisterNotifyHookBuffer->Buffer, 'NetF');
					}
					if (NT_SUCCESS(Status)) {
						pRegisterNotifyHookBuffer->Enable = FALSE;
					}
				}
			}
		}
	}

	if (pRegisterNotifyHookBuffer->Enable == Enable) {

		Status = STATUS_SUCCESS;
	}

	return Status;
}