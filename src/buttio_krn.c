/*
 * This is derived from fbportio driver, but with more control over allowed ports.
 */

/*
 * Many thanks to these useful references:
 *
 * https://github.com/freebasic/fbc/tree/master/src/rtlib/win32
 * http://www.beyondlogic.org/porttalk/porttalk.htm
 * http://www.codeproject.com/system/kportII.asp
 * http://www.logix4u.net/inpout32.htm
 *
 * This code was written to provide a tiny implementation of the theory described there.
 */

#include <ntddk.h>
#include "buttio.h"

void NTAPI Ke386SetIoAccessMap(int, UCHAR*);
void NTAPI Ke386QueryIoAccessMap(int, UCHAR*);
void NTAPI Ke386IoSetAccessProcess(PEPROCESS, int);

#define PP(_X) ((void*)(LONG_PTR)_X)

const WCHAR devicePath[]    = L"\\Device\\" DRIVER_NAME;
const WCHAR dosDevicePath[] = L"\\DosDevices\\" DRIVER_NAME;
UNICODE_STRING g_uniDevicePath, g_uniDosDevicePath;

static UCHAR* g_pIopmMap = NULL;

/////////////////////////////////////////////////////////////////////////////
//1 bit means blocked port, and viceversa
BOOL iopm_isIoDenied(UCHAR* pIopm, USHORT port, UCHAR width) {
    BOOL isDenied = 0;
    
    //Mysoft says real IO wraps
    for (UCHAR i=0; i < width; i++) {
        USHORT portOff = port+i;
        
        isDenied |= pIopm[portOff>>3] & (1<<i);
    }
    return isDenied;
}


void iopm_fillRange(UCHAR* pIopm, PortRange* pRange) {
    BOOL isEnabled = pRange->isEnabled ? 1 : 0; //sanitize
    
    for (ULONG i=pRange->first; i <= pRange->last; ) {
        ULONG todo = pRange->last - i;
        
        if (todo >= 8 && (i&7) == 0) {
            ULONG todo = (pRange->last& ~7) - i;
            __builtin_memset(&pIopm[i>>3], (isEnabled ? 0 : ~0), todo>>3);
            i += todo;
        } else {
            if (isEnabled) {
                pIopm[i>>3] &= ~(1<<(i&7));
            } else {
                pIopm[i>>3] |= (1<<(i&7));
            }
            
            i++;
        }
    }
}

static void iopm_fillAll(UCHAR* pIopm, BOOL isEnabled) {
    PortRange range = { 0x0000, 0xFFFF, isEnabled };
    
    iopm_fillRange(pIopm, &range);
}

/////////////////////////////////////////////////////////////////////////////
static NTSTATUS NTAPI device_dispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp) {
    (void)DeviceObject;
    
    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    
    return STATUS_SUCCESS;
}

static NTSTATUS NTAPI device_control(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp) {
    PIO_STACK_LOCATION stack;
    void* data;
    ULONG inSize, outSize;
    ULONG writtenBytes;
    NTSTATUS status;
    UCHAR ioSize;
    
    (void)DeviceObject;
    
    stack = IoGetCurrentIrpStackLocation(Irp);
    inSize = stack->Parameters.DeviceIoControl.InputBufferLength;
    outSize = stack->Parameters.DeviceIoControl.OutputBufferLength;
    data = (void*)Irp->AssociatedIrp.SystemBuffer;
    
    ioSize = 1;
    writtenBytes = 0;
    switch (stack->Parameters.DeviceIoControl.IoControlCode) {
        
        case IOCTL_GET_VERSION:
            if (outSize < sizeof(USHORT)) {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                *((PUSHORT)data) = BUTTIO_VERSION;
                
                writtenBytes = sizeof(USHORT);
                status = STATUS_SUCCESS;
            }
            break;
            
        case IOCTL_REGISTER_PORTRANGE:
            if (inSize < sizeof(PortRange)) {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                int count = inSize / sizeof(PortRange);
                PortRange* pRange = (PortRange*)data;
                
                for (int i=0; i < count; i++) iopm_fillRange(g_pIopmMap, &pRange[i]);
                
                //I believe Ke386SetIoAccessMap() acts on current process only.
                //So using PsLookupProcessByProcessId() won't do much good.
                //Ke386SetIoAccessMap(1, g_pIopmMap);
                //Ke386IoSetAccessProcess(PsGetCurrentProcess(), 1);
                status = STATUS_SUCCESS;
            }
            break;
            
        case IOCTL_UNREGISTER_ALLPORTS:
            iopm_fillAll(g_pIopmMap, FALSE);
            
            //Ke386SetIoAccessMap(1, g_pIopmMap);
            //Ke386IoSetAccessProcess(PsGetCurrentProcess(), 0);
            status = STATUS_SUCCESS;
            break;
        
        case IOCTL_READ_32:
            ioSize <<= 1; /* FALLTHRU */
        case IOCTL_READ_16:
            ioSize <<= 1; /* FALLTHRU */
        case IOCTL_READ_8:
            if (inSize < sizeof(USHORT) || outSize < ioSize) {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                USHORT port = *((PUSHORT)data);
                BOOL isDenied;
                
                isDenied = iopm_isIoDenied(g_pIopmMap, port, ioSize);
                if (isDenied) {
                    status = STATUS_ACCESS_DENIED;
                } else {
                    switch (ioSize) {
                        case sizeof(ULONG):
                            *((PULONG)data)  = READ_PORT_ULONG(PP(port));
                            break;
                        case sizeof(USHORT):
                            *((PUSHORT)data) = READ_PORT_USHORT(PP(port));
                            break;
                        case sizeof(UCHAR):
                            *((PUCHAR)data)  = READ_PORT_UCHAR(PP(port));
                            break;
                    }
                    writtenBytes = ioSize;
                    status = STATUS_SUCCESS;
                }
            }
            break;
        
        case IOCTL_WRITE_32:
            ioSize <<= 1; /* FALLTHRU */
        case IOCTL_WRITE_16:
            ioSize <<= 1; /* FALLTHRU */
        case IOCTL_WRITE_8:
            if (inSize < ioSize) {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                DriverWritePacket pack = *((DriverWritePacket*)data);
                BOOL isDenied;
                
                isDenied = iopm_isIoDenied(g_pIopmMap, pack.port, ioSize);
                if (isDenied) {
                    status = STATUS_ACCESS_DENIED;
                } else {
                    switch (ioSize) {
                        case sizeof(ULONG):
                            WRITE_PORT_USHORT(PP(pack.port), pack.data32);
                            break;
                        case sizeof(USHORT):
                            WRITE_PORT_USHORT(PP(pack.port), pack.data16);
                            break;
                        case sizeof(UCHAR):
                            WRITE_PORT_UCHAR(PP(pack.port), pack.data8);
                            break;
                    }
                    status = STATUS_SUCCESS;
                }
            }
            break;
        
        default:
            status = STATUS_UNSUCCESSFUL;
            break;
    }

    Irp->IoStatus.Information = writtenBytes;
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}

static VOID NTAPI driver_unload(IN PDRIVER_OBJECT DriverObject) {
    if (g_pIopmMap) MmFreeNonCachedMemory(g_pIopmMap, IOPM_SIZE*sizeof(UCHAR));
    
    IoDeleteSymbolicLink(&g_uniDosDevicePath);
    IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS NTAPI DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath) {
    PDEVICE_OBJECT device_object;
    NTSTATUS status;
    
    (void)RegistryPath;
    
    RtlInitUnicodeString(&g_uniDevicePath, devicePath);
    RtlInitUnicodeString(&g_uniDosDevicePath, dosDevicePath);
    
    g_pIopmMap = MmAllocateNonCachedMemory(IOPM_SIZE*sizeof(UCHAR));
    if (!g_pIopmMap) return STATUS_INSUFFICIENT_RESOURCES;

    iopm_fillAll(g_pIopmMap, FALSE);

    status = IoCreateDevice(DriverObject, 0, &g_uniDevicePath, FILE_DEVICE_UNKNOWN,
                            0, FALSE, &device_object);
    if (!NT_SUCCESS(status)) return status;

    status = IoCreateSymbolicLink(&g_uniDosDevicePath, &g_uniDevicePath);
    if (!NT_SUCCESS(status)) return status;

    DriverObject->MajorFunction[IRP_MJ_CREATE] = device_dispatch;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = device_control;
    DriverObject->DriverUnload = driver_unload;

    return STATUS_SUCCESS;
}
