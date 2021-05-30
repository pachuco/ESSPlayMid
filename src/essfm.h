#include <windows.h>
#include "buttio.h"

#define FMREGLENGTH 595

typedef struct {
    ButtioPortHandler pHandler;
    UCHAR* patches;
    UCHAR  registers[FMREGLENGTH];
    USHORT port;
} FmConfig;

void QPCuWait(DWORD uSecTime);

void synthInitNativeESFM(FmConfig* fmConf);
void synthSaveESFM(FmConfig* fmConf);
void synthRestoreESFM(FmConfig* fmConf);
void synthMidiSendFM(FmConfig* fmConf, USHORT address, UCHAR data);
//BOOL synthPresent(PUCHAR base, PUCHAR inbase, BOOL *pIsFired)
void synthMidiQuiet(FmConfig* fmConf);