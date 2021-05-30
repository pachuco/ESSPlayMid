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

static BOOL  g_isDriverRequired = FALSE;
static BOOL  g_isInit = FALSE;
static SC_HANDLE g_manager = NULL;
static SC_HANDLE g_service = NULL;
static HANDLE    g_driverFile = NULL;

typedef struct {
    BOOL (*ru8) (ButtioPortHandler* portHandler, USHORT port, UCHAR*  pData);
    BOOL (*ru16)(ButtioPortHandler* portHandler, USHORT port, USHORT* pData);
    BOOL (*ru32)(ButtioPortHandler* portHandler, USHORT port, ULONG*  pData);
    BOOL (*wu8) (ButtioPortHandler* portHandler, USHORT port, UCHAR   data);
    BOOL (*wu16)(ButtioPortHandler* portHandler, USHORT port, USHORT  data);
    BOOL (*wu32)(ButtioPortHandler* portHandler, USHORT port, ULONG   data);
    UCHAR ioMethod;
} IOVtable;

//BOOL DeviceIoControl(HANDLE dev, DWORD ioctl, LPVOID in, DWORD inSize, LPVOID out, DWORD outSize, LPDWORD pRetSize, NULL);

//--------------------------------------------------------------------
static BOOL direct_ru8(ButtioPortHandler* portHandler, USHORT port, UCHAR* pData) {
    if (iopm_isIoDenied(portHandler->iopm, port, sizeof(UCHAR))) return FALSE;
    __asm__ volatile("inb %1, %0" : "=a" (*pData) : "d" (port));
    return TRUE;
}
static BOOL direct_ru16(ButtioPortHandler* portHandler, USHORT port, USHORT* pData) {
    if (iopm_isIoDenied(portHandler->iopm, port, sizeof(USHORT))) return FALSE;
    __asm__ volatile("inw %1, %0" : "=a" (*pData) : "d" (port));
    return TRUE;
}
static BOOL direct_ru32(ButtioPortHandler* portHandler, USHORT port, ULONG* pData) {
    if (iopm_isIoDenied(portHandler->iopm, port, sizeof(ULONG))) return FALSE;
    __asm__ volatile("inl %1, %0" : "=a" (*pData) : "d" (port));
    return TRUE;
}
static BOOL direct_wu8(ButtioPortHandler* portHandler, USHORT port, UCHAR data) {
    if (iopm_isIoDenied(portHandler->iopm, port, sizeof(UCHAR))) return FALSE;
    __asm__ volatile("outb %0, %1" : : "a" (data), "d" (port));
    return TRUE;
}
static BOOL direct_wu16(ButtioPortHandler* portHandler, USHORT port, USHORT data) {
    if (iopm_isIoDenied(portHandler->iopm, port, sizeof(USHORT))) return FALSE;
    __asm__ volatile("outw %0, %1" : : "a" (data), "d" (port));
    return TRUE;
}
static BOOL direct_wu32(ButtioPortHandler* portHandler, USHORT port, ULONG data) {
    if (iopm_isIoDenied(portHandler->iopm, port, sizeof(ULONG))) return FALSE;
    __asm__ volatile("outl %0, %1" : : "a" (data), "d" (port));
    return TRUE;
}
static IOVtable direct_handler = {
    &direct_ru8,
    &direct_ru16,
    &direct_ru32,
    &direct_wu8,
    &direct_wu16,
    &direct_wu32,
    BUTTIO_MET_DIRECT
};
//--------------------------------------------------------------------
static IOVtable iopm_handler = {
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
static BOOL driverCall_ru8(ButtioPortHandler* portHandler, USHORT port, UCHAR* pData) {
    DWORD bytesWritten;
    
    if (iopm_isIoDenied(portHandler->iopm, port, sizeof(UCHAR))) return FALSE;
    return DeviceIoControl(g_driverFile, IOCTL_READ_8, &port, sizeof(USHORT), pData, sizeof(UCHAR), &bytesWritten, NULL);
}
static BOOL driverCall_ru16(ButtioPortHandler* portHandler, USHORT port, USHORT* pData) {
    DWORD bytesWritten;
    
    if (iopm_isIoDenied(portHandler->iopm, port, sizeof(USHORT))) return FALSE;
    return DeviceIoControl(g_driverFile, IOCTL_READ_16, &port, sizeof(USHORT), pData, sizeof(USHORT), &bytesWritten, NULL);
}
static BOOL driverCall_ru32(ButtioPortHandler* portHandler, USHORT port, ULONG* pData) {
    DWORD bytesWritten;
    
    if (iopm_isIoDenied(portHandler->iopm, port, sizeof(ULONG))) return FALSE;
    return DeviceIoControl(g_driverFile, IOCTL_READ_32, &port, sizeof(USHORT), pData, sizeof(ULONG), &bytesWritten, NULL);
}
static BOOL driverCall_wu8(ButtioPortHandler* portHandler, USHORT port, UCHAR data) {
    DWORD bytesWritten;
    DriverWritePacket pack = {port, {data}};
    
    if (iopm_isIoDenied(portHandler->iopm, port, sizeof(UCHAR))) return FALSE;
    return DeviceIoControl(g_driverFile, IOCTL_WRITE_8, &pack, sizeof(DriverWritePacket), NULL, 0, &bytesWritten, NULL);
}
static BOOL driverCall_wu16(ButtioPortHandler* portHandler, USHORT port, USHORT data) {
    DWORD bytesWritten;
    DriverWritePacket pack = {port, {data}};
    
    if (iopm_isIoDenied(portHandler->iopm, port, sizeof(USHORT))) return FALSE;
    return DeviceIoControl(g_driverFile, IOCTL_WRITE_16, &pack, sizeof(DriverWritePacket), NULL, 0, &bytesWritten, NULL);
}
static BOOL driverCall_wu32(ButtioPortHandler* portHandler, USHORT port, ULONG data) {
    DWORD bytesWritten;
    DriverWritePacket pack = {port, {data}};
    
    if (iopm_isIoDenied(portHandler->iopm, port, sizeof(ULONG))) return FALSE;
    return DeviceIoControl(g_driverFile, IOCTL_WRITE_32, &pack, sizeof(DriverWritePacket), NULL, 0, &bytesWritten, NULL);
}
static IOVtable driverCall_handler = {
    &driverCall_ru8,
    &driverCall_ru16,
    &driverCall_ru32,
    &driverCall_wu8,
    &driverCall_wu16,
    &driverCall_wu32,
    BUTTIO_MET_DRIVERCALL
};
//--------------------------------------------------------------------







void buttio_shutdown(ButtioPortHandler* portHandler) {
    SERVICE_STATUS status;
    
    iopm_fillAll(portHandler->iopm, FALSE);
    
    if (g_driverFile) {
        buttio_flushIOPMChanges(portHandler);
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
    
    memset(portHandler, 0, sizeof(IOVtable));
    
    g_isInit = FALSE;
}
BOOL buttio_init(ButtioPortHandler* portHandler, HANDLE modHand, UCHAR preferedIOMethod) {
    OSVERSIONINFOA verInfo = {0};
    
    verInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
    if (!GetVersionExA(&verInfo)) return FALSE;
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
            preferedIOMethod = BUTTIO_MET_DIRECT;
            memcpy(portHandler, &direct_handler, sizeof(IOVtable));
        } else {
            if (preferedIOMethod == BUTTIO_MET_DIRECT) {
                preferedIOMethod = BUTTIO_MET_IOPM;
            }
            
            if (preferedIOMethod == BUTTIO_MET_IOPM) {
                memcpy(portHandler, &iopm_handler, sizeof(IOVtable));
            } else {
                memcpy(portHandler, &driverCall_handler, sizeof(driverCall_handler));
            }
        }
    } else {
        buttio_shutdown(portHandler);
        return FALSE;
    }
    
    portHandler->ioMethod = preferedIOMethod;
    iopm_fillAll(portHandler->iopm, FALSE);
    
    return TRUE;
}

BOOL buttio_flushIOPMChanges(ButtioPortHandler* portHandler) {
    DWORD bytesWritten;
    BOOL ret = TRUE;
    
    if (!g_isInit) return FALSE;
    if (!g_isDriverRequired) return TRUE;
    
    if (portHandler->ioMethod == BUTTIO_MET_IOPM) {
        if (iopm_isIopmOpaque(portHandler->iopm)) {
            ret = DeviceIoControl(g_driverFile, IOCTLNR_IOPM_UNREGISTER, NULL, 0, NULL, 0, &bytesWritten, NULL);
        } else {
            ret = DeviceIoControl(g_driverFile, IOCTLNR_IOPM_REGISTER, portHandler->iopm, IOPM_SIZE, NULL, 0, &bytesWritten, NULL);
        }
        Sleep(1);
    }
    
    return ret;
}