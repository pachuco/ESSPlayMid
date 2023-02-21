#pragma once
#include <windows.h>

// deduced from PE/NE resource size in NT4 and win3.1 drivers
// may not be fixed size if bank can be different
#define BANKLEN 8288
typedef struct {
    USHORT melodicOffsets[128];
    USHORT drumOffsets[128];
    
    BYTE patchData[72 * 108];
} MidiBank;


typedef void (*FunWriteCB)(BYTE baseOffset, BYTE data);
typedef void (*FunDelayCB)(void);

void esfm_init(BYTE* pBank, FunWriteCB pfWr, FunDelayCB pfDly);
void esfm_setBank(BYTE* pBank);
void esfm_resetFM();
void esfm_startupDevice();
void esfm_shutdownDevice();
void esfm_midiShort(DWORD dwData);

// Include this to use C source, otherwise uses ASM source
#ifndef ASM_SRC
#include "crud/essfm.h"
#endif
