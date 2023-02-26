#pragma once
#if defined(WIN32) && _MSVC_VER < 1800
#include <windows.h>
typedef USHORT uint16_t;
typedef BYTE uint8_t;
typedef DWORD uint32_t;
typedef BOOL bool;
#else
#include <stdint.h>
#include <stdbool.h>
#endif


// deduced from PE/NE resource size in NT4 and win3.1 drivers
// may not be fixed size if bank can be different
#define BANKLEN 8288
typedef struct {
    uint16_t melodicOffsets[128];
    uint16_t drumOffsets[128];
    
    uint8_t patchData[72 * 108];
} MidiBank;


typedef void (*FunWriteCB)(uint8_t baseOffset, uint8_t data);
typedef void (*FunDelayCB)(void);

void esfm_init(uint8_t* pBank, FunWriteCB pfWr, FunDelayCB pfDly);
void esfm_setBank(uint8_t* pBank);
void esfm_resetFM();
void esfm_startupDevice();
void esfm_shutdownDevice();
void esfm_midiShort(uint32_t dwData);

// Include this to use C source, otherwise uses ASM source
#ifndef ASM_SRC
#include "crud/essfm.h"
#endif
