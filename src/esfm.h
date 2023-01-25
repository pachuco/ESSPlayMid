#include <windows.h>

#define BANKLEN 8288

BOOL esfm_init(USHORT port);
void esfm_shutdown();
BYTE* esfm_switchBank();
void esfm_midiShort(DWORD dwData);