#include <windows.h>

BYTE IODriver_readU8(USHORT port);
void IODriver_writeU8(USHORT port, BYTE value);
void IODriver_Exit(void);
BOOL IODriver_Init(USHORT first, USHORT last);
