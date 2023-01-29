#pragma once
#include <stdint.h>

#define BANKLEN 8288 //may or may not be fixed value
typedef void (*FunWriteCB)(uint8_t baseOffset, uint8_t data);
typedef void (*FunDelayCB)(void);

void esfm_init(uint8_t* pBank, FunWriteCB pfWr, FunDelayCB pfDly);
void esfm_setBank(uint8_t* pBank);
void esfm_resetFM();
void esfm_startupDevice();
void esfm_shutdownDevice();
void esfm_midiShort(uint32_t dwData);