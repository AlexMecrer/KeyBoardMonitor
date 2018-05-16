#include "KeyBoardMonitor.h"

NTSTATUS AttchAllDevice(PDRIVER_OBJECT Driver)
{
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING KeyBoardName = RTL_CONSTANT_STRING(KEYBOARD);
	PDRIVER_OBJECT KeyBoardDriver;
	PMEXT Me;
	status = ObReferenceObjectByName(&KeyBoardName, OBJ_CASE_INSENSITIVE, NULL, FILE_ALL_ACCESS, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&KeyBoardDriver);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("Open Driver Error\r\n"));
		return status;
	}
	PDEVICE_OBJECT FlteDev, Device = (PDEVICE_OBJECT)KeyBoardDriver->DeviceObject;
	do {
		status = IoCreateDevice(Driver,sizeof(MYEXT),NULL,Device->DeviceType,Device->Characteristics,FALSE,&FlteDev);
		if (!NT_SUCCESS(status))
		{
			break;
		}
		Me = (PMEXT)FlteDev->DeviceExtension;
		RtlZeroMemory(Me,sizeof(MYEXT));
		Me->FiltDev = FlteDev;
		Me->OrgnDev = Device;
		Me->AttchDev = IoAttachDeviceToDeviceStack(FlteDev,Device);
		FlteDev->StackSize = Device->StackSize + 1;
		Device = Device->NextDevice;
		if (Me->AttchDev == NULL)
		{
			KdPrint(("Attch Error\r\n"));
			IoDeleteDevice(FlteDev);
			Me->FiltDev = NULL;
			continue;
		}
		FlteDev->Flags|=(DO_BUFFERED_IO | DO_DIRECT_IO | DO_POWER_PAGABLE);
	} while (Device);
	return status;
}

NTSTATUS KeyBoardDispath(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	IoSkipCurrentIrpStackLocation(Irp);
	PMEXT Me = (PMEXT)DeviceObject->DeviceExtension;
	return IoCallDriver(Me->AttchDev,Irp);
}

NTSTATUS KeyBoardDispathRead(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS status;
	PMEXT Me = (PMEXT)DeviceObject->DeviceExtension;
	PKEYBOARD_INPUT_DATA Data;
	IoCopyCurrentIrpStackLocationToNext(Irp);
	IoSetCompletionRoutine(Irp,MyIoCompletionRoutine,NULL,TRUE,TRUE,TRUE);
	status=IoCallDriver(Me->AttchDev,Irp);
	return status;
}

VOID KeyBoardUnLoad(PDRIVER_OBJECT DriverObject)
{
	PDEVICE_OBJECT Device = DriverObject->DeviceObject;
	PMEXT Me = NULL;
	while(Device)
	{
		Me = (PMEXT)Device->DeviceExtension;
		if (Me->FiltDev != NULL)
		{
			IoDetachDevice(Me->AttchDev);
			IoDeleteDevice(Me->FiltDev);
			Me->FiltDev = NULL;
			Me->AttchDev = NULL;
		}
		Device = Device->NextDevice;
	};
	return;
}

NTSTATUS MyIoCompletionRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context)
{
	PKEYBOARD_INPUT_DATA Data = Irp->AssociatedIrp.SystemBuffer;
	KdPrint(("%x\r\n",Data->MakeCode));
	if (Irp->PendingReturned == TRUE)
	{
		IoMarkIrpPending(Irp);
	}
	return STATUS_SUCCESS;
}
