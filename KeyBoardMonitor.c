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
		Me->AttchDev = IoAttachDeviceToDeviceStack(Device,FlteDev);
		Device = Device->NextDevice;
		if (Me->AttchDev == NULL)
		{
			KdPrint(("Attch Error\r\n"));
			IoDeleteDevice(FlteDev);
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
	//KEVENT event;
	PKEYBOARD_INPUT_DATA Data;
	//KeInitializeEvent(&event,NotificationEvent,FALSE);
	//IoSetCompletionRoutine(Irp,IoCompletionRoutine,&event,TRUE,TRUE,TRUE);
	IoSkipCurrentIrpStackLocation(Irp);
	status=IoCallDriver(Me->AttchDev,Irp);
	/*if (status == STATUS_PENDING)
	{
		KeWaitForSingleObject(&event,Executive,KernelMode,FALSE,NULL);
		status = Irp->IoStatus.Status;
	}*/
	Data = Irp->AssociatedIrp.SystemBuffer;
	if (Data->MakeCode != 0)
	{
		KdPrint(("%x\r\n", Data->MakeCode));
	}
	return status;
}

VOID KeyBoardUnLoad(PDRIVER_OBJECT DriverObject)
{
	PDEVICE_OBJECT Device = DriverObject->DeviceObject;
	PMEXT Me = NULL;
	do {
		Me = (PMEXT)Device->DeviceExtension;
		IoDetachDevice(Me->AttchDev);
		IoDeleteDevice(Me->FiltDev);
		Device = Device->NextDevice;
	} while (Device);
	return;
}

/*NTSTATUS IoCompletionRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context)
{
	if (Irp->PendingReturned == TRUE)
	{
		KeSetEvent((PKEVENT)Context,IO_NO_INCREMENT,FALSE);
		IoMarkIrpPending(Irp);
	}
	return STATUS_SUCCESS;
}*/
