#pragma once

#include<wdm.h>
#include<ntddkbd.h>



#define DEVNAME L"\\Device\\AlexMecrer"
#define SYMNAME L"\\??\\jiuli"
#define KEYBOARD L"\\Driver\\Kbdclass"


typedef struct {
	PDEVICE_OBJECT OrgnDev;
	PDEVICE_OBJECT FiltDev;
	PDEVICE_OBJECT AttchDev;
}MYEXT,*PMEXT;


extern POBJECT_TYPE *IoDriverObjectType;



NTSTATUS
ObReferenceObjectByName(
	__in PUNICODE_STRING ObjectName,
	__in ULONG Attributes,
	__in_opt PACCESS_STATE AccessState,
	__in_opt ACCESS_MASK DesiredAccess,
	__in POBJECT_TYPE ObjectType,
	__in KPROCESSOR_MODE AccessMode,
	__inout_opt PVOID ParseContext,
	__out PVOID *Object
);

NTSTATUS AttchAllDevice(PDRIVER_OBJECT Driver);

NTSTATUS KeyBoardDispath(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	);

NTSTATUS KeyBoardDispathRead(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
);

VOID KeyBoardUnLoad(PDRIVER_OBJECT DriverObject);


NTSTATUS MyIoCompletionRoutine(
	PDEVICE_OBJECT DeviceObject,
	PIRP Irp,
	PVOID Context
);