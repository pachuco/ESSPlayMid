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

//BOOL DeviceIoControl(HANDLE dev, DWORD ioctl, LPVOID in, DWORD inSize, LPVOID out, DWORD outSize, LPDWORD pRetSize, NULL);


BOOL buttio_ru8(IOHandler* pIoHand, USHORT port, UCHAR* pData) {
    DWORD bytesWritten;
    
    if (iopm_isIoDenied(pIoHand->iopm, port, sizeof(UCHAR))) return FALSE;
    
    switch (pIoHand->ioMethod) {
        case BUTTIO_MET_DIRECT:  /* FALLTHRU */
        case BUTTIO_MET_IOPM:
            __asm__ volatile("inb %1, %0" : "=a" (*pData) : "d" (port));
            break;
        case BUTTIO_MET_DRIVERCALL:
            return DeviceIoControl(g_driverFile, IOCTL_READ_8, &port, sizeof(USHORT), pData, sizeof(UCHAR), &bytesWritten, NULL);
            break;
    }
    return TRUE;
}
BOOL buttio_ru16(IOHandler* pIoHand, USHORT port, USHORT* pData) {
    DWORD bytesWritten;
    
    if (iopm_isIoDenied(pIoHand->iopm, port, sizeof(USHORT))) return FALSE;
    
    switch (pIoHand->ioMethod) {
        case BUTTIO_MET_DIRECT:  /* FALLTHRU */
        case BUTTIO_MET_IOPM:
            __asm__ volatile("inw %1, %0" : "=a" (*pData) : "d" (port));
            break;
        case BUTTIO_MET_DRIVERCALL:
            return DeviceIoControl(g_driverFile, IOCTL_READ_16, &port, sizeof(USHORT), pData, sizeof(USHORT), &bytesWritten, NULL);
            break;
    }
    return TRUE;
}
BOOL buttio_ru32(IOHandler* pIoHand, USHORT port, ULONG* pData) {
    DWORD bytesWritten;
    
    if (iopm_isIoDenied(pIoHand->iopm, port, sizeof(ULONG))) return FALSE;
    
    switch (pIoHand->ioMethod) {
        case BUTTIO_MET_DIRECT:  /* FALLTHRU */
        case BUTTIO_MET_IOPM:
            __asm__ volatile("inl %1, %0" : "=a" (*pData) : "d" (port));
            break;
        case BUTTIO_MET_DRIVERCALL:
            return DeviceIoControl(g_driverFile, IOCTL_READ_32, &port, sizeof(USHORT), pData, sizeof(ULONG), &bytesWritten, NULL);
            break;
    }
    return TRUE;
}


BOOL buttio_wu8(IOHandler* pIoHand, USHORT port, UCHAR data) {
    DWORD bytesWritten;
    DriverWritePacket pack = {port, {data}};
    
    if (iopm_isIoDenied(pIoHand->iopm, port, sizeof(UCHAR))) return FALSE;
    
    switch (pIoHand->ioMethod) {
        case BUTTIO_MET_DIRECT:  /* FALLTHRU */
        case BUTTIO_MET_IOPM:
            __asm__ volatile("outb %0, %1" : : "a" (data), "d" (port));
            break;
        case BUTTIO_MET_DRIVERCALL:
            return DeviceIoControl(g_driverFile, IOCTL_WRITE_8, &pack, sizeof(DriverWritePacket), NULL, 0, &bytesWritten, NULL);
            break;
    }
    return TRUE;
}
BOOL buttio_wu16(IOHandler* pIoHand, USHORT port, USHORT data) {
    DWORD bytesWritten;
    DriverWritePacket pack = {port, {data}};
    
    if (iopm_isIoDenied(pIoHand->iopm, port, sizeof(USHORT))) return FALSE;
    
    switch (pIoHand->ioMethod) {
        case BUTTIO_MET_DIRECT:  /* FALLTHRU */
        case BUTTIO_MET_IOPM:
            __asm__ volatile("outw %0, %1" : : "a" (data), "d" (port));
            break;
        case BUTTIO_MET_DRIVERCALL:
            return DeviceIoControl(g_driverFile, IOCTL_WRITE_16, &pack, sizeof(DriverWritePacket), NULL, 0, &bytesWritten, NULL);
            break;
    }
    return TRUE;
}
BOOL buttio_wu32(IOHandler* pIoHand, USHORT port, ULONG data) {
    DWORD bytesWritten;
    DriverWritePacket pack = {port, {data}};
    
    if (iopm_isIoDenied(pIoHand->iopm, port, sizeof(ULONG))) return FALSE;
    
    switch (pIoHand->ioMethod) {
        case BUTTIO_MET_DIRECT:  /* FALLTHRU */
        case BUTTIO_MET_IOPM:
            __asm__ volatile("outl %0, %1" : : "a" (data), "d" (port));
            break;
        case BUTTIO_MET_DRIVERCALL:
            return DeviceIoControl(g_driverFile, IOCTL_WRITE_32, &pack, sizeof(DriverWritePacket), NULL, 0, &bytesWritten, NULL);
            break;
    }
    return TRUE;
}







void buttio_shutdown(IOHandler* pIoHand) {
    SERVICE_STATUS status;
    
    iopm_fillAll(pIoHand->iopm, FALSE);
    
    if (g_driverFile) {
        buttio_flushIOPMChanges(pIoHand);
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
BOOL buttio_init(IOHandler* pIoHand, HANDLE modHand, UCHAR preferedIOMethod) {
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
        } else {
            if (preferedIOMethod == BUTTIO_MET_DIRECT) {
                preferedIOMethod = BUTTIO_MET_IOPM;
            }
        }
    } else {
        buttio_shutdown(pIoHand);
        return FALSE;
    }
    
    iopm_fillAll(pIoHand->iopm, FALSE);
    
    return TRUE;
}

BOOL buttio_flushIOPMChanges(IOHandler* pIoHand) {
    DWORD bytesWritten;
    BOOL ret = TRUE;
    
    if (!g_isInit) return FALSE;
    if (!g_isDriverRequired) return TRUE;
    
    if (pIoHand->ioMethod == BUTTIO_MET_IOPM) {
        if (iopm_isIopmOpaque(pIoHand->iopm)) {
            ret = DeviceIoControl(g_driverFile, IOCTLNR_IOPM_UNREGISTER, NULL, 0, NULL, 0, &bytesWritten, NULL);
        } else {
            ret = DeviceIoControl(g_driverFile, IOCTLNR_IOPM_REGISTER, pIoHand->iopm, IOPM_SIZE, NULL, 0, &bytesWritten, NULL);
        }
        Sleep(1);
    }
    
    return ret;
}