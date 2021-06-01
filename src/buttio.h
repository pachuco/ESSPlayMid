#pragma once

#define IOPM_SIZE       0x2000

#ifdef CODEISDRIVER
    //driver specific
    #include <ntddk.h>
    

#else
    //userland specific
    #include <windows.h>
    
    enum {
        BUTTIO_MET_DIRECT,
        BUTTIO_MET_IOPM,
        BUTTIO_MET_DRIVERCALL
    };
    
    typedef struct _IOHandler IOHandler;
    typedef struct _IOHandler {
        UCHAR iopm[IOPM_SIZE];
        UCHAR ioMethod;
    } IOHandler;

    void buttio_shutdown          (IOHandler* pIoHand);
    BOOL buttio_init              (IOHandler* pIoHand, HANDLE modHand, UCHAR preferedIOMethod);
    BOOL buttio_flushIOPMChanges  (IOHandler* pIoHand);
    
    BOOL buttio_ru8 (IOHandler* pIoHand, USHORT port, UCHAR* pData);
    BOOL buttio_ru16(IOHandler* pIoHand, USHORT port, USHORT* pData);
    BOOL buttio_ru32(IOHandler* pIoHand, USHORT port, ULONG* pData);
    BOOL buttio_wu8 (IOHandler* pIoHand, USHORT port, UCHAR data);
    BOOL buttio_wu16(IOHandler* pIoHand, USHORT port, USHORT data);
    BOOL buttio_wu32(IOHandler* pIoHand, USHORT port, ULONG data);
#endif
BOOL iopm_isIopmOpaque(UCHAR* pIopm);
BOOL iopm_isIoDenied  (UCHAR* pIopm, USHORT port, UCHAR width);
void iopm_fillRange   (UCHAR* pIopm, USHORT first, USHORT last, BOOL isEnabled);
void iopm_fillAll     (UCHAR* pIopm, BOOL isEnabled);

typedef struct _DriverWritePacket {
    USHORT port;
    union {
        UCHAR  data8;
        USHORT data16;
        ULONG  data32;
    };
} DriverWritePacket;

#define DRIVER_NAME     "buttio"
#define BUTTIO_VERSION  2
#define BUTTIO_DEVTYPE  32768

enum {
    IOCTLNR_CUSTOM_BASE = 0x800,
    IOCTLNR_GET_VERSION,
    IOCTLNR_IOPM_REGISTER,
    IOCTLNR_IOPM_UNREGISTER,
    IOCTLNR_READ_32,
    IOCTLNR_READ_16,
    IOCTLNR_READ_8,
    IOCTLNR_WRITE_32,
    IOCTLNR_WRITE_16,
    IOCTLNR_WRITE_8,
};
#define IOCTL_GET_VERSION           CTL_CODE(BUTTIO_DEVTYPE, IOCTLNR_GET_VERSION,         METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_IOPM_REGISTER         CTL_CODE(BUTTIO_DEVTYPE, IOCTLNR_IOPM_REGISTER,       METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_IOPM_UNREGISTER       CTL_CODE(BUTTIO_DEVTYPE, IOCTLNR_IOPM_UNREGISTER,     METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_READ_32               CTL_CODE(BUTTIO_DEVTYPE, IOCTLNR_READ_32,             METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_READ_16               CTL_CODE(BUTTIO_DEVTYPE, IOCTLNR_READ_16,             METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_READ_8                CTL_CODE(BUTTIO_DEVTYPE, IOCTLNR_READ_8,              METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WRITE_32              CTL_CODE(BUTTIO_DEVTYPE, IOCTLNR_WRITE_32,            METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WRITE_16              CTL_CODE(BUTTIO_DEVTYPE, IOCTLNR_WRITE_16,            METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WRITE_8               CTL_CODE(BUTTIO_DEVTYPE, IOCTLNR_WRITE_8,             METHOD_BUFFERED, FILE_ANY_ACCESS)

//BUTTIO_VERSION history
//v1
// - original
//v2:
// - PortRange use discarded. Send whole bitmap to driver.
// - I/O permission checks moved to user side.