#pragma once
#include <stdint.h>

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