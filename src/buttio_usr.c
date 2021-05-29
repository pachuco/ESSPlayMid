#include <windows.h>
#include <winioctl.h>

#include "buttio.h"
#include "util.h"

//SERVICES_FOREACH(DEFFUN_OBJ, _, _);
//CloseServiceHandle
//ControlService
//CreateService
//DeleteService
//OpenSCManager
//OpenService
//StartService

static BOOL g_isDriverRequired = FALSE;
static BOOL g_isInit = FALSE;
static SC_HANDLE g_manager = NULL;
static SC_HANDLE g_service = NULL;
static HANDLE    g_driverFile = NULL;




//--------------------------------------------------------------------
static BOOL direct_ru8(USHORT port, UCHAR* pData) {
    __asm__ volatile("inb %1, %0" : "=a" (*pData) : "d" (port));
    return TRUE;
}
static BOOL direct_ru16(USHORT port, USHORT* pData) {
    __asm__ volatile("inw %1, %0" : "=a" (*pData) : "d" (port));
    return TRUE;
}
static BOOL direct_ru32(USHORT port, ULONG* pData) {
    __asm__ volatile("inl %1, %0" : "=a" (*pData) : "d" (port));
    return TRUE;
}
static BOOL direct_wu8(USHORT port, UCHAR data) {
    __asm__ volatile("outb %0, %1" : : "a" (data), "d" (port));
    return TRUE;
}
static BOOL direct_wu16(USHORT port, USHORT data) {
    __asm__ volatile("outw %0, %1" : : "a" (data), "d" (port));
    return TRUE;
}
static BOOL direct_wu32(USHORT port, ULONG data) {
    __asm__ volatile("outl %0, %1" : : "a" (data), "d" (port));
    return TRUE;
}
ButtioPortHandler direct_handler = {
    &direct_ru8,
    &direct_ru16,
    &direct_ru32,
    &direct_wu8,
    &direct_wu16,
    &direct_wu32,
    BUTTIO_MET_DIRECT
};
//--------------------------------------------------------------------
ButtioPortHandler iopm_handler = {
    &direct_ru8,
    &direct_ru16,
    &direct_ru32,
    &direct_wu8,
    &direct_wu16,
    &direct_wu32,
    BUTTIO_MET_IOPM
};
//--------------------------------------------------------------------
//TODO: maybe sanitize alignments
static BOOL driverCall_ru8(USHORT port, UCHAR* pData) {
    DWORD bytesWritten;
    return DeviceIoControl(g_driverFile, IOCTL_READ_8, &port, sizeof(USHORT), pData, sizeof(UCHAR), &bytesWritten, NULL);
}
static BOOL driverCall_ru16(USHORT port, USHORT* pData) {
    DWORD bytesWritten;
    return DeviceIoControl(g_driverFile, IOCTL_READ_16, &port, sizeof(USHORT), pData, sizeof(USHORT), &bytesWritten, NULL);
}
static BOOL driverCall_ru32(USHORT port, ULONG* pData) {
    DWORD bytesWritten;
    return DeviceIoControl(g_driverFile, IOCTL_READ_32, &port, sizeof(USHORT), pData, sizeof(ULONG), &bytesWritten, NULL);
}
static BOOL driverCall_wu8(USHORT port, UCHAR data) {
    DWORD bytesWritten;
    DriverWritePacket pack = {port, {data}};
    return DeviceIoControl(g_driverFile, IOCTL_WRITE_8, &pack, sizeof(DriverWritePacket), NULL, 0, &bytesWritten, NULL);
}
static BOOL driverCall_wu16(USHORT port, USHORT data) {
    DWORD bytesWritten;
    DriverWritePacket pack = {port, {data}};
    return DeviceIoControl(g_driverFile, IOCTL_WRITE_16, &pack, sizeof(DriverWritePacket), NULL, 0, &bytesWritten, NULL);
}
static BOOL driverCall_wu32(USHORT port, ULONG data) {
    DWORD bytesWritten;
    DriverWritePacket pack = {port, {data}};
    return DeviceIoControl(g_driverFile, IOCTL_WRITE_32, &pack, sizeof(DriverWritePacket), NULL, 0, &bytesWritten, NULL);
}
ButtioPortHandler driverCall_handler = {
    &driverCall_ru8,
    &driverCall_ru16,
    &driverCall_ru32,
    &driverCall_wu8,
    &driverCall_wu16,
    &driverCall_wu32,
    BUTTIO_MET_DRIVERCALL
};
//--------------------------------------------------------------------







void buttio_shutdown(void) {
    SERVICE_STATUS status;
    
    if (g_driverFile) {
        buttio_unregisterAllPorts();
        CloseHandle(g_driverFile);
        g_driverFile = NULL;
    }
    if (g_service) {
        ControlService(g_service, SERVICE_CONTROL_STOP, &status);
        DeleteService(g_service);
        CloseServiceHandle(g_service);
        g_service = NULL;
    }
    if (g_manager) {
        CloseServiceHandle(g_manager);
        g_manager = NULL;
    }
    g_isInit = FALSE;
}

ButtioPortHandler* buttio_init(HANDLE modHand, UCHAR preferedIOMethod) {
    OSVERSIONINFOA verInfo = {0};
    ButtioPortHandler* pHandler = NULL;
    
    verInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
    if (!GetVersionExA(&verInfo)) return NULL;
    g_isDriverRequired = (verInfo.dwPlatformId >= VER_PLATFORM_WIN32_NT);
    
    if (g_isDriverRequired) {
        //init driver
        g_manager = OpenSCManager(NULL, NULL, GENERIC_ALL);
        if (g_manager) {
            CHAR driverPath[MAX_PATH];
            SERVICE_STATUS status;
            
            g_service = OpenService(g_manager, DRIVER_NAME, SERVICE_ALL_ACCESS);
            if (g_service) {
                ControlService(g_service, SERVICE_CONTROL_STOP, &status);
                DeleteService(g_service);
                CloseServiceHandle(g_service);
                g_service = NULL;
            }
            
            if (GetModuleFileNameA(modHand, driverPath, MAX_PATH-1)) {
                driverPath[MAX_PATH-1] = '\0';
                util_getParentPathA(driverPath);
                strncat(driverPath, DRIVER_NAME".sys", MAX_PATH-1);
                
                
                g_service = CreateService(g_manager, DRIVER_NAME, DRIVER_NAME,
                    SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
                    driverPath, NULL, NULL, NULL, NULL, NULL);
                
                if (g_service && StartService(g_service, 0, NULL)) {
                    g_driverFile = CreateFile("\\\\.\\"DRIVER_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                    
                    if (g_driverFile) {
                        WORD driverVer = 0;
                        DWORD bytesWritten;
                        
                        DeviceIoControl(g_driverFile, IOCTL_GET_VERSION, NULL, 0, &driverVer, sizeof(WORD), &bytesWritten, NULL);
                        
                        if (driverVer == BUTTIO_VERSION) g_isInit = TRUE;
                    }
                }
            }
        }
    } else {
        g_isInit = TRUE;
    }
    
    //decide io method
    if (g_isInit) {
        if (!g_isDriverRequired) {
            pHandler = &direct_handler;
        } else {
            if (preferedIOMethod == BUTTIO_MET_IOPM) {
                pHandler = &iopm_handler;
            } else {
                pHandler = &driverCall_handler;
            }
        }
    } else {
        buttio_shutdown();
    }
    
    return pHandler;
}

BOOL buttio_registerPortRange(PortRange* pPortRange, int count) {
    DWORD bytesWritten;
    DWORD writeSize = sizeof(PortRange) * count;
    BOOL ret = TRUE;
    
    if (!g_isInit) return FALSE;
    if (!g_isDriverRequired) return TRUE;
    
    ret = DeviceIoControl(g_driverFile, IOCTL_REGISTER_PORTRANGE, pPortRange, writeSize, NULL, 0, &bytesWritten, NULL);
    Sleep(1);
    
    return ret;
}

BOOL buttio_unregisterAllPorts(void) {
    DWORD bytesWritten;
    BOOL ret = TRUE;
    
    if (!g_isInit) return FALSE;
    if (!g_isDriverRequired) return TRUE;
    
    ret = DeviceIoControl(g_driverFile, IOCTL_UNREGISTER_ALLPORTS, NULL, 0, NULL, 0, &bytesWritten, NULL);
    Sleep(1);
    
    return ret;
}