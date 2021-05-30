#include "buttio.h"

/////////////////////////////////////////////////////////////////////////////
//1 bit means blocked port, and viceversa
BOOL iopm_isIopmOpaque(UCHAR* pIopm) {
    ULONG  opacity = ~0;
    ULONG* src = (ULONG*)pIopm;
    
    for (ULONG i = 0; i < IOPM_SIZE/sizeof(ULONG); i++) opacity &= src[i];
    return !(~opacity);
}

BOOL iopm_isIoDenied(UCHAR* pIopm, USHORT port, UCHAR width) {
    BOOL isDenied = 0;
    
    //Mysoft says real I/O wraps
    for (UCHAR i=0; i < width; i++) {
        USHORT portOff = port+i;
        
        isDenied |= pIopm[portOff>>3] & (1<<i);
    }
    return isDenied;
}

void iopm_fillRange(UCHAR* pIopm, USHORT first, USHORT last, BOOL isEnabled) {
    for (ULONG i=first; i <= last; ) {
        ULONG todo = last - i;
        
        if (todo >= 8 && (i&7) == 0) {
            ULONG todo = (last& ~7) - i;
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

void iopm_fillAll(UCHAR* pIopm, BOOL isEnabled) {
    iopm_fillRange(pIopm, 0x0000, 0xFFFF, isEnabled);
}