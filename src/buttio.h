#pragma once

typedef struct _PortRange PortRange;

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
    
    typedef struct _ButtioPortHandler {
        BOOL (*ru8) (USHORT port, UCHAR* pData);
        BOOL (*ru16)(USHORT port, USHORT* pData);
        BOOL (*ru32)(USHORT port, ULONG* pData);
        BOOL (*wu8) (USHORT port, UCHAR data);
        BOOL (*wu16)(USHORT port, USHORT data);
        BOOL (*wu32)(USHORT port, ULONG data);
        UCHAR ioMethod;
    } ButtioPortHandler;
    
    void buttio_shutdown(void);
    ButtioPortHandler* buttio_init(HANDLE hand, UCHAR preferedIOMethod);
    BOOL buttio_registerPortRange(PortRange* pPortRange, int count);
    BOOL buttio_unregisterAllPorts(void);
#endif

typedef struct _PortRange {
    USHORT first;
    USHORT last;
    BOOL   isEnabled;
} PortRange;

typedef struct _DriverWritePacket {
    USHORT port;
    union {
        UCHAR  data8;
        USHORT data16;
        ULONG  data32;
    };
} DriverWritePacket;

#define DRIVER_NAME     "buttio"
#define BUTTIO_VERSION  1
#define BUTTIO_DEVTYPE  32768
#define IOPM_SIZE       0x2000

enum {
    IOCTLNR_CUSTOM_BASE = 0x800,
    IOCTLNR_GET_VERSION,
    IOCTLNR_REGISTER_PORTRANGE,
    IOCTLNR_UNREGISTER_ALLPORTS,
    IOCTLNR_READ_32,
    IOCTLNR_READ_16,
    IOCTLNR_READ_8,
    IOCTLNR_WRITE_32,
    IOCTLNR_WRITE_16,
    IOCTLNR_WRITE_8
};
#define IOCTL_GET_VERSION           CTL_CODE(BUTTIO_DEVTYPE, IOCTLNR_GET_VERSION,         METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_REGISTER_PORTRANGE    CTL_CODE(BUTTIO_DEVTYPE, IOCTLNR_REGISTER_PORTRANGE,  METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_UNREGISTER_ALLPORTS   CTL_CODE(BUTTIO_DEVTYPE, IOCTLNR_UNREGISTER_ALLPORTS, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_READ_32               CTL_CODE(BUTTIO_DEVTYPE, IOCTLNR_READ_32,             METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_READ_16               CTL_CODE(BUTTIO_DEVTYPE, IOCTLNR_READ_16,             METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_READ_8                CTL_CODE(BUTTIO_DEVTYPE, IOCTLNR_READ_8,              METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WRITE_32              CTL_CODE(BUTTIO_DEVTYPE, IOCTLNR_WRITE_32,            METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WRITE_16              CTL_CODE(BUTTIO_DEVTYPE, IOCTLNR_WRITE_16,            METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WRITE_8               CTL_CODE(BUTTIO_DEVTYPE, IOCTLNR_WRITE_8,             METHOD_BUFFERED, FILE_ANY_ACCESS)