#pragma once
#include <windows.h>

#define BANKLEN 8288
typedef struct {
    char fileName[32];
    char description[64];
    BYTE* pData;
} InstrBank;


BOOL esfm_init(USHORT port);
InstrBank* esfm_switchBank();
void esfm_shutdown();
void esfm_midiShort(DWORD dwData);