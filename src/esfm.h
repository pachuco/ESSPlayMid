#pragma once
#include <windows.h>

#define BANKLEN 8288
typedef struct {
    char name[32];
    char description[62];
    BYTE* pData;
} InstrBank;


BOOL esfm_init(USHORT port);
InstrBank* esfm_switchBank();
void esfm_shutdown();
void esfm_midiShort(DWORD dwData);