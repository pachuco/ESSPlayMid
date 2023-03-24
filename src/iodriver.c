#include "iodriver.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define IODRIVER_GIVEIO   0
#define IODRIVER_BUTTIO   1
#define IOEMU_ESFMU       2
#define IODRIVER          IODRIVER_BUTTIO



////////////////////////////////////////////////////////////////////////////////
#if IODRIVER==IODRIVER_GIVEIO

USHORT rangeFirst=0, rangeLast=0;

void IODriver_writeU8(USHORT port, BYTE value) {
    if (port >= rangeFirst && port <= rangeLast)
        _outp (port, value);
}

BYTE IODriver_readU8(USHORT port) {
    BYTE ret = 0;
    
    if (port >= rangeFirst && port <= rangeLast)
        ret = _inp (port);
    
    return ret;
}

// GiveIO
BOOL IODriver_Init(USHORT first, USHORT last) {
    HANDLE h;
    
    //does GiveIO allow ALL the ports? That's asking for trouble.
    h = CreateFile("\\\\.\\giveio", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    CloseHandle(h);
    
    rangeFirst = first;
    rangeLast  = last;
    
    return h != INVALID_HANDLE_VALUE;
}

void IODriver_Exit(void) {
    rangeFirst = 0;
    rangeLast  = 0;
}

////////////////////////////////////////////////////////////////////////////////
#elif IODRIVER==IODRIVER_BUTTIO
#pragma comment(lib, "buttio.lib")
#include "buttio.h"

static IOHandler buttioHand = {0};


void IODriver_writeU8(USHORT port, BYTE value) {
    buttio_wu8(&buttioHand, port, value);
}

BYTE IODriver_readU8(USHORT port) {
    BYTE b;
    buttio_ru8(&buttioHand, port, &b);
    return b;
}

BOOL IODriver_Init(USHORT first, USHORT last) {
    if (!buttio_init(&buttioHand, NULL, BUTTIO_MET_IOPM))
        return FALSE;
    iopm_fillRange(buttioHand.iopm, first, last, TRUE);
    buttio_flushIOPMChanges(&buttioHand);
    
    return TRUE;
}

void IODriver_Exit(void) {
    buttio_shutdown(&buttioHand);
}

////////////////////////////////////////////////////////////////////////////////
#elif IODRIVER==IOEMU_ESFMU
#include <esfm.h>

USHORT rangeFirst=0, rangeLast=0;
esfm_chip esfmchip = {0};

void IODriver_writeU8(USHORT port, BYTE value) {
    ESFM_write_port(&esfmchip, port - rangeFirst, value);
}

BYTE IODriver_readU8(USHORT port) {
    return ESFM_read_port(&esfmchip, port - rangeFirst);
}

BOOL IODriver_Init(USHORT first, USHORT last) {
    rangeFirst = first;
    rangeLast  = last;
    
    //TODO: sound init and driving
    
    ESFM_init(&esfmchip);
    
    return TRUE;
}

void IODriver_Exit(void) {
    ZeroMemory(&esfmchip, sizeof(esfm_chip));
    
    rangeFirst = 0;
    rangeLast  = 0;
}

#else
////////////////////////////////////////////////////////////////////////////////
#error IODRIVER not defined!
#endif
