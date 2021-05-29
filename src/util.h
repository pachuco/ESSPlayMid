#pragma once
#include <windows.h>

#define DEFFUN_OBJ(_HMOD, _ERR, _FNAME) __typeof__(_FNAME) *pf##_FNAME = NULL
#define DEFFUN_EXT(_HMOD, _ERR, _FNAME) extern __typeof__(_FNAME) *pf##_FNAME
#define DYNLOAD(_HMOD, _ERR, _FNAME) _ERR = _ERR ? _ERR : !(pf##_FNAME = (void*)GetProcAddress(_HMOD, #_FNAME))

//example usage
/*
#define INPOUT32_FOREACH(_ACT, _HMOD, _ERR) \
    _ACT(_HMOD, _ERR, Inp32); \
    _ACT(_HMOD, _ERR, Out32); \
    _ACT(_HMOD, _ERR, DlPortReadPortUchar); \
    _ACT(_HMOD, _ERR, DlPortWritePortUchar);
    
INPOUT32_FOREACH(DEFFUN_OBJ, _, _);

BOOL init() {
    HANDLE dllHandle = LoadLibraryA("inpout32.dll");
    
    if (dllHandle) {
        BOOL isFail = FALSE;
        
        INPOUT32_FOREACH(DYNLOAD, dllHandle, isFail);
        return !isFail;
    }
    return FALSE;
}
*/





#define COUNTOF(_ARR) (sizeof(_ARR) / sizeof(_ARR[0]))

void util_dPrintfA(const CHAR* fmt, ...);
void util_getParentPathA(char* out);